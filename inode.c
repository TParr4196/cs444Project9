#include "inode.h"
#include "image.h"
#include "block.h"
#include "free.h"
#include "pack.h"
#include "dir.h"
#include <stdio.h>
#include <string.h>

#define BLOCK_SIZE 4096
#define INODE_MAP 1
#define ROOT_INODE_NUM 0

static struct inode incore[MAX_SYS_OPEN_FILES] = {0};

void initialize_inode(struct inode *in){
    //set incore inode to all 0's with inode_num as index found above
    in->size=0, in->flags=0, in->link_count=0, in->owner_id=0, in->permissions=0,
        in->ref_count=0;

    //set block_ptr
    for(int i = 0; i < INODE_PTR_COUNT; i++){
        in->block_ptr[i]=0;
    }
}

struct inode *ialloc(void){
    //get inode map
    unsigned char buffer[4096]={0};
    bread(INODE_MAP, buffer);

    //find and set as used first available free inode
    int index=find_free(buffer);
    if (index<0)
        return NULL;

    //find and set an incore inode
    struct inode *in = iget(index);
    if(in == NULL){
        return NULL;
    }
    
    //write new map back out to disk
    set_free(buffer, index, 1);
    bwrite(1, buffer);

    initialize_inode(in);
    in->inode_num=index;

    return in;
}

//find an incore struct inode that is not in use
struct inode *incore_find_free(void){
    for(int i=0; i<MAX_SYS_OPEN_FILES; i++){
        if(incore[i].ref_count==0){
            return incore+i;
        }
    }
    return NULL;
}


//find an incore struct inode by inode_num
struct inode *incore_find(unsigned int inode_num){
    for(int i=0; i<MAX_SYS_OPEN_FILES; i++){
        if(incore[i].ref_count == 0 && incore[i].inode_num==inode_num){
            return incore+i;
        }
    }
    return NULL;
}

//for testing
void incore_free_all(void){
    for(int i=0; i<MAX_SYS_OPEN_FILES; i++){
        incore[i].ref_count=0;
    }
}

//for testing
void incore_all_used(void){
    for(int i=0; i<MAX_SYS_OPEN_FILES; i++){
        incore[i].ref_count=1;
    }
}

void read_inode(struct inode *in, int inode_num){
    //initialize location of inode in block
    int block_num = inode_num / INODES_PER_BLOCK + INODE_FIRST_BLOCK;
    int block_offset = inode_num % INODES_PER_BLOCK;
    int block_offset_bytes = block_offset * INODE_SIZE;

    //pull the inode block with inode_num
    unsigned char block[BLOCK_SIZE]={0};
    bread(block_num, block);

    //pull inode information from inode block and store in struct inode
    in->size = read_u32(block + block_offset_bytes + FILE_SIZE);
    in->owner_id = read_u16(block + block_offset_bytes + OWNER_ID);
    in->permissions = read_u8(block + block_offset_bytes + PERMISSIONS);
    in->flags = read_u8(block + block_offset_bytes + FLAGS);
    in->link_count = read_u8(block + block_offset_bytes + LINK_COUNT);
    for(int i=0; i<INODE_PTR_COUNT; i++){
        in->block_ptr[i] = read_u16(block + block_offset_bytes + BLOCK_POINTER_0 + i * BLOCK_POINTER_NUMBER);
    }
    in->inode_num=inode_num;
}

void write_inode(struct inode *in){
    //initialize location of inode in block
    int block_num = in->inode_num / INODES_PER_BLOCK + INODE_FIRST_BLOCK;
    int block_offset = in->inode_num % INODES_PER_BLOCK;
    int block_offset_bytes = block_offset * INODE_SIZE;
    
    //pull the inode block with inode_num
    unsigned char block[BLOCK_SIZE]={0};
    bread(block_num, block);

    //pull inode information from struct inode and store in block inode
    write_u32(block + block_offset_bytes + FILE_SIZE, in->size);
    write_u16(block + block_offset_bytes + OWNER_ID, in->owner_id);
    write_u8(block + block_offset_bytes + PERMISSIONS, in->permissions);
    write_u8(block + block_offset_bytes + FLAGS, in->flags);
    write_u8(block + block_offset_bytes + LINK_COUNT, in->link_count);
    for(int i=0; i<INODE_PTR_COUNT; i++){
        write_u16(block + block_offset_bytes + BLOCK_POINTER_0 + i * BLOCK_POINTER_NUMBER, in->block_ptr[i]);
    }
    bwrite(block_num, block);
}

struct inode *iget(int inode_num){
    //return a previously used incore struct inode if it exists
    struct inode *in = incore_find(inode_num);
    if(in!=NULL){
        in->ref_count++;
        return in;
    }
    //else reassign an unused incore struct inode if one exists
    in=incore_find_free();
    if(in!=NULL){
        read_inode(in, inode_num);
        in->ref_count++;
        in->inode_num=inode_num;
        return in;
    }
    return NULL;
}

void iput(struct inode *in){
    //mark a struct inode as not in use
    if(in->ref_count==0){
        return;
    }
    in->ref_count--;
    if(in->ref_count==0){
        write_inode(in);
    }
}

struct inode *namei(char *path){
    char result[INODE_PTR_COUNT];
    get_dirname(path, result);
    if(result[0]=='/'){
        struct inode *in = iget(ROOT_INODE_NUM);
        get_basename(path, result);
        if(strcmp("/", result)==0){
            return in;
        }
        //TODO
    }
    return NULL;
}