#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "prioque.h"

int Clock = 0;

typedef struct process{
  int pid;
  unsigned long arrival_time;
  Queue behaviors;

}Process;

typedef struct processBehavior{
  long unsigned int cpuburst, ioburst;
  int repeat;

}ProcessBehavior;

//populate ArrivalQueue
void read_process_descriptions(void) {
  Process p;
  ProcessBehavior b;
  int pid = 0, first = 1;
  unsigned long arrival;
  Queue ArrivalQ;
  Queue idleProcess;

  init_process(&p);
  arrival = 0;
  while (scanf("%lu", &arrival) != EOF) {
    scanf("%d %lu %lu %d",
      &pid,
      &b.cpuburst,
      &b.ioburst,
      &b.repeat);

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

int main(int argc, char *argv[]) {
  init_all_queues();
  init_process(&idleProcess);
  read_process_descriptions();

  while(processes_exist()) {
    Clock++;
    queue_new_arrivals();
    execute_highest_priority_process();
    do_I0();
  }
}
