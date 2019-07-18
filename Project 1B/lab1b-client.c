//NAME: Adam Cole
//EMAIL: adam.cole5621@gmail.com
//ID: 004912373

//client

//includes
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netdb.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <mcrypt.h>

//global variables
struct termios restoration;
const int buffsize = 256;
char buffer[256];
char* LOGFILE = NULL;
int logfd = 0;
char* EFILE = NULL;
int s_fd;
int logflag = 0;
int eflag = 0;
MCRYPT encr;
MCRYPT decr;


void sighandler(int signum){
  //handle sigpipe and sigint
  if (signum == SIGPIPE){
    fprintf(stderr, "SIGPIPE caught: %s\n", strerror(errno));
    exit(1);
  } else if (signum == SIGINT){
    fprintf(stderr, "SIGINT caught: %s\n", strerror(errno));
    exit(1);
  }
}

void restoreMode(void){
  tcsetattr(0, TCSANOW, &restoration);
}

void close_session(MCRYPT session){
  //from Zhou slides
  mcrypt_generic_deinit(session);
  mcrypt_module_close(session);
}

void exitfunc(void){
  //restore the terminal attributes
  restoreMode();
  //close the encryption session
  if (eflag == 1){
    close_session(encr);
    close_session(decr);
  }
  //close the socket
  close(s_fd);
}

void canonicalMode(void){
  //get current terminal nodes
  //save them for restoration
  tcgetattr(0, &restoration);
  //make a copy and edit changes
  struct termios copy;
  tcgetattr(0, &copy);
  copy.c_iflag = ISTRIP;
  copy.c_oflag = 0;
  copy.c_lflag = 0;
  //set the changes with TCSANOW option
  tcsetattr(0, TCSANOW, &copy);
  atexit(exitfunc);
}


void encrypt_buffer(char* orig_buf, unsigned long len){
  //from Zhou slides
  //will encrypt
  if (mcrypt_generic(encr, orig_buf, len) != 0){
    fprintf(stderr, "Error in encryption: %s\n", strerror(errno));
    exit(1);
  }
}

void decrypt_buffer(char* orig_buf, unsigned long len){
  //from Zhou slides
  //will decrypt
  if (mdecrypt_generic(decr, orig_buf, len) != 0){
    fprintf(stderr, "Error in encryption: %s\n", strerror(errno));
    exit(1);
  }
}


void logData(char* data, char s_r){
  switch(s_r){
  case 's':
    //sent data over socket
    write(logfd, "SENT 1 bytes: ", 14);
    write(logfd, data, 1);
    write(logfd, "\n", 1);
    break;
  case 'r':
    //received data over socket
    write(logfd, "RECEIVED 1 bytes: ", 18);
    write(logfd, data, 1);
    write(logfd, "\n", 1);
    break;
  }
}


void SocketIO(void) {
  //handles IO of the client to server/shell

  //Use polling to make sure read will not block before reading
  struct pollfd p[2];
  //assign the correct pipes to poll file descriptors
  p[0].fd = 0;             //client keyboard input
  p[1].fd = s_fd;          //socket descriptor to server/shell
  //assign the events to monitor
  //POLLIN = data to read
  //POLLHUP = pipe fd closed by other side
  //POLLERR = error
  p[0].events = POLLIN | POLLHUP | POLLERR;
  p[1].events = POLLIN | POLLHUP | POLLERR;


  //feed keyboard input to socket and log until completion
  while(1) {
    //run the poll  -1 indicates no time out.
    if (poll(p, 2, -1) == -1){
      fprintf(stderr, "Error polling: %s\n", strerror(errno));
      exit(1);
    }

    //keyboard input data to read - send over the socket
    //encrypt the data being sent over the socket before sending
    if (p[0].revents & POLLIN){
      //read bytes
      ssize_t readin = read(0, &buffer, buffsize);
      if (readin < 0){
	fprintf(stderr, "Error reading from client keyboard: %s\n", strerror(errno));
	exit(1);
      }
      if(readin == 0){
	//EOF triggering
	exit(0);
      }
      //handle special characters
      //print to stdout and pipe to shell
      int i = 0;
      for (; i < readin; i++){
	switch(buffer[i]){
	case 0x0D:
	case 0x0A:
	  //encrypt if encryption turned on
	  if(eflag == 1){
	    encrypt_buffer(&buffer[i], 1);
	  }
	  //if logging on, log the data
	  if(logflag == 1){
	    logData(&buffer[i], 's');
	  }
	  //print \r\n to stdout but the normal character to server
	  write(1, "\r\n", 2);
	  write(s_fd, &buffer[i], 1);
	  break;
	case 0x04:  // ctrlD
	  //encrypt if encryption turned on
	  if(eflag == 1){
	    encrypt_buffer(&buffer[i], 1);
	  }
	  //if logging on, log the data
	  if(logflag == 1){
	    logData(&buffer[i], 's');
	  }
	  //print to stdout
	  write(1, "^D", 2);
	  //send as a normal character to socket/shell
	  write(s_fd, &buffer[i], 1);
	  exit(0);
	  break;
	case 0x03:  // ctrlC
	  //kill the server process
	  //encrypt if encryption turned on
	  if(eflag == 1){
	    encrypt_buffer(&buffer[i], 1);
	  }
	  //if logging on, log the data
	  if(logflag == 1){
	    logData(&buffer[i], 's');
	  }
	  //print to stdout
	  //send it to server as a normal character
	  write(1, "^C", 2);
	  write(s_fd, &buffer[i], 1);
	  break;
	default:
	  //normal character, print and socket
	  write(1, &buffer[i], 1);
	  //encrypt if encryption turned on
	  if(eflag == 1){
	    encrypt_buffer(&buffer[i], 1);
	  }
	  //if logging on, log the data
	  if(logflag == 1){
	    logData(&buffer[i], 's');
	  }
	  //write to server
	  write(s_fd, &buffer[i], 1);
	  break;
	}
      }
    } 


    //read socket/shell output data
    //log the recieved encrypted data and then decrypt before printing
    //to terminal screen
    if (p[1].revents & POLLIN){
      //read data
      ssize_t readin = read(s_fd, &buffer, buffsize);
      if (readin < 0){
	fprintf(stderr, "Error reading from Shell output: %s\n", strerror(errno));
	exit(1);
      }
      if(readin == 0){
	//EOF triggering
	exit(0);
      }
      //handle special characters
      //print to stdout
      int i = 0;
      for(; i < readin; i++){
	//if logging on, log the data
	if(logflag == 1){
	  logData(&buffer[i], 'r');
	}
	//decrypt before switch
	//encrypt if encryption turned on
	if(eflag == 1){
	  decrypt_buffer(&buffer[i], 1);
	}
	switch(buffer[i]){
	case 0x0A: // '\n'
	  write(1, "\r\n", 2);
	  break;
	default:   // normal character
	  write(1, &buffer[i], 1);
	  break;
	}
      }
    }

    //no more socket/shell output or an error
    if (p[1].revents & (POLLHUP | POLLERR)){
      exit(0);
    }
  }
}


MCRYPT init_session(void){
  //from Zhou slides
  int key_buf_size = 128;
  char key_buf[key_buf_size];
  //read key from specified file, store key results in key_buf
  int key_fd = open(EFILE, O_RDONLY);
  if (key_fd < 0){
    fprintf(stderr, "Error opening keyfile: %s\n", strerror(errno));
    exit(1);
  }
  //store the length of the key in keylen
  struct stat kstat;
  int s = fstat(key_fd, &kstat);
  if(s < 0){
    fprintf(stderr, "Error in fstat: %s\n", strerror(errno));
    exit(1);
  }
  int keylen = kstat.st_size;
  //read the key into key_buf
  int bytesread = read(key_fd, &key_buf, kstat.st_size);
  if (bytesread < 0){
    fprintf(stderr, "Error reading keyfile: %s\n", strerror(errno));
    exit(1);
  }
  //create a temporary session
  MCRYPT session;
  session = mcrypt_module_open("twofish", NULL, "cfb", NULL);
  //iv = initial vector
  int mal = mcrypt_enc_get_iv_size(session);
  char* iv = malloc(mal);
  memset(iv, 0, mcrypt_enc_get_iv_size(session));
  mcrypt_generic_init(session, key_buf, keylen, iv);
  return session;
}


int client_connect(char* host_name, unsigned int port){
  //from Zhou's slides  
  //create a structure to hold server information
  //encodes the IP address and port for the remote
  struct sockaddr_in serv_addr;

  //create socket with TCP connection and IPv4
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if(sock < 0){
    fprintf(stderr, "Error creating client socket: %s\n", strerror(errno));
    exit(1);
  }

  //convert host_name to IP address
  struct hostent *server = NULL;
  server = gethostbyname(host_name);
  if(server == NULL){
    fprintf(stderr, "Error retrieving server structure: %s\n", strerror(errno));
    exit(1);
  }
  memset(&serv_addr, 0, sizeof(struct sockaddr_in));
  serv_addr.sin_family = AF_INET;

  //copy IP address from server to serv_addr
  memcpy(&serv_addr.sin_addr.s_addr, server->h_addr,server->h_length);

  //setup the port and initiate connection to server
  serv_addr.sin_port = htons(port);
  if(connect(sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0){
    fprintf(stderr, "Error connecting client socket: %s\n", strerror(errno));
    exit(1);
  }

  //return the socket descriptor
  return sock;
}


int main(int argc, char* argv[]){
  //long options for the --log= and --port= options
  struct option options[] = {
    {"port", 1, 0, 'p'},
    {"log", 1, 0, 'l'},
    {"encrypt", 1, 0, 'e'},
    {0, 0, 0, 0}
  };

  int x = 0;
  int port = 0;
  int pflag = 0;
  while((x = getopt_long(argc, argv, "p:l:e:", options, 0)) != -1){
    switch(x){
    case 'p':
      //PORT
      pflag = 1;
      //atoi turns string integer into an int
      port = atoi(optarg);
      break;
    case 'l':
      //LOG
      logflag = 1;
      LOGFILE = optarg;
      break;
    case 'e':
      //ENCRYPT
      eflag = 1;
      EFILE = optarg;
      break;
    default:
      printf("Correct usage: ./lab1b --port=PORT --log=FILENAME --encypt=KEYFILE\n");
      exit(1);
      break;
    }
  }

  //register sighandler
  signal(SIGINT, sighandler);
  signal(SIGPIPE, sighandler);

  //log handling
  if(LOGFILE != NULL){
    //0644 = Owner:RW Group:R Other:R
    logfd = open(LOGFILE, O_RDWR | O_APPEND | O_CREAT, 0666);
    if (logfd < 0){
      fprintf(stderr, "Error opening log file: %s\n", strerror(errno));
      exit(1);
    }
  }

  //check if port was given
  if(pflag != 1){
    fprintf(stderr, "Error: Client must be given a port for connection\n");
    exit(1);
  }
  
  //set up the client
  s_fd = client_connect("localhost", port);

  //encrypt the data packets
  if (eflag == 1){
    //init the MCRYPT sessions
    encr = init_session();
    decr = init_session();
  }

  //put program into non-canonical mode with no echo
  canonicalMode();

  //read and write through socket
  SocketIO();
  
  //exit
  exit(0);
}
