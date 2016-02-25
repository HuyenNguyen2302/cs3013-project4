/*
 * File: memory.c
 * @author Yang Liu & Huyen Nguyen
 * Date:
 */
#include "virtualmemory.h"
#include <stdio.h>
#include <unistd.h>
 #include <stdlib.h>

#define DEBUG FALSE

int main(int argc, const char *argv[]) {
  init_memory();
  test_memory();
}

void test_memory() {
  int *value;
  int *value2;
  int *value3;
  // Allocate memory
  vAddr address = create_page();
  if (address != -1) {
    value = (int *)get_value(address);
    printf("%d\n", *value);

    store_value(address, 1239);

    value2 = (int *)get_value(address);
    printf("%d\n", *value2);

    free_page(address);

    value3 = (int *)get_value(address);
    printf("%d\n", *value3);

  } else {
    puts("Fail to allocate memory");
  }
}

void init_memory() {
  int i;

  // initialize RAM
  memset(ram, 0, sizeof(ram));

  // initialize SSD
  memset(ssd, 0, sizeof(ssd));

  // initialize HDD
  memset(hdd, 0, sizeof(hdd));

  // Initialize page table
  for (i = 0; i < NUM_VIRTUAL_PAGES; i++) {
    ram_page_table[i].present = FALSE;
    ram_ref_table[i].io_scheduled = FALSE;
  }

  // Initialize SSD page table
  for (i = 0; i < NUM_VIRTUAL_PAGES; i++) {
    ssd_page_table[i].present = FALSE;
    ssd_ref_table[i].io_scheduled = FALSE;
  }

  // Initialize empty virtual address page reference
  // Circular linked list
  addr_empty_head = &addr_ref_table[0];
  init_avai_pages(addr_ref_table, NUM_VIRTUAL_PAGES);

  // Initialize empty RAM page reference
  // Circular linked list
  ram_empty_head = &ram_ref_table[0];
  init_avai_pages(ram_ref_table, NUM_PAGE_RAM);

  // Initialize empty SSD page reference
  // Circular linked list
  ssd_empty_head = &ssd_ref_table[0];
  init_avai_pages(ssd_ref_table, NUM_PAGE_HDD);
}

/*
 * Initialize available lists
 */
void init_avai_pages(struct page_ref *ref_table, int size) {
  int i; // Loop counter

  // Initialize list head
  ref_table[0].page_index = 0;

  // Create double linked list for empty pages
  for (i = 1; i < size; i++) {
    // Set up reference table
    ref_table[i].page_index = i;
    // List item double references
    ref_table[i].last = &ref_table[i-1];
    ref_table[i-1].next = &ref_table[i];
  }

  // Reference head
  ref_table[size - 1].next = &ref_table[0];
  ref_table[0].last = &ref_table[size - 1];
}

/*
 * reserves a new memory location in the emulated RAM
 * @return vAddr the virtual address of
 * the new memory location, return -1 if
 * no memory is available
 */
vAddr create_page() {
  int ram_page_index = -1;
  int virtual_page_index;  // Page page index
  vAddr address;           // Keeping virtual address

  // Find available virtual page
  virtual_page_index = allocate_virtual_page();

  if (virtual_page_index == -1) {
    // Fail to allocate memory
    return -1;
  }

  ram_page_index = allocate_ram_page(); 

  ram_page_table[ram_page_index].present = TRUE;

  // 0111111111100000
  address = virtual_page_index << 5;
  return address;
}

/*
 * obtains the physical memory address
 * indicated by the given virtual address
 * @param address the virtual address
 * @return u_int32_t * an integer pointer to the location in emulated
 * RAM,
 * returns NULL if the pointer cannot be provided (e.g., a page with the
 * given address does not exist).
 */
u_int32_t *get_value(vAddr address) {
  int ram_page_num = 0;
  // Mask off page index from virtual address
  // 0111111111100000
  unsigned short virtual_page_index =
      (PAGE_INDEX_MASK & address) >> PAGE_SIZE_BITS;
  // 0000000000011111
  // unsigned short offset = (PAGE_OFFSET_MASK & address) / BYTE_SIZE;

  if (DEBUG) {
    printf("Page index:%d\n", virtual_page_index);
    // printf("Page offset:%d\n", offset);
  }

  // Invalid address
  if (virtual_page_index > NUM_VIRTUAL_PAGES - 1) return NULL;

  // Retrive page from ssd
  if (!ram_page_table[virtual_page_index].present) {
    if (DEBUG) puts("RAM Page fault");

    // Retrive page
    ram_page_num = get_ram_page_num();
    ram[ram_page_num] = retrive_value_from_ssd(virtual_page_index);
    ram_page_table[virtual_page_index].page_page_num = ram_page_num;
    ram_page_table[virtual_page_index].present = TRUE;
    if (DEBUG) puts("Back from SSD");
  }

  if (DEBUG)
    printf("Modified: %d\n", ram_page_table[virtual_page_index].modified);

  ram_page_table[virtual_page_index].referenced = TRUE;
  usleep(DELAY_RAM * ONE_SECOND);
  return (u_int32_t *)(&ram[ram_page_table[virtual_page_index].page_page_num]);
}

/*
 * Try to get target page from ssd
 */
u_int32_t retrive_value_from_ssd(int virtual_page_index) {
  int ssd_page_num = 0;

  // Retrive page from disk
  if (!ssd_page_table[virtual_page_index].present) {
    if (DEBUG) puts("SSD Page fault");

    // Retrive page
    ssd_page_num = get_ssd_page_num();
    ssd[ssd_page_num] = retrive_value_from_hdd(virtual_page_index);
    ssd_page_table[virtual_page_index].page_page_num = ssd_page_num;
    ssd_page_table[virtual_page_index].present = TRUE;
    if (DEBUG) puts("Back from disk");
  }

  ssd_page_num = ssd_page_table[virtual_page_index].page_page_num;
  ssd_page_table[virtual_page_index].referenced = TRUE;
  usleep(DELAY_SSD * ONE_SECOND);
  return ssd[ssd_page_table[virtual_page_index].page_page_num];
}

/*
 * Try to get target page from hdd
 */
u_int32_t retrive_value_from_hdd(int virtual_page_index) {
  if (DEBUG) puts("Disk");
  usleep(DELAY_HDD * ONE_SECOND);
  return hdd[virtual_page_index];
}

/*
 * Allocate virtual page
 */
int allocate_virtual_page() {
  struct page_ref *head;
  struct page_ref *tail;
  int virtual_page_index;

  // No virtual address available
  if (addr_empty_head == NULL) {
    return -1;
  }

  // Get virtual page index
  virtual_page_index = addr_empty_head->page_index;

  // Remove virtual page from available list
  if (addr_empty_head->next == addr_empty_head && addr_empty_head->last == addr_empty_head)
    addr_empty_head = NULL;
  else {
    // Reconnect the ring
    head = addr_empty_head->next;
    tail = addr_empty_head->last;
    head->last = tail;
    tail->next = head;
  }

  // Return virtual page index
  return virtual_page_index;
}

/*
 * Get empty page number in memory. Evict a page when it is full
 */
int allocate_ram_page() {
  struct page_ref *head;
  struct page_ref *tail;
  int ram_page_index;

  // No empty ram page available
  if (ram_empty_head == NULL) {
    return evict_page(RAM);
  }

  ram_page_index = ram_empty_head->page_index;

  // Remove virtual page from available list
  if (ram_empty_head->next == ram_empty_head && ram_empty_head->last == ram_empty_head)
    ram_empty_head = NULL;
  else {
    // Reconnect the ring
    head = ram_empty_head->next;
    tail = ram_empty_head->last;
    head->last = tail;
    tail->next = head;
  }

  // return ram page index
  return ram_page_index;
}

/*
 * Get empty page in ssd. Evict a page when it is full
 */
int get_ssd_page_num() { return 90; }

/*
 * updates the contents of a page
 * @param address the virtual address
 * @param value the value that should be stored in the page
 */
void store_value(vAddr address, u_int32_t value) {
  int ram_page_num;
  // Mask off page index from virtual address
  // 0111111111100000
  unsigned short virtual_page_index =
      (PAGE_INDEX_MASK & address) >> PAGE_SIZE_BITS;
  // // 0000000000011111
  // unsigned short offset = (PAGE_OFFSET_MASK & address) / BYTE_SIZE;

  // Invalid address
  if (virtual_page_index > NUM_VIRTUAL_PAGES - 1) return;

  if (!ram_page_table[virtual_page_index].present) {
    // Not present in ram, first retrive from hdd
    get_value(address);
  }

  ram_page_num = ram_page_table[virtual_page_index].page_page_num;

  ram[ram_page_num] = value;

  if (DEBUG) printf("Store: %d, %d\n", value, ram[ram_page_num]);
  ram_page_table[virtual_page_index].referenced = TRUE;
  ram_page_table[virtual_page_index].modified = TRUE;
}

/*
 * frees the page, regardless of where it is in the memory hierarchy
 * @param address the virtual address
 */
void free_page(vAddr address) {
  // int ram_page_num;
  unsigned short virtual_page_index =
      (PAGE_INDEX_MASK & address) >> PAGE_SIZE_BITS;

  // // Mark ram page as free
  // ram_page_num = ram_page_table[virtual_page_index].page_page_num;
  // ram_page_ref[ram_page_num] = NULL;

  // Free ram
  ram_page_table[virtual_page_index].present = FALSE;
  // Free ssd
  ssd_page_table[virtual_page_index].present = FALSE;
}

/*
 * Write page back to ssd
 */
void write_back_to_ssd(page_table_entry *src_page) {

}

/*
 * Write page back to hdd
 */
void write_back_to_hdd(page_table_entry *src_page) {

}

int evict_page(int from){

  page_table_entry *page_table; 
  struct page_ref* ref_table;
  struct page_ref** hand_pointer; 
  int size;
  int target_device;

  switch (from) {
    case RAM:
      page_table = ssd_page_table;
      ref_table = ssd_ref_table;
      hand_pointer = &ram_alloc_head;
      size = NUM_PAGE_RAM;
      target_device = SSD;
      break;
    case SSD:
      page_table = ssd_page_table;
      ref_table = ssd_ref_table;
      hand_pointer = &ssd_alloc_head;
      size = NUM_PAGE_SSD;
      target_device = HDD;
      break;
  }

  return clock_page_replacement(page_table, ref_table, hand_pointer, size, target_device);
}

/*
 * Replace a page with FIFO algorithm
 */
int fifo_page_replacement() { return 0; }

/*
 * Replace a page with clock algorithm
 */
int clock_page_replacement(
  page_table_entry *page_table, 
  struct page_ref* ref_table, 
  struct page_ref** hand_pointer, 
  int size, 
  int target_device) {

  int num_ssd_io = 0; // Number of I/O scheduled
  int index;
  struct page_ref* head;
  head = *hand_pointer;

  // loop through page index
  while(1) {
      index = (*hand_pointer)->page_index;
      if(!(*hand_pointer)->io_scheduled 
        && !page_table[index].referenced){
        // Memory not referenced
        if (!page_table[index].modified)
          // Find a clean page 
          return index;

        // Find a dirty page, schedule write back to target_device
        if(num_ssd_io < SSD_IO_LIMIT)
        {
          ref_table[index].io_scheduled = TRUE;
          num_ssd_io++;

          switch (target_device) {
            case SSD:
              write_back_to_ssd(&page_table[index]);
              break;
            case HDD:
              write_back_to_hdd(&page_table[index]);
              break;
          }
        }
    }

    // 
    hand_pointer = &((*hand_pointer)->next);

    // All pages are clean, evict this page
    if((*hand_pointer) == head && num_ssd_io == 0)
      return (*hand_pointer)->page_index;
  }
}

/*
 * Replace a page with aging algorithm
 */
int aging_page_replacement() { return 0; }
