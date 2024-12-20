#include <inc/memlayout.h>
#include "shared_memory_manager.h"

#include <inc/mmu.h>
#include <inc/error.h>
#include <inc/string.h>
#include <inc/assert.h>
#include <inc/queue.h>
#include <inc/environment_definitions.h>

#include <kern/proc/user_environment.h>
#include <kern/trap/syscall.h>
#include "kheap.h"
#include "memory_manager.h"

//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//
struct Share* get_share(int32 ownerID, char* name);

//===========================
// [1] INITIALIZE SHARES:
//===========================
//Initialize the list and the corresponding lock
void sharing_init()
{
#if USE_KHEAP
	LIST_INIT(&AllShares.shares_list) ;
	init_spinlock(&AllShares.shareslock, "shares lock");
#else
	panic("not handled when KERN HEAP is disabled");
#endif
}

//==============================
// [2] Get Size of Share Object:
//==============================
int getSizeOfSharedObject(int32 ownerID, char* shareName)
{
	//[PROJECT'24.MS2] DONE
	// This function should return the size of the given shared object
	// RETURN:
	//	a) If found, return size of shared object
	//	b) Else, return E_SHARED_MEM_NOT_EXISTS
	//
	struct Share* ptr_share = get_share(ownerID, shareName);
	if (ptr_share == NULL)
		return E_SHARED_MEM_NOT_EXISTS;
	else
		return ptr_share->size;

	return 0;
}

//===========================================================


//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//
//===========================
// [1] Create frames_storage:
//===========================
// Create the frames_storage and initialize it by 0
inline struct FrameInfo** create_frames_storage(int numOfFrames)
{
	//TODO: [PROJECT'24.MS2 - #16] [4] SHARED MEMORY - create_frames_storage()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("create_frames_storage is not implemented yet");
	//Your Code is Here...

	unsigned int size = numOfFrames * sizeof(struct FrameInfo*);

	struct FrameInfo** frames_storage = (struct FrameInfo**) kmalloc(size);

	if(frames_storage == NULL)
		return NULL;

    for (int i = 0; i < numOfFrames; i++)
        frames_storage[i] = 0;

	return frames_storage;
}

//=====================================
// [2] Alloc & Initialize Share Object:
//=====================================
//Allocates a new shared object and initialize its member
//It dynamically creates the "framesStorage"
//Return: allocatedObject (pointer to struct Share) passed by reference
struct Share* create_share(int32 ownerID, char* shareName, uint32 size, uint8 isWritable)
{
	//TODO: [PROJECT'24.MS2 - #16] [4] SHARED MEMORY - create_share()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("create_share is not implemented yet");
	//Your Code is Here...

	unsigned int structSize = sizeof(struct Share);
//	structSize = ROUNDUP(structSize,PAGE_SIZE);

	struct Share * share_obj = (struct Share*) kmalloc(structSize);

	if(share_obj == NULL)
		return NULL;

	share_obj->ID = (int)share_obj & 0x7FFFFFFF;
	share_obj->ownerID = ownerID;
	strcpy(share_obj->name, shareName);
	share_obj->size = size;
	share_obj->references = 1;
	share_obj->isWritable = isWritable;

	int numOfFrames = ROUNDUP(size,PAGE_SIZE)/PAGE_SIZE;
	share_obj->framesStorage = create_frames_storage(numOfFrames);

	if (share_obj->framesStorage == NULL) {
		kfree((void*)share_obj);
		return NULL;
	}

	return share_obj;
}

//=============================
// [3] Search for Share Object:
//=============================
//Search for the given shared object in the "shares_list"
//Return:
//	a) if found: ptr to Share object
//	b) else: NULL
struct Share* get_share(int32 ownerID, char* name)
{
	//TODO: [PROJECT'24.MS2 - #17] [4] SHARED MEMORY - get_share()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("get_share is not implemented yet");
	//Your Code is Here...

	struct Share * findshare;
	acquire_spinlock(&AllShares.shareslock);
	LIST_FOREACH(findshare, &(AllShares.shares_list)){
		if(findshare->ownerID == ownerID
		&& strcmp(findshare->name, name) == 0){
			release_spinlock(&AllShares.shareslock);
			return findshare;
		}
	}
	release_spinlock(&AllShares.shareslock);
	return NULL;
}

//=========================
// [4] Create Share Object:
//=========================
int createSharedObject(int32 ownerID, char* shareName, uint32 size, uint8 isWritable, void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #19] [4] SHARED MEMORY [KERNEL SIDE] - createSharedObject()
    //COMMENT THE FOLLOWING LINE BEFORE START CODING
    //panic("createSharedObject is not implemented yet");
    //Your Code is Here...

	struct Env* myenv = get_cpu_proc(); //The calling environment

	int numOfFrames =ROUNDUP(size,PAGE_SIZE)/PAGE_SIZE;
	uint32* ptr_page_table;

	if(get_share(ownerID,shareName) != NULL)
	{

		return E_SHARED_MEM_EXISTS;
	}
	acquire_spinlock(&AllShares.shareslock);
	struct Share* sharedObject = create_share(ownerID,shareName,size,isWritable);

	uint32 pcount=0;
	size = ROUNDUP(size,PAGE_SIZE);
	for (uint32 i = (uint32)virtual_address ;i < (uint32)virtual_address + size ; i+=PAGE_SIZE ){
		struct FrameInfo * newframe;
		if(allocate_frame(&newframe) != 0)
		{
			release_spinlock(&AllShares.shareslock);
			return E_NO_SHARE;
		}

		if(map_frame(myenv->env_page_directory, newframe, i, PERM_MARKED|PERM_USER|PERM_WRITEABLE) != 0)
		{

			unmap_frame(myenv->env_page_directory,i);
			release_spinlock(&AllShares.shareslock);
			return E_NO_SHARE;
		}

		sharedObject->framesStorage[pcount] = newframe;
		pcount++;
	}


		LIST_INSERT_TAIL(&AllShares.shares_list,sharedObject);
	release_spinlock(&AllShares.shareslock);
	return sharedObject->ID;
}

//======================
// [5] Get Share Object:
//======================

int getSharedObject(int32 ownerID, char* shareName, void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #21] [4] SHARED MEMORY [KERNEL SIDE] - getSharedObject()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("getSharedObject is not implemented yet");
	//Your Code is Here...

	struct Env* myenv = get_cpu_proc(); //The calling environment

	struct Share * current_obj = get_share(ownerID, shareName);

	if(current_obj == NULL){
		return E_SHARED_MEM_NOT_EXISTS;
	}

	int perm = (current_obj->isWritable)? (PERM_MARKED|PERM_USER|PERM_WRITEABLE) : (PERM_MARKED|PERM_USER);

	uint32 framesNum = ROUNDUP(current_obj->size, PAGE_SIZE) /PAGE_SIZE;

//	cprintf("\n\nNUmbe of\t\t\t allocated: %d\n\n",framesNum);

	for (uint32 i = 0; i < framesNum; i++){
		//Won't we update frameInfo refrences ?
		int ret = map_frame(myenv->env_page_directory, current_obj->framesStorage[i], (uint32)virtual_address, perm);

		if (ret != 0){
			cprintf("Error: Failed to map frame %d to virtual address 0x%x\n", i,(uint32)virtual_address);
		}


		virtual_address += PAGE_SIZE;
	}

	current_obj->references ++;
	return current_obj->ID;

}

//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//==========================
// [B1] Delete Share Object:
//==========================
//delete the given shared object from the "shares_list"
//it should free its framesStorage and the share object itself
void free_share(struct Share* ptrShare)
{
	//TODO: [PROJECT'24.MS2 - BONUS#4] [4] SHARED MEMORY [KERNEL SIDE] - free_share()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	panic("free_share is not implemented yet");
	//Your Code is Here...
	struct Share * checkshare;
	LIST_FOREACH(checkshare, &(AllShares.shares_list)){
			if(checkshare == ptrShare){
				LIST_REMOVE(&(AllShares.shares_list), ptrShare);
			}
		}
	kfree((int*)ptrShare->framesStorage);
	kfree((int*)ptrShare);


}
//========================
// [B2] Free Share Object:
//========================
int freeSharedObject(int32 sharedObjectID, void *startVA)
{
	//TODO: [PROJECT'24.MS2 - BONUS#4] [4] SHARED MEMORY [KERNEL SIDE] - freeSharedObject()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	panic("freeSharedObject is not implemented yet");
	//Your Code is Here...

}
