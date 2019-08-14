//NAME: Adam Cole
//EMAIL: ###########@gmail.com
//ID: #########

//lab2_list.c

//libraries
#include <pthread.h> //threads
#include <getopt.h>  //parsing options
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>  //Sighandling
#include <time.h>    //timespec
#include <errno.h>   //errno
#include <string.h>  //fprintf
#include "SortedList.h"  //Sorted List Header


//global varibles
int numthreads = 1;
int numiterations = 1;
int numelements = 0;
int opt_yield = 0;
int mutex = 0;
int spin = 0;
int finsert = 0;
int flookup = 0;
int fdelete = 0;
SortedListElement_t * listelements;
SortedList_t * list = NULL;
pthread_mutex_t lock;
int slock = 0;


//sighandler
void sighandler(int signum){
  if (signum == SIGSEGV){
    fprintf(stderr, "Segmentation fault caught\n");
    exit(2);
  }
}

//multithreading function
void * threadfunc(void * t){
  long s = (long)t * numiterations;
  long i = s;
  //insert each element into the SortedList
  for (; i < s + numiterations; i++){
    if(mutex == 1){
      //lock variable, perform the add, unlock variable
      pthread_mutex_lock(&lock);
      SortedList_insert(list, &listelements[i]);
      pthread_mutex_unlock(&lock);
    } else if (spin == 1){
      //lock variable, perform the add, unlock variable
      while(__sync_lock_test_and_set(&slock, 1));
      SortedList_insert(list, &listelements[i]);
      __sync_lock_release(&slock);
    } else {
      SortedList_insert(list, &listelements[i]);
    }
  }

  //calculate the length of the list
  if (mutex == 1){
    pthread_mutex_lock(&lock);
    SortedList_length(list);
    pthread_mutex_unlock(&lock);
  } else if (spin == 1){
    while(__sync_lock_test_and_set(&slock, 1));
    SortedList_length(list);
    __sync_lock_release(&slock);
  } else {
    SortedList_length(list);
  }

  i = s;
  //lookup and delete each element in the list
  for (; i < s + numiterations; i++){
    if (mutex == 1){
      pthread_mutex_lock(&lock);
      SortedListElement_t * find = SortedList_lookup(list, listelements[i].key);
      if (find == NULL){
	fprintf(stderr, "Element not found in SortedList\n");
	exit(2);
      }
      SortedList_delete(find);
      pthread_mutex_unlock(&lock);
    } else if (spin == 1){
      while(__sync_lock_test_and_set(&slock, 1));
      SortedListElement_t * find = SortedList_lookup(list, listelements[i].key);
      if (find == NULL){
	fprintf(stderr, "Element not found in SortedList\n");
	exit(2);
      }
      SortedList_delete(find);
      __sync_lock_release(&slock);
    } else {
      //printf("searching %s\n", listelements[i].key);
      SortedListElement_t * find = SortedList_lookup(list, listelements[i].key);
      if (find == NULL){
	fprintf(stderr, "Element not found in SortedList\n");
	exit(2);
      }
      SortedList_delete(find);
    }
  }

  return NULL;
}


void keys(void){
  //create the seed
  srand(time(NULL));
  //length of random key
  int keylen = 4;
  //produce a random key value for each element
  int i = 0;
  for(; i < numelements; i++){
    char * key = malloc((keylen+1)*sizeof(char));
    //randomly add 4 characters
    int j = 0;
    for (; j < keylen; j++){
      int random = rand()%26;
      //randomly adds an offset to a
      key[j] = 'a' + random;
    }
    //end with null byte
    key[keylen] = '\0';
    listelements[i].key = key;
  }
  return;
}

char * yieldopts(void){
  //INSERT_YIELD = 0x01
  //DELETE_YIELD = 0x02
  //LOOKUP_YIELD = 0x04
  if ((opt_yield & 0x07) == 0){
    //000
    return "none";
  } else if ((opt_yield & 0x07) == 1){
    //001
    return "i";
  } else if ((opt_yield & 0x07) == 4) {
    //100
    return "l";
  } else if ((opt_yield & 0x07) == 2) {
    //010
    return "d";
  } else if ((opt_yield & 0x07) == 3) {
    //011
    return "id";
  } else if ((opt_yield & 0x07) == 5) {
    //101
    return "il";
  } else if ((opt_yield & 0x07) == 6) {
    //110
    return "dl";
  } else {
    //111
    return "idl";
  }
}

char * syncopts(void){
  //return the tags for sync
  if (mutex) {
    return "m";
  } else if (spin){
    return "s";
  } else {
    return "none";
  }
}


int main(int argc, char* argv[]){  
  //struct for holding argument options
  struct option options[] = {
    {"threads", 1, 0, 't'},
    {"iterations", 1, 0, 'i'},
    {"yield", 1, 0, 'y'},
    {"sync", 1, 0, 's'},
    {0, 0, 0, 0}
  };

  //parse through the command line arguments
  int x = 0;
  unsigned int n = 0;
  while((x = getopt_long(argc, argv, "t:i:y:s:", options, 0)) != -1){
    switch(x){
    case 't':
      numthreads = atoi(optarg);
      break;
    case 'i':
      numiterations = atoi(optarg);
      break;
    case 'y':
      for (; n < strlen(optarg); n++){
	if (optarg[n] == 'i'){
	  opt_yield = opt_yield | INSERT_YIELD;
	} else if (optarg[n] == 'l'){
	  opt_yield = opt_yield | LOOKUP_YIELD;
	} else if (optarg[n] == 'd'){
	  opt_yield = opt_yield | DELETE_YIELD;
	} else {
	  fprintf(stderr, "incorrect yield section\n");
	  exit(1);
	}
      }
      break;
    case 's':
      //synchronize and define mutex
      if (optarg[0] == 'm'){
	mutex = 1;
      } else if (optarg[0] == 's'){
	spin = 1;
      } else {
	fprintf(stderr, "incorrect sync option\n");
	exit(1);
      }
      break;
    default:
      printf("Correct usage: ./lab2_list --threads=NUMTHREADS --iterations=NUMITERATIONS --yield=SECTION --sync=LOCK\n");
      exit(1);
      break;
    }
  }

  //register sighandler
  signal(SIGSEGV, sighandler);

  //initialize empty list
  list = (SortedList_t*)malloc(sizeof(SortedList_t));
  list->key = NULL;
  list->next = list;
  list->prev = list;

  //create threads
  pthread_t *threads = malloc(sizeof(pthread_t) * numthreads);

  //create mutex
  if (mutex){
    //initialize the mutex
    if (pthread_mutex_init(&lock, NULL) != 0){
      fprintf(stderr, "Error initializing mutex: %s\n", strerror(errno));
      exit(2);
    }
  }

  //create list elements to be added with random keys
  numelements = numthreads * numiterations;
  listelements = (SortedListElement_t*)malloc(numelements*sizeof(SortedListElement_t));
  keys();

  //start the timer to keep track of time
  struct timespec start;
  clock_gettime(CLOCK_MONOTONIC, &start);

  //create numthreads threads to run linked list operations
  long i = 0;
  for (; i < numthreads; i++){
    if (pthread_create(&threads[i], NULL, threadfunc, (void*)i) != 0){
      fprintf(stderr, "Error creating thread: %s\n", strerror(errno));
      exit(2);
    }
  }

  //wait for all the threads, make them join
  i = 0;
  for (; i < numthreads; i++){
    if (pthread_join(threads[i], NULL) != 0){
      fprintf(stderr, "Error joining thread: %s\n", strerror(errno));
      exit(2);
    }
  }

  //record the time elapsed in add
  struct timespec finish;
  clock_gettime(CLOCK_MONOTONIC, &finish);

  //test that the list ends empty
  if (SortedList_length(list) != 0){
    fprintf(stderr, "Error: List was not cleared\n");
    exit(2);
  }

  //store the elapsed time
  //convert to nanoseconds
  long long totaltime = (finish.tv_sec - start.tv_sec) * 1000000000;
  totaltime += finish.tv_nsec;
  totaltime -= start.tv_nsec;
  int operations = numthreads * numiterations * 3;
  long long average = totaltime / operations;


  //create a string to hold tags
  char tags[75];
  //zero out the string
  memset(tags, 0, 75*sizeof(tags[0]));

  //store tags in string
  sprintf(tags, "list-%s-%s,%d,%d,1,%d,%lld,%lld\n", yieldopts(), syncopts(), numthreads, numiterations, operations, totaltime, average);

  //print tags
  printf("%s", tags);

  //free heap memory
  free(threads);
  int k = 0;
  for(; k < numelements; k++){
    free((void*)listelements[k].key);
  }
  free(listelements);
  free(list);

  //destroy the mutex if created
  if (mutex){
    pthread_mutex_destroy(&lock);
  }

  //exit
  exit(0);
}
