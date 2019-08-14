//NAME: Adam Cole
//EMAIL: adam.cole5621@gmail.com
//ID: 004912373

//lab2_add.c


//libraries
#include <pthread.h> //threads
#include <getopt.h>  //parsing options
#include <stdio.h>
#include <stdlib.h>
#include <time.h>    //timespec
#include <errno.h>   //errno
#include <string.h>  //fprintf


//global varibles
int numthreads = 1;
int numiterations = 1;
int opt_yield = 0;
int mutex = 0;
int spin = 0;
int compswap = 0;
pthread_mutex_t lock;
int slock = 0;


//basic add routine
void add(long long *pointer, long long value){
  long long sum = *pointer + value;
  if (opt_yield){
    sched_yield();
  }
  *pointer = sum;
}


void synchronize(long long* ptr, long long value){
  //modify addition and locking based on arguments
  //mutex
  if(mutex == 1){
    //lock variable, perform the add, unlock variable
    pthread_mutex_lock(&lock);
    add(ptr, value);
    pthread_mutex_unlock(&lock);
  }
  //Spin
  else if (spin == 1){
    //lock variable, perform the add, unlock variable
    while(__sync_lock_test_and_set(&slock, 1));
    add(ptr, value);
    __sync_lock_release(&slock);
  }
  //compare-and-swap
  else if (compswap == 1){
    long long first, new;
    do {
      first = *ptr;
      new = first + value;
      //if yield flag, yield
      if(opt_yield){
	sched_yield();
      }
    } while(__sync_val_compare_and_swap(ptr, first, new) != first);
  }
  //no synchronization
  else {
    add(ptr, value);
  }
}


//multithreading function
void * threadfunc(void * t){
  long long *ptr = (long long*)t;
  //add 1 to counter for each thread
  int i = 0;
  for (; i < numiterations; i++){
    synchronize(ptr, 1);
  }
  //subtract 1 from counter for each thread
  for (i=0; i < numiterations; i++){
    synchronize(ptr, -1);
  }
  return NULL;
}


int main(int argc, char* argv[]){
  //struct for holding argument options
  struct option options[] = {
    {"threads", 1, 0, 't'},
    {"iterations", 1, 0, 'i'},
    {"yield", 0, 0, 'y'},
    {"sync", 1, 0, 's'},
    {0, 0, 0, 0}
  };

  //parse through the command line arguments
  int x = 0;
  while((x = getopt_long(argc, argv, "t:i:ys:", options, 0)) != -1){
    switch(x){
    case 't':
      numthreads = atoi(optarg);
      break;
    case 'i':
      numiterations = atoi(optarg);
      break;
    case 'y':
      opt_yield = 1;
      break;
    case 's':
      //synchronize and define mutex
      if (optarg[0] == 'm'){
	mutex = 1;
      } else if (optarg[0] == 's'){
	spin = 1;
      } else if (optarg[0] == 'c'){
	compswap = 1;
      }
      break;
    default:
      printf("Correct usage: ./lab2_add --threads=NUMTHREADS --iterations=NUMITERATIONS --sync=LOCK --yield\n");
      exit(1);
      break;
    }
  }

  //initialize counter
  long long counter = 0;

  //create threads
  pthread_t *threads = malloc(sizeof(pthread_t) * numthreads);

  //create mutex
  if (mutex){
    //initialize the mutex
    if (pthread_mutex_init(&lock, NULL) != 0){
      fprintf(stderr, "Error initializing mutex: %s\n", strerror(errno));
      exit(1);
    }
  }

  //start the timer to keep track of time
  struct timespec start;
  clock_gettime(CLOCK_MONOTONIC, &start);

  //create numthreads threads to run add function
  int i = 0;
  for (; i < numthreads; i++){
    if (pthread_create(&threads[i], NULL, threadfunc, &counter) != 0){
      fprintf(stderr, "Error creating thread: %s\n", strerror(errno));
      exit(1);
    }
  }
  
  //wait for all the threads, make them join
  i = 0;
  for (; i < numthreads; i++){
    if (pthread_join(threads[i], NULL) != 0){
      fprintf(stderr, "Error joining thread: %s\n", strerror(errno));
      exit(1);
    }
  }

  //record the time elapsed in add
  struct timespec finish;
  clock_gettime(CLOCK_MONOTONIC, &finish);


  //store the elapsed time
  //convert to nanoseconds
  long long totaltime = (finish.tv_sec - start.tv_sec) * 1000000000;
  totaltime += finish.tv_nsec;
  totaltime -= start.tv_nsec;
  int operations = numthreads * numiterations * 2;
  long long average = totaltime / operations;


  //create a string to hold tag
  char tags[75];
  //zero out the string
  memset(tags, 0, 75*sizeof(tags[0]));

  //store tags in string
  //add-m
  if (!opt_yield && mutex){
    sprintf(tags, "add-m,%d,%d,%d,%lld,%lld,%lld", numthreads, numiterations, operations, totaltime, average, counter);
  }
  //add-s
  else if (!opt_yield && spin){
    sprintf(tags, "add-s,%d,%d,%d,%lld,%lld,%lld", numthreads, numiterations, operations, totaltime, average, counter);
  }
  //add-c
  else if (!opt_yield && compswap){
    sprintf(tags, "add-c,%d,%d,%d,%lld,%lld,%lld", numthreads, numiterations, operations, totaltime, average, counter);
  }
  //add-yield-m
  else if (opt_yield && mutex){
    sprintf(tags, "add-yield-m,%d,%d,%d,%lld,%lld,%lld", numthreads, numiterations, operations, totaltime, average, counter);
  }
  //add-yield-s
  else if (opt_yield && spin){
    sprintf(tags, "add-yield-s,%d,%d,%d,%lld,%lld,%lld", numthreads, numiterations, operations, totaltime, average, counter);
  }
  //add-yield-c
  else if (opt_yield && compswap){
    sprintf(tags, "add-yield-c,%d,%d,%d,%lld,%lld,%lld", numthreads, numiterations, operations, totaltime, average, counter);
  }
  //add-yield-none
  else if (opt_yield){
    sprintf(tags, "add-yield-none,%d,%d,%d,%lld,%lld,%lld", numthreads, numiterations, operations, totaltime, average, counter);
  }
  //add-none
  else {
    sprintf(tags, "add-none,%d,%d,%d,%lld,%lld,%lld", numthreads, numiterations, operations, totaltime, average, counter);
  }

  //print tags
  printf("%s\n", tags);
  
  //free heap memory
  free(threads);

  //destroy the mutex if created
  if (mutex){
    pthread_mutex_destroy(&lock);
  }

  //exit
  exit(0);
}
