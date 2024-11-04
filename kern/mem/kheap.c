#include "kheap.h"

#include <inc/memlayout.h>
#include <inc/dynamic_allocator.h>
#include "memory_manager.h"

//Initialize the dynamic allocator of kernel heap with the given start address, size & limit
//All pages in the given range should be allocated
//Remember: call the initialize_dynamic_allocator(..) to complete the initialization
//Return:
//	On success: 0
//	Otherwise (if no memory OR initial size exceed the given limit): PANIC
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
	 {
	   return (void*)-1;
	 }


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

	 }

	 void *oldBreak=(void*)segmentBr;
	 void*ptr=(void*)segmentBr;
	 set_block_data(ptr,size,1);
	 segmentBr+=size;

	 uint32 *newEndBlock=(uint32*)segmentBr-1;
	 *newEndBlock=1;

	 free_block(ptr);

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

	    if (size == 0) {
	        return NULL;
	    }


	    if (size <= DYN_ALLOC_MAX_BLOCK_SIZE) {
	        return alloc_block_FF(size);

	    }


	    uint32 numPages = (ROUNDUP(size, PAGE_SIZE)) / PAGE_SIZE;
	//    cprintf("num of pages %d \n",numPages);
	    uint32 contiguous_pages = 0;
	    uint32 start_search_address = hLimit + PAGE_SIZE;
	    void* start_address = NULL;
	    uint32 *ptr_page_table = NULL;

	    for (uint32 i = start_search_address; i < KERNEL_HEAP_MAX; i += PAGE_SIZE) {
	        struct FrameInfo *frame = get_frame_info(ptr_page_directory, i, &ptr_page_table);

	        if (frame == NULL) {
	            if (contiguous_pages == 0) {
	                start_address = (void*)i;
	            }
	            contiguous_pages++;


	            if (contiguous_pages == numPages) {
	                break;
	            }
	        }
	          else {
	            contiguous_pages = 0;
	            start_address = NULL;
	        }
	    }


	    if (contiguous_pages < numPages) {
	    //cprintf("Error: Not enough contiguous space found for %d pages\n", numPages);
	        return NULL;
	    }


	    uint32 allocated_pages = 0;
	    for (uint32 i = (uint32)start_address; allocated_pages < numPages; i += PAGE_SIZE) {
	        struct FrameInfo *new_frame = NULL;


	        if (allocate_frame(&new_frame) != 0) {
	            cprintf("Error: Frame allocation failed at address %p\n", (void*)i);
	            return NULL;
	        }


	        if (map_frame(ptr_page_directory, new_frame, i, PERM_WRITEABLE | PERM_PRESENT ) != 0) {
	            cprintf("Error: Frame mapping failed at address %p\n", (void*)i);
	            return NULL;
	        }

	        allocated_pages++;
	    }


	    cprintf("Allocated %d pages starting at address %p\n", numPages, start_address);

	    return start_address;

}

void kfree(void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #04] [1] KERNEL HEAP - kfree
	// Write your code here, remove the panic and write your code
	panic("kfree() is not implemented yet...!!");

	//you need to get the size of the given allocation using its address
	//refer to the project presentation and documentation for details

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

	uint32 offset = virtual_address&(0x00000FFF);
	uint32 Page_Index=PTX(virtual_address);
	uint32* ptr_page_table;
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
	//panic("kheap_virtual_address() is not implemented yet...!!");

	//return the virtual address corresponding to given physical_address
	//refer to the project presentation and documentation for details

	//EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================

	struct FrameInfo *frame;
	frame = to_frame_info(physical_address);
	if(frame==NULL)
		return 0;
	return frame->bufferedVA;

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
	return NULL;
	panic("krealloc() is not implemented yet...!!");
}
