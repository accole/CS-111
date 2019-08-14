//NAME: Adam Cole
//EMAIL: #############@gmail.com
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
SortedList_t * list;
pthread_mutex_t lock;
int slock = 0;

//additional globals
int lists = 0;
int numlists = 1;
SortedListElement_t ** listArr = NULL;  //array of lists
pthread_mutex_t * lockArr = NULL;	//array of mutexes
int * slockArr = NULL;			//array of spin locks
long long * locktimes = NULL;		//array of time spent aquiring lock per thread
int keeptimes = 0;			//flag variable if we should use locktimes


//sighandler
void sighandler(int signum){
  if (signum == SIGSEGV){
    fprintf(stderr, "Segmentation fault caught\n");
    exit(2);
  }
}

//hashing function to systematically distribute locks to be waited on
int hashing(const char* key){
  int hash = 0;
  hash = (key[0] + key[2]) % numlists;
  return hash;
}

//multithreading function
void * threadfunc(void * t){
  long thread = (long)t;
  long s = thread * numiterations;
  long i = s;

  //insert each element into the SortedList
  if (lists){
    for (; i < s + numiterations; i++){
      const char * curr = listelements[i].key;		//get correct lock to wait on
      int hashval = hashing(curr);
      if (spin){
        struct timespec start;
        clock_gettime(CLOCK_MONOTONIC, &start);
        while(__sync_lock_test_and_set(&slockArr[hashval], 1));	//lock spin lock
        struct timespec finish;					//record time
        clock_gettime(CLOCK_MONOTONIC, &finish);
        long long totaltime = (finish.tv_sec - start.tv_sec) * 1000000000;
        totaltime += finish.tv_nsec;				//convert to nanoseconds
        totaltime -= start.tv_nsec;
        locktimes[thread] += totaltime;				//store time
        SortedList_insert(listArr[hashval], &listelements[i]);	//perform operation
        __sync_lock_release(&slockArr[hashval]);		//unlock spin lock
      } else if (mutex) {
        struct timespec start;
        clock_gettime(CLOCK_MONOTONIC, &start);
        pthread_mutex_lock(&lockArr[hashval]);			//lock mutex
        struct timespec finish;					//record lock time
        clock_gettime(CLOCK_MONOTONIC, &finish);
        long long totaltime = (finish.tv_sec - start.tv_sec) * 1000000000;
        totaltime += finish.tv_nsec;				//convert to nanoseconds
        totaltime -= start.tv_nsec;
        locktimes[thread] += totaltime;				//store time
        SortedList_insert(listArr[hashval], &listelements[i]);	//perform operation
        pthread_mutex_unlock(&lockArr[hashval]);		//unlock mutex
      } else {
        SortedList_insert(listArr[hashval], &listelements[i]);	//no time to record
      }
    }
  } else {	//single list
    for (; i < s + numiterations; i++){
      if(mutex == 1){
	struct timespec start;
        clock_gettime(CLOCK_MONOTONIC, &start);
        pthread_mutex_lock(&lock);			//lock mutex
	struct timespec finish;				//record lock time
        clock_gettime(CLOCK_MONOTONIC, &finish);
	long long totaltime = (finish.tv_sec - start.tv_sec) * 1000000000;
        totaltime += finish.tv_nsec;
        totaltime -= start.tv_nsec;			//convert to nanoseconds
        locktimes[thread] += totaltime;			//store time
        SortedList_insert(list, &listelements[i]);	//perform operation
        pthread_mutex_unlock(&lock);			//unlock mutex
      } else if (spin == 1){
	struct timespec start;
        clock_gettime(CLOCK_MONOTONIC, &start);
        while(__sync_lock_test_and_set(&slock, 1));	//lock spin lock
	struct timespec finish;				//record lock time
        clock_gettime(CLOCK_MONOTONIC, &finish);
	long long totaltime = (finish.tv_sec - start.tv_sec) * 1000000000;
        totaltime += finish.tv_nsec;
        totaltime -= start.tv_nsec;			//convert to nanoseconds
        locktimes[thread] += totaltime;			//store time
        SortedList_insert(list, &listelements[i]);	//perform operation
        __sync_lock_release(&slock);			//unlock spin lock
      } else {
        SortedList_insert(list, &listelements[i]);	//no time to record
      }
    }
  }
 

  //calculate the length of the list
  if (lists){
    int m = 0;
    for (; m < numlists; m++){
      if (spin){
        struct timespec start;
        clock_gettime(CLOCK_MONOTONIC, &start);
        while(__sync_lock_test_and_set(&slockArr[m], 1));	//lock spin lock
        struct timespec finish;					//record time
        clock_gettime(CLOCK_MONOTONIC, &finish);
        long long totaltime = (finish.tv_sec - start.tv_sec) * 1000000000;
        totaltime += finish.tv_nsec;			//convert to nanoseconds
        totaltime -= start.tv_nsec;
        locktimes[thread] += totaltime;			//store time
        SortedList_length(listArr[m]); 			//perform operation
        __sync_lock_release(&slockArr[m]);		//unlock spin lock
      } else if (mutex) {
        struct timespec start;
        clock_gettime(CLOCK_MONOTONIC, &start);		//record time
        pthread_mutex_lock(&lockArr[m]);		//lock mutex
        struct timespec finish;
        clock_gettime(CLOCK_MONOTONIC, &finish);
        long long totaltime = (finish.tv_sec - start.tv_sec) * 1000000000;
        totaltime += finish.tv_nsec;			//convert to nanoseconds
        totaltime -= start.tv_nsec;
        locktimes[thread] += totaltime;			//store time
        SortedList_length(listArr[m]);			//perform operation
        pthread_mutex_unlock(&lockArr[m]);		//unlock mutex
      } else {
        SortedList_length(listArr[m]);			//no time to record
      }
    }
  } else {              //single list
    if (mutex == 1){
      struct timespec start;
      clock_gettime(CLOCK_MONOTONIC, &start);
      pthread_mutex_lock(&lock);		//lock mutex
      struct timespec finish;			//record lock time
      clock_gettime(CLOCK_MONOTONIC, &finish);
      long long totaltime = (finish.tv_sec - start.tv_sec) * 1000000000;
      totaltime += finish.tv_nsec;
      totaltime -= start.tv_nsec;		//convert to nanoseconds
      locktimes[thread] += totaltime;		//store time
      SortedList_length(list);		//perform operation
      pthread_mutex_unlock(&lock);	//lock mutex
    } else if (spin == 1){
      struct timespec start;
      clock_gettime(CLOCK_MONOTONIC, &start);
      while(__sync_lock_test_and_set(&slock, 1));	//lock spin lock
      struct timespec finish;				//record lock time
      clock_gettime(CLOCK_MONOTONIC, &finish);
      long long totaltime = (finish.tv_sec - start.tv_sec) * 1000000000;
      totaltime += finish.tv_nsec;
      totaltime -= start.tv_nsec;		//convert to nanoseconds
      locktimes[thread] += totaltime;		//store time
      SortedList_length(list);		//perform operation
      __sync_lock_release(&slock);	//unlock spin lock
    } else {
      SortedList_length(list);
    }
  }
  

  i = s;
  //lookup and delete each element in the list
  if (lists){
    for (; i < s + numiterations; i++){
      const char * curr = listelements[i].key;		//get correct lock to wait on
      int hashval = hashing(curr);
      if (spin){
        struct timespec start;
        clock_gettime(CLOCK_MONOTONIC, &start);
        while(__sync_lock_test_and_set(&slockArr[hashval], 1));	//lock spin lock
        struct timespec finish;					//record time
        clock_gettime(CLOCK_MONOTONIC, &finish);
        long long totaltime = (finish.tv_sec - start.tv_sec) * 1000000000;
        totaltime += finish.tv_nsec;				//convert to nanoseconds
        totaltime -= start.tv_nsec;
        locktimes[thread] += totaltime;				//store time
        SortedListElement_t * find = SortedList_lookup(listArr[hashval], listelements[i].key);
        if (find == NULL){
	  fprintf(stderr, "Element not found in SortedList\n");
	  exit(2);
        }
        SortedList_delete(find);				//perform operations
        __sync_lock_release(&slockArr[hashval]);		//unlock spin lock
      } else if (mutex) {
        struct timespec start;
        clock_gettime(CLOCK_MONOTONIC, &start);
        pthread_mutex_lock(&lockArr[hashval]);			//lock mutex
        struct timespec finish;					//record lock time
        clock_gettime(CLOCK_MONOTONIC, &finish);
        long long totaltime = (finish.tv_sec - start.tv_sec) * 1000000000;
        totaltime += finish.tv_nsec;				//convert to nanoseconds
        totaltime -= start.tv_nsec;
        locktimes[thread] += totaltime;				//store time
        SortedListElement_t * find = SortedList_lookup(listArr[hashval], listelements[i].key);
        if (find == NULL){
	  fprintf(stderr, "Element not found in SortedList\n");
	  exit(2);
        }
        SortedList_delete(find);				//perform operations
        pthread_mutex_unlock(&lockArr[hashval]);		//unlock mutex
      } else {							//no time to record
        SortedListElement_t * find = SortedList_lookup(listArr[hashval], listelements[i].key);
        if (find == NULL){
	  fprintf(stderr, "Element not found in SortedList\n");
	  exit(2);
        }
        SortedList_delete(find);				//perform operations
      }
    }
  } else {             //single list
    for (; i < s + numiterations; i++){
      if (mutex == 1){
        struct timespec start;
        clock_gettime(CLOCK_MONOTONIC, &start);
        pthread_mutex_lock(&lock);		//lock mutex
        struct timespec finish;			//record lock time
        clock_gettime(CLOCK_MONOTONIC, &finish);
        long long totaltime = (finish.tv_sec - start.tv_sec) * 1000000000;
        totaltime += finish.tv_nsec;
        totaltime -= start.tv_nsec;		//convert to nanoseconds
        locktimes[thread] += totaltime;		//store time
        SortedListElement_t * find = SortedList_lookup(list, listelements[i].key);
        if (find == NULL){
	  fprintf(stderr, "Element not found in SortedList\n");
	  exit(2);
        }
        SortedList_delete(find);	//perform operations
        pthread_mutex_unlock(&lock);	//unlock mutex
      } else if (spin == 1){
        struct timespec start;
        clock_gettime(CLOCK_MONOTONIC, &start);
        while(__sync_lock_test_and_set(&slock, 1));	//lock spin lock
        struct timespec finish;				//record lock time
        clock_gettime(CLOCK_MONOTONIC, &finish);
        long long totaltime = (finish.tv_sec - start.tv_sec) * 1000000000;
        totaltime += finish.tv_nsec;
        totaltime -= start.tv_nsec;		//convert to nanoseconds
        locktimes[thread] += totaltime;		//store time
        SortedListElement_t * find = SortedList_lookup(list, listelements[i].key);
        if (find == NULL){
	  fprintf(stderr, "Element not found in SortedList\n");
	  exit(2);
        }
        SortedList_delete(find);		//perform operations
        __sync_lock_release(&slock);		//unlock spin lock
      } else {
        SortedListElement_t * find = SortedList_lookup(list, listelements[i].key);
        if (find == NULL){
	  fprintf(stderr, "Element not found in SortedList\n");
	  exit(2);
        }
        SortedList_delete(find);
      }
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
    {"lists", 1, 0, 'l'},
    {0, 0, 0, 0}
  };

  //parse through the command line arguments
  int x = 0;
  unsigned int n = 0;
  while((x = getopt_long(argc, argv, "t:i:y:s:l:", options, 0)) != -1){
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
    case 'l':
      //use multiple lists
      numlists = atoi(optarg);
      if (numlists > 1){
	//only use multiple lists route if more than 1 list
	lists = 1;
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

  //initialize empty list for 1 thread case
  list = (SortedList_t*)malloc(sizeof(SortedList_t));
  list->key = NULL;
  list->next = list;
  list->prev = list;

  //create threads
  pthread_t *threads = malloc(sizeof(pthread_t) * numthreads);

  //malloc locktimes to keep track of lock aquisition for each thread
  //only allocate if a lock is used
  keeptimes = (spin | mutex);
  if (keeptimes){
    locktimes = (long long *)malloc(numthreads*sizeof(long));
    int h = 0;
    for (; h < numthreads; h++){
      locktimes[h] = 0;
    }
  }

  //create mutex for a single list
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

  //malloc and initialize data for multiple lists  
  if (lists) {
    //malloc listArr and initialize
    listArr = (SortedListElement_t **)malloc(numlists*sizeof(SortedListElement_t*));
    int j = 0;
    for (; j < numlists; j++){
      listArr[j] = (SortedList_t *)malloc(sizeof(SortedList_t));
      listArr[j]->next = listArr[j];
      listArr[j]->prev = listArr[j];
      listArr[j]->key = NULL;	//signifies head
    }

    //malloc lockArr and initialize
    if (mutex){
      lockArr = (pthread_mutex_t *)malloc(numlists*sizeof(pthread_mutex_t));
      memset(lockArr, 0, numlists*sizeof(pthread_mutex_t));
      j = 0;
      for(; j < numlists; j++){
	if (pthread_mutex_init(&lockArr[j], NULL) != 0){
	  fprintf(stderr, "Error initializing mutex: %s\n", strerror(errno));
	  exit(2);
	}
      }
    }

    //malloc slockArr and initialize
    if(spin) {
      slockArr = (int *)malloc(numlists*sizeof(int));
      j = 0;
      for(; j < numlists; j++){
	slockArr[j] = 0;
      }
    }
  }

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

  //store the elapsed time
  //convert to nanoseconds
  long long totaltime = (finish.tv_sec - start.tv_sec) * 1000000000;
  totaltime += finish.tv_nsec;
  totaltime -= start.tv_nsec;

  //find total lock aquisition time if lock was used
  long long totalLocktime = 0;
  int v = 0;
  if (keeptimes){
    for(; v < numthreads; v++){
      totalLocktime += locktimes[v];
    }
  }

  int operations = numthreads * numiterations * 3;
  long long averageOP = totaltime / operations;
  long long averageLOCK = totalLocktime / operations;


  //create a string to hold tags
  char tags[75];
  //zero out the string
  memset(tags, 0, 75*sizeof(tags[0]));

  //store tags in string
  sprintf(tags, "list-%s-%s,%d,%d,%d,%d,%lld,%lld,%lld\n", yieldopts(), syncopts(), numthreads, numiterations, numlists, operations, totaltime, averageOP, averageLOCK);

  //print tags
  printf("%s", tags);

  //test that the lists were not corrupted
  if(lists){
    v = 0;
    for(; v < numlists; v++){
      if (SortedList_length(listArr[v]) != 0){
        fprintf(stderr, "list corrupted!\n");
  	exit(2);
      }
    }
  } else {
    if (SortedList_length(list) != 0){
      fprintf(stderr, "list corrupted!\n");
      exit(2);
    }
  }

  //destroy the mutexes if created
  int k = 0;
  if (mutex){
    pthread_mutex_destroy(&lock);
    //delete the rest if multiple lists
    if (lists){
      k = 0;
      for (; k < numlists; k++){
	pthread_mutex_destroy(&lockArr[k]);
      }
    }
  }

  //free heap memory
  if(keeptimes){
    free(locktimes);		//free locktimes
  }
  if(spin){
    free(slockArr);             //free spin locks
  }
  if (mutex){
    free(lockArr);		//free mutexes after destroying
  }
  if (lists){
    k = 0;
    for(; k < numlists; k++){
      free(listArr[k]);		//free every sublist
    }
  }
  free(listArr);		//free array of sublists
  free(threads);		//free thread array
  k = 0;
  for(; k < numelements; k++){
    free((void*)listelements[k].key);	//free element keys
  }
  free(listelements);		//free all the elements
  free(list);			//free the single list

  //exit
  exit(0);
}
