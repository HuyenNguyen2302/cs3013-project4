/** File: memory.c
	* @author Huyen Nguyen
	* Date: 
	*/

#include "memory.h"

int main(int argc, const char *argv[]) {
	start_time = time(NULL);
	srand(time(NULL));
	init_memory();
	print_all();

	// create multiple processes



}


void init_memory() {
	int i;

	// initialize RAM 
	RAM_num_free_slot = PAGE_RAM;
	memset(RAM_free_slot, TRUE, sizeof(RAM_free_slot)); // TRUE means the page is not free 
	for (i = 0; i < PAGE_RAM; i++) {
		Page_frame entry;
		entry.value = NA;
		entry.referenced_time = NA;
		entry.referenced_num = 0;
		RAM_arr[i] = entry;
	}

	// initialize SSD struct
	SSD_num_free_slot = PAGE_SSD;
	memset(SSD_free_slot, TRUE, sizeof(SSD_free_slot)); // TRUE means the page is not free 
	for (i = 0; i < PAGE_SSD; i++) {
		Page_frame entry;
		entry.value = NA;
		entry.referenced_time = NA;
		entry.referenced_num = 0;
		SSD_arr[i] = entry;
	}

	// initialize HDD struct
	HDD_num_free_slot = PAGE_HDD;
	memset(HDD_free_slot, TRUE, sizeof(HDD_free_slot)); // TRUE means the page is not free 
	for (i = 0; i < PAGE_HDD; i++) {
		Page_frame entry;
		entry.value = NA;
		entry.referenced_time = NA;
		entry.referenced_num = 0;
		HDD_arr[i] = entry;
	}

	// initialize the page table
	for (i = 0; i < PAGE_TABLE; i++) {
		PTE entry;
		entry.memory_location = 0;
		entry.page_frame_num = NA;
		entry.referenced_bit = NA;
		entry.present_bit = FALSE;
		page_table[i] = entry;
	}
}

// update PTE associated with a physical address after it has been evicted
void update_PTE(BYTE old_memory_location, BYTE new_memory_location, 
								int old_page_frame_num, int new_page_frame_num) {
	int i;
	for (i = 0; i < PAGE_TABLE; i++) {
		PTE entry = page_table[i];
		if (entry.memory_location == old_memory_location && 
			entry.page_frame_num == old_page_frame_num) {
			entry.memory_location = new_memory_location;
			entry.page_frame_num = new_page_frame_num;
		}
	}
}

// deal with things that happen when the page frame you want is in RAM
/*
void physical_addr_in_RAM(int index) {
	// Case 1: the RAM is full 
	if (RAM_num_free_slot == 0) {

	}
}
*/

// deal with things that happen when the page frame you want is in SSD
// return addr in RAM
int physical_addr_in_SSD(int index) {
	int free_index;
	int chosen_page_frame_SSD = -1;
	int chosen_page_frame_RAM = -1;
	int value;


	// Best case: there's an empty slot in RAM
	if (RAM_num_free_slot > 0) {
		value = SSD_arr[index].value;
		free_index = find_free_slot(RAM);
		move_page_frame(SSD, index, value, RAM, free_index);
		return free_index;
	}

	// If the best case doesn't happen, then...
	// Case 1: the SSD is not full
	if (SSD_num_free_slot > 0) {

		// move one page frame from RAM to SSD
		free_index = find_free_slot(SSD);
		chosen_page_frame_RAM = choose_page_frame(RAM);
		value = RAM_arr[chosen_page_frame_RAM].value;
		move_page_frame(RAM, chosen_page_frame_RAM, value, SSD, free_index);

		// move the desired page frame from SSD to RAM
		value = RAM_arr[index].value;
		move_page_frame(SSD, index, value, RAM, chosen_page_frame_RAM);
		return chosen_page_frame_RAM;

	} else { // Case 2: the SSD is full

		// move one page frame from SSD to the HDD
		free_index = find_free_slot(HDD);
		while (chosen_page_frame_SSD == -1  || chosen_page_frame_SSD == index) // don't swap the page frame we want to get to the RAM
			chosen_page_frame_SSD = choose_page_frame(SSD);
		value = SSD_arr[chosen_page_frame_SSD].value;
		move_page_frame(SSD, chosen_page_frame_SSD, value, HDD, free_index); 

		// move one page frame from RAM to SSD 
		chosen_page_frame_RAM = choose_page_frame(RAM);
		value = RAM_arr[chosen_page_frame_RAM].value;
		move_page_frame(RAM, chosen_page_frame_RAM, value, SSD, chosen_page_frame_SSD);
		
		// move the desired page frame from SSD to RAM
		value = RAM_arr[index].value;
		move_page_frame(SSD, index, value, RAM, chosen_page_frame_RAM);
		return chosen_page_frame_RAM;
	}
}

// deal with things that happen when the page frame you want is in HDD
int physical_addr_in_HDD(int index) {
	// strategy: move the desired page frame to the SSD, and use 
	// physical_addr_in_SSD(int index)

	int value;
	int chosen_page_frame_SSD;

	// Case 1: if the SSD isn't full, just move the 
	// desired page frame from HDD to SSD
	if (SSD_num_free_slot > 0) {
		value = HDD_arr[index].value;
		chosen_page_frame_SSD = find_free_slot(SSD);
		move_page_frame(HDD, index, value, SSD, chosen_page_frame_SSD);	
		return physical_addr_in_SSD(chosen_page_frame_SSD);;
	}

	// Case 2: if the SSD is full, then move 1 page from SSD to HDD
	// then move the desired page from HDD to SSD
	else {
		// move one page frame from SSD to the HDD
		int free_index = find_free_slot(HDD);
		chosen_page_frame_SSD = choose_page_frame(SSD);
		value = SSD_arr[chosen_page_frame_SSD].value;
		move_page_frame(SSD, chosen_page_frame_SSD, value, HDD, free_index); 

		// move one page frame from HDD to SSD
		value = HDD_arr[index].value;
		move_page_frame(HDD, index, value, SSD, chosen_page_frame_SSD);
		return physical_addr_in_SSD(chosen_page_frame_SSD);		
	}
}

// decide which algorithm to use and return its result
int choose_page_frame(int memory_location) {
	int chosen_page_frame;
	switch(option) {
		case RANDOM:
			chosen_page_frame = choose_page_frame_random(memory_location);
			break;
		case FIFO:
			chosen_page_frame = choose_page_frame_fifo(memory_location);
			break;
		case HISTORY:
			chosen_page_frame = choose_page_frame_history(memory_location);
			break;
	}
	return chosen_page_frame;
}

int choose_page_frame_random(int memory_location) {
	int chosen_page_frame;
	switch(memory_location) {
		case RAM:
			chosen_page_frame = random_page(PAGE_RAM);
			break;
		case SSD:
			chosen_page_frame = random_page(PAGE_SSD);
			break;
		case HDD:
			chosen_page_frame = random_page(PAGE_HDD);
			break;
	}
	return chosen_page_frame;
}

int choose_page_frame_fifo(BYTE memory_location) {
	int chosen_page_frame;

	switch(memory_location) {
		case RAM:
			chosen_page_frame = find_fi_page(RAM);
			break;
		case SSD:
			chosen_page_frame = find_fi_page(SSD);
			break;
		case HDD:
			chosen_page_frame = find_fi_page(HDD);
			break;
	}
	return chosen_page_frame;	
}

int find_fi_page(BYTE memory_location) {
	int i;
	unsigned long earliest = MAX_TIME;
	int chosen_page_frame;

	switch(memory_location) {
		case RAM:
			for (i = 0; i < PAGE_RAM; i++) {
				if (RAM_free_slot[i] == FALSE && RAM_arr[i].referenced_time < earliest) {
					earliest = RAM_arr[i].referenced_time;
					chosen_page_frame = i;
				}
			}
			break;
		case SSD:
			for (i = 0; i < PAGE_SSD; i++) {
				if (SSD_free_slot[i] == FALSE && SSD_arr[i].referenced_time < earliest) {
					earliest = SSD_arr[i].referenced_time;
					chosen_page_frame = i;
				}
			}
			break;
		case HDD:
			for (i = 0; i < PAGE_HDD; i++) {
				if (HDD_free_slot[i] == FALSE && HDD_arr[i].referenced_time < earliest) {
					earliest = HDD_arr[i].referenced_time;
					chosen_page_frame = i;
				}
			}
			break;
	}
	return chosen_page_frame;
}

int choose_page_frame_history(BYTE memory_location) {
	int chosen_page_frame;

	switch(memory_location) {
		case RAM:
			chosen_page_frame = find_max_referenced_page(RAM);
			break;
		case SSD:
			chosen_page_frame = find_max_referenced_page(SSD);
			break;
		case HDD:
			chosen_page_frame = find_max_referenced_page(HDD);
			break;
	}
	return chosen_page_frame;	
}

int find_max_referenced_page(BYTE memory_location) {
	int i;
	int max_referenced_num = NA;
	int chosen_page_frame;


	switch(memory_location) {
		case RAM:
			for (i = 0; i < PAGE_RAM; i++) {
				if (RAM_free_slot[i] == FALSE && RAM_arr[i].referenced_num > max_referenced_num) {
					max_referenced_num = RAM_arr[i].referenced_num;
					chosen_page_frame = i;
				}
			}
			break;
		case SSD:
			for (i = 0; i < PAGE_SSD; i++) {
				if (SSD_free_slot[i] == FALSE && SSD_arr[i].referenced_num > max_referenced_num) {
					max_referenced_num = SSD_arr[i].referenced_num;
					chosen_page_frame = i;
				}
			}
			break;
		case HDD:
			for (i = 0; i < PAGE_HDD; i++) {
				if (HDD_free_slot[i] == FALSE && HDD_arr[i].referenced_num > max_referenced_num) {
					max_referenced_num = HDD_arr[i].referenced_num;
					chosen_page_frame = i;
				}
			}
			break;
	}
	return chosen_page_frame;
}

// find a free slot in the given memory location
int find_free_slot(BYTE memory_location) {
	int i;
	int free_slot;

	if (memory_location == RAM) {
		for (i = 0; i < PAGE_RAM; i++) 
			if (RAM_free_slot[i] == TRUE) {
				free_slot = i;
				break;
			}
	}

	if (memory_location == SSD) {
		for (i = 0; i < PAGE_SSD; i++) 
			if (SSD_free_slot[i] == TRUE) {
				free_slot = i;
				break;
			} 
	}

	if (memory_location == HDD) {
		for (i = 0; i < PAGE_HDD; i++) 
			if (HDD_free_slot[i] == TRUE) {
				free_slot = i;
				break;
			} 
	}

	else {
		for (i = 0; i < PAGE_TABLE; i++) {
			PTE page_table_entry = page_table[i];
			if (page_table_entry.present_bit == FALSE) {
				free_slot = i;
				break;
			}
		}
	}
	return free_slot;
}

// actually evict a page frame and move it to another memory storage
// the destination storage at to_index is guaranteed to be free
void move_page_frame(BYTE memory_location_from, int from_index, int value, 
										 BYTE memory_location_to, int to_index) {
	unsigned long current_time = get_current_time();
	int referenced_num;
	Page_frame new_page_frame;
	new_page_frame.value = value;
	new_page_frame.referenced_time = current_time;

	// get the referenced_num
	if (memory_location_from == RAM)
		referenced_num = RAM_arr[from_index].referenced_num; 
	if (memory_location_from == SSD)
		referenced_num = SSD_arr[from_index].referenced_num;
	if (memory_location_from == HDD)
		referenced_num = HDD_arr[from_index].referenced_num;

	new_page_frame.referenced_num = referenced_num + 1;

	if (memory_location_to == RAM)		
		RAM_arr[to_index] = new_page_frame;

	if (memory_location_to == SSD)
		RAM_arr[to_index] = new_page_frame;

	if (memory_location_from == HDD)
		HDD_arr[to_index] = new_page_frame;

	update_PTE(memory_location_from, memory_location_to, from_index, to_index);
}

int random_page(int max_value) {
	return rand() % max_value;
}

unsigned long get_current_time() {
	return time(NULL) - start_time;
}

/** 
	* reserves a new memory location in the emulated RAM
	* @return vAddr the virtual address of 
	* the new memory location, return -1 if 
	* no memory is available
	*/
vAddr create_page() {

	int chosen_page_frame_RAM;
	int chosen_page_frame_SSD;
	int chosen_page;
	int free_index;
	int value;

	unsigned long current_time = get_current_time(); 

	// Case 1: If the page table is full, return -1
	chosen_page = find_free_slot(TABLE);
	if (chosen_page == NA) 
		return -1;

	// Otherwise...
	// Case 2: If the RAM isn't full, then just
	// insert the desired page frame
	if (RAM_num_free_slot > 0) {
		chosen_page_frame_RAM = find_free_slot(RAM);	

		// update the fields for the PTE
		PTE page_table_entry = page_table[chosen_page];
		page_table_entry.memory_location = RAM;
		page_table_entry.page_frame_num = chosen_page_frame_RAM;
		page_table_entry.referenced_bit = current_time;
		page_table_entry.present_bit = TRUE; 

		// update the fields for the Page frame in RAM
		Page_frame page_frame = RAM_arr[chosen_page_frame_RAM];
		page_frame.referenced_time = current_time;
		page_frame.referenced_num++;
		return chosen_page;
	}

	// Case 3: If the RAM is full, then 
	// Case 3a: If the SSD isn't full, move one page from RAM to SSD
	if (SSD_num_free_slot > 0) {
		// move one page frame from RAM to SSD
		free_index = find_free_slot(SSD);
		chosen_page_frame_RAM = choose_page_frame(RAM);
		value = RAM_arr[chosen_page_frame_RAM].value;
		move_page_frame(RAM, chosen_page_frame_RAM, value, SSD, free_index);

		// update the fields for the PTE
		PTE page_table_entry = page_table[chosen_page];
		page_table_entry.memory_location = RAM;
		page_table_entry.page_frame_num = chosen_page_frame_RAM;
		page_table_entry.referenced_bit = current_time;
		page_table_entry.present_bit = TRUE; 

		// update the fields for the Page frame in RAM
		Page_frame page_frame = RAM_arr[chosen_page_frame_RAM];
		page_frame.referenced_time = current_time;
		page_frame.referenced_num++;
		return chosen_page;
	}

	// Case 3b: If the SSD is full, then 
	// move 1 page from SSD to HDD, then
	// move 1 page from RAM to SSD
	else {
		// move one page frame from SSD to the HDD
		free_index = find_free_slot(HDD);
		chosen_page_frame_SSD = choose_page_frame(SSD);
		value = SSD_arr[chosen_page_frame_SSD].value;
		move_page_frame(SSD, chosen_page_frame_SSD, value, HDD, free_index); 

		// move one page frame from RAM to SSD 
		chosen_page_frame_RAM = choose_page_frame(RAM);
		value = RAM_arr[chosen_page_frame_RAM].value;
		move_page_frame(RAM, chosen_page_frame_RAM, value, SSD, chosen_page_frame_SSD);
		
		// update the fields for the PTE
		PTE page_table_entry = page_table[chosen_page];
		page_table_entry.memory_location = RAM;
		page_table_entry.page_frame_num = chosen_page_frame_RAM;
		page_table_entry.referenced_bit = current_time;
		page_table_entry.present_bit = TRUE; 

		// update the fields for the Page frame in RAM
		Page_frame page_frame = RAM_arr[chosen_page_frame_RAM];
		page_frame.referenced_time = current_time;
		page_frame.referenced_num++;
		return chosen_page;
	}
}



/** 
	* obtains the physical memory address 
	* indicated by the given virtual address
	* @param address the virtual address
	* @return u_int32_t * an integer pointer to the location in emulated RAM,
	* returns NULL if the pointer cannot be provided (e.g., a page with the given address does not exist).
	*/
u_int32_t get_value(vAddr address, int *valid) {
	int i;
	u_int32_t value;
	int chosen_page_frame_RAM;
	unsigned long current_time = get_current_time(); 

	// Case 1: invalid vAddr or page with the given vAddr doesn't exist
	if (address > PAGE_TABLE - 1 || page_table[address].present_bit == FALSE) {
		*valid = FALSE;
		return FALSE;
	}

	int memory_location = page_table[address].memory_location;
	int page_frame_num = page_table[address].page_frame_num;

	// Case 2: the physical address is in RAM
	switch(memory_location) {
		case RAM:
			value = RAM_arr[page_frame_num].value;
			RAM_arr[page_frame_num].referenced_time = current_time;
			RAM_arr[page_frame_num].referenced_num++;
			break;
		case SSD:
			chosen_page_frame_RAM = physical_addr_in_SSD(page_frame_num);
			value = RAM_arr[chosen_page_frame_RAM].value;
			RAM_arr[chosen_page_frame_RAM].referenced_time = current_time;
			RAM_arr[chosen_page_frame_RAM].referenced_num++;
			break;
		case HDD:
			chosen_page_frame_RAM = physical_addr_in_HDD(page_frame_num);
			value = RAM_arr[chosen_page_frame_RAM].value;
			RAM_arr[chosen_page_frame_RAM].referenced_time = current_time;
			RAM_arr[chosen_page_frame_RAM].referenced_num++;
			break;
	}	
	*valid = TRUE;
	return value;
}

/** 
	* updates the contents of a page 
	* @param address the virtual address
	* @param value the value that should be stored in the page
	*/
void store_value(vAddr address, u_int32_t new_value) {
	int valid;
	int current_time = get_current_time();
	get_value(address, &valid); // after this, the desired page frame has already been moved to the RAM

	if (valid == TRUE) {
		int page_frame_num = page_table[address].page_frame_num;
		RAM_arr[page_frame_num].value = new_value;
		RAM_arr[page_frame_num].referenced_time = current_time;
		RAM_arr[page_frame_num].referenced_num++;
	} else {
		return; // don't do anything if vAddr is invalid
	}
}

/** 
	* frees the page, regardless of where it is in the memory hierarchy 
	* @param address the virtual address
	*/
void free_page(vAddr address) {

	// Case 1: invalid vAddr
	if (address > PAGE_TABLE - 1)
		return;

	// Case 2: valid vAddr
	int memory_location = page_table[address].memory_location;
	int page_frame_num = page_table[address].page_frame_num;

	if (memory_location == RAM) {
		RAM_num_free_slot++;
		RAM_free_slot[page_frame_num] = TRUE;
		RAM_arr[page_frame_num].value = NA;
		RAM_arr[page_frame_num].referenced_time = NA;
		RAM_arr[page_frame_num].referenced_num = 0;
	}

	if (memory_location == SSD) {
		SSD_num_free_slot++;
		SSD_free_slot[page_frame_num] = TRUE;
		SSD_arr[page_frame_num].value = NA;
		SSD_arr[page_frame_num].referenced_time = NA;
		SSD_arr[page_frame_num].referenced_num = 0;
	}

	if (memory_location == HDD) {
		HDD_num_free_slot++;
		HDD_free_slot[page_frame_num] = TRUE;
		HDD_arr[page_frame_num].value = NA;
		HDD_arr[page_frame_num].referenced_time = NA;
		HDD_arr[page_frame_num].referenced_num = 0;
	}

	page_table[address].memory_location = NA;
	page_table[address].page_frame_num = NA;
	page_table[address].referenced_bit = NA;
	page_table[address].present_bit = FALSE;

	return;
}

void print_all() {
	int i; 

	printf("============RAM=============\n");

	// initialize RAM 
	for (i = 0; i < PAGE_RAM; i++) {
		
		Page_frame entry = RAM_arr[i];
		printf("\t i = %d\n", i);
		printf("\t\t value = %d\n", entry.value);
		printf("\t\t referenced_time = %ld\n", entry.referenced_time);
		printf("\t\t referenced_num = %d\n", entry.referenced_num);
	}

	printf("============SSD=============\n");

	// initialize SSD struct
	for (i = 0; i < PAGE_SSD; i++) {
		
		Page_frame entry = SSD_arr[i];
		printf("\t i = %d\n", i);
		printf("\t\t value = %d\n", entry.value);
		printf("\t\t referenced_time = %ld\n", entry.referenced_time);
		printf("\t\t referenced_num = %d\n", entry.referenced_num);
	}

	printf("============HDD=============\n");

	// initialize HDD struct
	for (i = 0; i < PAGE_HDD; i++) {
		
		Page_frame entry = HDD_arr[i];
		printf("\t i = %d\n", i);
		printf("\t\t value = %d\n", entry.value);
		printf("\t\t referenced_time = %ld\n", entry.referenced_time);
		printf("\t\t referenced_num = %d\n", entry.referenced_num);
	}

	printf("============PAGE_TABLE=============\n");

	// initialize the page table
	for (i = 0; i < PAGE_TABLE; i++) {
	
		PTE entry = page_table[i];
		printf("\t i = %d\n", i);
		printf("\t\t memory_location = %d\n", entry.memory_location);
		printf("\t\t page_frame_num = %d\n", entry.page_frame_num);
		printf("\t\t referenced_bit = %ld\n", entry.referenced_bit);
		printf("\t\t present_bit = %d\n", entry.present_bit);
	}
}