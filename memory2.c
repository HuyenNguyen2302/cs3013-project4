#include "memory.h"

int main(int argc, const char *argv[]) {
	int i;
	int valid;

	start_time = time(NULL);
	srand(time(NULL));
	init_memory();

	if (argc != 2) {
		printf("Usage: ./memory2 [algorithm_choice]\n");
		printf("\t1: RANDOM ALGORITHM\n"); 
		printf("\t2: EVICT THE FIRST PAGE THAT'S NOT THE PAGE THAT SHOULDN'T BE EVICTED\n");
		printf("\t3: LEAST RECENTLY USED\n");
		return 1;
	}

	option = atoi(argv[1]); // save the algorithm option


	test_create_page(PAGE_TABLE + 1);
	test_store_value_and_get_value(1, 10);
	test_free_page_and_get_value(1);

	return 0;
}


/** 
	* testing the create_page function,
	* in this case, the number of pages being created is
	* always bigger than the size of the page table
	* @param num_page the number of pages should be created
	* @return 0 if all the pages are created, -1 if not
	*/ 
int test_create_page(int num_page){
	int i;
	for (i = 0; i < num_page; ++i) {
		vAddr result_create_page = create_page();
		if (result_create_page == -1) { 
			printf("The page table is full. Failed to add page %d.\n", i);
			return -1;
		} 
	}
	return 0;
}

/** 
	* testing the store_value and get_value functions,
	* @param address the virtual address
	* @param new_value the new value to be updated to the page frame
	* associated with the given virtual address
	* @return 0 if the value in the page frame is successfully 
	* stored AND retrieved, -1 if not
	*/
int test_store_value_and_get_value(vAddr address, int new_value) {
	int valid;
	store_value(address, new_value);
	int result = get_value(address, &valid);
	if (valid == TRUE) {
		printf("CONGRATS: NEW_VALUE = %d\n", result);
		return 0;
	} else { 
		printf("ERROR: Sorry. Can't get the new value");
		return 1;
	}
}

/** 
	* testing the free_page and get_value functions,
	* @param address the virtual address
	* @return 0 if the value in the page frame is successfully 
	* retrieved (which shouldn't happen because it 
	* will be freed before getting to get_value), -1 if not
	*/
int test_free_page_and_get_value(vAddr address) {
	free_page(address);
	int valid;
	int result = get_value(address, &valid);
	if (valid == FALSE) {
		printf("ERROR: Sorry. The page frame was deleted.\n");
		return 1;
	}
	return 0;
}

/**
	* Initialize all the RAM, SSD, HDD, and page table arrays
	*/
void init_memory() {
	int i;

	// initialize RAM 
	RAM_num_free_slot = PAGE_RAM;
	memset(RAM_free_slot, TRUE, sizeof(RAM_free_slot)); // TRUE means the page is not free 
	for (i = 0; i < PAGE_RAM; i++) {
		Page_frame entry;
		entry.value = NA;
		entry.referenced_time = NA;
		RAM_arr[i] = entry;
	}

	// initialize SSD struct
	SSD_num_free_slot = PAGE_SSD;
	memset(SSD_free_slot, TRUE, sizeof(SSD_free_slot)); // TRUE means the page is not free 
	for (i = 0; i < PAGE_SSD; i++) {
		Page_frame entry;
		entry.value = NA;
		entry.referenced_time = NA;
		SSD_arr[i] = entry;
	}

	// initialize HDD struct
	HDD_num_free_slot = PAGE_HDD;
	memset(HDD_free_slot, TRUE, sizeof(HDD_free_slot)); // TRUE means the page is not free 
	for (i = 0; i < PAGE_HDD; i++) {
		Page_frame entry;
		entry.value = NA;
		entry.referenced_time = NA;
		HDD_arr[i] = entry;
	}

	// initialize the page table
	for (i = 0; i < PAGE_TABLE; i++) {
		PTE entry;
		entry.memory_location = 0;
		entry.page_frame_num = NA;
		entry.present_bit = FALSE;
		page_table[i] = entry;
	}
}

/**
	* get the current time in the computer since January 1, 1970
	* @return unsigned long the current time 
	*/
unsigned long get_current_time() {
	return time(NULL);
}

/** find a free page frame in RAM/SSD/HDD/Page table
	* @param memory_location the memory location where you're finding the free page
	* @param size the size of the array of the memory location
	* @return return the index of the free slot in the array, 
	* or -1 if no free slot is found
	*/
int find_free_page(int memory_location, int size) {
	int i;
	for (i = 0; i < size; i++) {
		if (memory_location == RAM && RAM_free_slot[i] == TRUE)
			return i;
		if (memory_location == SSD && SSD_free_slot[i] == TRUE)
			return i;
		if (memory_location == HDD && HDD_free_slot[i] == TRUE)
			return i;
		if (memory_location == TABLE && page_table[i].present_bit == FALSE) {
			return i;
		}
	}
	return -1; 
}

/** 
	* evict a page frame from RAM to SSD
	* @return int the index of the free slot in the RAM
	* made by moving it to SSD
	*/
int evict_RAM_to_SSD() {
	int to_evict_page = choose_page_frame(RAM, -1);
	int PTE_index = RAM_arr[to_evict_page].PTE_index;
	int free_slot = find_free_page(SSD, PAGE_SSD);

	// update referenced_time for the page in the RAM
	RAM_arr[to_evict_page].referenced_time = get_current_time();

	// update SSD-related stuff
	SSD_num_free_slot--;
	SSD_free_slot[free_slot] = FALSE;
	SSD_arr[free_slot] = RAM_arr[to_evict_page];

	// update RAM-related stuff
	RAM_num_free_slot++;
	RAM_free_slot[to_evict_page] = TRUE;
	RAM_arr[to_evict_page].value = -1;
	RAM_arr[to_evict_page].referenced_time = -1;

	// update page table
	page_table[PTE_index].memory_location = SSD;
	page_table[PTE_index].page_frame_num = free_slot;
	page_table[PTE_index].present_bit = TRUE;

	// sleep
	usleep(TIME_RAM);
	usleep(TIME_SSD);

	return to_evict_page;
}

/** 
	* evict a page frame from SSD to HDD
	* @param not_evict the index of the page in SSD 
	* that must NOT be evicted
	* @return int the index of the free slot in the SSD
	* made by moving it to HDD
	*/
int evict_SSD_to_HDD(int not_evict) {
	int to_evict_page = choose_page_frame(SSD, not_evict);
	int PTE_index = SSD_arr[to_evict_page].PTE_index;
	int free_slot = find_free_page(HDD, PAGE_HDD);

	// update HDD-related stuff
	HDD_num_free_slot--;
	HDD_free_slot[free_slot] = FALSE;
	HDD_arr[free_slot] = SSD_arr[to_evict_page];

	// update SSD-related stuff
	SSD_num_free_slot++;
	SSD_free_slot[to_evict_page] = TRUE;
	SSD_arr[to_evict_page].value = -1;
	SSD_arr[to_evict_page].referenced_time = -1;

	// update page table
	page_table[PTE_index].memory_location = HDD;
	page_table[PTE_index].page_frame_num = free_slot;
	page_table[PTE_index].present_bit = TRUE;

	usleep(TIME_SSD);
	usleep(TIME_HDD);

	return to_evict_page;
}

/** 
	* evict a page frame from SSD to RAM
	* @param must_evict the index of the page in SSD 
	* that MUST be evicted
	* @return int the index of the free slot in the SSD
	* made by moving it to RAM
	*/
int evict_SSD_to_RAM(int must_evict) {
	int to_evict_page = must_evict;
	int PTE_index = SSD_arr[to_evict_page].PTE_index;
	int free_slot = find_free_page(RAM, PAGE_RAM);

	// update referenced_time for the page in the RAM
	SSD_arr[to_evict_page].referenced_time = get_current_time();

	// update RAM-related stuff
	RAM_num_free_slot--;
	RAM_free_slot[free_slot] = FALSE;
	RAM_arr[free_slot] = SSD_arr[to_evict_page];

	// update SSD-related stuff
	SSD_num_free_slot++;
	SSD_free_slot[to_evict_page] = TRUE;
	SSD_arr[to_evict_page].value = -1;
	SSD_arr[to_evict_page].referenced_time = -1;

	// update page table
	page_table[PTE_index].memory_location = RAM;
	page_table[PTE_index].page_frame_num = free_slot;
	page_table[PTE_index].present_bit = TRUE;

	usleep(TIME_RAM);
	usleep(TIME_SSD);

	return to_evict_page;
}

/** 
	* evict a page frame from HDD to SSD
	* @param must_evict the index of the page in HDD 
	* that MUST be evicted
	* @return int the index of the free slot in the HDD
	* made by moving it to SSD
	*/
int evict_HDD_to_SSD(int must_evict) {
	int to_evict_page = must_evict;
	int PTE_index = SSD_arr[to_evict_page].PTE_index;
	int free_slot = find_free_page(RAM, PAGE_RAM);

	// update referenced_time for the page in the RAM
	HDD_arr[to_evict_page].referenced_time = get_current_time();

	// update RAM-related stuff
	SSD_num_free_slot--;
	SSD_free_slot[free_slot] = FALSE;
	SSD_arr[free_slot] = HDD_arr[to_evict_page];

	// update SSD-related stuff
	HDD_num_free_slot++;
	HDD_free_slot[to_evict_page] = TRUE;
	HDD_arr[to_evict_page].value = -1;
	HDD_arr[to_evict_page].referenced_time = -1;

	// update page table
	page_table[PTE_index].memory_location = SSD;
	page_table[PTE_index].page_frame_num = free_slot;
	page_table[PTE_index].present_bit = TRUE;

	usleep(TIME_HDD);
	usleep(TIME_SSD);

	return to_evict_page;
}

/**
	* choose a random page to evict from the memory location
	* @param memory_location RAM/SSD/HDD
	* @param avoid_index the index that must not be chosen
	* @return the index of the page chosen
	*/
int choose_page_frame_random(int memory_location, int avoid_index) {
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

	if (chosen_page_frame != avoid_index)
		return chosen_page_frame;
	else 
		return choose_page_frame(memory_location, avoid_index);
} 

/** 
	* choose a random page 
	* @param max_value the limit at which the returned 
	* random page must NOT exceed or be equal to
	* @return the index of the random page
	*/
int random_page(int max_value) {
	return rand() % max_value;
}

/**
	* choose the first page to evict from the memory location
	* (must be different from avoid_index)
	* @param memory_location RAM/SSD/HDD
	* @param avoid_index the index that must not be chosen
	* @return the index of the first page chosen
	*/
int choose_page_frame_first(int memory_location, int avoid_index) {
	int chosen_page_frame;
	int size;
	int i;
	switch(memory_location) {
		case RAM:
			size = PAGE_RAM;
			break;
		case SSD:
			size = PAGE_SSD;
			break;
		case HDD:
			size = PAGE_HDD;
			break;
	}	
	for (i = 0; i < size; i++) {
		if (i != avoid_index)
			return i;
	} 
	return 0;
}

/**
	* choose a random page to evict from the memory location
	* @param memory_location RAM/SSD/HDD
	* @param avoid_index the index that must not be chosen
	* @return the index of the page chosen
	*/
int choose_page_frame_history(int memory_location, int avoid_index) {
	int chosen_page_frame;
	int i;
	unsigned long smallest_time = MAX_TIME;

	switch(memory_location) {
		case RAM:
			for (i = 0; i < PAGE_RAM; i++) {
				if (RAM_arr[i].referenced_time < smallest_time)
					smallest_time = RAM_arr[i].referenced_time;
			}
			break;
		case SSD:
			for (i = 0; i < PAGE_SSD; i++) {
				if (SSD_arr[i].referenced_time < smallest_time)
					smallest_time = SSD_arr[i].referenced_time;
			}
			break;
		case HDD:
			for (i = 0; i < PAGE_HDD; i++) {
				if (HDD_arr[i].referenced_time < smallest_time)
					smallest_time = HDD_arr[i].referenced_time;
			}
			break;
	}	

	if (chosen_page_frame != avoid_index)
		return chosen_page_frame;
	else 
		return choose_page_frame(memory_location, avoid_index);
}

/** 
	* decide which algorithm to use and return its result
	* according to the user's choice
	* @param memory_location RAM/SSD/HDD
	* @param avoid_index the index that must not be chosen
	* @return the index of the page chosen
	*/
int choose_page_frame(int memory_location, int avoid_index) {
	int chosen_page_frame;
	switch(option) {
		case RANDOM:
			chosen_page_frame = choose_page_frame_random(memory_location, avoid_index);
			break;
		case FIRST:
			chosen_page_frame = choose_page_frame_first(memory_location, avoid_index);
			break;
		case HISTORY:
			chosen_page_frame = choose_page_frame_history(memory_location, avoid_index);
			break;
	}
	return chosen_page_frame;
}

/** 
	* reserves a new memory location in the emulated RAM
	* @return vAddr the virtual address of 
	* the new memory location, return -1 if 
	* no memory is available
	*/
vAddr create_page() {

	// Are there any free slots in the page table?
	int free_PTE = find_free_page(TABLE, PAGE_TABLE);

	// If no, just return -1
	if (free_PTE == -1) { 
		return -1;
	}

	// If yes, do some work...

	// If there's still free slots in the RAM
	if (RAM_num_free_slot > 0) {

		// find the free slot
		int free_page_RAM = find_free_page(RAM, PAGE_RAM);

		// put a new page in the empty slot + 
		// update the RAM-related stuff 
		RAM_arr[free_page_RAM].referenced_time = get_current_time();
		RAM_arr[free_page_RAM].PTE_index = free_PTE;
		RAM_num_free_slot--;
		RAM_free_slot[free_page_RAM] = FALSE;

		// update the page_table
		PTE new_PTE;
		new_PTE.memory_location = RAM;
		new_PTE.page_frame_num = free_page_RAM;
		new_PTE.present_bit = TRUE;
		page_table[free_PTE] = new_PTE;

		return free_PTE;
	} 



	// If there's no free slots in the RAM
	// If the SSD is not full
	if (SSD_num_free_slot > 0) {

		int free_page_RAM = evict_RAM_to_SSD();
		
		// update the RAM-related stuff 
		RAM_arr[free_page_RAM].referenced_time = get_current_time();
		RAM_arr[free_page_RAM].PTE_index = free_PTE;
		RAM_num_free_slot--;
		RAM_free_slot[free_page_RAM] = FALSE;
	
		// update the page_table
		PTE new_PTE;
		new_PTE.memory_location = RAM;
		new_PTE.page_frame_num = free_page_RAM;
		new_PTE.present_bit = TRUE;
		page_table[free_PTE] = new_PTE;
		
		return free_PTE;
	}

	
	// If the SSD is full 
	int free_page_SSD = evict_SSD_to_HDD(-1);
	int free_page_RAM = evict_RAM_to_SSD();
	// update the RAM-related stuff 
	RAM_arr[free_page_RAM].referenced_time = get_current_time();
	RAM_arr[free_page_RAM].PTE_index = free_PTE;
	RAM_num_free_slot--;
	RAM_free_slot[free_page_RAM] = FALSE;

	// update the page_table
	PTE new_PTE;
	new_PTE.memory_location = RAM;
	new_PTE.page_frame_num = free_page_RAM;
	new_PTE.present_bit = TRUE;
	page_table[free_PTE] = new_PTE;
	
	return free_PTE;
}

/** 
	* obtains the physical memory address 
	* indicated by the given virtual address
	* @param address the virtual address
	* @return u_int32_t * an integer pointer to the location in emulated RAM,
	* returns NULL if the pointer cannot be provided (e.g., a page with the given address does not exist).
	*/
u_int32_t get_value(vAddr address, int *valid) {
	// invalid vAddr or page with the given vAddr doesn't exist
	if (address > PAGE_TABLE - 1 || page_table[address].present_bit == FALSE) {
		*valid = FALSE;
		return FALSE;
	}

	// otherwise, find out where the physical address is
	// also, set *valid = TRUE
	int memory_location = page_table[address].memory_location;
	int index = page_table[address].page_frame_num;
	*valid = TRUE;

	// if the page in is RAM, just get what you want
	if (memory_location == RAM) {
		usleep(TIME_RAM);
		return RAM_arr[index].value;
	}

	// if the page is in the SSD, then
	// move the page from SSD to RAM
	if (memory_location == SSD) {
		
			// If there's still free slots in the RAM,
			// just move it there
			if (RAM_num_free_slot > 0) {
				int new_page_frame_RAM = evict_SSD_to_RAM(index);
				usleep(TIME_RAM); 				
				return RAM_arr[new_page_frame_RAM].value;

			} else { // if there's no free slot in RAM

				// if the SSD isn't full, move one page 
				// from RAM to SSD
				if (SSD_num_free_slot > 0) {					
					evict_RAM_to_SSD();
					int new_page_frame_RAM = evict_SSD_to_RAM(index);
					usleep(TIME_RAM); 				
					return RAM_arr[new_page_frame_RAM].value;

				} else {
					// If the SSD is full, make a new slot in RAM 
					evict_SSD_to_HDD(index);
					evict_RAM_to_SSD();
					int new_page_frame_RAM = evict_SSD_to_RAM(index);
					usleep(TIME_RAM); 				
					return RAM_arr[new_page_frame_RAM].value;
				}
			}		
	}

	// if the page is in the HDD,
	// and if SSD isn't full, move it to the SSD
	if (SSD_num_free_slot > 0) {
		evict_HDD_to_SSD(index);
		// If there's still free slots in the RAM,
		// just move it there
		if (RAM_num_free_slot > 0) {
			int new_page_frame_RAM = evict_SSD_to_RAM(index);
			usleep(TIME_RAM); 				
			return RAM_arr[new_page_frame_RAM].value;

		} else { // if there's no free slot in RAM

			// if the SSD isn't full, move one page 
			// from RAM to SSD
			if (SSD_num_free_slot > 0) {					
				evict_RAM_to_SSD();
				int new_page_frame_RAM = evict_SSD_to_RAM(index);
				usleep(TIME_RAM); 				
				return RAM_arr[new_page_frame_RAM].value;

			} else {
				// If the SSD is full, make a new slot in RAM 
				evict_SSD_to_HDD(index);
				evict_RAM_to_SSD();
				int new_page_frame_RAM = evict_SSD_to_RAM(index); 
				usleep(TIME_RAM);				
				return RAM_arr[new_page_frame_RAM].value;
			}
		}		
	} else {
		evict_SSD_to_HDD(-1);
		evict_HDD_to_SSD(index);
		// If there's still free slots in the RAM,
		// just move it there
		if (RAM_num_free_slot > 0) {
			int new_page_frame_RAM = evict_SSD_to_RAM(index);
			usleep(TIME_RAM); 				
			return RAM_arr[new_page_frame_RAM].value;

		} else { // if there's no free slot in RAM

			// if the SSD isn't full, move one page 
			// from RAM to SSD
			if (SSD_num_free_slot > 0) {					
				evict_RAM_to_SSD();
				int new_page_frame_RAM = evict_SSD_to_RAM(index); 
				usleep(TIME_RAM);				
				return RAM_arr[new_page_frame_RAM].value;

			} else {
				// If the SSD is full, make a new slot in RAM 
				evict_SSD_to_HDD(index);
				evict_RAM_to_SSD();
				int new_page_frame_RAM = evict_SSD_to_RAM(index);
				usleep(TIME_RAM); 				
				return RAM_arr[new_page_frame_RAM].value;
			}
		}		
	}
	return -1;
}

/** 
	* updates the contents of a page 
	* @param address the virtual address
	* @param value the value that should be stored in the page
	*/
void store_value(vAddr address, u_int32_t new_value) {
	// invalid vAddr or page with the given vAddr doesn't exist
	if (address > PAGE_TABLE - 1 || page_table[address].present_bit == FALSE) {
		return;
	}

	// otherwise, find out where the physical address is
	int memory_location = page_table[address].memory_location;
	int index = page_table[address].page_frame_num;
	

	// if the page in is RAM, just get what you want
	if (memory_location == RAM) {
		RAM_arr[index].value = new_value;
		usleep(TIME_RAM);
		return;
	}

	// if the page is in the SSD, then
	// move the page from SSD to RAM
	if (memory_location == SSD) {
		
			// If there's still free slots in the RAM,
			// just move it there
			if (RAM_num_free_slot > 0) {
				int new_page_frame_RAM = evict_SSD_to_RAM(index); 				
				RAM_arr[new_page_frame_RAM].value = new_value;
				usleep(TIME_RAM);
				return;

			} else { // if there's no free slot in RAM

				// if the SSD isn't full, move one page 
				// from RAM to SSD
				if (SSD_num_free_slot > 0) {					
					evict_RAM_to_SSD();
					int new_page_frame_RAM = evict_SSD_to_RAM(index); 				
					RAM_arr[new_page_frame_RAM].value = new_value;
					usleep(TIME_RAM);
					return;

				} else {
					// If the SSD is full, make a new slot in RAM 
					evict_SSD_to_HDD(index);
					evict_RAM_to_SSD();
					int new_page_frame_RAM = evict_SSD_to_RAM(index); 				
					RAM_arr[new_page_frame_RAM].value = new_value;
					usleep(TIME_RAM);
					return;
				}
			}		
	}

	// if the page is in the HDD,
	// and if SSD isn't full, move it to the SSD
	if (SSD_num_free_slot > 0) {
		evict_HDD_to_SSD(index);
		// If there's still free slots in the RAM,
		// just move it there
		if (RAM_num_free_slot > 0) {
			int new_page_frame_RAM = evict_SSD_to_RAM(index); 				
			RAM_arr[new_page_frame_RAM].value = new_value;
			usleep(TIME_RAM);
			return;

		} else { // if there's no free slot in RAM

			// if the SSD isn't full, move one page 
			// from RAM to SSD
			if (SSD_num_free_slot > 0) {					
				evict_RAM_to_SSD();
				int new_page_frame_RAM = evict_SSD_to_RAM(index); 				
				RAM_arr[new_page_frame_RAM].value = new_value;
				usleep(TIME_RAM);
				return;

			} else {
				// If the SSD is full, make a new slot in RAM 
				evict_SSD_to_HDD(index);
				evict_RAM_to_SSD();
				int new_page_frame_RAM = evict_SSD_to_RAM(index); 				
				RAM_arr[new_page_frame_RAM].value = new_value;
				usleep(TIME_RAM);
				return;
			}
		}		
	} else {
		evict_SSD_to_HDD(-1);
		evict_HDD_to_SSD(index);
		// If there's still free slots in the RAM,
		// just move it there
		if (RAM_num_free_slot > 0) {
			int new_page_frame_RAM = evict_SSD_to_RAM(index); 				
			RAM_arr[new_page_frame_RAM].value = new_value;
			usleep(TIME_RAM);
			return;

		} else { // if there's no free slot in RAM

			// if the SSD isn't full, move one page 
			// from RAM to SSD
			if (SSD_num_free_slot > 0) {					
				evict_RAM_to_SSD();
				int new_page_frame_RAM = evict_SSD_to_RAM(index); 				
				RAM_arr[new_page_frame_RAM].value = new_value;
				usleep(TIME_RAM);
				return;

			} else {
				// If the SSD is full, make a new slot in RAM 
				evict_SSD_to_HDD(index);
				evict_RAM_to_SSD();
				int new_page_frame_RAM = evict_SSD_to_RAM(index); 				
				RAM_arr[new_page_frame_RAM].value = new_value;
				usleep(TIME_RAM);
				return;
			}
		}		
	}
	return;
}

/** 
	* frees the page, regardless of where it is in the memory hierarchy 
	* @param address the virtual address
	*/
void free_page(vAddr address) {
	Page_frame to_delete_page_frame;

	// Case 1: invalid vAddr
	if (address > PAGE_TABLE - 1)
		return;

	// Case 2: valid vAddr
	int memory_location = page_table[address].memory_location;
	int page_frame_num = page_table[address].page_frame_num;

	if (memory_location == RAM) {
		to_delete_page_frame = RAM_arr[page_frame_num];
		RAM_num_free_slot++;
		RAM_free_slot[page_frame_num] = TRUE;
	}

	if (memory_location == SSD) {
		to_delete_page_frame = SSD_arr[page_frame_num];
		SSD_num_free_slot++;
		SSD_free_slot[page_frame_num] = TRUE;
	}

	if (memory_location == HDD) {
		to_delete_page_frame = HDD_arr[page_frame_num];
		HDD_num_free_slot++;
		HDD_free_slot[page_frame_num] = TRUE;		
	}

	

	// update the page table
	int page_table_index = to_delete_page_frame.PTE_index;
	PTE to_delete_PTE;
	to_delete_PTE.memory_location = 0;
	to_delete_PTE.page_frame_num = NA;
	to_delete_PTE.present_bit = FALSE;
	page_table[page_table_index] = to_delete_PTE;

	// delete all information about the page frame
	to_delete_page_frame.value = NA;
	to_delete_page_frame.referenced_time = NA;
	to_delete_page_frame.PTE_index = NA;

	return;
}

/**
	* prints all information about every single entry
	* in the RAM/SSD/HDD/page table arrays
	*/
void print_all() {
	int i; 

	printf("============RAM=============\n");

	// initialize RAM 
	for (i = 0; i < PAGE_RAM; i++) {
		
		Page_frame entry = RAM_arr[i];
		printf("\t i = %d\n", i);
		printf("\t\t value = %d\n", entry.value);
		printf("\t\t referenced_time = %ld\n", entry.referenced_time);
	}

	printf("============SSD=============\n");

	// initialize SSD struct
	for (i = 0; i < PAGE_SSD; i++) {
		
		Page_frame entry = SSD_arr[i];
		printf("\t i = %d\n", i);
		printf("\t\t value = %d\n", entry.value);
		printf("\t\t referenced_time = %ld\n", entry.referenced_time);
	}

	printf("============HDD=============\n");

	// initialize HDD struct
	for (i = 0; i < PAGE_HDD; i++) {
		
		Page_frame entry = HDD_arr[i];
		printf("\t i = %d\n", i);
		printf("\t\t value = %d\n", entry.value);
		printf("\t\t referenced_time = %ld\n", entry.referenced_time);
	}

	printf("============PAGE_TABLE=============\n");

	// initialize the page table
	for (i = 0; i < PAGE_TABLE; i++) {
	
		PTE entry = page_table[i];
		printf("\t i = %d\n", i);
		printf("\t\t memory_location = %d\n", entry.memory_location);
		printf("\t\t page_frame_num = %d\n", entry.page_frame_num);
		printf("\t\t present_bit = %d\n", entry.present_bit);
	}
}

