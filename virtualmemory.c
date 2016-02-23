/*
 * File: memory.c
 * @author Huyen Nguyen
 * Date:
 */
#include "virtualmemory.h"
#include <stdio.h>
#include <unistd.h>

#define DEBUG 0

int main(int argc, const char *argv[]) {
  init_storage();
  test_memory();
}

void test_memory() {
  int *value;
  int *value2;
  int *value3;
  // Allocate memory
  vAddr address = create_page();
  if (address != -1) {

    value = get_value(address);
    printf("%d\n", *value);

    store_value(address, 1239);

    value2 = get_value(address);
    printf("%d\n", *value2);

    free_page(address);

    value3 = get_value(address);
    printf("%d\n", *value3);

  } else {
    puts("Fail to allocate memory");
  }
}

void init_storage() {
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
  }

  // Initialize SSD page table
  for (i = 0; i < NUM_VIRTUAL_PAGES; i++) {
    ssd_page_table[i].present = FALSE;
  }
}

/*
 * reserves a new memory location in the emulated RAM
 * @return vAddr the virtual address of
 * the new memory location, return -1 if
 * no memory is available
 */
vAddr create_page() {
  unsigned short page_frame_index; // Page frame index
  vAddr address;                   // Keeping virtual address

  // Allocate memory for
  page_frame_index = allocate_memory();

  if (page_frame_index == -1) {
    // Fail to allocate memory
    return -1;
  }

  // 0111111111100000
  address = page_frame_index << 5;
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
  int ram_frame_num;
  // Mask off page index from virtual address
  // 0111111111100000
  unsigned short page_frame_index =
      (PAGE_INDEX_MASK & address) >> PAGE_SIZE_BITS;
  // 0000000000011111
  // unsigned short offset = (PAGE_OFFSET_MASK & address) / BYTE_SIZE;

  if (DEBUG) {
    printf("Page index:%d\n", page_frame_index);
    // printf("Page offset:%d\n", offset);
  }

  // Invalid address
  if (page_frame_index > NUM_VIRTUAL_PAGES - 1)
    return NULL;

  // Retrive page from ssd
  if (!ram_page_table[page_frame_index].present) {
    if (DEBUG)
      puts("RAM Page fault");

    // Retrive page
    ram_frame_num = get_empty_ram_frame_num();
    ram[ram_frame_num] = retrive_value_from_ssd(page_frame_index);
    ram_page_table[page_frame_index].page_frame_num = ram_frame_num;
    ram_page_table[page_frame_index].present = TRUE;
    if (DEBUG)
      puts("Back from SSD");
  }

  if (DEBUG)
    printf("Modified: %d\n", ram_page_table[page_frame_index].modified);

  ram_page_table[page_frame_index].referenced = TRUE;
  usleep(DELAY_RAM * ONE_SECOND);
  // return (u_int32_t *)((int)&ram[ram_frame_num] + offset);
  return (u_int32_t *)(&ram[ram_frame_num]);
}

/*
 * Try to get target page from ssd
 */
u_int32_t retrive_value_from_ssd(int page_frame_index) {
  int ssd_frame_num;
  // Retrive page from disk
  if (!ssd_page_table[page_frame_index].present) {
    if (DEBUG)
      puts("SSD Page fault");

    // Retrive page
    ssd_frame_num = get_empty_ssd_frame_num();
    ssd[ssd_frame_num] = retrive_value_from_hdd(page_frame_index);
    ssd_page_table[page_frame_index].page_frame_num = ssd_frame_num;
    ssd_page_table[page_frame_index].present = TRUE;
    if (DEBUG)
      puts("Back from disk");
  }

  ssd_page_table[page_frame_index].referenced = TRUE;
  usleep(DELAY_SSD * ONE_SECOND);
  return ssd[ssd_frame_num];
}

/*
 * Try to get target page from hdd
 */
u_int32_t retrive_value_from_hdd(int page_frame_index) {
  if (DEBUG)
    puts("Disk");
  usleep(DELAY_HDD * ONE_SECOND);
  return hdd[page_frame_index];
}

/*
 * Allocate memory from ram
 */
int allocate_memory() { return 500; }

/*
 * Get empty frame number in memory. Evict a frame when it is full
 */
int get_empty_ram_frame_num() { return 20; }

/*
 * Get empty frame in ssd. Evict a frame when it is full
 */
int get_empty_ssd_frame_num() { return 90; }

/*
 * updates the contents of a page
 * @param address the virtual address
 * @param value the value that should be stored in the page
 */
void store_value(vAddr address, u_int32_t value) {
  int ram_frame_num;
  // Mask off page index from virtual address
  // 0111111111100000
  unsigned short page_frame_index =
      (PAGE_INDEX_MASK & address) >> PAGE_SIZE_BITS;
  // // 0000000000011111
  // unsigned short offset = (PAGE_OFFSET_MASK & address) / BYTE_SIZE;

  // Invalid address
  if (page_frame_index > NUM_VIRTUAL_PAGES - 1)
    return;

  if (!ram_page_table[page_frame_index].present) {
    // Not present in ram, first retrive from hdd
    get_value(address);
  }

  ram_frame_num = ram_page_table[page_frame_index].page_frame_num;

  ram[ram_frame_num] = value;

  if (DEBUG)
    printf("Store: %d, %d\n", value, ram[ram_frame_num]);
  ram_page_table[page_frame_index].referenced = TRUE;
  ram_page_table[page_frame_index].modified = TRUE;
}

/*
 * frees the page, regardless of where it is in the memory hierarchy
 * @param address the virtual address
 */
void free_page(vAddr address) {
  unsigned short page_frame_index =
      (PAGE_INDEX_MASK & address) >> PAGE_SIZE_BITS;

  // Free ram
  ram_page_table[page_frame_index].present = FALSE;
  // Free ssd
  ssd_page_table[page_frame_index].present = FALSE;
}

/*
 * Replace a page with FIFO algorithm
 */
void fifo_page_replacement() {}

/*
 * Replace a page with clock algorithm
 */
void clock_page_replacement() {}

/*
 * Replace a page with aging algorithm
 */
void aging_page_replacement() {}
