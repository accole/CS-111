//NAME: Adam Cole
//EMAIL: adam.cole5621@gmail.com
//ID: 004912373

//4c_tls.c

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
#include <aio.h>
#include <unistd.h>
#include <mraa.h>
#include <mraa/aio.h>

//includes added for 4C
#include <sys/socket.h>
#include <netdb.h>
#include <openssl/ssl.h>

//DUMMY
#ifdef DUMMY
//Fake implementations for local testing
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

//globals added for 4C
char * id = NULL;
char * host = NULL;
int portnum = -2;
int sfd;

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
void shutdownn(){
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
    shutdownn();
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


SSL_CTX * ssl_init(void){
  //given in Zhou Slides
  SSL_CTX * newContext = NULL;
  if (SSL_library_init() < 0){
    fprintf(stderr, "Error initializing SSL context.\n");
    exit(2);
  }
  SSL_load_error_strings();
  OpenSSL_add_all_algorithms();
  newContext = SSL_CTX_new(TLSv1_client_method());
  if (newContext == NULL){
    fprintf(stderr, "Error initializing SSL context.\n");
    exit(2);
  }
  return newContext;
}


int main(int argc, char* argv[]){
  //struct for holding argument options
  struct option options[] = {
    {"period", 1, 0, 'p'},
    {"scale", 1, 0, 's'},
    {"log", 1, 0, 'l'},
    {"id", 1, 0, 'i'},
    {"host", 1, 0, 'h'},
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
    case 'i':
      //check to make sure ID is 9 digits
      if (strlen(optarg) != 9){
	fprintf(stderr, "Error: Incorrect ID length.\n");
	exit(1);
      }
      id = optarg;
      break;
    case 'h':
      //store hostname in the global
      host = optarg;
      break;
    default:
      printf("Correct usage: ./lab4b --period=SECONDS --scale=TEMPSCALE --log=LOGFILE\n");
      exit(1);
      break;
    }
  }

  //then the port number is given outside of the options
  portnum = atoi(argv[optind]);
  if (portnum <= 0){
    fprintf(stderr, "Error: Invalid Port.\n");
    exit(1);
  }

  //make sure all required arguments given
  if (id == NULL || host == NULL || lflag != 1){
    fprintf(stderr, "Error: You must provide an id, host, and log file");
    exit(1);
  }
  

  //set up beaglebone sensors
  //mraa_gpio_context button;     //button not included in lab 4C
  //GPIO 50 maps to I/O 60
  //button = mraa_gpio_init(60);
  //mraa_gpio_dir(button, MRAA_GPIO_IN);

  mraa_aio_context tempsensor;  //temperature sensor
  tempsensor = mraa_aio_init(1);
  int sensorVal;               //value returned from sensor


  //establish TCP connection
  //given in Zhou Slides
  sfd = socket(AF_INET, SOCK_STREAM, 0);  //connects socket
  if (sfd < 0){
    fprintf(stderr, "Error connecting socket.\n");
    exit(2);
  }
  struct hostent* serv = gethostbyname(host);       //connect to the server
  if (serv == NULL){
    fprintf(stderr, "Error connecting to the server.\n");
    exit(2);
  }
  struct sockaddr_in serv_addr;    //encodes ip address and port
  memset(&serv_addr, 0, sizeof(struct sockaddr_in));
  serv_addr.sin_family = AF_INET;
  memcpy(&serv_addr.sin_addr.s_addr, serv->h_addr, serv->h_length);
  serv_addr.sin_port = htons(portnum);     //setup the port
  int conn = connect(sfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
  if (conn < 0){
    fprintf(stderr, "Error connecting from client.\n");
    exit(2);
  }


  //setup SSL encryption
  SSL_CTX * context = ssl_init();
  SSL * sslClient = SSL_new(context);
  //attach SSL session to the socket
  if(SSL_set_fd(sslClient, sfd) != 1){
    fprintf(stderr, "Error attaching socket to SSL.\n");
    exit(2);
  }
  if(SSL_connect(sslClient) != 1){
    fprintf(stderr, "Error connecting SSL.\n");
    exit(2);
  }

  //send ID number using SSL
  fprintf(log_f, "ID=%s\n", id);
  char IDbuff[13];
  sprintf(IDbuff, "ID=%s\n", id);
  if (SSL_write(sslClient, IDbuff, 13) < 0){
    fprintf(stderr, "Error writing across the SSL socket (1st write).\n");
    exit(2);
  }
  
  //socket redirection
  close(STDIN_FILENO);   //fd # 0 now open
  int d = dup(sfd);      //sfd duplicate now fd # 0
  if (d == -1){
    fprintf(stderr, "Error duplicating socket fd in client: %s\n", strerror(errno));
    exit(2);
  }
 
  //poll from the beaglebone
  struct pollfd p[1];
  p[0].fd = sfd;   //pollin from the socket
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
      char socketBuff[75];
      int r = sprintf(socketBuff, "%.2d:%.2d:%.2d %.1f\n", local->tm_hour, local->tm_min, local->tm_sec, temp);
      if (r < 0){
	fprintf(stderr, "Error recording to socket Buffer.\n");
	exit(2);
      }
      if (SSL_write(sslClient, socketBuff, r) < 0){
	fprintf(stderr, "Error writing across the SSL socket (2nd write).\n");
	exit(2);
      }
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
      //if(mraa_gpio_read(button)){
      //  shutdownn();
      //}
      //poll for input
      int pval = poll(p,1,0);
      if (pval == -1){
	fprintf(stderr, "Error polling: %s\n", strerror(errno));
	exit(2);
      }
      //poll in from command line
      if (p[0].revents & POLLIN){
	char buff[125];
	memset(buff, 0, 125);
	//read in from socket
	if (SSL_read(sslClient, buff, 125) < 0){
	  fprintf(stderr, "Error reading over SSL socket.\n");
	  exit(2);
	}
	//handle the command received
	commandline(buff);
      }
      time(&finish);
    }
    
  }
  fclose(log_f);
  close(sfd);
  //exit correctly
  SSL_shutdown(sslClient);
  SSL_free(sslClient);
  //exit successfully
  exit(0);
}
