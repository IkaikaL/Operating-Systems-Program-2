#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "prioque.h"

#define HighestPriority 0
#define MediumPriority 1
#define LowestPriority 2

int Clock = 0;
static FILE* output = NULL;
Queue ArrivalQ;
Queue ReadyQ;
Queue IOQ;
static Process null = { .pid = 0, .total_cpu_usage = 0 };

static const int quantumArray [3] = { 10, 30, 100 };
static const int demotionArray[3] = { 1, 2, 'X' };
static const int promotionArray[3] = { 'X', 2, 1 };

typedef struct process{
  int pid;
  int priority_cache;
  Queue behaviors;
  unsigned int arrival_time;
  unsigned int units;
  unsigned int quanta;
  unsigned int progress;
  unsigned int promotion;
  unsigned int demotion;
  unsigned int total_cpu_usage;

}Process;

typedef struct processBehavior{
  long unsigned int cpuburst;
  long unsigned int ioburst;
  unsigned int repeats;

}ProcessBehavior;

void init_all_queues(void) {

}

void init_process(Process *process) {
  process->pid = 0;
  process->priority_cache = 0;
  process->arrival_time = 0;
  process->units = 0;
  process->quanta = 0;
  process->progress = 0;
  process->promotion = 0;
  process->demotion = 0;
  process->total_cpu_usage = 0;

    init_queue(&process->behaviors, sizeof(ProcessBehavior), TRUE, NULL, FALSE);
}

//populate ArrivalQueue
void read_process_descriptions(void) {
  Process p;
  ProcessBehavior b;
  int pid = 0, first = 1;
  unsigned long arrival;
  

  init_process(&p);
  arrival = 0;
  while (scanf("%lu", &arrival) != EOF) {
    scanf("%d %lu %lu %d",
      &pid,
      &b.cpuburst,
      &b.ioburst,
      &b.repeats);

    if (!first && p.pid != pid) {
      add_to_queue(&ArrivalQ, &p, p.arrival_time);
      init_process(&p);
    }
    p.pid = pid;
    p.arrival_time = arrival;
    first = 0;
    add_to_queue(&p.behaviors, &b, 1);
  }
  add_to_queue(&ArrivalQ, &p, p.arrival_time);
}

void queue_new_arrivals(void) {
  Process process, previous = null;

    // save current active process pid
    if (queue_length(&ReadyQ) > 0) {
        peek_at_current(&ReadyQ, &previous);
        previous.priority_cache = current_priority(&ReadyQ);
    }

    // schedule arrival processes.
    while (queue_length(&ReadyQ) > 0 && current_priority(&ReadyQ) <= Clock) {
        remove_from_front(&ArrivalQ, &process);
        add_to_queue(&ReadyQ, &process, HighestPriority);
        fprintf(output, "CREATE: Process %d entered the ready queue at time %d.\n", process.pid, Clock);
    }

    // return io processes to cpu.
    while (queue_length(&IOQ) > 0 && current_priority(&IOQ) <= Clock) {
        remove_from_front(&IOQ, &process);
        add_to_queue(&ReadyQ, &process, process.priority_cache);
        // log queueing when leaving io
        fprintf(output, "QUEUED: Process %d queued at level %d at time %u.\n", process.pid, process.priority_cache + 1, Clock);
    }

    // log preemption
    if (queue_length(&ReadyQ) > 0 && previous.pid != null.pid) {
        peek_at_current(&ReadyQ, &process);
        if (previous.pid != process.pid) {
            fprintf(output, "QUEUED: Process %d queued at level %d at time %u.\n", previous.pid, previous.priority_cache + 1, Clock);
        }
    }
}

void execute_highest_priority_process(void) {
  if (queue_length(&ReadyQ) == 0) {
        // Run null process
        null.total_cpu_usage ++;
    }

    else {
        // get process
        Process process;
        peek_at_current(&ReadyQ, &process);

        // Update counters
        process.units ++;
        process.total_cpu_usage ++;

        // save changes
        update_current(&ReadyQ, &process);
    }
}

void do_IO(void) {
  Process process;
  ProcessBehavior processBehavior;

    int priority = current_priority(&ReadyQ);
    remove_from_front(&ReadyQ, &process);
    peek_at_current(&process.behaviors, &processBehavior);

    process.promotion ++;
    process.demotion = 0;

    // promote process
    if (process.promotion >= promotionArray[priority]) {
        process.promotion = 0;
        if (priority != HighestPriority) { priority --; }
    }

    // store priority in the process struct.
    process.priority_cache = priority;

    process.progress ++;
    process.units = 0;
    process.quanta = 0;

    add_to_queue(&IOQ, &process, Clock + processBehavior.ioburst);
    fprintf(output, "I/O: Process %d blocked for I/O at time %u.\n", process.pid, Clock);
}

//void processes_exist(void) { }

void final_report() {
    Process process;
    Queue logs;
    fprintf(output, "\nTotal CPU usage for all processes scheduled:\n\n");
    while (queue_length(&logs) != 0) {
        fprintf(output, "Process ");
        remove_from_front(&logs, &process);
        switch (process.pid) {
            case 0: fprintf(output, "<<null>> "); break;
            default: fprintf(output, "%d ", process.pid); break;
        }
        fprintf(output, ": %d time units.\n", process.total_cpu_usage);
    }
    destroy_queue(&logs);
}

int main(int argc, char *argv[]) {
  Process idleProcess;
  init_all_queues();
  init_process(&idleProcess);
  read_process_descriptions();

  while(processes_exist()) {
    Clock++;
    queue_new_arrivals();
    execute_highest_priority_process();
    do_IO();
  }

  Clock++;
  final_report();
  return 0;
}
