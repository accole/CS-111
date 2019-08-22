//NAME: Adam Cole
//EMAIL: adam.cole5621@gmail.com
//ID: 004912373

//Lab0.c

//includes
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>


//signal handler
void sighandler(int signal){
  if(signal == SIGSEGV){
    fprintf(stderr, "Segmentation fault caught\n");
    exit(4);
  }
}


//subroutine call to cause a segmentation fault
void SegSubroutine(int seg){
  char *c = NULL;
  if(seg == 1){
    *c = 'e';
  }
}


//display the correct usage for incorrect arguments
void displayusage(){
  printf("Correct Usage: (--input is a required argument)\n./lab0 --input=FILE --output=FILE --segfault --catch\n");
}


//main function
int main(int argc, char* argv[]){

  //required_arg == 1
  //no_arg == 0
  //include all 0's all null byte at the end

  //long options for getopt
  struct option options[]={
    {"input", 1, 0, 'i'},
    {"output", 1, 0, 'o'},
    {"segfault", 0, 0, 's'},
    {"catch", 0, 0, 'c'},
    {0,0,0,0}
  };

  int seg = 0;  //indicates if we want to segfault
  int readin, writeout;  //file descriptors
  
  //read the argument options with getopt
  int x = 0;
  //optstring can define the shorthand version of the long options
  //colons represent whether there has to be an input or not
  //this lab only tests long options
  while((x = getopt_long(argc, argv, "i:o:sc", options, 0)) != -1){
    switch(x){
    case 'i':  //input
      readin = open(optarg, O_RDONLY);
      //if open is successful, clone and close
      if (readin >= 0){
	//by closing STDIN, we free file descriptor 0
	close(0);
	//then when we duplicate, the smallest fd available is 0
	dup(readin);
	//therefore, we redirect STDIN to be the given file
	close(readin);
      } else {
	fprintf(stderr, "Error opening file %s: %s\n", optarg, strerror(errno));
	exit(2);
      }
      break;
    case 'o':  //output
      writeout = creat(optarg, S_IRWXU);
      //if creat is successful, clone and close
      if (writeout >= 0){
	//same idea with STDOUT
	close(1);
	//redirects the given file to be STDOUT
	dup(writeout);
	close(writeout);
      } else {
	fprintf(stderr, "Error opening output file %s: %s\n", optarg, strerror(errno));
	exit(3);
      }
      break;
    case 's':   //segfault
      //indicates we want to create a fault
      seg = 1;
      break;
    case 'c':  //catch
      signal(SIGSEGV, sighandler);
      break;
    default:
      //display the correct usage line
      displayusage();
      exit(1);
      break;
    }
  }

  //create a segmentation fault if option set
  SegSubroutine(seg);

  //create a buffer of size 1 byte
  int buffersize = 1;
  char bytes[buffersize+1];

  //continue reading / writing byte by byte until EOF
  ssize_t text = 0;
  while((text = read(STDIN_FILENO, &bytes, buffersize)) > 0){

    if(write(STDOUT_FILENO, &bytes, buffersize) == -1){
      perror("Error writing to STDOUT\n");
      exit(-1);

    }
  }

  if(text == -1){
    perror("Error reading from STDIN\n");
    exit(-1);
  }

  //exit successfully
  exit(0);
}
