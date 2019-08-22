//NAME: Adam Cole, Jake Wallin
//EMAIL: ###########@gmail.com, ###########@gmail.com
//ID: #########, #########

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include "ext2_fs.h"

//globals
char* imgfile = NULL;               //image file
int imgfd;                          //image file descriptor
int SUPERBLOCK_OFFSET = 1024;       //superblock offset
struct ext2_super_block SuperBlock; //superblock struct
int BLOCKSIZE;                      //block size
int BLOCKCOUNT;                     //number of blocks
int INODESCOUNT;                    //number of inodes
int BLOCKpGROUP;                    //blocks per group
int INODESpGROUP;                   //inodes per group
int NUMGROUPS = 1;                  //total number of groups



unsigned long calc_offset(unsigned int blocknum){
  //calculates the offset of a given block
  //from TA slides
  return (long) SUPERBLOCK_OFFSET + (blocknum - 1)*BLOCKSIZE;
}


void SuperBlockInfo(void){
  //offset 1024 bytes into the image to find superblock
  //store superblock data in the structure
  pread(imgfd, &SuperBlock, sizeof(struct ext2_super_block), SUPERBLOCK_OFFSET);
  //print the information stored in the structure to stdout
  fprintf(stdout, "SUPERBLOCK,%d,", SuperBlock.s_blocks_count);  //number of blocks
  BLOCKCOUNT = SuperBlock.s_blocks_count;
  fprintf(stdout, "%d,", SuperBlock.s_inodes_count);      //number of inodes
  INODESCOUNT = SuperBlock.s_inodes_count;
  //given in the header file
  BLOCKSIZE = EXT2_MIN_BLOCK_SIZE << SuperBlock.s_log_block_size;
  fprintf(stdout, "%d,", BLOCKSIZE);                      //blocksize
  fprintf(stdout, "%d,", SuperBlock.s_inode_size);        //inode size
  fprintf(stdout, "%d,", SuperBlock.s_blocks_per_group);  //blocks/group
  BLOCKpGROUP = SuperBlock.s_blocks_per_group;
  fprintf(stdout, "%d,", SuperBlock.s_inodes_per_group);  //inodes/group
  INODESpGROUP = SuperBlock.s_inodes_per_group;
  fprintf(stdout, "%d\n", SuperBlock.s_first_ino);        //first non-reserved inode

  return;
}


void BlocksInfo(int gr, unsigned int block_bitmap){
  //reports all of the free blocks in the bitmap
  //malloc memory to store blocks
  char * byte_arr = (char *) malloc(BLOCKSIZE);
  //calculate offset and current block
  unsigned long offset = calc_offset(block_bitmap);
  unsigned int curr_block = (gr * BLOCKpGROUP) + SuperBlock.s_first_data_block;
  //read block data into memory
  pread(imgfd, byte_arr, BLOCKSIZE, offset);
  int i, j, k, num_bits;
  num_bits = 8;
  for(i = 0; i < BLOCKSIZE; i++){
    //get bit that signifies if used or not
    k = byte_arr[i];
    //loop through each bit
    for(j = 0; j < num_bits; j++){
      if(!(k & 0x01)) {
	//print free block
	fprintf(stdout, "BFREE,%d\n", curr_block);
      }
      //reset k to 0
      k >>= 1;
      //increment block
      curr_block++;
    }
  }
  //free allocated memory
  free(byte_arr);
  return;
}


void directoryInfo(unsigned int parent, unsigned int block){
  //given a directory entry, print directory summary
  //create a struct to store info
  struct ext2_dir_entry dir;
  //calculate the offset
  unsigned long offset = calc_offset(block);
  //loop through the directory
  int len = 0;
  int bytes = 0;
  //fprintf(stdout, "Parent: %d \n", parent);
  for (; bytes < BLOCKSIZE; bytes += len){
    //clear directory entry from struct
    //256 defined in ext2_fs.h
    memset(dir.name, 0, 256);
    //read in next directory entry
    pread(imgfd, &dir, sizeof(struct ext2_dir_entry), offset+bytes);
    //if entry is not empty
    if(dir.inode != 0){
      //clear the directory entry name
      int namelen = dir.name_len;
      memset(&dir.name[namelen], 0, 256 - namelen);
      //print the directory info
      fprintf(stdout, "DIRENT,%d,", parent);    //parent inode
      fprintf(stdout, "%d,", bytes);            //byte offset into directory
      fprintf(stdout, "%d,", dir.inode);        //inode number
      fprintf(stdout, "%d,", dir.rec_len);      //entry length
      fprintf(stdout, "%d,", namelen);          //name length
      fprintf(stdout, "'%s'\n", dir.name);      //name of entry
    }
    //directory entry length
    len = dir.rec_len;
  }
  return;
}

void singleInfo(struct ext2_inode inode, char type, unsigned int curr) {
  //first indirect is in 12th block address
  //allocate enough memory to store inode block addresses
  uint32_t *blocks = malloc(BLOCKSIZE);
  unsigned int addrnum = BLOCKSIZE / sizeof(uint32_t);
  //calculate the offset
  unsigned long offset = calc_offset(inode.i_block[12]);
  //read block addresses into memory
  pread(imgfd, blocks, BLOCKSIZE, offset);

  //loop through all the block addresses checking file type
  unsigned int j = 0;
  fprintf(stdout, "ENTERED SINGLE\n");
  for (; j < addrnum; j++) {
    if (blocks[j] != 0 && type == 'd') {
      directoryInfo(curr, blocks[j]);
    }
    else if (blocks[j] != 0) {
      //then print the indirect information
      fprintf(stdout, "INDIRECT,%d,", curr);    //inode number
      fprintf(stdout, "%d,", 1);          //indirect level 1
      fprintf(stdout, "%d,", 12 + j);       //block offset
      fprintf(stdout, "%d,", inode.i_block[12]);  //indirect block
      fprintf(stdout, "%d\n", blocks[j]);     //reference block
    }
  }
  //free allocated memory
  free(blocks);

  return;
}

void doubleInfo(struct ext2_inode inode, char type, unsigned int curr) {
  //double indirection
  //allocate enough memory to store inode block addresses
  uint32_t *blocks = malloc(BLOCKSIZE);
  unsigned int addrnum = BLOCKSIZE / sizeof(uint32_t);
  //calculate the offset
  unsigned long offset = calc_offset(inode.i_block[13]);
  //read the addresses into memory
  pread(imgfd, blocks, BLOCKSIZE, offset);

  //loop through the indirect block addresses and print out info
  unsigned int j = 0;
  //fprintf(stdout, "ENTERED DOUBLE \n");
  for (; j < addrnum; j++) {
    if (blocks[j] != 0) {
      //print info
      fprintf(stdout, "INDIRECT,%d,", curr);   //inode number
      fprintf(stdout, "%d,", 2);          //indirect level 2
      fprintf(stdout, "%d,", 256 + 12 + j);   //block offset
      fprintf(stdout, "%d,", inode.i_block[13]);  //indirect block
      fprintf(stdout, "%d\n", blocks[j]);     //reference block
    }

    //repeat code from singleInfo() with updated data
    uint32_t *doubleblocks = malloc(BLOCKSIZE);
    unsigned long doubleoffset = calc_offset(blocks[j]);
    pread(imgfd, doubleblocks, BLOCKSIZE, doubleoffset);
    unsigned int x = 0;
    for (; x < addrnum; x++) {
      if (doubleblocks[x] != 0 && type == 'd') {
        directoryInfo(curr, doubleblocks[x]);
      }
      else if (doubleblocks[x] != 0 && blocks[j] != 0) {
        fprintf(stdout, "INDIRECT,%d,", curr);    //inode number
        fprintf(stdout, "%d,", 1);          //indirect level 1
        fprintf(stdout, "%d,", 256 + 12 + x);   //block offset
        fprintf(stdout, "%d,", blocks[j]);      //indirect block
        fprintf(stdout, "%d\n", doubleblocks[x]); //reference block
      }
    }
    free(doubleblocks);
  }

  //free allocated memory
  free(blocks);

  return;
}

void tripleInfo(struct ext2_inode inode, char type, unsigned int curr) {
  //triple indirection
  //allocate enough memory to store inode block addresses
  uint32_t *blocks = malloc(BLOCKSIZE);
  unsigned int addrnum = BLOCKSIZE / sizeof(uint32_t);
  //calculate the offset
  unsigned long offset = calc_offset(inode.i_block[14]);
  //read the addresses into memory
  pread(imgfd, blocks, BLOCKSIZE, offset);

  //loop through the indirect block addresses and print out info
  unsigned int j = 0;
  //fprintf(stdout, "ENTERED TRIPLE \n");
  int print = 0;
  for (; j < addrnum; j++) {
    uint32_t prev2 = 0;
    if (blocks[j] != 0 && blocks[j] != prev2) {
      //print info
      fprintf(stdout, "INDIRECT,%d,", curr);     //inode number
      fprintf(stdout, "%d,", 3);            //indirect level 3
      fprintf(stdout, "%d,", 65536 + 256 + 12 + j); //block offset
      fprintf(stdout, "%d,", inode.i_block[14]);    //indirect block
      fprintf(stdout, "%d\n", blocks[j]);       //reference block
      //fprintf(stdout, " FIRST ONE %d\n", blocks[j]);
      prev2 = blocks[j];
    }

    //repeat code from doubleInfo() with updated data
    uint32_t *doubleblocks = malloc(BLOCKSIZE);
    unsigned long doubleoffset = calc_offset(blocks[j]);
    pread(imgfd, doubleblocks, BLOCKSIZE, doubleoffset);
    unsigned int x = 0;
    for (; x < addrnum; x++) {
      uint32_t prev1 = 0;
      if (doubleblocks[x] != 0 && blocks[j] != 0 && doubleblocks[x] != prev1 ) {
        fprintf(stdout, "INDIRECT,%d,", curr);     //inode number
        fprintf(stdout, "%d,", 2);            //indirect level 2
        fprintf(stdout, "%d,", 65536 + 256 + 12 + x); //block offset
        fprintf(stdout, "%d,", blocks[j]);        //indirect block
        fprintf(stdout, "%d\n", doubleblocks[x]);   //reference block
        //fprintf(stdout, " SECOND ONE %d\n", doubleblocks[x]);
        prev1 = doubleblocks[x];
      }

      //repeat code from singleInfo() with updated data
      uint32_t *tripleblocks = malloc(BLOCKSIZE);
      unsigned long tripleoffset = calc_offset(doubleblocks[x]);
      pread(imgfd, tripleblocks, BLOCKSIZE, tripleoffset);
      unsigned int y = 0;
      uint32_t prev = 0;
      for (; y < addrnum; y++) {
        if (tripleblocks[y] != 0 && type == 'd') {
          directoryInfo(curr, tripleblocks[y]);
        }
        else if (tripleblocks[y] != 0 && doubleblocks[x] != 0 && tripleblocks[y] != prev && !print) {
          fprintf(stdout, "INDIRECT,%d,", curr);      //inode number
          fprintf(stdout, "%d,", 1);            //indirect level 1
          fprintf(stdout, "%d,", 65536 + 256 + 12 + y); //block offset
          fprintf(stdout, "%d,", doubleblocks[x]);    //indirect block
          fprintf(stdout, "%d\n", tripleblocks[y]);   //reference block
          //fprintf(stdout, " THIRD ONE %d\n", print);
          prev = tripleblocks[y];
          print = 1;
        }
      }
      free(tripleblocks);
    }
    free(doubleblocks);
  }

  //free allocated memory
  free(blocks);

  return;
}


void TimePrint(time_t t, char* buff){
  //given a raw time, update buffer with the formatted time
  struct tm format = *gmtime(&t);
  strftime(buff, 20, "%m/%d/%y %H:%M:%S", &format);
  fprintf(stdout, "%s,", buff); 
  return;
}


void readInode(unsigned int current, unsigned int inode_table){
  //reads information about the current allocated inode
  //create a struct to store the information about the inode
  struct ext2_inode str_inode;
  //calculate the offset of the inode table
  unsigned int tableoffset = current - 1;
  unsigned long offset = calc_offset(inode_table) + tableoffset*sizeof(struct ext2_inode);
  //read into the structure
  pread(imgfd, &str_inode, sizeof(struct ext2_inode), offset);

  //don't print out reserved inodes or that don't have links
  if (str_inode.i_mode == 0){
    //i_mode corrupted
    return;
  } else if (str_inode.i_links_count == 0){
    //links_count == how many times the inode is referred to
    return;
  }

  //get the file type
  char c = 'x';   //x = not set yet
  unsigned short int bits = str_inode.i_mode;
  unsigned short int new_bits = (bits >> 12) << 12;   //clear out lower 12 bits
  if (new_bits == 0xa000){
    //symbolic link
    c = 's';
  } else if (new_bits == 0x8000){
    //normal file
    c = 'f';
  } else if (new_bits == 0x4000){
    //directory
    c = 'd';
  }

  //get the creation, access, and modification times of the inode
  //use time function

  //print inode information
  fprintf(stdout, "INODE,%d,", current);               //inode number
  fprintf(stdout, "%c,", c);                           //file type
  fprintf(stdout, "%o,", str_inode.i_mode & 0xFFF);    //mode
  fprintf(stdout, "%d,", str_inode.i_uid);             //user access
  fprintf(stdout, "%d,", str_inode.i_gid);             //group access
  fprintf(stdout, "%d,", str_inode.i_links_count);     //how many times linked
  char creation[20];
  char modification[20];
  char access[20];
  TimePrint(str_inode.i_ctime, creation); //creation time
  TimePrint(str_inode.i_mtime, modification); //modification time
  TimePrint(str_inode.i_atime, access); //access time
  fprintf(stdout, "%d,", str_inode.i_size);            //file size
  fprintf(stdout, "%d,", str_inode.i_blocks);          //number of blocks
  int i = 0;                                           //block addresses
  for(; i < 14; i++){
    //each inode has 15 pointers in block[]
    fprintf(stdout, "%d,", str_inode.i_block[i]);
  }
  fprintf(stdout, "%d", str_inode.i_block[14]);
  fprintf(stdout, "\n");

  //if it's a directory, print directory listing
  //0-11 block addresses are normal
 
  for (i = 0; i < 12; i++){
    if (str_inode.i_block[i] != 0){
      //if address is valid
      if (c == 'd'){
	//and if the file is a directory
	       directoryInfo(current, str_inode.i_block[i]);
      }
    }
  }
  if(c == 'x' || c == 's') return; //no indirect entries for symbolic (x is also bad)
    //indirect entries
  //12th block address
  if (str_inode.i_block[12] != 0) {
    singleInfo(str_inode, c, current);
  }

  //doubly indirect entries
  //13th block address
  if (str_inode.i_block[13] != 0) {
    doubleInfo(str_inode, c, current);
  }

  //triply indirect entries
  //14th block address
  if (str_inode.i_block[14] != 0) {
    tripleInfo(str_inode, c, current);
  }


  return;
}


void InodeInfo(unsigned int inode_bitmap, unsigned int inode_table){
  //reads the inode bitmap to find all the used/free inodes
  //calculate the number of bytes needed to store inode information
  //this works because an inode is represented by 1 bit. 8 bits per byte
  int inodeBytes = INODESpGROUP / 8;
  char * byte_arr = (char*) malloc(inodeBytes);
  //calculate the offset and the current node
  unsigned long offset = calc_offset(inode_bitmap);
  //read data into memory
  pread(imgfd, byte_arr, inodeBytes, offset);
  //loop through all the inodes in the bitmap
  int free_size = (inodeBytes * 8) + 1;
  int inode_free_array[free_size];
  int x;
  for(x = 0; x < inodeBytes * 8; x++){
    inode_free_array[x] = 0;
  }
  int bit_var = 0;
  for(x = 0; x < free_size; x++){
    if (x == 0) bit_var = byte_arr[0];
    else if((x-1) % 8 == 0) bit_var = byte_arr[x/8];
    if(!(bit_var & 0x01)) inode_free_array[x] = 0;
    else inode_free_array[x] = 1;
    bit_var >>= 1;
  }
  //start at 1. not supposed to print inode 0
  for(x = 1; x < free_size; x++){
    if(inode_free_array[x] == 0) fprintf(stdout, "IFREE,%d\n", x);
  }
   for(x = 1; x < free_size; x++){
    if(inode_free_array[x] == 1) readInode(x, inode_table);
  }  
  //free allocated memory
  free(byte_arr);
  return;
}



void GroupInfo(){
  //use a structure to hold all the information about current group
  int i;
  //check for remainder
  for(i = 0; i < NUMGROUPS; i++){
      struct ext2_group_desc currgroup;

      //calculate group descriptor
      //from Zhou slides
      int option = 0;
      if(BLOCKSIZE == 1024){
        option = 2;
      } else {
        option = 1;
      }
  
      //calculate the offset of the group
      long offset = BLOCKSIZE*option + 32*i;

      //read from the image file into the group structure
      pread(imgfd, &currgroup, sizeof(struct ext2_group_desc), offset);
    
      //printf group information to stdout
      fprintf(stdout, "GROUP,%d,", i);      //group number
      fprintf(stdout, "%d,", BLOCKCOUNT);       //total number of blocks
      fprintf(stdout, "%d,", INODESCOUNT);       //total number of inodes
      fprintf(stdout, "%d,", currgroup.bg_free_blocks_count);     //number of free blocks
      fprintf(stdout, "%d,", currgroup.bg_free_inodes_count);     //number of free inodes
      fprintf(stdout, "%d,", currgroup.bg_block_bitmap);   //block number of free block bitmap
      fprintf(stdout, "%d,", currgroup.bg_inode_bitmap);   //block number of free inode bitmap
      fprintf(stdout, "%d\n", currgroup.bg_inode_table);    //block number of first inode

      //get the information from the block bitmap
      BlocksInfo(i, currgroup.bg_block_bitmap);

      //get the information from the inode bitmap
      //fprintf(stdout, "gr: %d\n", gr);
      //fprintf(stdout, "inode table: %d\n", currgroup.bg_inode_table);
      //fprintf(stdout, "inode bitmap: %d\n", currgroup.bg_inode_bitmap);
      InodeInfo(currgroup.bg_inode_bitmap, currgroup.bg_inode_table);
  }
  return;
}


int main(int argc, char* argv[]){  
  //test for correct usage
  if (argc != 2){
    fprintf(stderr, "Incorrect number of arguments.\n");
    exit(1);
  }
  //take the image
  imgfile = argv[1];
  //open the image
  imgfd = open(imgfile, O_RDONLY);
  if (imgfd < 0){
    fprintf(stderr, "Error opening image file: %s", strerror(errno));
    exit(1);
  }

  //first create the superblock summary
  SuperBlockInfo();
  //then handle the group info
  GroupInfo();
  //close file descriptor
  close(imgfd);

  //exit successfully
  exit(0);
}
