#ifndef VIRTUALMEMORY_H
#define VIRTUALMEMORY_H

#include <string.h>
#include <sys/types.h>

#define PAGE_INDEX_MASK (0x7CE0)
#define PAGE_OFFSET_MASK (0x1F)

#define PAGE_SIZE_BITS 5
#define BYTE_SIZE 4

#define NUM_VIRTUAL_PAGES 1000
#define NUM_PAGE_RAM 25
#define NUM_PAGE_SSD 100
#define NUM_PAGE_HDD 1000

#define DELAY_RAM 0.01
#define DELAY_SSD 0.1
#define DELAY_HDD 2.5
#define ONE_SECOND 100000

#define TRUE 1
#define FALSE 0
typedef signed short vAddr;

// int num_ram_pages;
// BYTE ram_pages[NUM_PAGE_RAM]; // array of indices of the free slots
u_int32_t ram[NUM_PAGE_RAM];

// int num_ssd_free_slot;
// BYTE ssd_free_slot[NUM_PAGE_SSD]; // array of indices of the free slots
u_int32_t ssd[NUM_PAGE_SSD];

// int num_hdd_free_slot;
// BYTE hdd_free_slot[NUM_PAGE_HDD]; // array of indices of the free slots
u_int32_t hdd[NUM_PAGE_HDD];

// struct for Page Table Entry
typedef struct {
  int present;
  int page_frame_num;
  int modified;
  int referenced;
} page_table_entry;

page_table_entry ssd_page_table[NUM_VIRTUAL_PAGES];
page_table_entry ram_page_table[NUM_VIRTUAL_PAGES];

// function prototypes
vAddr create_page();
u_int32_t *get_value(vAddr address);
void store_value(vAddr address, u_int32_t value);
void free_page(vAddr address);
void init_storage();
u_int32_t retrive_value_from_ssd(int page_frame_index);
u_int32_t retrive_value_from_hdd(int page_frame_index);
int get_empty_ram_frame_num();
int get_empty_ssd_frame_num();
int allocate_memory();
void test_memory();
#endif
