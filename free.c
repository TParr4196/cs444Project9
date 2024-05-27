#include "image.h"
#include "block.h"
#include "free.h"

#define BITS_PER_BYTE 8

void set_free(unsigned char *block, int num, int set){

    //retrieve the byte that corresponds to num, and the bit index in that byte
    int byte=block[num/BITS_PER_BYTE];
    int bit_index=num % BITS_PER_BYTE;

    //the bitwise operations here are bitwise shifted left by bit_index
    //first AND the existing byte with a byte that is all 1's except bit_index
    //This allows us to make sure the rest of the byte is unchanged 
    //Then OR that result with the value in set to set bit_index to set
    block[num/BITS_PER_BYTE]=(byte &~(1 << bit_index)) | (set << bit_index);
    //source is bitmanip.c from discord: my solution was much clunkier
}

//my solution:
//void set_bit(int index, int value)
//{
//    int byte=bitmap[index/BITS_PER_BYTE];
//    int bit_index=1<< index % BITS_PER_BYTE;
//    if (value == 1){
//        byte=byte | bit_index;
//    }
//    else{
//        byte=byte & (~bit_index);
//    }
//    bitmap[index/BITS_PER_BYTE]=byte;
//}

int find_free(unsigned char *block){
    unsigned char byte;

    //loops through every byte in the block
    for (int i = 0; i < BLOCK_SIZE; i++) {
        byte = block[i];
        //skip the byte if it is 11111111
        if (byte == 0xff)
            continue;

        //loops through every bit in the byte
        for (int j = 0; j < BITS_PER_BYTE; j++) {

            //returns true if byte left shifted j is 0 aka free
            if (!(byte & (1 << j)))
                return i * BITS_PER_BYTE + j;
        }
    }

    //will only return if every byte is full
    return -1;
    //source for continue block and byte declaration is ffz.c from discord
}