/** File: memory.c
	* @author Huyen Nguyen
	* Date: 
	*/

#include "memory.h"

int main(int argc, const char *argv[]) {
	init_structs();
}

void init_structs() {
	int i;

	// initialize RAM struct
	RAM_struct.RAM_num_free_slot = PAGE_RAM;
	for (i = 0; i < PAGE_RAM; i++) 
		RAM_struct.RAM_free_slot[i] = i;
	memset(RAM_struct.RAM_arr, 0, sizeof (RAM_struct.RAM_arr));

	// initialize SSD struct
	SSD_struct.SSD_num_free_slot = PAGE_SSD;
	for (i = 0; i < PAGE_SSD; i++) 
		SSD_struct.SSD_free_slot[i] = i;
	memset(SSD_struct.SSD_arr, 0, sizeof (SSD_struct.SSD_arr));

	// initialize HARD_DRIVE struct
	HARD_DRIVE_struct.HARD_DRIVE_num_free_slot = PAGE_HARD_DRIVE;
	for (i = 0; i < PAGE_HARD_DRIVE; i++) 
		HARD_DRIVE_struct.HARD_DRIVE_free_slot[i] = i;
	memset(HARD_DRIVE_struct.HARD_DRIVE_arr, 0, sizeof (HARD_DRIVE_struct.HARD_DRIVE_arr));
}
/** 
	* reserves a new memory location in the emulated RAM
	* @return vAddr the virtual address of 
	* the new memory location, return -1 if 
	* no memory is available
	*/
vAddr create_page() {
	
}

/** 
	* obtains the physical memory address 
	* indicated by the given virtual address
	* @param address the virtual address
	* @return u_int32_t * an integer pointer to the location in emulated RAM,
	* returns NULL if the pointer cannot be provided (e.g., a page with the given address does not exist).
	*/
u_int32_t *get_value(vAddr address) {

}

/** 
	* updates the contents of a page 
	* @param address the virtual address
	* @param value the value that should be stored in the page
	*/
void store_value(vAddr address, u_int32_t value) {

}

/** 
	* frees the page, regardless of where it is in the memory hierarchy 
	* @param address the virtual address
	*/
void free_page(vAddr address) {

}