#ifndef VIRTUALMEMORY_H
#define VIRTUALMEMORY_H

#include <string.h>
#include <sys/types.h>

#define PAGE_INDEX_MASK (0x7CE0)
#define PAGE_OFFSET_MASK (0x1F)

#define PAGE_SIZE_BITS 5
#define BYTE_SIZE 4

#define SSD_IO_LIMIT 50

#define NUM_VIRTUAL_PAGES 1000
#define NUM_PAGE_RAM 25
#define NUM_PAGE_SSD 100
#define NUM_PAGE_HDD 1000

#define RAM 1
#define SSD 2
#define HDD 3

#define DELAY_RAM 0.01
#define DELAY_SSD 0.1
#define DELAY_HDD 2.5
#define ONE_SECOND 100000

#define TRUE 1
#define FALSE 0
typedef signed short vAddr;

// Page hand pointer
struct page_ref *page_hand_pointer = 0;

// RAM hand pointer
struct page_ref *ram_hand_pointer = 0;

// int num_ram_pages;
// BYTE ram_pages[NUM_PAGE_RAM]; // array of indices of the free slots
u_int32_t ram[NUM_PAGE_RAM];

// int num_ssd_free_slot;
// BYTE ssd_free_slot[NUM_PAGE_SSD]; // array of indices of the free slots
u_int32_t ssd[NUM_PAGE_SSD];

// int num_hdd_free_slot;
// BYTE hdd_free_slot[NUM_PAGE_HDD]; // array of indices of the free slots
u_int32_t hdd[NUM_PAGE_HDD];

// Struct for page reference
struct page_ref{ 
  int page_index;
  int io_scheduled;
  long ref_time;
  struct page_ref *next;
  struct page_ref *last;
};

// struct for Page Table Entry
typedef struct {
  int present;
  int page_page_num;
  int modified;
  int referenced;
} page_table_entry;

// Available virtual pages
struct page_ref *addr_empty_head = NULL;
// Available ram pages
struct page_ref *ram_empty_head = NULL;
// Available ssd pages
struct page_ref *ssd_empty_head = NULL;

// Allocated virtual pages
struct page_ref *addr_alloc_head = NULL;
// Allocated ram pages
struct page_ref *ram_alloc_head = NULL;
// Allocated ssd pages
struct page_ref *ssd_alloc_head = NULL;

// Virtual address reference
struct page_ref addr_ref_table[NUM_VIRTUAL_PAGES];
// RAM reference
struct page_ref ram_ref_table[NUM_PAGE_RAM];
// SSD reference
struct page_ref ssd_ref_table[NUM_PAGE_SSD];

page_table_entry ram_page_table[NUM_VIRTUAL_PAGES];
page_table_entry ssd_page_table[NUM_VIRTUAL_PAGES];

// function prototypes
vAddr create_page();
u_int32_t *get_value(vAddr address);
void store_value(vAddr address, u_int32_t value);
void free_page(vAddr address);
void init_memory();
u_int32_t retrive_value_from_ssd(int page_page_index);
u_int32_t retrive_value_from_hdd(int page_page_index);
int get_ram_page_num();
int get_ssd_page_num();
void test_memory();
int fifo_page_replacement();
int clock_page_replacement(
  page_table_entry *page_table, 
  struct page_ref* ref_table, 
  struct page_ref** hand_pointer, 
  int size, 
  int target_device);
int aging_page_replacement();
int get_ava_virtual_page();
void write_back_to_ssd(page_table_entry *src_page);
void write_back_to_hdd(page_table_entry *src_page);
void init_avai_pages(struct page_ref *ref_table, int size);
int allocate_virtual_page();
int evict_page();
int allocate_ram_page();
#endif
