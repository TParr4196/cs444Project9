#include "image.h"
#include <fcntl.h>
#include <unistd.h>

extern int image_fd;
int image_fd=-1;
//declare and define image_fd to -1 so that if it has not been opened yet or draws an error it won't do anything when used

int image_open(char *filename, int truncate){
    //create the "disk" as specified by truncate
    if(truncate){
        image_fd=open(filename, O_RDWR|O_CREAT|O_TRUNC, 0600);
    }
    else{
        image_fd=open(filename, O_RDWR|O_CREAT, 0600);
    }
    return image_fd;
}

int image_close(void){
    //close the "disk"
    return close(image_fd);
}