#ifndef MEMORY_H
#define MEMORY_H

#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>  

#define PAGE_RAM 2  // DELETE
#define PAGE_SSD 2
#define PAGE_HDD 2
#define PAGE_TABLE 5

#define TIME_RAM 0.01 * 1000000
#define TIME_SSD 0.1 * 1000000
#define TIME_HDD 2.5 * 1000000

#define TRUE 0
#define FALSE 1

#define RAM 1
#define SSD 2
#define HDD 3
#define TABLE 4

#define NA -1 

#define RANDOM 1
#define FIRST 2
#define HISTORY 3 

#define MAX_TIME 4294967295u

typedef signed short vAddr;

typedef struct {
	int value;
	unsigned long referenced_time; // when was the last time it was referenced to?
	int PTE_index;
} Page_frame;

// struct for Page Table Entry
typedef struct {
	int memory_location; // -1: NA (Not Applicable) 1: RAM, 2: SSD, 3: HDD
	int page_frame_num; 
	int present_bit;
} PTE; 

PTE page_table[PAGE_TABLE];

int first_free_entry = 0; 

int RAM_num_free_slot;
int RAM_free_slot[PAGE_RAM]; // array of indices of the free slots
Page_frame RAM_arr[PAGE_RAM];

int SSD_num_free_slot;
int SSD_free_slot[PAGE_SSD]; // array of indices of the free slots
Page_frame SSD_arr[PAGE_SSD];

int HDD_num_free_slot;
int HDD_free_slot[PAGE_HDD]; // array of indices of the free slots
Page_frame HDD_arr[PAGE_HDD];


int option;
unsigned long start_time;

// APIs
vAddr create_page();
u_int32_t get_value(vAddr address, int *valid);
void store_value(vAddr address, u_int32_t new_value);
void free_page(vAddr address);

// Helper functions
void init_memory();
int choose_page_frame(int memory_location, int avoid_index);
int choose_page_frame_random(int memory_location, int avoid_index);
int choose_page_frame_first(int memory_location, int avoid_index);
int choose_page_frame_history(int memory_location, int avoid_index);
int find_free_slot(int memory_location);
int random_page(int max_value);										 								
unsigned long get_current_time(); 
void print_all();
int evict_RAM_to_SSD();
int evict_SSD_to_HDD(int not_evict);
int evict_SSD_to_RAM(int must_evict);
int evict_HDD_to_SSD(int must_evict);

// Testing functions
int test_create_page(int num_page);
int test_store_value_and_get_value(vAddr address, int new_value);
int test_free_page_and_get_value(vAddr address);

#endif