# Test File System

## Building
`make` to build. An executable called `testfs` will be produced.
`make clean` to remove .o build products
`make pristine` to clean up all build products

## Files
`testfs.c`: Project 3 of cs444 designed to explore use of semaphores through producer/consumer threads.
`block.c`: defines functions to read and write to and from buffers and the disk
`image.c`: defines functionalilty to open, close, and store a global file descriptor representing a disk
`inode.c`: defines functionality for inode creation and management
`pack.h`: read and write 1, 2, or 4 bytes from a pointer
`free.c`: contains functions to set or find individual free bits in a block
`dir.c`: sets up file system, root directory, and other directories
`block.h`: header file for block.c
`image.h`: header file for image.c
`ctest.h`: header file for Beej's Test Code
`inode.h`: header file for inode.c
`free.h`: header file for free.c
`pack.h`: header file for pack.c
`dir.h`: header file for pack.c

## Data
makes use of reading and writing to a text file in order to help simulate management of a disk on a computer. 
It is partitioned into blocks of size 4096 bytes. These bytes are represented by the unsigned char type.

## Functions
`main`
    `image_open`: opens a global file descriptor representing the disk
    `image_close`: closes the global file descriptor
    `bread`: writes an unsigned char array to the designated block
    `bwrite`: writes from the "disk" to the provided buffer
    `ialloc`: allocates and returns the incore inode of the first free inode in the inode map
        `find_free`: finds the first 0 bit in an unsigned char array
        `set_free`: sets the designated index of an unsigned char array to the designated value
    `alloc`: allocates and returns the index of the first free block in the block map
    `incore_find_free`: find the first free incore inode
    `incore_find`: find a free incore inode with a given inode_num
    `read_inode`: read inode from disk to struct inode
    `write_inode`: write struct inode to disk
    `iget`: incore_find a free incore inode or create one if needed and there is room
    `iput`: mark an incore inode as free
    `mkfs`: initialize block map, inode map, superblock and open root directory
    `directory_open`: open the directory located at the given inode num
    `directory_get`: get the next entry in the directory
    `directory_close`: mark inode as free and free memory for directory
    `directory_make`: make a new directory

## Notes
