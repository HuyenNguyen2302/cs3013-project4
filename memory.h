#ifndef MEMORY_H
#define MEMORY_H

#include <sys/types.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>  // DELETE

#define PAGE_RAM 2  // DELETE
#define PAGE_SSD 2
#define PAGE_HDD 2
#define PAGE_TABLE 5

#define TIME_RAM 0.01
#define TIME_SSD 0.1
#define TIME_HDD 2.5

#define TRUE 0
#define FALSE 1

#define RAM 1
#define SSD 2
#define HDD 3
#define TABLE 4

#define NA -1 

#define RANDOM 1
#define FIFO 2
#define HISTORY 3 

#define MAX_TIME 4294967295

typedef unsigned char BYTE;
typedef signed short vAddr;

typedef struct {
	int value;
	unsigned long referenced_time; // when was the last time it was referenced to?
	int referenced_num; // number of times it was referenced to 
} Page_frame;

// struct for Page Table Entry
typedef struct {
	BYTE memory_location; // -1: NA (Not Applicable) 1: RAM, 2: SSD, 3: HDD
	int page_frame_num; 
	unsigned long referenced_bit; // last time the PTE was referenced
	BYTE present_bit;
} PTE; 

PTE page_table[PAGE_TABLE];

int first_free_entry = 0; 

int RAM_num_free_slot;
BYTE RAM_free_slot[PAGE_RAM]; // array of indices of the free slots
Page_frame RAM_arr[PAGE_RAM];

int SSD_num_free_slot;
BYTE SSD_free_slot[PAGE_SSD]; // array of indices of the free slots
Page_frame SSD_arr[PAGE_SSD];

int HDD_num_free_slot;
BYTE HDD_free_slot[PAGE_HDD]; // array of indices of the free slots
Page_frame HDD_arr[PAGE_HDD];


// 
int option = RANDOM;
unsigned long start_time;

// APIs
vAddr create_page();
u_int32_t get_value(vAddr address, int *valid);
void store_value(vAddr address, u_int32_t new_value);
void free_page(vAddr address);

// Other functions
void init_memory();
void update_PTE(BYTE old_memory_location, BYTE new_memory_location, 
								int old_page_frame_num, int new_page_frame_num);
int physical_addr_in_SSD(int index);
int physical_addr_in_HDD(int index);
int choose_page_frame(int memory_location);
int choose_page_frame_random(int memory_location);
int choose_page_frame_fifo(BYTE memory_location);
int find_fi_page(BYTE memory_location);
int choose_page_frame_history(BYTE memory_location);
int find_max_referenced_page(BYTE memory_location);
int find_free_slot(BYTE memory_location);
void move_page_frame(BYTE memory_location_from, int from_index, int value, 
										 BYTE memory_location_to, int to_index);
int random_page(int max_value);										 								
unsigned long get_current_time(); 
void print_all();

#endif