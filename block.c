#include "block.h"
#include "image.h"
#include "free.c"
#include <stdio.h>
#include <unistd.h>

const int BLOCK_SIZE = 4096;
//block size of the disk we are representing

unsigned char *bread(int block_num, unsigned char *block){
    int chk=lseek(image_fd, block_num*BLOCK_SIZE, SEEK_SET);
    if(chk<0) fprintf(stderr, "lseek failed");
    //move global file descriptor to location corresponding to block_num and check if it worked

    chk=read(image_fd, block, BLOCK_SIZE);
    if(chk<0) fprintf(stderr, "write() failed");
    //read to the buffer from the "disk" and check if it worked

    return block;
    //return buffer
}

void bwrite(int block_num, unsigned char *block){

    int chk=lseek(image_fd, block_num*BLOCK_SIZE, SEEK_SET);
    if(chk<0) fprintf(stderr, "lseek failed");
    //move global file descriptor to location corresponding to block_num and check if it worked

    chk=write(image_fd, block, BLOCK_SIZE);
    if(chk<0) fprintf(stderr, "read() failed");
    //write from the buffer to the "disk" and check if it worked
}

int alloc(void){
    //get block map
    unsigned char buffer[4096]={0};
    bread(2, buffer);

    //find and set as used first available free block
    int index=find_free(buffer);
    if (index<0){
        return -1;
    }
    set_free(buffer, index, 1);

    //write new map back out to disk
    bwrite(2, buffer);

    return index;
}