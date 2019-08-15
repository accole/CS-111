//NAME: Adam Cole
//EMAIL: adam.cole5621@gmail.com
//ID: 004912373


//includes
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <poll.h>
#include <time.h>
#include <ctype.h>
#include <math.h>
#include <unistd.h>
#include <mraa.h>
#include <mraa/aio.h>
#include <aio.h>

//DUMMY
#ifdef DUMMY
// Mock implementations for local dev/test
typedef int mraa_aio_context;
typedef int mraa_gpio_context;
int mraa_aio_init(int input)
{
  input++;
  return 100;
}
int mraa_aio_read(int temp)
{
  temp++;
  return 500;
}

typedef int mraa_gpio_context;
int MRAA_GPIO_IN = 5;
int mraa_gpio_init(int val)
{
  val++;
  return -5;
}
void mraa_gpio_dir(int val, int temp)
{
  val++;
  temp++;
}
int mraa_gpio_read(int val)
{
  (val)++;
  return 0;
}
#endif


//globals
const int R0 = 100000;
const int B = 4275;
int period = 1;
char tempscale = 'F';
FILE * log_f = NULL;
int lflag = 0;
int pflag = 0;
int stop = 0;


//function to calculate temperature
double calcTemp(int sensor){
  //algorithm given in TA slides
  //calculate in celsius and then convert to farenheit if need be
  double R = 1023.0/(double)(sensor) - 1.0;
  R = R0*R;
  double celcius = 1.0/(log(R/R0)/B + 1/298.15) - 273.15;
  if(tempscale == 'F'){
    return celcius * (9/5) + 32;
  }
  return celcius;
}


//button has been pushed, shut down
void shutdown(){
  time_t t;
  struct tm * local;
  time(&t);
  local = localtime(&t);
  //print shutdown stamp to STDOUT
  fprintf(stdout, "%.2d:%.2d:%.2d SHUTDOWN\n", local->tm_hour, local->tm_min, local->tm_sec);
  if(lflag){
    //print command and shutdown to log
    fprintf(log_f, "%.2d:%.2d:%.2d SHUTDOWN\n", local->tm_hour, local->tm_min, local->tm_sec);
  }
  //exit successfully
  exit(0);
}


//handle scale change
void scaleChange(char c){
  if (c == 'C'){
    tempscale = 'C';
    if (lflag & !stop){
      fprintf(log_f, "SCALE=C\n");
    }
  } else if (c == 'F'){
    tempscale = 'F';
    if (lflag & !stop){
      fprintf(log_f, "SCALE=F\n");
    }
  }
}


//handle period change
void periodChange(int seconds){
  if (seconds <= 0){
    if (lflag){
      fprintf(log_f, "Invalid period: Period not modified\n");
    }
  } else {
    period = seconds;
    if (lflag & !stop){
      fprintf(log_f, "PERIOD=%d\n", seconds);
    }
  }
}


//handle stop and start commands
void startStop(int startStop){
  if (startStop == 1){
    //start
    stop = 0;
    if (lflag){
      fprintf(log_f, "START\n");
    }
  } else {
    //stop
    stop = 1;
    if (lflag){
      fprintf(log_f, "STOP\n");
    }
  }
}

//handle / log the invalid command
void invalid(const char* buff){
  if (lflag){
    fprintf(log_f, "Invalid command: %s", buff);
  }
}


//log message to log
void logMessage(char * buff, int count){
  if(lflag){
    char p[count];
    int i = 0;
    for (; i < count; i++){
      p[i] = buff[i];
    }
    fprintf(log_f, "LOG %s\n", p);
  }
  return;
}


//handle commands
void commandline(const char * buff){
  //test buff to see what command was given
  if (buff == NULL){
    fprintf(stderr, "Error recieving command: %s\n", strerror(errno));
    exit(1);
  }

  //check for perfect commands
  //off
  if (strcmp(buff, "OFF\n") == 0){
    fprintf(log_f, "OFF\n");
    shutdown();
  } 

  //start and stop
  if (strcmp(buff, "STOP\n") == 0){
    startStop(0);
    return;
  } else if (strcmp(buff, "START\n") == 0){
    startStop(1);
    return;
  }

  //change scale
  if (strcmp(buff, "SCALE=F\n") == 0){
    scaleChange('F');
    return;
  } else if (strcmp(buff, "SCALE=f\n") == 0){
    scaleChange('F');
    return;
  } else if (strcmp(buff, "SCALE=C\n") == 0){
    scaleChange('C');
    return;
  } else if (strcmp(buff, "SCALE=c\n") == 0){
    scaleChange('C');
    return;
  }

  //log
  char * logcheck = "LOG ";
  unsigned int i = 0;
  int flag = 0;
  for (; i < strlen(logcheck); i++){
    if (buff[i] == logcheck[i]){
      flag = 1;
    } else {
      flag = 0;
      break;
    }
  }
  if (flag == 1){
    //log message
    i = 0;
    char copy[36];
    while((buff[i+4] != '\0' && buff[i+4] != '\n') && (i+4 < strlen(buff))){
      copy[i] = buff[i+4];
      i++;
    }
    logMessage(copy, i);
    return;
  }

  //period
  char * periodcheck = "PERIOD=";
  i = 0;
  flag = 0;
  for (; i < strlen(periodcheck); i++){
    if (buff[i] == periodcheck[i]){
      flag = 1;
    } else {
      flag = 0;
      break;
    }
  }
  if (flag == 1){
    //period change
    //i stays at the right index
    unsigned int count = 0;
    while(buff[i] != '\n'){
      if (!isdigit(buff[i])){
	invalid(buff);
	return;
      }
      i++;
      count++;
    }
    char number[count];
    for (i = 7; i < count+7; i++){
      number[i-7] = buff[i];
    }
    int new = atoi(number);
    periodChange(new);
    return;
  }

  //else invalid argument
  invalid(buff);

  //return
  return;
}


int main(int argc, char* argv[]){  
  //struct for holding argument options
  struct option options[] = {
    {"period", 1, 0, 'p'},
    {"scale", 1, 0, 's'},
    {"log", 1, 0, 'l'},
    {0, 0, 0, 0}
  };

  //parse through the command line arguments
  int x = 0;
  while((x = getopt_long(argc, argv, "t:i:y:s:", options, 0)) != -1){
    switch(x){
    case 'p':
      pflag = 1;
      period = atoi(optarg);
      break;
    case 'l':
      lflag = 1;
      log_f = fopen(optarg, "w");
      if (log_f == NULL){
	fprintf(stderr, "Error opening log file: %s", strerror(errno));
	exit(1);
      }
      break;
    case 's':
      //Celcius or Farenheit
      if (optarg[0] == 'c' || optarg[0] == 'C'){
	tempscale = 'C';
      } else if (optarg[0] == 'f' || optarg[0] == 'F'){
	tempscale = 'F';
      } else {
	fprintf(stderr, "incorrect scale option\n");
	exit(1);
      }
      break;
    default:
      printf("Correct usage: ./lab4b --period=SECONDS --scale=TEMPSCALE --log=LOGFILE\n");
      exit(1);
      break;
    }
  }

  //set up beaglebone sensors
  mraa_gpio_context button;     //button
  //GPIO 50 maps to I/O 60
  button = mraa_gpio_init(60);
  mraa_gpio_dir(button, MRAA_GPIO_IN);

  mraa_aio_context tempsensor;  //temperature sensor
  tempsensor = mraa_aio_init(1);
  int sensorVal;               //value returned from sensor
 
  //poll from the beaglebone
  struct pollfd p[1];
  p[0].fd = STDIN_FILENO;
  p[0].events = POLLIN | POLLHUP | POLLERR;
  
  //continue to read until no more events to read
  while(1){
    sensorVal = mraa_aio_read(tempsensor);
    double temp = calcTemp(sensorVal);
    //get raw time details
    time_t t;
    time(&t);
    //convert to local time
    struct tm * local;
    local = localtime(&t);

    //print the temperature and time
    if(!stop){
      fprintf(stdout, "%.2d:%.2d:%.2d %.1f\n", local->tm_hour, local->tm_min, local->tm_sec, temp);
      if (lflag){
	fprintf(log_f, "%.2d:%.2d:%.2d %.1f\n", local->tm_hour, local->tm_min, local->tm_sec, temp);
      }
    }
    
    time_t start, finish;
    time(&start);
    time(&finish);
    //while the temperature sensor is not reading
    while(difftime(finish, start) < period){
      //check for button clicks
      if(mraa_gpio_read(button)){
	shutdown();
      }
      //poll for input
      int pval = poll(p,1,0);
      if (pval == -1){
	fprintf(stderr, "Error polling: %s", strerror(errno));
      }
      //poll in from command line
      if (p[0].revents & POLLIN){
	char buff[32];
	memset(buff, 0, 32);
	fgets(buff, 32, stdin);
	//handle the command received
	commandline(buff);
      }
      time(&finish);
    }
    
  }
  
  fclose(log_f);
  //exit successfully
  exit(0);
}
