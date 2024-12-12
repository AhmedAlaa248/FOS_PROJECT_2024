// User-level Semaphore

#include "inc/lib.h"

struct semaphore create_semaphore(char *semaphoreName, uint32 value)
{
	//TODO: [PROJECT'24.MS3 - #02] [2] USER-LEVEL SEMAPHORE - create_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("create_semaphore is not implemented yet");
	//Your Code is Here...

	struct semaphore sem;
		struct __semdata s;

		strcpy(s.name,semaphoreName);
		s.count=value;
		s.lock=0;


		int size = sizeof(struct __semdata);
		struct __semdata *temp=smalloc(semaphoreName,size,1);
		*temp=s;
		sem.semdata=temp;

		return sem;

}
struct semaphore get_semaphore(int32 ownerEnvID, char* semaphoreName)
{
	//TODO: [PROJECT'24.MS3 - #03] [2] USER-LEVEL SEMAPHORE - get_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("get_semaphore is not implemented yet");
	//Your Code is Here...
	struct semaphore shared_semaphore;
	shared_semaphore.semdata = NULL;

	void * VA = sget(ownerEnvID, semaphoreName);

	if(VA == NULL) {
		return shared_semaphore;
	}

	shared_semaphore.semdata = (struct __semdata*)VA;

	return shared_semaphore;
}

void wait_semaphore(struct semaphore sem)
{
	//TODO: [PROJECT'24.MS3 - #04] [2] USER-LEVEL SEMAPHORE - wait_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("wait_semaphore is not implemented yet");
	//Your Code is Here...
	while(xchg(&(sem.semdata->lock), 1) != 0);

		if (sem.semdata->count>0)
		{
		  (sem.semdata->count)--;
		}
		else  if (sem.semdata->count==0)
		{
			struct Env_Queue* queue=&(sem.semdata->queue);
			uint32*ptrLock=&(sem.semdata->lock);
			sys_enqueue(queue,ptrLock);
		}

		sem.semdata->lock=0;

}

void signal_semaphore(struct semaphore sem)
{
	//TODO: [PROJECT'24.MS3 - #05] [2] USER-LEVEL SEMAPHORE - signal_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("signal_semaphore is not implemented yet");
	//Your Code is Here...
	while(xchg(&(sem.semdata->lock), 1) != 0);

		if(sem.semdata->count==0&&sem.semdata->queue.size>0)//there is blocked process
		{
			struct Env_Queue* queue=&(sem.semdata->queue);
			sys_dequeue(queue);
		}
		else
		{
		  (sem.semdata->count)++;
		}

		sem.semdata->lock=0;
}

int semaphore_count(struct semaphore sem)
{
	return sem.semdata->count;
}
