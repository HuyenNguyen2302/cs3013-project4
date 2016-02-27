#ifndef VIRTUALMEMORY_H
#define VIRTUALMEMORY_H

#include <string.h>
#include <sys/types.h>

#define PAGE_INDEX_MASK (0x7FE0)
#define PAGE_OFFSET_MASK (0x1F)

#define PAGE_SIZE_BITS 5
#define BYTE_SIZE 4

#define IO_LIMIT 10

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
#define ONE_SECOND 100

#define TRUE 1
#define FALSE 0
typedef signed short vAddr;

// RAM hand pointer
struct page_ref *ram_hand_pointer = NULL;

// SSD hand pointer
struct page_ref *ssd_hand_pointer = NULL;

// int num_ram_pages;
// BYTE ram_pages[NUM_PAGE_RAM]; // array of indices of the free slots
u_int32_t ram[NUM_PAGE_RAM];

// int num_ssd_free_slot;
// BYTE ssd_free_slot[NUM_PAGE_SSD]; // array of indices of the free slots
u_int32_t ssd[NUM_PAGE_SSD];

// int num_hdd_free_slot;
// BYTE hdd_free_slot[NUM_PAGE_HDD]; // array of indices of the free slots
u_int32_t hdd[NUM_PAGE_HDD];

// Number of I/O scheduled
int num_ssd_io = 0;
int num_hdd_io = 0; 

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
  int page_frame_num;
  int modified;
  int referenced;
} page_table_entry;

// Available virtual pages
struct page_ref *addr_empty_head = NULL;
// Available ram pages
struct page_ref *ram_empty_head = NULL;
// Available ssd pages
struct page_ref *ssd_empty_head = NULL;

// // Allocated virtual pages
// struct page_ref *addr_alloc_head = NULL;
// // Allocated ram pages
// struct page_ref *ram_alloc_head = NULL;
// // Allocated ssd pages
// struct page_ref *ssd_alloc_head = NULL;

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
u_int32_t retrive_value_from_ssd(int page_index);
u_int32_t retrive_value_from_hdd(int page_index);
void test_memory();
int fifo_page_replacement();
int clock_page_replacement(page_table_entry *page_table,
  struct page_ref *ref_table,
  struct page_ref *hand_pointer, 
  int size,
  int (*get_io_counter)(),
  void (*increment_io_counter)(),
  struct page_ref *(*advance_pointer)(),
  void *(*write_back)(int));

int aging_page_replacement();
int get_ava_virtual_page();
void *write_back_to_ssd(int src_page_index);
void *write_back_to_hdd(int src_page_index);
void init_avai_pages(struct page_ref *ref_table, int size);
int allocate_virtual_page();
int allocate_page(
  struct page_ref *empty_head, 
  page_table_entry *page_table, 
  struct page_ref *hand_pointer,
  void (*empty_list)(), 
  int (*evict_page)(), 
  struct page_ref* (set_hand_pointer(struct page_ref *)),
  void (set_empty_head(struct page_ref *))
);

struct page_ref *advance_ram_pointer();
void empty_ram_list();
int evict_ram_page();
struct page_ref* set_ram_hand_pointer(struct page_ref * curr);
struct page_ref *advance_ssd_pointer();
void empty_ssd_list();
int evict_ssd_page();
struct page_ref* set_ssd_hand_pointer(struct page_ref * curr);
void allocate_ram_page();
void allocate_ssd_page();
int reference_cleaner();
int get_ssd_io_counter();
int get_hdd_io_counter();
void increment_ssd_io_counter();
void increment_hdd_io_counter();
void set_ram_empty_head(struct page_ref *head);
void set_ssd_empty_head(struct page_ref *head);

void free_ram_page(
  struct page_ref *to_free, 
  struct page_ref *empty_head, 
  struct page_ref *hand_pointer, 
  struct page_ref* (init_empty_list(struct page_ref *)),
  void (*free_empty_hand)(), 
  struct page_ref* (free_set_empty_head(struct page_ref *)),
  void (free_set_hand_pointer(struct page_ref *)));

void add_page_to_free_list(
  struct page_ref *to_free, 
  struct page_ref *empty_head,
  void (*free_empty_hand)(), 
  struct page_ref* (free_set_empty_head(struct page_ref *)),
  void (free_set_hand_pointer(struct page_ref *))
  );
void free_ram_empty_hand();
struct page_ref* free_set_ram_empty_head(struct page_ref * curr);
void free_set_ram_hand_pointer(struct page_ref * curr);
void init_ssd_empty_list();
void free_ssd_empty_hand();
struct page_ref* free_set_ssd_empty_head(struct page_ref * curr);
void free_set_ssd_hand_pointer(struct page_ref * curr);
struct page_ref* free_set_addr_empty_head(struct page_ref * curr);
#endif
