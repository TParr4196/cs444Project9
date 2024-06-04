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
#define ROOT_INODE_NUM 0
#define BLOCK_POINTER_SIZE 16
#define OWNER_ID_NUM 0
#define DEFAULT_LINKS 1
#define FILE_DATA_BLOCK_0 7

void mkfs(void){
    //initialize basic metadeta
    unsigned char block[BLOCK_SIZE]={0};
    for(int i=0; i<BLOCK_SIZE; i++){
        block[i]=0x0;
    }
    for(int i=0; i<FILE_DATA_BLOCK_0; i++){
        set_free(block, i, 1);
    }
    bwrite(BLOCK_MAP, block);
    for(int i=0; i<FILE_DATA_BLOCK_0; i++){
        set_free(block, i, 0);
    }

    //not sure what goes in the superblock so giving it empty block
    bwrite(SUPERBLOCK, block); 
    bwrite(INODE_MAP, block);

    //grab inode and block for that inode
    struct inode *in = ialloc();
    int block_index = alloc();
    bread(block_index, block);

    //initialize inode data
    in->size = 2*DIRECTORY_SIZE; //directory contains itself and parent directory, which for root directory is itself again
    in->owner_id = OWNER_ID_NUM; //no users yet?
    in->permissions = RWX; //r 4 w 2 x 1
    in->flags = DIRECTORY; //marks as directory
    in->link_count = DEFAULT_LINKS; //only link to root directory is itself
    in->block_ptr[0] = block_index;

    //create directory entries for . and ..
    write_u16(block, in->inode_num);
    write_u16(block+DIRECTORY_SIZE, in->inode_num);

    strcpy((char*) (block+INODE_NUM_SIZE), ".");
    strcpy((char*) (block+DIRECTORY_SIZE+INODE_NUM_SIZE), "..");

    //write data out to disk
    bwrite(block_index, block);

    //mark inode as free
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
    if(dir->offset >= dir->inode->size){
        return -1;
    }

    //get the location in data block corresponding to the inode_num of dir
    int data_block_index = dir->offset / BLOCK_SIZE;
    int data_block_num = dir->inode->block_ptr[data_block_index];
    unsigned char block[BLOCK_SIZE]={0};
    bread(data_block_num, block);
    int offset_in_block = dir->offset % BLOCK_SIZE;

    //get directory entry out of block
    ent->inode_num = read_u16(block + offset_in_block);
    strcpy(ent->name, (char*)(block + offset_in_block + INODE_NUM_SIZE));
    dir->offset+=DIRECTORY_SIZE;
    return 0;
}

void directory_close(struct directory *d){
    iput(d->inode);
    free(d);
}

//provided by Beej to get directory name from a string
char *get_dirname(const char *path, char *dirname){
    strcpy(dirname, path);

    char *p = strrchr(dirname, '/');

    if (p == NULL) {
        strcpy(dirname, ".");
        return dirname;
    }

    if (p == dirname)  // Last slash is the root /
        *(p+1) = '\0';

    else
        *p = '\0';  // Last slash is not the root /

    return dirname;
}

//provided by Beej to get name of last part of path
char *get_basename(const char *path, char *basename){
    if (strcmp(path, "/") == 0) {
        strcpy(basename, path);
        return basename;
    }

    const char *p = strrchr(path, '/');

    if (p == NULL)
        p = path;   // No slash in name, start at beginning
    else
        p++;        // Start just after slash

    strcpy(basename, p);

    return basename;
}

int directory_make(char *path){
    //later case
    if(path[0]!='/')
        return -1;
    //get parent directory's inode
    char buffer[16];
    get_dirname(path, buffer);
    struct inode *parent_in = namei(buffer);

    //allocate new block and inode
    struct inode *in = ialloc();
    int block_index = alloc();

    //return -1 if any of the above functions fail
    if(parent_in==NULL || in == NULL || block_index == -1)
        return -1;

    //intialize block_ptr by creating directory entries for . and ..
    unsigned char block[BLOCK_SIZE]={0};
    write_u16(block, in->inode_num);
    strcpy((char*)block+INODE_NUM_SIZE, ".");
    write_u16(block+DIRECTORY_SIZE, parent_in->inode_num);
    strcpy((char*)(block+DIRECTORY_SIZE+INODE_NUM_SIZE), "..");

    //initialize inode data
    in->size=2*DIRECTORY_SIZE; //directory contains itself and parent directory
    in->owner_id=OWNER_ID_NUM; //no users yet?
    in->permissions=RWX; //r 4 w 2 x 1
    in->flags=DIRECTORY; //marks as directory
    in->link_count=DEFAULT_LINKS; //only link to directory is parent directory
    in->block_ptr[0]=*block;

    //write new directory block out to disk
    bwrite(block_index, block);

    //get parent directory's block information TODO double check
    int parent_block_index = parent_in->block_ptr[0];
    bread(parent_block_index, block);

    //write new entry into parent directory
    write_u16(block + parent_in->size, in->inode_num);
    get_basename(path, buffer);
    strcpy((char*)(block + parent_in->size + INODE_NUM_SIZE), buffer);
    bwrite(parent_block_index, block);

    //update parent inode
    parent_in->size+=DIRECTORY_SIZE;
    parent_in->link_count+=1;

    //free in and parent_in
    iput(in);
    iput(parent_in);
    
    return 0;
}

void ls(void){
    struct directory *dir;
    struct directory_entry ent;

    dir=directory_open(0);

    while (directory_get(dir, &ent) != -1)
        printf("%d %s\n", ent.inode_num, ent.name);

    directory_close(dir);
}