//NAME: Adam Cole
//EMAIL: #############@gmail.com
//ID: #########

//server

//includes
#include <unistd.h>
#include <termios.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netdb.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <mcrypt.h>


//global variables
const int buffsize = 256;
char buffer[256];
int pipeP2C[2];
int pipeC2P[2];
pid_t cpid;
int forked = 1;
int s_fd = 0;
char* EFILE = NULL;
MCRYPT encr;
MCRYPT decr;
int eflag = 0;


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
    fprintf(stderr, "Error in decryption: %s\n", strerror(errno));
    exit(1);
  }
}


void SocketIO(void){
  //handles IO of the shell

  //want to write in P2C[1] and read in C2P[0]
  close(pipeP2C[0]);
  close(pipeC2P[1]);

  //Use polling to make sure read will not block before reading
  struct pollfd p[2];
  //assign the correct pipes to poll file descriptors
  p[0].fd = s_fd;             //client input from socket 
  p[1].fd = pipeC2P[0];       //shell output from child in C2P
  //assign the events to monitor
  //POLLIN = data to read
  //POLLHUP = pipe fd closed by other side
  //POLLERR = error
  p[0].events = POLLIN | POLLHUP | POLLERR;
  p[1].events = POLLIN | POLLHUP | POLLERR;

  //feed keyboard input to shell until completion
  while(1) {
    //run the poll  -1 indicates no time out.
    if (poll(p, 2, -1) == -1){
      fprintf(stderr, "Error polling: %s\n", strerror(errno));
      exit(1);
    }

    //read from client socket port to server process
    //decrypt before sending to shell process
    if (p[0].revents & POLLIN){
      //read bytes
      ssize_t readin = read(s_fd, &buffer, buffsize);
      if (readin < 0){
	fprintf(stderr, "Error reading from client socket: %s\n", strerror(errno));
	exit(1);
      }
      //handle special characters
      //only pipe to shell, dont print to socket
      //since client process already prints
      int i = 0;
      for (; i < readin; i++){
	//decrypt
	if(eflag == 1){
	  decrypt_buffer(&buffer[i], 1);
	}
	//fprintf(stderr, &buffer[i]);
	switch(buffer[i]){
	case 0x0D:
	case 0x0A:
	  //print \r\n back to client but only \n to pipe
	  write(pipeP2C[1], "\n", 1);
	  break;
	case 0x04:
	  //ctrlD - no more input
	  //close the pipe that goes to the child process
	  close(pipeP2C[1]);
	  //dont close the other end because output from
	  //shell could still be processing
	  break;
	case 0x03:  // ctrlC
	  //kill the shell process to avoid
	  //a never ending program
	  kill(cpid, SIGINT);
	  break;
	default:
	  //normal character, send to pipe
	  write(pipeP2C[1], &buffer[i], 1);
	  break;
	}
      }
    } 


    //send shell output data to client
    //encrypt before sending to client terminal
    if (p[1].revents & POLLIN){
      //read data
      ssize_t readin = read(pipeC2P[0], &buffer, buffsize);
      if (readin < 0){
	fprintf(stderr, "Error reading from Shell output: %s\n", strerror(errno));
	exit(1);
      }
      //handle special characters
      //send to client all characters encrypted
      int i = 0;
      for(; i < readin; i++){
	if (eflag == 1){
	    encrypt_buffer(&buffer[i], 1);
	}
	write(s_fd, &buffer[i], 1);
      }
    }

    //no more shell output or an error
    if (p[1].revents & (POLLHUP | POLLERR)){
      exit(0);
    }
  }
}


void checkStatus(void){
  //check exit status of child process if fork ran
  if (forked){
    int estatus;
    if (waitpid(cpid, &estatus, 0) == -1){
      fprintf(stderr, "Error in waitpid: %s\n", strerror(errno));
      exit(1);
    }
    if (WIFEXITED(estatus)){
      fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", WTERMSIG(estatus), WEXITSTATUS(estatus));
      exit(0);
    }
  }
}

void close_session(MCRYPT session){
  //from Zhou slides
  mcrypt_generic_deinit(session);
  mcrypt_module_close(session);
}


void exitfunc(void){
  //get shell exit status if shell ran
  checkStatus();
  //close the encryption session
  if (eflag == 1){
    close_session(encr);
    close_session(decr);
  }
  //close the socket
  close(s_fd);
}

void sighandler(int signum){
  //SIGPIPE
  if (signum == SIGPIPE){
    fprintf(stderr, "SIGPIPE caught: %s\n", strerror(errno));
    exit(1);
  } else if (signum == SIGINT){
    fprintf(stderr, "SIGINT caught: %s\n", strerror(errno));
    exit(1);
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


int server_connect(unsigned int port){
  //server side code
  //from Zhou slides

  //create structs for server and client addresses
  struct sockaddr_in serv_addr, client_addr;
  unsigned int client_len = sizeof(struct sockaddr_in);
  int listenfd = socket(AF_INET, SOCK_STREAM, 0);
  int retfd = 0;
  memset(&serv_addr, 0, sizeof(struct sockaddr_in));
  //IPv4 address
  serv_addr.sin_family = AF_INET;
  //server can accept connection from any client IP
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  //set up port number
  serv_addr.sin_port = htons(port);
  //blind socket to port
  bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
  //let the socket listen, maximum 5 pending connections
  listen(listenfd, 5);
  //wait for client's connection, client_addr stores client's address
  retfd = accept(listenfd, (struct sockaddr*)&client_addr, &client_len);
  //retfd is returned, not listenfd
  return retfd;
}



int main(int argc, char* argv[]){
  //at exit, exit function
  atexit(exitfunc);

  //long options for the --arguments
  struct option options[] = {
    {"port", 1, 0, 'p'},
    {"encrypt", 1, 0, 'e'},
    {0, 0, 0, 0}
  };

  int x = 0;
  int pflag = 0;
  int port = 0;
  char* FILE = "/bin/bash";
  while((x = getopt_long(argc, argv, "p:e:", options, 0)) != -1){
    switch(x){
    case 'p':
      pflag = 1;
      port = atoi(optarg);
      break;
    case 'e':
      //encyption
      eflag = 1;
      EFILE = optarg;
      break;
    default:
      printf("Correct usage: ./lab1a --shell=PROGRAM --port=PORT --encrypt=KEYFILE\n");
      exit(1);
      break;
    }
  }

  //server side code
  if (pflag != 1){
    //error port must be given
    fprintf(stderr,"Error: no port provided\n");
    exit(1);
  }

  //connect the server to clients
  s_fd = server_connect(port);

  //register sigpipe handler
  signal(SIGPIPE, sighandler);
  signal(SIGINT, sighandler);

  //set up encryption
  if (eflag == 1){
    //init the MCRYPT sessions
    encr = init_session();
    decr = init_session();
  }

  //Both Parent and Child process must have access to the
  //pipe file descriptors
  //therefore we must pipe first and then fork
  //create two pipes since both parent and child will run

  if (pipe(pipeP2C) == -1){
    fprintf(stderr, "Error creating pipe: %s\n", strerror(errno));
    exit(1);
  }
  if (pipe(pipeC2P) == -1) {
    fprintf(stderr, "Error creating pipe: %s\n", strerror(errno));
    exit(1);
  }
  
  //fork if shell called
  //new process pid_t

  if (forked){
    //create a new process
    cpid = fork();

    if (cpid == -1){
      //error creating new process
      fprintf(stderr, "Error forking: %s\n", strerror(errno));
      exit(1);

    } else if (cpid == 0){                                //child process 
                                                          //- runs the shell
      //want to read in P2C[0] and write in C2P[1]
      close(pipeP2C[1]);
      close(pipeC2P[0]);
      //input redirection
      close(0);           //read from STDIN = pipeP2C[0]
      dup(pipeP2C[0]);
      close(1);           //write to STDOUT = pipeC2P[1]
      dup(pipeC2P[1]);
      close(2);           //write to STDERR = pipeC2P[1]
      dup(pipeC2P[1]);
      //close duplicate files
      close(pipeP2C[0]);
      close(pipeC2P[1]);
      //run the shell
      if(execlp(FILE, FILE, (char*)NULL) == -1){
	fprintf(stderr, "Error executing program: %s\n", strerror(errno));
	forked = 0;
	exit(1);
      }

    } else {                        //parent process 
      SocketIO();                   //- handles shell IO
    }
  }

  //exit
  exit(0);
}
