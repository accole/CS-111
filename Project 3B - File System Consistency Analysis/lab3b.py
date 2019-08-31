#!/usr/bin/python

#NAME: Adam Cole, Jake Wallin
#EMAIL: #############@gmail.com, #############@gmail.com
#ID: #########, #########

#imports
import csv
import sys
from collections import defaultdict

#globals
sysimagecsv=""
inode_list = []
ref_dict = defaultdict(list)
directory_list = []
free_block_set = set()
free_inode_set = set()
total_inode = 0
total_block = 0
group = 0


#class definitions
#class used to check for block references
class BlockReference:
    def __init__(self,arg):
        self.offset = arg[0]
        self.indirect_number = arg[1]
        self.block_num = arg[2]
        self.inode_num =arg[3]

#class to store information on allocated Inodes
class Inode:
    def __init__(self,arg):
        self.inode = int(arg[1])
        self.filetype = arg[2]
        self.num_links = int(arg[6])
        self.inode_blocks = arg[12:27]
        #turn all strings into ints
        for x in range(0, len(self.inode_blocks)):
            self.inode_blocks[x] = int (self.inode_blocks[x])


#class to store information for Directory Entries
class Directory:
    def __init__(self,arg):
        self.parent_inode = int(arg[1])
        self.file_inode = int(arg[3])
        self.directory_length = int(arg[4])
        self.name_length = int(arg[5])
        self.name = arg[6]



def main():
    #must input command line argument
    if (len(sys.argv) != 2):
        sys.stderr.write("Correct Usage: ./lab3b.py filename.csv")
        exit(1);

    #update global image csv
    global sysimagecsv
    sysimagecsv = sys.argv[1];

    #Open the csv and loop through the contents
    with open(sysimagecsv, 'rb') as csvimage:
        readin = csv.reader(csvimage)
        for line in readin:
            #line[0] is the first word in the line row
            s = line[0]
            if s == "SUPERBLOCK":
                #create superblock class and store it in a global
                total_block = int(line[1])
                total_inode = int(line[2])
            elif s == "GROUP":
                #do nothing with the group lines
                group = 1
            elif s == "BFREE":
                #store free block numbers in the free block set
                global free_block_set
                #print "INT %d" %(int(line[1]))
                free_block_set.add(int(line[1]))
            elif s == "IFREE":
                #store free node numbers in free inode set
                global free_inode_set
                free_inode_set.add(int(line[1]))
            elif s == "INODE":
                #store in a class and in Inode list
                global inode_list
                inode_list.append(Inode(line))
            elif s == "DIRENT":
                #store in a class and in directory list
                global directory_list
                directory_list.append(Directory(line))
            elif s == "INDIRECT":
                inode_owner = int(line[1])
                indirect_number = int(line[2])
                offset = int(line[3])
                block_num = int(line[4])
                block_ref = int(line[5])
                free_block_set.add(int(line[5]))
                w = ""
                #check if block number is invalid
                b1 = block_ref < 0
                b2 = block_ref > total_block
                b3 = block_ref < 8
                b4 = block_ref != 0
                if (b1 | b2):
                    w = "INVALID"
                #now check if the block number is reserved
                elif(b3 & b4):
                    w = "RESERVED"
                #if reserved or invalid
                if indirect_number == 1:
                    type = "INDIRECT "
                elif indirect_number == 2:
                    type = "DOUBLE INDIRECT "
                elif indirect_number == 3:
                    type = "TRIPLE INDIRECT "
                if (len(w) != 0):
                    print "%s %sBLOCK %d IN INODE %d AT OFFSET %d" %(w, type, block_ref, inode_owner, offset)            
            else:
                sys.stderr.write("Error: CSV file formatted incorrect\n")
                exit(1)

    #now that we have everything sorted into lists by row, we
    #must double check the nodes add up

    #check blocks
    for h in range(0, total_inode):
        #if not the first 10 inodes
        b1 = (h >= 11)
        b2 = (h == 2)
        if (b1 | b2):
            #create two flags for if allocated or free
            #print "INDEX %d" %(h)
            f_bool = False
            a_bool = False
            #for each inode in the list
            for node in inode_list:
                #check if allocated
                if (node.inode == h):
                    a_bool = True
                    break
            for free in free_inode_set:
                #print(free)
                if(free == h):
                    f_bool = True
                    break
            #for each inode in the free set
            #print "INDEX: %d" %(h)
            #print "F FLAG  %d" %(f_bool)
            #print "A FLAG  %d" %(a_bool)
            #check if both flags are equal
            if (a_bool == True and f_bool == True):
                #either allocated inode on freelist
                print "ALLOCATED INODE %d ON FREELIST" %(h)
            if (a_bool == False and f_bool == False):
                print "UNALLOCATED INODE %d NOT ON FREELIST" %(h)

    #for each node in the inode list
    parentdict = {}
    printbool1 = False
    printbool2 = False
    #loop through all the inodes, set up the reference dictionary for the blocks
    for i in inode_list:
        #for each block in the inode
        #print "INDEX %d" %(i.inode)
        #counter
        count = 0
        #loop through all the directories
        for d in directory_list:
            #if parent, skip parent
            #print("F INODE %d") %(f.inode)
            #print("D PARENT INODE %d") %(d.file_inode)
            #print("TOTAL %d") %(total)  
            if (i.inode == d.file_inode):
                count += 1
            #check that inodes are valid 
            elif (i.inode == d.parent_inode and (d.file_inode < 1 or d.file_inode > total_inode)):
                print "DIRECTORY INODE %d NAME %s INVALID INODE %d" %(d.parent_inode, d.name, d.file_inode)
            #check that the inodes are allocated
            elif (i.inode == d.parent_inode and not any(p.inode == d.file_inode for p in inode_list)):
                print "DIRECTORY INODE %d NAME %s UNALLOCATED INODE %d" %(d.parent_inode, d.name, d.file_inode)
            #add parent to dictionary to check parent links
            if ((d.name == "'.'" and d.name == "'..'") or d.parent_inode == 2):
                parentdict[d.file_inode] = d.parent_inode
            #parent links
            if (d.name == "'.'" and d.file_inode != d.parent_inode and printbool1 == False):
                print "DIRECTORY INODE %d NAME '.' LINK TO INODE %d SHOULD BE %d" %(d.parent_inode, d.file_inode, d.parent_inode)
                printbool1 = True
            #self links
            elif (d.name == "'..'" and d.file_inode != parentdict[d.parent_inode] and printbool2 == False):
                print "DIRECTORY INODE %d NAME '..' LINK TO INODE %d SHOULD BE %d" %(d.parent_inode, d.file_inode, parentdict[d.parent_inode])
                printbool2 = True

        if (count != i.num_links):
            print "INODE %d HAS %d LINKS BUT LINKCOUNT IS %d" %(i.inode, count, i.num_links)

        for j in range(0, len(i.inode_blocks)): #loop through all the blocks for each inode
            #get the current block
            currblock = i.inode_blocks[j]
            #define variables
            w = ""
            offset = 0
            #calculate offset
            if (j == 12):
                #valid offset
                offset = j
            elif (j == 13):
                offset = 268
            elif (j == 14):
                offset = 65804
            type = "" 
            if j == 12:
                type = "INDIRECT "
            elif j == 13:
                type = "DOUBLE INDIRECT "
            elif j == 14:
                type = "TRIPLE INDIRECT "            
            #first check if the block is invalid
            b1 = currblock < 0
            b2 = currblock > total_block
            b3 = currblock < 8
            b4 = currblock != 0
            if (b1 | b2):
                w = "INVALID"
            #then check if the block is reserved
            elif (b3 & b4):
                w = "RESERVED"
            #reserved or invalid
            if (len(w) != 0):
                print "%s %sBLOCK %d IN INODE %d AT OFFSET %d" %(w, type, currblock, i.inode, offset)
            #else valid block - check offsets
            else:
                #print("REF DICT %d") %(currblock) 
                global ref_dict
                ref_dict[currblock].append(BlockReference([offset, j%11, currblock, i.inode]))

    #loop through all blocks
    for m in range(0, total_block):
        #if the block is not in the free block set or been referenced
        #print "REFDICT %s  INDEX %d" %(ref_dict.get(m), m)
        if (m not in free_block_set and ref_dict.get(m) == None and m >= 8):
            print "UNREFERENCED BLOCK %d" %(m)
        elif (m in free_block_set and ref_dict.get(m) != None and m >= 8):
            print "ALLOCATED BLOCK %d ON FREELIST" %(m)
    #check for allocated blocks reported on free list
        elif (ref_dict.get(m) != None and len(ref_dict[m]) >= 2 and m >= 8):
            #for each of the duplicate blocks reported
            for r in ref_dict[m]:
                type = ""
                if r.indirect_number == 1:
                   type = "INDIRECT "
                elif r.indirect_number == 2:
                   type = "DOUBLE INDIRECT "
                elif r.indirect_number == 3:
                    type = "TRIPLE INDIRECT "
                print "DUPLICATE %sBLOCK %d IN INODE %d AT OFFSET %d" %(type, r.block_num, r.inode_num, r.offset)

#main
if __name__ == '__main__':
    main()
