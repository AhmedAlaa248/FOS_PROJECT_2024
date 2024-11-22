/*
 * dynamic_allocator.c
 *
 *  Created on: Sep 21, 2023
 *      Author: HP
 */
#include <inc/assert.h>
#include <inc/string.h>
#include "../inc/dynamic_allocator.h"


//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

//=====================================================
// 1) GET BLOCK SIZE (including size of its meta data):
//=====================================================
__inline__ uint32 get_block_size(void* va)
{
	uint32 *curBlkMetaData = ((uint32 *)va - 1) ;
	return (*curBlkMetaData) & ~(0x1);
}

//===========================
// 2) GET BLOCK STATUS:
//===========================
__inline__ int8 is_free_block(void* va)
{
	uint32 *curBlkMetaData = ((uint32 *)va - 1) ;
	return (~(*curBlkMetaData) & 0x1) ;
}

//===========================
// 3) ALLOCATE BLOCK:
//===========================

void *alloc_block(uint32 size, int ALLOC_STRATEGY)
{
	void *va = NULL;
	switch (ALLOC_STRATEGY)
	{
	case DA_FF:
		va = alloc_block_FF(size);
		break;
	case DA_NF:
		va = alloc_block_NF(size);
		break;
	case DA_BF:
		va = alloc_block_BF(size);
		break;
	case DA_WF:
		va = alloc_block_WF(size);
		break;
	default:
		cprintf("Invalid allocation strategy\n");
		break;
	}
	return va;
}

//===========================
// 4) PRINT BLOCKS LIST:
//===========================

void print_blocks_list(struct MemBlock_LIST list)
{
	cprintf("=========================================\n");
	struct BlockElement* blk ;
	cprintf("\nDynAlloc Blocks List:\n");
	LIST_FOREACH(blk, &list)
	{
		cprintf("(size: %d, isFree: %d)\n", get_block_size(blk), is_free_block(blk)) ;
	}
	cprintf("=========================================\n");

}
//
////********************************************************************************//
////********************************************************************************//

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

bool is_initialized = 0;
//==================================
// [1] INITIALIZE DYNAMIC ALLOCATOR:
//==================================
void initialize_dynamic_allocator(uint32 daStart, uint32 initSizeOfAllocatedSpace)
{
	//==================================================================================
	//DON'T CHANGE THESE LINES==========================================================
	//==================================================================================
	{
		if (initSizeOfAllocatedSpace % 2 != 0) initSizeOfAllocatedSpace++; //ensure it's multiple of 2
		if (initSizeOfAllocatedSpace == 0)
			return ;
		is_initialized = 1;
	}
	//==================================================================================
	//==================================================================================

	//TODO: [PROJECT'24.MS1 - #04] [3] DYNAMIC ALLOCATOR - initialize_dynamic_allocator
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("initialize_dynamic_allocator is not implemented yet");
	//Your Code is Here...

	uint32* dynamicAllocatorBeg=(uint32*) daStart;
	uint32* dynamicAllocatorEnd=(uint32*) (daStart+initSizeOfAllocatedSpace-sizeof(int));
	uint32* blkHeader=(uint32*) (daStart+sizeof(int));
	uint32* blkFooter=(uint32*) (daStart+initSizeOfAllocatedSpace-2*sizeof(int));
	*dynamicAllocatorBeg=1;
	*dynamicAllocatorEnd=1;
	*blkFooter=initSizeOfAllocatedSpace - 2*sizeof(int);
	*blkHeader=*blkFooter;
	struct BlockElement* FirstBlock=(struct BlockElement*)(daStart+2*sizeof(int));
	LIST_INIT(&freeBlocksList);
	LIST_INSERT_HEAD(&freeBlocksList,FirstBlock);
	cprintf("\n");

}
//==================================
// [2] SET BLOCK HEADER & FOOTER:
//==================================
void set_block_data(void* va, uint32 totalSize, bool isAllocated)
{
	//TODO: [PROJECT'24.MS1 - #05] [3] DYNAMIC ALLOCATOR - set_block_data
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("set_block_data is not implemented yet");
	//Your Code is Here...

	uint32* header = (uint32*)(va-sizeof(int));//points to header
	*header=totalSize; // set header with size value
	uint32* footer = (uint32*)(va+totalSize-(2*sizeof(int)));//points to footer
	*footer=totalSize; // set footer with size value

	if(isAllocated)
	{
		*header |= 0x1;
		*footer |= 0x1;
	}

	else
	{
		*header &= ~0x1;
		*footer &= ~0x1;
	}
}


//=========================================
// [3] ALLOCATE BLOCK BY FIRST FIT:
//=========================================
void *alloc_block_FF(uint32 size)
{
	//==================================================================================
	//DON'T CHANGE THESE LINES==========================================================
	//==================================================================================
	{
		if (size % 2 != 0) size++;	//ensure that the size is even (to use LSB as allocation flag)
		if (size < DYN_ALLOC_MIN_BLOCK_SIZE)
			size = DYN_ALLOC_MIN_BLOCK_SIZE ;
		if (!is_initialized)
		{
			uint32 required_size = size + 2*sizeof(int) /*header & footer*/ + 2*sizeof(int) /*da begin & end*/ ;
			uint32 da_start = (uint32)sbrk(ROUNDUP(required_size, PAGE_SIZE)/PAGE_SIZE);
			uint32 da_break = (uint32)sbrk(0);
			initialize_dynamic_allocator(da_start, da_break - da_start);
		}
	}
	//==================================================================================
	//==================================================================================

	//TODO: [PROJECT'24.MS1 - #06] [3] DYNAMIC ALLOCATOR - alloc_block_FF
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("alloc_block_FF is not implemented yet");
	//Your Code is Here...

	if(size<8||size==0) // it is not possible
		return NULL;

	uint32 total_size=size+8; // footer+header+needed size
    struct BlockElement *ptr;
    void *temp;

	LIST_FOREACH(ptr,&freeBlocksList)
	{
		uint32 x = get_block_size(ptr);
		if(x>=total_size) //founded a suitable block
		{
			break;
		}
	}

	if(ptr==NULL) // if no block is free in the list , or no suitable free block
	{
		void * result = sbrk(1);
		if(result==(void*)-1)
			return NULL;
		else
		{
			set_block_data(result,PAGE_SIZE,1);
			void*temp=(char*)result+PAGE_SIZE-4;
			uint32* endBlock=(uint32*)temp;
			*endBlock=1;
			free_block(result);

			ptr=LIST_LAST(&freeBlocksList);
		}

	}

	uint32 size_of_founded_free_block=get_block_size(ptr);
	uint32 size_of_new_freeblock=size_of_founded_free_block-total_size; //rest of free block

	if(size_of_new_freeblock>=16) // no internal fragmentation
	{
    set_block_data(ptr,total_size,1); // setting the data for the allocated block

	temp = (char*)ptr+total_size;

    set_block_data(temp,size_of_new_freeblock,0);

	struct BlockElement* blockelement=(struct BlockElement*)(temp);


	LIST_INSERT_AFTER(&freeBlocksList,ptr,blockelement);
	LIST_REMOVE(&freeBlocksList,ptr);
	}

	else
	{
		set_block_data(ptr,size_of_founded_free_block,1);
		LIST_REMOVE(&freeBlocksList,ptr);
	}


	return (void*) ptr;
}
//=========================================
// [4] ALLOCATE BLOCK BY BEST FIT:
//=========================================
void *alloc_block_BF(uint32 size)
{

	{
		if (size % 2 != 0) size++;	//ensure that the size is even (to use LSB as allocation flag)
		if (size < DYN_ALLOC_MIN_BLOCK_SIZE)
			size = DYN_ALLOC_MIN_BLOCK_SIZE ;
		if (!is_initialized)
		{
			uint32 required_size = size + 2*sizeof(int) /*header & footer*/ + 2*sizeof(int) /*da begin & end*/ ;
			uint32 da_start = (uint32)sbrk(ROUNDUP(required_size, PAGE_SIZE)/PAGE_SIZE);
			uint32 da_break = (uint32)sbrk(0);
			initialize_dynamic_allocator(da_start, da_break - da_start);
		}
	}


	//TODO: [PROJECT'24.MS1 - BONUS] [3] DYNAMIC ALLOCATOR - alloc_block_BF
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("alloc_block_BF is not implemented yet");
	//Your Code is Here...

	if(size<8||size==0)
		return NULL;

		uint32 total_size=size+8;
		struct BlockElement *ptr;
		struct BlockElement *temp;
	    uint32 difference;

		LIST_FOREACH(ptr,&freeBlocksList)
			{
				uint32 x=get_block_size(ptr);
				if(x>=total_size)
				{	difference=x-total_size;
					break;
				}
			}

		if(ptr==NULL)
		{
			void * result = sbrk(1);
					if(result==(void*)-1)
						return NULL;
					else
						return alloc_block_BF(size);
		}

		LIST_FOREACH(ptr,&freeBlocksList)
			{
				uint32 x=get_block_size(ptr);
				if(x<total_size)
					continue;
				else
				{
					if(x-total_size<=difference)
					{
						difference=x-total_size;
						temp=ptr;
					}
				}
			}


		uint32 size_of_founded_block=get_block_size(temp);
		uint32 remaining_block_size=size_of_founded_block-total_size;
		if(remaining_block_size>=16)
		{
			set_block_data(temp,total_size,1);
			void*temp2=(char*)temp+total_size;
			set_block_data(temp2,remaining_block_size,0);
			struct BlockElement* new_free_block=(struct BlockElement*)temp2;
			LIST_INSERT_AFTER(&freeBlocksList,temp,new_free_block);
			LIST_REMOVE(&freeBlocksList,temp);

		}

		else
		{
			set_block_data(temp,size_of_founded_block,1);
			LIST_REMOVE(&freeBlocksList,temp);
		}

		return (void*)temp;
}
//===================================================
// [5] FREE BLOCK WITH COALESCING:
//===================================================
void free_block(void *va)
{
	//TODO: [PROJECT'24.MS1 - #07] [3] DYNAMIC ALLOCATOR - free_block
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("free_block is not implemented yet");
	//Your Code is Here...
	if(va==NULL)
		return;

	uint32 size_free_block=get_block_size(va);

	//Before Block
	void*temp1 = (char*)va -4; //points before footer
	uint32 size_befor_block = get_block_size(temp1);
	temp1 = (char*)temp1 - size_befor_block+4;
	int8 res1 = is_free_block(temp1);


	//After BLock
	void* temp2 = (char*)va + size_free_block;
	uint32 size_after_block = get_block_size(temp2);
	int8 res2 = is_free_block(temp2);

	//First Case if both are allocated
	if(res1==0&&res2==0)
	{
		set_block_data(va,size_free_block,0);
		struct BlockElement *ptr;
		struct BlockElement* blockelement=(struct BlockElement*)(va);

		LIST_FOREACH(ptr,&freeBlocksList)
		{
			if(ptr>blockelement)
				break;
		}

		if(ptr==NULL)
		{
			LIST_INSERT_TAIL(&freeBlocksList,blockelement);
		}
		else
		{
			LIST_INSERT_BEFORE(&freeBlocksList,ptr,blockelement);
		}
	}

	//Second Case if both are Free
	else if(res1==1&&res2==1)
	{
		uint32 total_size=size_befor_block+size_free_block+size_after_block;
		set_block_data(temp1,total_size,0);
		struct BlockElement* ptr=(struct BlockElement*)temp2;//block element pointer to after block to delete it from list
		LIST_REMOVE(&freeBlocksList,ptr);


	}

	//Third Case if before is Allocated and after is free
	else if(res1==0&&res2==1)
	{
		uint32 total_size=size_free_block+size_after_block;
		set_block_data(va,total_size,0);
		struct BlockElement* ptr=(struct BlockElement*)va;
		struct BlockElement* after_block=(struct BlockElement*)temp2;
		LIST_INSERT_BEFORE(&freeBlocksList,after_block,ptr);
		LIST_REMOVE(&freeBlocksList,after_block);
	}

	//Fourth case if before is free and after is Allocated
	else
	{
		uint32 total_size=size_befor_block+size_free_block;
		set_block_data(temp1,total_size,0);
	}

}

void forsplitting(void* va,uint32 new_size,uint32 sizeOfCurrBlock,uint32 remainingSize){
	if (remainingSize >= DYN_ALLOC_MIN_BLOCK_SIZE)
	{

		set_block_data(va, new_size, 1);

		void* newFreeBlock = (void*)((uint32)va + new_size);

		set_block_data(newFreeBlock, remainingSize, 0);
		LIST_INSERT_HEAD(&freeBlocksList, (struct BlockElement*)newFreeBlock);
	}
}

//=========================================
// [6] REALLOCATE BLOCK BY FIRST FIT:
//=========================================
void *realloc_block_FF(void* va, uint32 new_size)
{
	//TODO: [PROJECT'24.MS1 - #08] [3] DYNAMIC ALLOCATOR - realloc_block_FF
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("realloc_block_FF is not implemented yet");
	//Your Code is Here...

    if (va == NULL && new_size == 0)
    {
        alloc_block_FF(0);
        cprintf("VA NULL & SIZE = 0\n");
        return NULL;
    }


    if (new_size == 0)
    {
        free_block(va);
        cprintf("SIZE = 0\n");
        return NULL;
    }


    if (va == NULL)
    {
    	cprintf("VA NULL\n");
        return (void*)alloc_block_FF(new_size);
    }



    uint32 sizeOfCurrBlock = get_block_size(va);
    new_size = ROUNDUP(new_size + 2 * sizeof(int), 4);


    if (new_size <= sizeOfCurrBlock)
    {
    	uint32 remainingSize = sizeOfCurrBlock - new_size;
    	forsplitting(va, new_size, sizeOfCurrBlock,remainingSize);
    	//cprintf("NEW size less than or equal size\n");
        return (void*)va;
    }
    if (new_size > sizeOfCurrBlock)
    {
        void* afterVA = (void*)((uint32)va + sizeOfCurrBlock);
        if ((uint32)afterVA % sizeof(int) != 0) {
            afterVA = (void*)((uint32)afterVA + sizeof(int));
        }
        uint32 sizeOfafterBlock = get_block_size(afterVA);
        int8 isafterBlockFree = is_free_block(afterVA);


        if (isafterBlockFree == 1 && (sizeOfCurrBlock + sizeOfafterBlock >= new_size))
        {

            LIST_REMOVE(&freeBlocksList, (struct BlockElement*)afterVA);


            set_block_data(va, new_size, 1);


            uint32 remainingSize = (sizeOfCurrBlock + sizeOfafterBlock) - new_size;

            if (remainingSize >= DYN_ALLOC_MIN_BLOCK_SIZE)
            {

                void* newFreeBlock = (void*)((uint32)va + new_size);
                set_block_data(newFreeBlock, remainingSize, 0);

                LIST_INSERT_HEAD(&freeBlocksList, (struct BlockElement*)newFreeBlock);
            }
            //cprintf("NEW size > size and next is free\n");
            return (void*)va;
        }
        else if(isafterBlockFree == 0 && (sizeOfCurrBlock + sizeOfafterBlock >= new_size))
        {
        	void* ret = alloc_block_FF(new_size);
        	//free_block(va);
        	va=ret;
        	//NOT HANDLED IN TESTS
        	cprintf("NEW size > size and next is not free\n");
        	return (void*)ret;
        }
        else
        {
        	void* ret = sbrk(new_size);
        	set_block_data(ret,new_size,1);
        	cprintf("SBRK\n");
        	return (void*)ret;
        }
    }

    cprintf("Nothing\n");
    return NULL;
}

/*********************************************************************************************/
/*********************************************************************************************/
/*********************************************************************************************/
//=========================================
// [7] ALLOCATE BLOCK BY WORST FIT:
//=========================================
void *alloc_block_WF(uint32 size)
{
	panic("alloc_block_WF is not implemented yet");
	return NULL;
}

//=========================================
// [8] ALLOCATE BLOCK BY NEXT FIT:
//=========================================
void *alloc_block_NF(uint32 size)
{
	panic("alloc_block_NF is not implemented yet");
	return NULL;
}
