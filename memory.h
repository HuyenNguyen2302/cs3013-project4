#ifndef MEMORY_H
#define MEMORY_H

#include <sys/types.h>
#include <string.h>

#define PAGE_RAM 25
#define PAGE_SSD 100
#define PAGE_HDD 1000

#define TIME_RAM 0.01
#define TIME_SSD 0.1
#define TIME_HDD 2.5

#define TRUE 0
#define FALSE 1

typedef unsigned char BYTE;
typedef signed short vAddr;

typedef struct {
	int page_num;
	int value;
} Page;


int RAM_num_free_slot;
BYTE RAM_free_slot[PAGE_RAM]; // array of indices of the free slots
Page RAM_arr[PAGE_RAM];

int SSD_num_free_slot;
BYTE SSD_free_slot[PAGE_SSD]; // array of indices of the free slots
Page SSD_arr[PAGE_SSD];



int HDD_num_free_slot;
BYTE HDD_free_slot[PAGE_HDD]; // array of indices of the free slots
Page HDD_arr[PAGE_HDD];


// struct for Page Table Entry
typedef struct {
	void *memory_location;
	int page_frame_num; 
	int offset;
	int modified_bit;
	int referenced_bit;
} PTE; 



// function prototypes
vAddr create_page();
u_int32_t *get_value(vAddr address);
void store_value(vAddr address, u_int32_t value);
void free_page(vAddr address);
void init_structs();

#endif