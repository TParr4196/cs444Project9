#include "dir.h"
#include "inode.h"
#include "block.h"
#include "pack.h"
#include "free.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//constants to define metadata
#define BLOCK_SIZE 4096
#define SUPERBLOCK 0
#define INODE_MAP 1
#define BLOCK_MAP 2
#define INODE_DATA_BLOCK_0 3
#define DIRECTORY_SIZE 32
#define INODE_NUM_SIZE 2
#define RWX 7
#define DIRECTORY 2

void mkfs(void){
    //initialize basic metadeta
    unsigned char block[BLOCK_SIZE]={0};
    for(int i=0; i<BLOCK_SIZE; i++){
        block[i]=0x0;
    }
    for(int i=0; i<3; i++){
        set_free(block, i, 1);
    }
    bwrite(BLOCK_MAP,block);
    for(int i=0; i<31; i++){
        set_free(block, i, 0);
    }

    //not sure what goes in the superblock so giving it empty block
    bwrite(SUPERBLOCK, block); 
    bwrite(INODE_MAP, block);

    struct inode *in = ialloc();
    int block_index = alloc();
    bread(block_index, block);

    //initialize inode data
    in->size=2*DIRECTORY_SIZE; //directory contains itself and parent directory, which for root directory is itself again
    in->owner_id=0; //no users yet?
    in->permissions=7; //r 4 w 2 x 1
    in->flags=DIRECTORY; //marks as directory
    in->link_count=1; //only link to root directory is itself
    in->block_ptr[0]=*block;

    //create directory entries for . and ..
    write_u16(block, in->inode_num);
    strcpy((char*)block+INODE_NUM_SIZE, ".");
    write_u16(block+DIRECTORY_SIZE, in->inode_num);
    strcpy((char*)(block+DIRECTORY_SIZE+INODE_NUM_SIZE), "..");
    bwrite(block_index, block);
    iput(in);
}

struct directory *directory_open(int inode_num){
    //initialize location in memory
    struct directory *dir=(struct directory*)malloc(sizeof(struct directory));
    dir->inode=iget(inode_num);
    if(dir->inode==NULL){
        //return NULL if there is no availability for inode_num
        return NULL;
    }
    //offset measures how far into the directory we have read
    dir->offset=0;
    return dir;
}

int directory_get(struct directory *dir, struct directory_entry *ent){
    //return -1 if the directory has been fully read
    if(dir->offset>=dir->inode->size){
        return -1;
    }

    //get the location in data block corresponding to the inode_num of dir
    int data_block_index = dir->offset / BLOCK_SIZE;
    int data_block_num = dir->inode->block_ptr[data_block_index];
    unsigned char block[BLOCK_SIZE]={0};
    bread(data_block_num+INODE_DATA_BLOCK_0, block);
    int offset_in_block = dir->offset %BLOCK_SIZE;
    ent->inode_num=read_u16(block+offset_in_block);
    strcpy((char *)block+offset_in_block+INODE_NUM_SIZE, ent->name);
    dir->offset+=DIRECTORY_SIZE;
    return 0;
}

void directory_close(struct directory *d){
    iput(d->inode);
    free(d);
}

void ls(void){
    struct directory *dir;
    struct directory_entry ent;

    dir=directory_open(0);

    while (directory_get(dir, &ent) != -1)
        printf("%d %s\n", ent.inode_num, ent.name);

    directory_close(dir);
}