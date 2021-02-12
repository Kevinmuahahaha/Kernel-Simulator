#ifndef INCLUDE_MEMORY_SPACE
#define INCLUDE_MEMORY_SPACE
#include <vector>
#include <mutex>
#include "DataStructures.h"
class MemorySpace{
	public:
		// push into readyQueue, called by HighLevelScheduler
		static bool readyQueue_push_back(PCB* pcb);
		static bool runningQueue_push_back(PCB* pcb);
		static bool blockedQueue_push_back(PCB* pcb, const std::vector<int> blocked_request);
		
		// pull current readyQueue, "length" provided
		/*
		 *	when readyQueue.size >= length
		 *	return vector.size = length
		 *
		 *	when readyQueue.size < length
		 *	return vector.size = readyQueue.size
		 *
		 * */
		static std::vector<PCB*> readyQueue_pull(int length);
		static std::vector<PCB*> runningQueue_pull(int length);

		// Pushing into such queue, when it's length >= maxLength,
		// will be rejected (returns false)
		static const int runningQueueMaxLength	= 1; // =1, single core
		static const int readyQueueMaxLength 	= 5;
		static const int blockedQueueMaxLength	= 5;

		static int runningQueueCurrentLength;
		static int readyQueueCurrentLength;
		static int blockedQueueCurrentLength;
		static PCB* blockedQueue_pop_id( int id );

		static std::mutex * mutex_blockedOperation; 
		static std::vector<Item*> blockedItemQueue; // for banker's convenience
		/*
		 * when Banker is editing tables.
		 * when Items are being appended to blocked queue.
		 *
		 * */


	private:

		static std::vector<PCB*> readyQueue;
		static std::vector<PCB*> runningQueue;
		static std::vector<PCB*> blockedQueue;

		static std::mutex * mutex_readyQueue;
		static std::mutex * mutex_runningQueue;

		static std::mutex * mutex_blockedQueue;	    // unecessary, but for cleaner codes.


		template<typename Type>
		static bool values_push_back(std::mutex *vector_mutex, std::vector<Type*> &destination, const int maxLength, Type *appendData);
		/*
		 thread-safe implementation of push_back
		 true: push success
		 false: something went wrong.
		 Inclusive erase [begin, end]:
		 erase( 0, 0 ) only [0] will be erased
		 erase( 0, 1 ) [0],[1] will be erased
		*/
		template<typename Type>
		static std::vector<Type*> values_pop_index(std::mutex *vector_mutex, std::vector<Type*> &destination, const int index_begin, const int index_end);
};
#endif 
