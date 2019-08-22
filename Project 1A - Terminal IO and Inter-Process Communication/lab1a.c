//NAME: Adam Cole
//EMAIL: adam.cole5621@gmail.com
//ID: 004912373


//includes
#include <unistd.h>
#include <termios.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <poll.h>
#include <signal.h>



//global variables
struct termios restoration;
const int buffsize = 256;
const char buffer[256];
int pipeP2C[2];
int pipeC2P[2];
pid_t cpid;
int forked = 0;



void ParentProcess(void){
  //handles IO of the shell

  //want to write in P2C[1] and read in C2P[0]
  close(pipeP2C[0]);
  close(pipeC2P[1]);

  //Use polling to make sure read will not block before reading
  struct pollfd p[2];
  //assign the correct pipes to poll file descriptors
  p[0].fd = 0;             //keyboard input to parent
  p[1].fd = pipeC2P[0];    //shell output from child in C2P
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

    //read keyboard input data to read
    if (p[0].revents & POLLIN){
      //read bytes
      ssize_t readin = read(0, &buffer, buffsize);
      if (readin < 0){
	fprintf(stderr, "Error reading from keyboard: %s\n", strerror(errno));
	exit(1);
      }
      //handle special characters
      //print to stdout and pipe to shell
      int i = 0;
      for (; i < readin; i++){
	switch(buffer[i]){
	case 0x0D:
	case 0x0A:
	  //print \r\n to stdout but only \n to pipe
	  write(1, "\r\n", 2);
	  write(pipeP2C[1], "\n", 1);
	  break;
	case 0x04:
	  //ctrlD - no more input
	  //print to stdout
	  //write(0, "^D", 2);
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
	  //normal character, print and pipe
	  write(1, &buffer[i], 1);
	  write(pipeP2C[1], &buffer[i], 1);
	  break;
	}
      }
    } 


    //read shell output data
    if (p[1].revents & POLLIN){
      //read data
      ssize_t readin = read(pipeC2P[0], &buffer, buffsize);
      if (readin < 0){
	fprintf(stderr, "Error reading from Shell output: %s\n", strerror(errno));
	exit(1);
      }
      //handle special characters
      //print to stdout
      int i = 0;
      for(; i < readin; i++){
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

    //no more shell output or an error
    if (p[1].revents & (POLLHUP | POLLERR)){
      exit(0);
    }
  }
}



void CharByChar(void){
  //Normal Console Mode
  //read ASCII input from the keyboard into a buffer
  //in character-at-a-time mode.
  //only stop on a ctrl D, not a \r or \n

  ssize_t readin = 0;
  while ((readin = read(0, &buffer, buffsize)) > 0){
    int i;
    for (i=0; i < readin; i++){
      switch(buffer[i]){
      case 0x0D:// '\r'
      case 0x0A:// '\n'
	//Print both <cr><lf> for either case
	write(1, "\r\n", 2);
	break;
      case 0x04:// '^D'
	//EOF detected
	write(1, "^D", 2);
	exit(0);
	break;
      default:
	//write character
	write(1, &buffer[i], 1);
	break;
      }
    }
  }
}


void restoreMode(void){
  tcsetattr(0, TCSANOW, &restoration);
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



void exitfunc(void){
  //restore terminal attributes
  restoreMode();
  //get shell exit status if shell ran
  checkStatus();
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



int main(int argc, char* argv[]){
  //long options for the --shell argument
  struct option options[] = {
    {"shell", 1, 0, 's'},
    {0, 0, 0, 0}
  };

  int x = 0;
  char* FILE = NULL;
  while((x = getopt_long(argc, argv, "s", options, 0)) != -1){
    switch(x){
    case 's':
      forked = 1;
      FILE = optarg;
      break;
    default:
      printf("Correct usage: ./lab1a --shell=PROGRAM\n");
      exit(1);
      break;
    }
  }

  //put program into non-canonical mode with no echo
  canonicalMode();
  
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
      ParentProcess();             //- handles shell IO
    }
  }

  //if no --shell used
  CharByChar();

  //exit
  exit(0);
}
