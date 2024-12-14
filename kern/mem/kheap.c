#include "kheap.h"

#include <inc/memlayout.h>
#include <inc/dynamic_allocator.h>
#include "memory_manager.h"
#define MAX_SHEFO 1024
struct AllocationInfo {
    void* start_address;
    uint32 num_pages;
};
uint32 physical_to_virtual_map[65536];
struct AllocationInfo allocations[MAX_SHEFO];
//Initialize the dynamic allocator of kernel heap with the given start address, size & limit
//All pages in the given range should be allocated
//Remember: call the initialize_dynamic_allocator(..) to complete the initialization
//Return:
//	On success: 0
//	Otherwise (if no memory OR initial size exceed the given limit): PANIC
void initphysicaltovirtualmap()
{
	for(int i=0;i<number_of_frames-1;i++)
	{
		physical_to_virtual_map[i]=0;
	}
}
int initialize_kheap_dynamic_allocator(uint32 daStart, uint32 initSizeToAllocate, uint32 daLimit)
{
	//TODO: [PROJECT'24.MS2 - #01] [1] KERNEL HEAP - initialize_kheap_dynamic_allocator
	// Write your code here, remove the panic and write your code
	//panic("initialize_kheap_dynamic_allocator() is not implemented yet...!!");

	if (initSizeToAllocate % PAGE_SIZE != 0) {
		initSizeToAllocate = ROUNDUP(initSizeToAllocate, PAGE_SIZE);
	}

	hStart = daStart;
	hLimit = daLimit;
	segmentBr = daStart + initSizeToAllocate;
	//initphysicaltovirtualmap();
	init_spinlock(&ellolLk, "Kernel lock");
	if (initSizeToAllocate > daLimit) {
		panic("ya bro the Initial size exceeds the given limit");
	}

	for (uint32 i = daStart; i < segmentBr; i += PAGE_SIZE) {
		struct FrameInfo *frametobeallocated;
		int ret = allocate_frame(&frametobeallocated);

		 if(ret == E_NO_MEM){
			panic("There is NO ENOUGH MEMORY");
		}

		int mapret = map_frame(ptr_page_directory, frametobeallocated, i, PERM_PRESENT|PERM_WRITEABLE);

		if (mapret == E_NO_MEM) {
			panic("There is NO ENOUGH MEMORY");
			free_frame(frametobeallocated) ;
		}

		uint32 frameNo=to_physical_address(frametobeallocated);
		frameNo=(frameNo>>12);
		physical_to_virtual_map[frameNo] = i;
	}

	initialize_dynamic_allocator(hStart, initSizeToAllocate);
	return 0;
}

void* sbrk(int numOfPages)
{

	/* numOfPages > 0: move the segment break of the kernel to increase the size of its heap by the given numOfPages,
	 * 				you should allocate pages and map them into the kernel virtual address space,
	 * 				and returns the address of the previous break (i.e. the beginning of newly mapped memory).
	 * numOfPages = 0: just return the current position of the segment break
	 *
	 * NOTES:
	 * 	1) Allocating additional pages for a kernel dynamic allocator will fail if the free frames are exhausted
	 * 		or the break exceed the limit of the dynamic allocator. If sbrk fails, return -1
	 */

	//MS2: COMMENT THIS LINE BEFORE START CODING==========
	//return (void*)-1 ;
	//====================================================

	//TODO: [PROJECT'24.MS2 - #02] [1] KERNEL HEAP - sbrk
	// Write your code here, remove the panic and write your code
	//panic("sbrk() is not implemented yet...!!");

	if(numOfPages<0)
	{
	  panic("can't Allocate no. of page with negative value");
	  return (void*)-1;
	}

	 if(numOfPages==0)
		 return (void*)segmentBr;


	 uint32 size =numOfPages*PAGE_SIZE;

	 if(segmentBr+size>hLimit)
	   return (void*)-1;

	 for(int i=segmentBr,j=0;j<numOfPages;i+=PAGE_SIZE,j++)
	 {
		 struct FrameInfo *freeFrame;
		 int ret = allocate_frame(&freeFrame);
		 if(ret==E_NO_MEM)
			 return (void*)-1;

		 ret=map_frame(ptr_page_directory,freeFrame,i,PERM_WRITEABLE|PERM_PRESENT);
		 if(ret==E_NO_MEM)
		 {
			 free_frame(freeFrame);
			 return (void*)-1;
		 }

		 uint32 frameNo=to_physical_address(freeFrame);
		 frameNo=(frameNo>>12);
		 physical_to_virtual_map[frameNo] = i;

	 }

	 void *oldBreak=(void*)segmentBr;

	 segmentBr+=size;


	 return oldBreak;
}

//TODO: [PROJECT'24.MS2 - BONUS#2] [1] KERNEL HEAP - Fast Page Allocator

void* kmalloc(unsigned int size)
{
	//TODO: [PROJECT'24.MS2 - #03] [1] KERNEL HEAP - kmalloc
	// Write your code here, remove the panic and write your code
	//kpanic_into_prompt("kmalloc() is not implemented yet...!!");
	// use "isKHeapPlacementStrategyFIRSTFIT() ..." functions to check the current strategy
	// Return NULL for zero allocation requests

	acquire_spinlock(&ellolLk);
	if (size == 0)
	{
		release_spinlock(&ellolLk);
		return NULL;
	}

	if (size <= DYN_ALLOC_MAX_BLOCK_SIZE){
//		cprintf("keda nta 8alat \n");
		release_spinlock(&ellolLk);
		return alloc_block_FF(size);
	}

	uint32 numPages = (ROUNDUP(size, PAGE_SIZE)) / PAGE_SIZE;
	uint32 contiguous_pages = 0;
	uint32 start_search_address = hLimit + PAGE_SIZE;
	void* start_address = NULL;
	uint32 *ptr_page_table = NULL;

	for (uint32 i = start_search_address; i < KERNEL_HEAP_MAX; i += PAGE_SIZE) {
		struct FrameInfo *frame = get_frame_info(ptr_page_directory, i, &ptr_page_table);
		if (frame == NULL) {
			if (contiguous_pages == 0)
				start_address = (void*)i;

			contiguous_pages++;

			if (contiguous_pages == numPages)
				break;

		}else {
			contiguous_pages = 0;
			start_address = NULL;
		}
	}

	if (contiguous_pages < numPages) {
	//cprintf("Error: Not enough contiguous space found for %d pages\n", numPages);
		release_spinlock(&ellolLk);
		return NULL;
	}

	uint32 allocated_pages = 0;
	for (uint32 i = (uint32)start_address; allocated_pages < numPages; i += PAGE_SIZE) {
		struct FrameInfo *new_frame = NULL;

		if (allocate_frame(&new_frame) != 0) {
			cprintf("Error: Frame allocation failed at address %p\n", (void*)i);
			release_spinlock(&ellolLk);
			return NULL;
		}

		if (map_frame(ptr_page_directory, new_frame, i, PERM_WRITEABLE) != 0) {
			cprintf("Error: Frame mapping failed at address %p\n", (void*)i);
			release_spinlock(&ellolLk);
			return NULL;
		}
//		cprintf("           Mapped frame at virtual address %p to physical frame %p\n", (void*)i, (void*)to_physical_address(new_frame));
        uint32 frame_numberr = to_physical_address(new_frame) / PAGE_SIZE;
        physical_to_virtual_map[frame_numberr] = i;
//        cprintf("           Updated physical to virtual map: frame %u -> virtual address %p\n", frame_numberr, (void*)i);
		allocated_pages++;
	}

	if (allocation_co < MAX_SHEFO) {
		allocations[allocation_co].start_address = start_address;
		allocations[allocation_co].num_pages = numPages;
		allocation_co++;
	}
	release_spinlock(&ellolLk);
	return start_address;
}

void kfree(void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #04] [1] KERNEL HEAP - kfree
	// Write your code here, remove the panic and write your code
	//	panic("kfree() is not implemented yet...!!");
	//you need to get the size of the given allocation using its address
	//refer to the project presentation and documentation for details

	acquire_spinlock(&ellolLk);
	if ((uint32)virtual_address >= KERNEL_HEAP_START
			&& (uint32)virtual_address < hLimit) {

		free_block(virtual_address);
		release_spinlock(&ellolLk);
		return;
	}else if ((uint32)virtual_address >= hLimit + PAGE_SIZE
			&& (uint32)virtual_address < KERNEL_HEAP_MAX)
	{
		uint32 *ptr_page_table = NULL;
		uint32 va = (uint32)virtual_address;
		bool found =0;
		uint32 number=0;
		uint32 index = 0;
		for(uint32 i =0 ;i < allocation_co ; i++){
			if (allocations[i].start_address == virtual_address){
				found=1;
				number= allocations[i].num_pages;
				index= i;
				break;
			}
		}
		for (uint32 i = (uint32)virtual_address; i < (uint32)virtual_address + (PAGE_SIZE*number); i += PAGE_SIZE)
		{
			struct FrameInfo* freedFrame = get_frame_info(ptr_page_directory,i, &ptr_page_table);
				if (freedFrame != NULL) {

					uint32 frame_numberr = to_physical_address(freedFrame) / PAGE_SIZE;
					physical_to_virtual_map[frame_numberr] = 0;

//					if(freedFrame->references == 1)
//						free_frame(freedFrame);

					unmap_frame(ptr_page_directory, i);
				}
		}
		allocations[index].start_address = NULL;
		allocations[index].num_pages = 0;
	}
	release_spinlock(&ellolLk);

}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #05] [1] KERNEL HEAP - kheap_physical_address
	// Write your code here, remove the panic and write your code
	//panic("kheap_physical_address() is not implemented yet...!!");

	//return the physical address corresponding to given virtual_address
	//refer to the project presentation and documentation for details

	//EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================
/*
	uint32 offset = virtual_address&(0x00000FFF);
	uint32* ptr_page_table;
	struct FrameInfo *frame_info = get_frame_info(ptr_page_directory,virtual_address,&ptr_page_table);
	uint32 pa=to_physical_address(frame_info)+offset+550;
	return pa;
*/
	uint32* ptr_page_table;
	struct FrameInfo *frame_info = get_frame_info(ptr_page_directory,virtual_address,&ptr_page_table);
	if(frame_info==NULL)
		return 0;

	uint32 offset = virtual_address&(0x00000FFF);
	uint32 Page_Index=PTX(virtual_address);
	get_page_table(ptr_page_directory,virtual_address,&ptr_page_table);
	uint32 entry = ptr_page_table[Page_Index];
	uint32 frameNo=entry&(0xFFFFF000);
	uint32 pa = frameNo+offset;
	return pa;

}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
	//TODO: [PROJECT'24.MS2 - #06] [1] KERNEL HEAP - kheap_virtual_address
	// Write your code here, remove the panic and write your code
//	panic("kheap_virtual_address() is not implemented yet...!!");
	   uint32 frame_number = physical_address / PAGE_SIZE;

	    if (frame_number >= 65536 || physical_to_virtual_map[frame_number] == 0) {
	        return 0;  // No mapping found
	    }

	    uint32 offset = physical_address & (PAGE_SIZE - 1);
	    unsigned int virtual_address = physical_to_virtual_map[frame_number] + offset;

	    return virtual_address;
	//return the virtual address corresponding to given physical_address
	//refer to the project presentation and documentation for details

	//EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================


}
//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, if moved to another loc: the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().

void *krealloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT'24.MS2 - BONUS#1] [1] KERNEL HEAP - krealloc
	// Write your code here, remove the panic and write your code
	//return NULL;
	//panic("krealloc() is not implemented yet...!!");

	if((uint32)virtual_address>=hLimit+PAGE_SIZE&&(uint32)virtual_address<KERNEL_HEAP_MAX) /**Page Allocator**/
	{

		if(new_size>DYN_ALLOC_MAX_BLOCK_SIZE) //reallocate at page allocator
		{
			uint32 newNoOfPages=ROUNDUP(new_size, PAGE_SIZE);
			//uint32 start=hLimit+PAGE_SIZE;
			int noOfPages,index;
			uint32 startAdd=(uint32)virtual_address;

			for(uint32 i=0;allocation_co ; i++)
				{
				  if(allocations[i].start_address==virtual_address)
					{
					  noOfPages=allocations[i].num_pages;
					  index=i;
					  break;
					}
				}


		if(newNoOfPages>noOfPages)
		{
			uint32 *ptr_page_table = NULL;
			uint32 foundedFreePages=0;
			uint32 neededPages=newNoOfPages-noOfPages;
			uint32 startSearchAddr=startAdd+(noOfPages*PAGE_SIZE);

			for(uint32 i=startSearchAddr,j=0;i<KERNEL_HEAP_MAX,j<neededPages;i+=PAGE_SIZE,j++)
			{
			   struct FrameInfo* frame = get_frame_info(ptr_page_directory,i, &ptr_page_table);
			   if(frame==NULL)
				   foundedFreePages++;
			   else
				   break;
			}
			if(foundedFreePages==neededPages) //can expand
			{
				uint32 end=startSearchAddr+(PAGE_SIZE*neededPages);
				for(uint32 i=startSearchAddr;i<end;i+=PAGE_SIZE)
				{
				   struct FrameInfo *new_frame = NULL;
				   int ret=allocate_frame(&new_frame);
				   if(ret!=0)
					 return virtual_address;

				   ret=map_frame(ptr_page_directory, new_frame, i, PERM_WRITEABLE);
				    if(ret!=0)
				   	return virtual_address;
				}

				allocations[index].num_pages=newNoOfPages;
			}
			else
			{
				void*temp=kmalloc(new_size);
				if(temp==NULL)
				  return virtual_address;

				kfree(virtual_address);
				return temp;
			}



		  }
		else if(newNoOfPages<noOfPages)
		{
			uint32 pagesTobeFreed=noOfPages-newNoOfPages;
			uint32 start=startAdd+(newNoOfPages*PAGE_SIZE);
			uint32 end=startAdd+(noOfPages*PAGE_SIZE);

			for(uint32 i=start;i<end;i+=PAGE_SIZE)
			{
				unmap_frame(ptr_page_directory, i);
			}

			allocations[index].num_pages=newNoOfPages;
		}

		else
		{
		  return virtual_address;
		}


		}

		else //reallocate to block allocator
		{
			void* temp=alloc_block_FF(new_size);
			if(temp==NULL)
			  return virtual_address;
			kfree(virtual_address);
			return temp;
		}
	}




	else if((uint32)virtual_address>=hStart&&(uint32)virtual_address<segmentBr) /**Block Allocator**/
	{
		if(new_size<=DYN_ALLOC_MAX_BLOCK_SIZE) //still at block allocator
		{
			return realloc_block_FF(virtual_address,new_size);
		}
		else
		{
			void*temp=kmalloc(new_size);
			if(temp==NULL)
			 return virtual_address;

			free_block(virtual_address);

		}
	}
	return virtual_address;
}
