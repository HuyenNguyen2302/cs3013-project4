#ifndef MEMORY_H
#define MEMORY_H

#include <sys/types.h>
#include <string.h>

#define PAGE_RAM 25
#define PAGE_SSD 100
#define PAGE_HARD_DRIVE 1000

#define TIME_RAM 0.01
#define TIME_SSD 0.1
#define TIME_HARD_DRIVE 2.5

#define TRUE 0
#define FALSE 1
typedef signed short vAddr;

typedef struct {
	int page_num;
	int value;
} Page;

typedef struct {
	int RAM_num_free_slot;
	int RAM_free_slot[PAGE_RAM]; // array of indices of the free slots
	Page RAM_arr[PAGE_RAM];
} RAM;

typedef struct {
	int SSD_num_free_slot;
	int SSD_free_slot[PAGE_SSD]; // array of indices of the free slots
	Page SSD_arr[PAGE_SSD];
} SSD;

typedef struct {
	int HARD_DRIVE_num_free_slot;
	int HARD_DRIVE_free_slot[PAGE_HARD_DRIVE]; // array of indices of the free slots
	Page HARD_DRIVE_arr[PAGE_HARD_DRIVE];
} HARD_DRIVE;

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

// global variables
RAM RAM_struct;
SSD SSD_struct;
HARD_DRIVE HARD_DRIVE_struct;

#endif