/*
 * File: memory.c
 * @author Yang Liu & Huyen Nguyen
 * Date:
 */
#include "virtualmemory.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

#define DEBUG FALSE
pthread_mutex_t mutex;

void test_memory() {
  int i;
  // int *value;
  int tmp;
  int *value;
  int *value1;
  struct page_ref* head;
  struct page_ref* loop;

  // Allocate memory
  vAddr address1[NUM_VIRTUAL_PAGES];
  vAddr address2[NUM_VIRTUAL_PAGES];
  for (i = 0; i < 300; i++) {
    address1[i] = create_page();
  }

  for (i = 0; i < 300; i++) {
    // tmp = rand() % 1000;
    tmp = i;
    if (address1[i] != -1) {
      store_value(address1[i], tmp);
      value = (int *)get_value(address1[i]);
      printf("%d\n", *value);
    }
    // printf("%d ", tmp);
  }

  // value = NULL;
  // for (i = 0; i < 300; i++) {
  //   // value = (int *)get_value(address[i]);
  //   free_page(address1[i]);
  // }

  // for (i = 0; i < 10; i++) {
  //   address2[i] = create_page();
  // }

  // for (i = 0; i < 10; i++) {
  //   if (address2[i] != -1) {
  //     store_value(address2[i], tmp);
  //     value1 = (int *)get_value(address2[i]);
  //     printf("%d\n", *value1);
  //   }
  // }

  // for (i = 0; i < 200; i++) {
  //   // tmp = rand() % 1000;
  //   tmp = i;
  //   if (address[i] != -1) {
  //     store_value(address[i], tmp);
  //     value = (int *)get_value(address[i]);
  //     printf("%d\n", *value);
  //   }
  //   // printf("%d ", tmp);
  // }

  // head = ram_empty_head;
  // if(head != NULL) {
  //   printf("RAM:%d\n", head->page_index);
  //   for (loop = head->next; loop!= head;loop = loop->next) {
  //     printf("RAM:%d\n", loop->page_index);
  //   }
  // }

  // head = ssd_empty_head;
  // if(head != NULL) {
  //   printf("SSD:%d\n", head->page_index);
  //   for (loop = head->next; loop!= head;loop = loop->next) {
  //     printf("SSD:%d\n", loop->page_index);
  //   }
  // }

  //  if (1) {
  //   puts("SSD");
  //   for (i = 0; i < 1000; i++) {
  //     printf("page:%d, present=%d, modified=%d, ref=%d, \n",i, ssd_page_table[i].present, ssd_page_table[i].modified, ssd_page_table[i].referenced);
  //   }
  // }

  // puts("");

  // head = addr_empty_head;
  // printf("Addr:%d\n", head->page_index);
  // for (loop = head->next; loop!= head;loop = loop->next) {
  //   printf("Addr:%d\n", loop->page_index);
  // }

  // for (i = 5; i < 20; i++) {
  //   address[i] = create_page();
  //   if (address[i] != -1) {
  //     store_value(address[i], i);
  //     value = (int *)get_value(address[i]);
  //     printf("%p\n", value);
  //   }
  // }

  // if (1) {
  //   puts("RAM");
  //   for (i = 0; i < 25; i++) {
  //     int tmp = ram_ref_table[i].page_index;
  //     printf("data:%u page:%d, present=%d, modified=%d, ref=%d, \n", ram[i], i, ram_page_table[tmp].present, ram_page_table[tmp].modified, ram_page_table[tmp].referenced);
  //   }

  //   puts("");
  //   puts("SSD");
  //   for (i = 0; i < 100; i++) {
  //     int tmp = ssd_ref_table[i].page_index;
  //     printf("data:%u page:%d, present=%d, modified=%d, ref=%d, \n", ssd[i], i, ssd_page_table[tmp].present, ssd_page_table[tmp].modified, ssd_page_table[tmp].referenced);
  //   }

  //   puts("");
  //   puts("HDD");
  //   for (i = 0; i < 1000; i++) {
  //     printf("data:%u page:%d\n", hdd[i], i);
  //   }
  // }
}

/*
 * frees the page, regardless of where it is in the memory hierarchy
 * @param address the virtual address
 */
void free_page(vAddr address) {
  int ram_page_index;
  int ssd_page_index;
  struct page_ref *to_free;
  struct page_ref *phead;
  struct page_ref *ptail;

  unsigned short virtual_page_index =
      (PAGE_INDEX_MASK & address) >> PAGE_SIZE_BITS;

  // Free invalid memory address
  if (virtual_page_index > NUM_VIRTUAL_PAGES - 1)
    return;

  // Add virtual address back to free list
  to_free = &addr_ref_table[virtual_page_index];

  if(addr_empty_head == NULL) {
    addr_empty_head = free_set_addr_empty_head(to_free);
    addr_empty_head->last = addr_empty_head;
    addr_empty_head->next = addr_empty_head;
  } else {
    phead = addr_empty_head;
    ptail = addr_empty_head->last;
    phead->last = to_free;
    ptail->next = to_free;
    to_free->next = phead;
    to_free->last = ptail;
  }

  // Add ram page back to free list
  if (DEBUG) printf("Addr:%d\n", virtual_page_index);
  if(ram_page_table[virtual_page_index].present == TRUE) {
    ram_page_index = ram_page_table[virtual_page_index].page_frame_num;

    if (DEBUG) printf("Exist:%d\n", ram_page_index);

    add_page_to_free_list(
      &ram_ref_table[ram_page_index], 
      ram_empty_head, 
      free_ram_empty_hand, 
      free_set_ram_empty_head,
      free_set_ram_hand_pointer);

    // Mark ram page as free
    ram_ref_table[ram_page_index].page_index = -1;
    // Free ram
    ram_page_table[virtual_page_index].present = FALSE;
  }

  // Add ssd page back to free list
  if(ssd_page_table[virtual_page_index].present == TRUE) {
    ssd_page_index = ssd_page_table[virtual_page_index].page_frame_num;
    add_page_to_free_list(
      &ssd_ref_table[ssd_page_index], 
      ssd_empty_head, 
      free_ssd_empty_hand, 
      free_set_ssd_empty_head,
      free_set_ssd_hand_pointer);

    // Mark SSD page as free
    ssd_ref_table[ssd_page_index].page_index = -1;
    // Free ssd
    ssd_page_table[virtual_page_index].present = FALSE;
  }
}

void add_page_to_free_list(
  struct page_ref *to_free, 
  struct page_ref *empty_head,
  void (*free_empty_hand)(), 
  struct page_ref* (free_set_empty_head(struct page_ref *)),
  void (free_set_hand_pointer(struct page_ref *))
  ) {

  struct page_ref *head;
  struct page_ref *curr;
  struct page_ref *tail;
  struct page_ref *phead;
  struct page_ref *ptail;

  curr = to_free;

  if (DEBUG) printf("%d\n", to_free->page_index);

  // Remove page from available list
  if (to_free->next == to_free &&
      to_free->last == to_free) {
    free_empty_hand();
  } else {
    // Reconnect the ring
    head = to_free->next;
    tail = to_free->last;
    head->last = tail;
    tail->next = head;
  }

  // Add to allocated list
  if(empty_head == NULL) {
    empty_head = free_set_empty_head(curr);
    empty_head->last = empty_head;
    empty_head->next = empty_head;
  } else {
    phead = empty_head;
    ptail = empty_head->last;
    phead->last = curr;
    ptail->next = curr;
    curr->next = phead;
    curr->last = ptail;
  }
}

/*
 * Replace a page with clock algorithm
 */
int clock_page_replacement(page_table_entry *page_table,
  struct page_ref *ref_table,
  struct page_ref *hand_pointer, 
  int size,
  int (*get_io_counter)(),
  void (*increment_io_counter)(),
  struct page_ref *(*advance_pointer)(),
  void *(*write_back)(int)) {

  pthread_t io_writer;
  int counter = 0;

  struct page_ref *clean = NULL;

  // loop through page index
  while(TRUE) {
    if (!hand_pointer->io_scheduled) {
      // When is not scheduled for I/O
      if (!page_table[hand_pointer->page_index].referenced) {
        // Memory not referenced
        if(!page_table[hand_pointer->page_index].modified){
          // Evict a clean page
          page_table[hand_pointer->page_index].present = FALSE;
          advance_pointer();
          if (DEBUG) puts("Evict not referenced");
          return page_table[hand_pointer->page_index].page_frame_num;
        } else {
          // Find a dirty page, schedule write back to target_device
          if (get_io_counter() < IO_LIMIT) {
            // puts("+++++IO");
            hand_pointer->io_scheduled = TRUE;
            increment_io_counter();
            write_back(page_table[hand_pointer->page_index].page_frame_num);  
          }
        }
      } else {
        // Clear R bit when referenced
        page_table[hand_pointer->page_index].referenced = FALSE;
      }

      // printf("+++++++++++++%d\n", page_table[index].modified);
      if (counter == size ) {
        if(get_io_counter() == 0) {
          // No writes have been scheduled
          if(clean != NULL) {

            page_table[clean->page_index].present = FALSE;
            advance_pointer();
            if (DEBUG) printf("Clean up head:%d\n\n", page_table[clean->page_index].page_frame_num);
            return page_table[clean->page_index].page_frame_num;

          } else {
              // puts("I/O");
              hand_pointer->io_scheduled = TRUE;
              increment_io_counter();
              // hand_pointer->io_scheduled = FALSE;
              write_back(page_table[hand_pointer->page_index].page_frame_num);
          }
        }
      }

      if(!page_table[hand_pointer->page_index].modified) {
          clean = hand_pointer;
      }
    }

    hand_pointer = advance_pointer();
    counter = (counter + 1) % (size + 1);
  }
}

/*
 * Write page back to SSD
 */
void *write_back_to_ssd(int ram_page_index) {
  int ssd_page_index;
  int virtual_page_index = ram_ref_table[ram_page_index].page_index;

  u_int32_t data = ram[ram_page_index];
  if (DEBUG) printf("RAM Data(%d): %u\n", ram_page_index, data);

  // Push data to SSD
  ssd_page_index =  allocate_page(
    ssd_empty_head,
    ssd_page_table, 
    ssd_hand_pointer,
    empty_ssd_list, 
    evict_ssd_page, 
    set_ssd_hand_pointer, 
    set_ssd_empty_head);

  usleep(DELAY_SSD * ONE_SECOND);

  ssd[ssd_page_index] = data;
  if (DEBUG) printf("TO SSD: ram:%d, src:%d, dest%d\n", ram_page_index,data,ssd[ssd_page_index]);

  // // Free memory page
  ram_page_table[virtual_page_index].modified = FALSE;
  ssd_page_table[virtual_page_index].page_frame_num = ssd_page_index;

  ssd_ref_table[ssd_page_index].page_index = virtual_page_index;

  ssd_page_table[virtual_page_index].present = TRUE;
  ssd_page_table[virtual_page_index].modified = TRUE;

  ram_ref_table[ram_page_index].io_scheduled = FALSE;
  num_ssd_io--;
  if (DEBUG) puts("RAM write back");
  return NULL;
}

/*
 * Write page back to HDD
 */
void *write_back_to_hdd(int ssd_page_index) {
  // pthread_mutex_lock(&mutex);
  // int src_page_index = ((struct page_ref*)page)->page_index;
  u_int32_t data = ssd[ssd_page_index];
  int virtual_page_index = ssd_ref_table[ssd_page_index].page_index;

  if (DEBUG) printf("SSD Data(%d): %u\n", ssd_page_index, data);

  // I/O delay
  usleep(DELAY_HDD * ONE_SECOND);
  hdd[virtual_page_index] = data;

  if (DEBUG) printf("TO HDD:%d\n", hdd[virtual_page_index]);

  // Free memory page
  ssd_page_table[virtual_page_index].modified = FALSE;
  ssd_page_table[virtual_page_index].referenced = FALSE;

  ssd_ref_table[ssd_page_index].io_scheduled = FALSE;
  num_hdd_io--;
  // pthread_mutex_unlock(&mutex);
  if (DEBUG) puts("SSD write back");
  return NULL;
  // pthread_exit(0);
}

/*
 * updates the contents of a page
 * @param address the virtual address
 * @param value the value that should be stored in the page
 */
void store_value(vAddr address, u_int32_t value) {
  int ram_page_index;
  // Mask off page index from virtual address
  // 0111111111100000
  unsigned short virtual_page_index =
      (PAGE_INDEX_MASK & address) >> PAGE_SIZE_BITS;
  // unsigned short offset = (PAGE_OFFSET_MASK & address) / BYTE_SIZE;

  // Invalid address
  // Not allocated
  if (virtual_page_index < NUM_VIRTUAL_PAGES) {
    if (!ram_page_table[virtual_page_index].present) {
      // Not present in ram, first retrive from hdd
      ram_page_index = allocate_page(
          ram_empty_head, ram_page_table, ram_hand_pointer, empty_ram_list,
          evict_ram_page, set_ram_hand_pointer, set_ram_empty_head);

      // printf("+++++++++++++:%d\n", ram_page_index);

      ram_page_table[virtual_page_index].page_frame_num = ram_page_index;
      ram_ref_table[ram_page_index].page_index = virtual_page_index;
    } else {
      ram_page_index = ram_page_table[virtual_page_index].page_frame_num;
      // printf("-------------:%d\n", ram_page_index);
    }

    ram[ram_page_index] = value;
    ram_page_table[virtual_page_index].modified = TRUE;
    ram_page_table[virtual_page_index].present = TRUE;
  }
}

/*
 * Replace a page with aging algorithm
 */
int aging_page_replacement() { return 0; }

/*
 * reserves a new memory location in the emulated RAM
 * @return vAddr the virtual address of
 * the new memory location, return -1 if
 * no memory is available
 */
vAddr create_page() {
  int ram_page_index = -1;
  int virtual_page_index; // Page page index
  vAddr address;          // Keeping virtual address

  // Find available virtual page
  virtual_page_index = allocate_virtual_page();

  if (virtual_page_index == -1) {
    // Fail to allocate memory
    return -1;
  }

  ram_page_index = allocate_page(
    ram_empty_head, 
    ram_page_table, 
    ram_hand_pointer,
    empty_ram_list, 
    evict_ram_page, 
    set_ram_hand_pointer, 
    set_ram_empty_head);

  ram_ref_table[ram_page_index].page_index = virtual_page_index;
  ram_page_table[virtual_page_index].page_frame_num = ram_page_index;

  ram_ref_table[ram_page_index].io_scheduled = FALSE;
  ram_page_table[virtual_page_index].present = TRUE;

  // 0111111111100000
  address = virtual_page_index << PAGE_SIZE_BITS;
  return address;
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
  if (addr_empty_head->next == addr_empty_head &&
      addr_empty_head->last == addr_empty_head)
    addr_empty_head = NULL;
  else {
    // Reconnect the ring
    head = addr_empty_head->next;
    tail = addr_empty_head->last;
    head->last = tail;
    tail->next = head;
    addr_empty_head = head;
  }

  // Return virtual page index
  return virtual_page_index;
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
  int ram_page_index = -1;
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
  if (virtual_page_index > NUM_VIRTUAL_PAGES - 1)
    return NULL;

  // Retrieve page from ssd
  if (!ram_page_table[virtual_page_index].present) {
    if (DEBUG)
      puts("RAM Page fault");

    // Retrieve page
    ram_page_index = allocate_page(
    ram_empty_head, 
    ram_page_table, 
    ram_hand_pointer,
    empty_ram_list, 
    evict_ram_page, 
    set_ram_hand_pointer,
    set_ram_empty_head);

    ram[ram_page_index] = retrive_value_from_ssd(virtual_page_index);

    ram_ref_table[ram_page_index].page_index = virtual_page_index;
    ram_ref_table[ram_page_index].io_scheduled = FALSE;

    ram_page_table[virtual_page_index].page_frame_num = ram_page_index;
    ram_page_table[virtual_page_index].present = TRUE;
    ssd_page_table[virtual_page_index].modified = FALSE;

    if (DEBUG)
      puts("Back from SSD");
  }

  ram_page_table[virtual_page_index].referenced = TRUE;

  if (DEBUG)
    printf("Modified: %d\n", ram_page_table[virtual_page_index].modified);
  usleep(DELAY_RAM * ONE_SECOND);
  return (u_int32_t *)(&ram[ram_page_table[virtual_page_index].page_frame_num]);
}

/*
 * Try to get target page from ssd
 */
u_int32_t retrive_value_from_ssd(int virtual_page_index) {
  int ssd_page_num = 0;

  // Retrieve page from disk
  if (!ssd_page_table[virtual_page_index].present) {
    if (DEBUG)
      puts("SSD Page fault");

    // Retrieve page
    ssd_page_num =  allocate_page(
    ssd_empty_head, 
    ssd_page_table, 
    ssd_hand_pointer,
    empty_ssd_list, 
    evict_ssd_page, 
    set_ssd_hand_pointer, 
    set_ssd_empty_head);

    ssd[ssd_page_num] = retrive_value_from_hdd(virtual_page_index);
    ssd_page_table[virtual_page_index].page_frame_num = ssd_page_num;
    ssd_page_table[virtual_page_index].present = TRUE;
    ssd_page_table[virtual_page_index].modified = FALSE;
    ssd_ref_table[virtual_page_index].io_scheduled = FALSE;
    if (DEBUG)
      puts("Back from disk");
  }

  ssd_page_table[virtual_page_index].referenced = TRUE;
  usleep(DELAY_SSD * ONE_SECOND);
  return ssd[ssd_page_table[virtual_page_index].page_frame_num];
}

/*
 * Try to get target page from hdd
 */
u_int32_t retrive_value_from_hdd(int virtual_page_index) {
  if (DEBUG)
    puts("Disk");
  usleep(DELAY_HDD * ONE_SECOND);
  return hdd[virtual_page_index];
}

struct page_ref *advance_ram_pointer() {
  ram_hand_pointer = ram_hand_pointer->next;
  return ram_hand_pointer;
}

void empty_ram_list(){
  ram_empty_head = NULL;
} 

int evict_ram_page(){
  // return second_chance_page_replacement(ram_page_table,
  // ram_ref_table,
  // ram_hand_pointer,
  // advance_ram_pointer,
  // write_back_to_ssd);
  // return random_page_replacement(NUM_PAGE_RAM);
  return clock_page_replacement(
    ram_page_table, 
    ram_ref_table, 
    ram_hand_pointer, 
    NUM_PAGE_RAM,
    get_ssd_io_counter,
    increment_ssd_io_counter,
    advance_ram_pointer, 
    write_back_to_ssd);
}

int get_ssd_io_counter() {
  return num_ssd_io;
}

struct page_ref* set_ram_hand_pointer(struct page_ref * curr) {
  ram_hand_pointer = curr;
  return ram_hand_pointer;
} 

void set_ram_empty_head(struct page_ref *head) {
  ram_empty_head = head;
}

void increment_ssd_io_counter() {
  num_ssd_io++;
}


struct page_ref *advance_ssd_pointer() {
  ssd_hand_pointer = ssd_hand_pointer->next;
  return ssd_hand_pointer;
}

void empty_ssd_list(){
  ssd_empty_head = NULL;
} 

int evict_ssd_page(){
  // return second_chance_page_replacement(ssd_page_table,
  // ssd_ref_table,
  // ssd_hand_pointer,
  // advance_ssd_pointer,
  // write_back_to_hdd);
  // return random_page_replacement(NUM_PAGE_SSD);
  return clock_page_replacement(
    ssd_page_table, 
    ssd_ref_table, 
    ssd_hand_pointer, 
    NUM_PAGE_SSD,
    get_hdd_io_counter,
    increment_hdd_io_counter,
    advance_ssd_pointer, 
    write_back_to_hdd);
} 

int get_hdd_io_counter() {
  return num_hdd_io;
}

void increment_hdd_io_counter() {
  num_hdd_io++;
}

void free_ram_empty_hand(){
  ram_hand_pointer = NULL;
} 

struct page_ref* free_set_ram_empty_head(struct page_ref * curr){
  ram_empty_head = curr;
  return ram_empty_head;
}

void free_set_ram_hand_pointer(struct page_ref * curr){
  ram_hand_pointer = curr;
}

struct page_ref* set_ssd_hand_pointer(struct page_ref * curr) {
  ssd_hand_pointer = curr;
  return ssd_hand_pointer;
}

void set_ssd_empty_head(struct page_ref *head) {
  ssd_empty_head = head;
}

void free_ssd_empty_hand(){
  ssd_hand_pointer = NULL;
}

struct page_ref* free_set_ssd_empty_head(struct page_ref * curr){
  ssd_empty_head = curr;
  return ssd_empty_head;
}

void free_set_ssd_hand_pointer(struct page_ref * curr){
  ssd_hand_pointer = curr;
}

struct page_ref* free_set_addr_empty_head(struct page_ref * curr){
  addr_empty_head = curr;
  return addr_empty_head;
}

/*
 * Replace a page with FIFO algorithm
 */
int second_chance_page_replacement(page_table_entry *page_table,
  struct page_ref *ref_table,
  struct page_ref *hand_pointer,
  struct page_ref *(*advance_pointer)(),
  void *(*write_back)(int)) {

  pthread_t io_writer;

  // loop through page index
  while(TRUE) {
    if (!hand_pointer->io_scheduled) {
      // When is not scheduled for I/O
      if (!page_table[hand_pointer->page_index].referenced) {
        // Memory not referenced
        if(!page_table[hand_pointer->page_index].modified){
          // Evict a clean page
          page_table[hand_pointer->page_index].present = FALSE;
          advance_pointer();
          if (DEBUG) puts("Evict not referenced");
          return page_table[hand_pointer->page_index].page_frame_num;
        } else {
          // Find a dirty page, schedule write back to target_device
            hand_pointer->io_scheduled = TRUE;
            write_back(page_table[hand_pointer->page_index].page_frame_num);  
        }
      } else {
        // Clear R bit when referenced
        page_table[hand_pointer->page_index].referenced = FALSE;
      }
    }
    hand_pointer = advance_pointer();
  }
}

/*
 * Replace a page with random algorithm
 */
int random_page_replacement(int num_avail_pages) {
  return random() % num_avail_pages;
}

/*
 * Get empty page number in memory. Evict a page when it is full
 */
int allocate_page(
  struct page_ref *empty_head, 
  page_table_entry *page_table, 
  struct page_ref *hand_pointer,
  void (*empty_list)(), 
  int (*evict_page)(), 
  struct page_ref* (set_hand_pointer(struct page_ref *)),
  void (set_empty_head(struct page_ref *))
  ){
  struct page_ref *head;
  struct page_ref *curr;
  struct page_ref *tail;
  struct page_ref *phead;
  struct page_ref *ptail;
  int page_index;

  // No empty ram page available
  if (empty_head == NULL) {
    return evict_page();
  }

  page_index = empty_head->page_index;

  // Remove page from available list
  if (empty_head->next == empty_head &&
      empty_head->last == empty_head) {
    curr = empty_head;
    empty_list();
  } else {
    // Reconnect the ring
    head = empty_head->next;
    tail = empty_head->last;
    head->last = tail;
    tail->next = head;

    curr = empty_head;
    set_empty_head(head);
  }

  // Add to allocated list
  if(hand_pointer == NULL) {
    hand_pointer = set_hand_pointer(curr);
    hand_pointer->last = hand_pointer;
    hand_pointer->next = hand_pointer;
  } else {
    phead = hand_pointer;
    ptail = hand_pointer->last;
    phead->last = curr;
    ptail->next = curr;
    curr->next = phead;
    curr->last = ptail;
  }

  page_table[page_index].present = TRUE;
  // return ram page index
  return page_index;
}

void init_memory() {
  int i;
  srand(time(NULL));
  // pthread_t cleaner;

  // initialize RAM
  memset(ram, 0, sizeof(ram));

  // initialize SSD
  memset(ssd, 0, sizeof(ssd));

  // initialize HDD
  memset(hdd, 0, sizeof(hdd));

  // Initialize page table
  for (i = 0; i < NUM_VIRTUAL_PAGES; i++) {
    ram_page_table[i].present = FALSE;
    ram_page_table[i].modified = FALSE;
    ram_page_table[i].page_frame_num = -1;
    ram_ref_table[i].io_scheduled = FALSE;
    ssd_ref_table[i].page_index = -1;
  }

  // Initialize SSD page table
  for (i = 0; i < NUM_VIRTUAL_PAGES; i++) {
    ssd_page_table[i].present = FALSE;
    ssd_page_table[i].modified = FALSE;
    ssd_page_table[i].page_frame_num = -1;
    ssd_ref_table[i].io_scheduled = FALSE;
    ssd_ref_table[i].page_index = -1;
  }

  // Initialize empty virtual address page reference
  // Circular linked list
  addr_empty_head = &addr_ref_table[0];
  init_avai_pages(addr_ref_table, NUM_VIRTUAL_PAGES);

  // Initialize empty RAM page reference
  // Circular linked list
  ram_empty_head = &ram_ref_table[0];
  init_avai_pages(ram_ref_table, NUM_PAGE_RAM);
  ram_hand_pointer = NULL;

  // Initialize empty SSD page reference
  // Circular linked list
  ssd_empty_head = &ssd_ref_table[0];
  init_avai_pages(ssd_ref_table, NUM_PAGE_SSD);
  ssd_hand_pointer = NULL;

  // Schedule daemon to clean up memory
  // pthread_create(&cleaner, 0, &reference_cleaner, 0);
  // pthread_detach(cleaner);
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
    ref_table[i].last = &ref_table[i - 1];
    ref_table[i - 1].next = &ref_table[i];
  }

  // Reference head
  ref_table[size - 1].next = &ref_table[0];
  ref_table[0].last = &ref_table[size - 1];
}

int main(int argc, const char *argv[]) {
  init_memory();
  test_memory();
}