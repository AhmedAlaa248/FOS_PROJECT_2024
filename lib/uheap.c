#include <inc/lib.h>

struct heapS {
	void * returnedVA;
	uint32 pagesNum;
	bool marked;
};

struct heapS pagesArray[NUM_OF_UHEAP_PAGES];

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//=============================================
// [1] CHANGE THE BREAK LIMIT OF THE USER HEAP:
//=============================================
/*2023*/
void* sbrk(int increment)
{
	return (void*) sys_sbrk(increment);
}

//=================================
// [2] ALLOCATE SPACE IN USER HEAP:
//=================================
void* malloc(uint32 size)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	if (size == 0) return NULL ;
	//==============================================================
	//TODO: [PROJECT'24.MS2 - #12] [3] USER HEAP [USER SIDE] - malloc()
	// Write your code here, remove the panic and write your code

	if(size <= DYN_ALLOC_MAX_BLOCK_SIZE)
		return alloc_block_FF(size);

	if(sys_isUHeapPlacementStrategyFIRSTFIT()){

		uint32 pageNumToAlloc = ROUNDUP(size , PAGE_SIZE) / PAGE_SIZE;
		uint32 freePages = 0;
		uint32 starti = (uint32) myEnv->hard_limit + PAGE_SIZE;

		void *startVA = (void *)starti;
		uint32 *ptr_page_table = NULL;
		uint32 page_number ;

		for (uint32 i = starti; i < USER_HEAP_MAX; i += PAGE_SIZE){
			page_number =(i - starti) / PAGE_SIZE;
			if (pagesArray[page_number].marked == 0){
				if(freePages == 0)
					startVA =(void*) i;

				freePages++;
				if(freePages == pageNumToAlloc) break;
			}else{
				freePages = 0;
				startVA = NULL;
			}
		}

		if(freePages < pageNumToAlloc)
			return NULL;

		for(uint32 i = (uint32) startVA; i < (uint32) startVA + (freePages * PAGE_SIZE) ; i+= PAGE_SIZE){
			page_number = (i - starti) / PAGE_SIZE;
			pagesArray[page_number].marked = 1;
		}

		page_number = ((uint32)startVA - starti) / PAGE_SIZE;
		pagesArray[page_number].returnedVA = startVA;
		pagesArray[page_number].pagesNum = freePages;

		sys_allocate_user_mem((uint32)startVA, pageNumToAlloc * PAGE_SIZE);
		return startVA;

	}

	return NULL;
	//Use sys_isUHeapPlacementStrategyFIRSTFIT() and	sys_isUHeapPlacementStrategyBESTFIT()
	//to check the current strategy

}

//=================================
// [3] FREE SPACE FROM USER HEAP:
//=================================
void free(void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #14] [3] USER HEAP [USER SIDE] - free()
	// Write your code here, remove the panic and write your code
//	panic("free() is not implemented yet...!!");
	if((uint32)virtual_address<myEnv->hard_limit){
				free_block(virtual_address);
	}
	else{
		uint32 numtobeunmarked = pagesArray[((uint32)virtual_address-(myEnv->hard_limit+PAGE_SIZE))/PAGE_SIZE].pagesNum;
		uint32 page_number;
		if (pagesArray[((uint32)virtual_address - (myEnv->hard_limit + PAGE_SIZE)) / PAGE_SIZE].returnedVA != NULL) {
		for(uint32 i = (uint32) virtual_address; i < (uint32) virtual_address + (numtobeunmarked * PAGE_SIZE) ; i+= PAGE_SIZE){
					page_number = (i -(myEnv->hard_limit+PAGE_SIZE) ) / PAGE_SIZE;
					pagesArray[page_number].marked = 0;
		}
		sys_free_user_mem((uint32)virtual_address,numtobeunmarked*PAGE_SIZE);
		page_number = ((uint32)virtual_address - (((myEnv->hard_limit)+PAGE_SIZE))) / PAGE_SIZE;
		pagesArray[page_number].returnedVA = NULL;
		pagesArray[page_number].pagesNum = 0;
		}
		else{
			panic("Invalid Memory yacta");
		}

	}
}


//=================================
// [4] ALLOCATE SHARED VARIABLE:
//=================================
void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	if (size == 0) return NULL ;
	//==============================================================
	//TODO: [PROJECT'24.MS2 - #18] [4] SHARED MEMORY [USER SIDE] - smalloc()
	// Write your code here, remove the panic and write your code
	//panic("smalloc() is not implemented yet...!!");

	/*	uint32 pagealloc_start = (uint32)myEnv->hard_limit + PAGE_SIZE;
		if(size > USER_HEAP_MAX - pagealloc_start - PAGE_SIZE)
		{
			cprintf("Size is bigger than the user heap");
			return NULL;
		}
		if(sys_isUHeapPlacementStrategyFIRSTFIT()){
		cprintf("smalloc started \n");
		uint32 numOfAllocatedFrames=0;
		uint32 numOfPagesToAlloc = ROUNDUP(size , PAGE_SIZE) / PAGE_SIZE;
		uint32 startAddr = pagealloc_start;
		uint32 pageCounter=0;
		void* startVAofAllocFrames;
		uint32 currPage;
		for(uint32 i =startAddr; i < USER_HEAP_MAX;i+=PAGE_SIZE)
		{
			currPage =(i - startAddr) / PAGE_SIZE;
						if (pagesArray[currPage].marked == 0){
							if(pageCounter == 0){
								startVAofAllocFrames =(void*) i;
							}
							pageCounter++;
							if(pageCounter == numOfPagesToAlloc)
								break;
							}
						else{
							pageCounter = 0;
							startVAofAllocFrames = NULL;
						}


		}
		if(pageCounter < numOfPagesToAlloc || startVAofAllocFrames==NULL)
		{
			cprintf("Not Enough Size \n");
			return NULL;
		}
		for(uint32 i = (uint32)startVAofAllocFrames;i < (uint32) startVAofAllocFrames + (pageCounter * PAGE_SIZE);i+=PAGE_SIZE)
		{
			currPage = (i - startAddr) / PAGE_SIZE;
			pagesArray[currPage].marked = 1;
		}




		int ret = sys_createSharedObject(sharedVarName,size,isWritable,(void*)startVAofAllocFrames);
		return (void*)startVAofAllocFrames;

}



	return NULL;*/
	uint32 pagealloc_start = (uint32) myEnv->hard_limit + PAGE_SIZE;

		cprintf("134\n");
	//	if(size > USER_HEAP_MAX - pagealloc_start - PAGE_SIZE)
	//	{
	//		cprintf("Size is bigger than the user heap");
	//		return NULL;
	//	}

		if(sys_isUHeapPlacementStrategyFIRSTFIT()){
			cprintf("smalloc started \n");

			uint32 numOfAllocatedFrames=0;
			uint32 numOfPagesToAlloc = ROUNDUP(size , PAGE_SIZE) / PAGE_SIZE;
			uint32 startAddr = pagealloc_start;
			uint32 pageCounter=0;
			void* startVAofAllocFrames;
			uint32 currPage;
			cprintf("150\n");
			for(uint32 i =startAddr; i < USER_HEAP_MAX;i+=PAGE_SIZE)
			{
				currPage =(i - startAddr) / PAGE_SIZE;
							if (pagesArray[currPage].marked == 0){
								if(pageCounter == 0){
									startVAofAllocFrames =(void*) i;
								}
								pageCounter++;
								if(pageCounter == numOfPagesToAlloc)
									break;
								}
							else{
								pageCounter = 0;
								startVAofAllocFrames = NULL;
							}


			}
			cprintf("169\n");
			if(pageCounter < numOfPagesToAlloc || startVAofAllocFrames==NULL)
			{
				cprintf("Not Enough Size \n");
				return NULL;
			}
			for(uint32 i = (uint32)startVAofAllocFrames;i < (uint32) startVAofAllocFrames + (pageCounter * PAGE_SIZE);i+=PAGE_SIZE)
			{
				currPage = (i - startAddr) / PAGE_SIZE;
				pagesArray[currPage].marked = 1;
			}
			cprintf("180\n");
			int ret = sys_createSharedObject(sharedVarName, size, isWritable, (void*)startVAofAllocFrames);
			cprintf("return of create shared object %d\n",ret);
			if(ret == E_SHARED_MEM_EXISTS || ret == E_NO_SHARE)
				return NULL;
			else{
				cprintf("186\n");
				return (void*)startVAofAllocFrames;
			}
		}

		return NULL;

}

//========================================
// [5] SHARE ON ALLOCATED SHARED VARIABLE:
//========================================
void* sget(int32 ownerEnvID, char *sharedVarName)
{
	//TODO: [PROJECT'24.MS2 - #20] [4] SHARED MEMORY [USER SIDE] - sget()
	// Write your code here, remove the panic and write your code
	panic("sget() is not implemented yet...!!");
	return NULL;
}


//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//=================================
// FREE SHARED VARIABLE:
//=================================
//	This function frees the shared variable at the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from main memory then switch back to the user again.
//
//	use sys_freeSharedObject(...); which switches to the kernel mode,
//	calls freeSharedObject(...) in "shared_memory_manager.c", then switch back to the user mode here
//	the freeSharedObject() function is empty, make sure to implement it.

void sfree(void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - BONUS#4] [4] SHARED MEMORY [USER SIDE] - sfree()
	// Write your code here, remove the panic and write your code
	//panic("sfree() is not implemented yet...!!");
	uint32 ID = (uint32)virtual_address & 0x7FFFFFFF;
	sys_freeSharedObject(ID, virtual_address);

}


//=================================
// REALLOC USER SPACE:
//=================================
//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to malloc().
//	A call with new_size = zero is equivalent to free().

//  Hint: you may need to use the sys_move_user_mem(...)
//		which switches to the kernel mode, calls move_user_mem(...)
//		in "kern/mem/chunk_operations.c", then switch back to the user mode here
//	the move_user_mem() function is empty, make sure to implement it.
void *realloc(void *virtual_address, uint32 new_size)
{
	//[PROJECT]
	// Write your code here, remove the panic and write your code
	panic("realloc() is not implemented yet...!!");
	return NULL;

}


//==================================================================================//
//========================== MODIFICATION FUNCTIONS ================================//
//==================================================================================//

void expand(uint32 newSize)
{
	panic("Not Implemented");

}
void shrink(uint32 newSize)
{
	panic("Not Implemented");

}
void freeHeap(void* virtual_address)
{
	panic("Not Implemented");

}
