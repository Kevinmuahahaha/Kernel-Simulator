#include <vector>
#include <stdexcept>
#include <mutex>
#include "MemorySpace.h"
#include "ModuleBanker.h"
#include "DataStructures.h"
#include "utils.h"


template<typename Type>
bool MemorySpace::values_push_back(std::mutex *vector_mutex, std::vector<Type*> &destination, const int maxLength, Type *appendData){
	try{
		//std::lock_guard<std::mutex> lk(*vector_mutex);
		std::unique_lock<std::mutex> guard(*vector_mutex);
		if( destination.size() >= maxLength ){
			guard.unlock();
		       	return false;
		}
		destination.push_back(appendData);
		guard.unlock();
	}
	catch( ... ){
		utils::log("error","MemorySpace","Exception when pushing into a task queue.");
		return false;
	}
	return true;
}

// returns a vector of (pointers to) pop-ed PCBs
// pop as much as the vector can. Shouldn't get any error.
// Inclusive pop
// e.g.  pop( 0, 0 ) only [0] will be poped.
// e.g.  pop( 0, 1 ) [0], [1], will all be poped.
template<typename Type>
std::vector<Type*> MemorySpace::values_pop_index(std::mutex *vector_mutex, std::vector<Type*> &destination, const int index_begin, const int index_end){
	std::vector<PCB*> ret;
	try{
		//std::lock_guard<std::mutex> lk(*vector_mutex);
		std::unique_lock<std::mutex> guard(*vector_mutex);
		int destination_size = destination.size();
		int actual_end_index = 0;
		int i = 0;

		for(
				i=index_begin; 
				i < destination_size && i <= index_end; 
				i++
				){
					ret.push_back( destination[i] );
				}

		actual_end_index = i;

		if( (actual_end_index - index_begin) > 0 ){
			destination.erase(destination.begin()+index_begin, destination.begin()+actual_end_index);
		}
		guard.unlock();

	}
	catch( ... ){
		utils::log("error","MemorySpace","Exception when erasing a task.");
		return ret;
	}
	return ret;
}

// each queue has it's own set of 
// push, pull functions.
// For the ease of unified coding.
//
// Operations for ready Queue:
bool MemorySpace::readyQueue_push_back(PCB* pcb){
	bool status =  MemorySpace::values_push_back<PCB>( 
			MemorySpace::mutex_readyQueue,
			MemorySpace::readyQueue,
			MemorySpace::readyQueueMaxLength,
			pcb);
	if( status ){
		MemorySpace::readyQueueCurrentLength ++;
	}
	return status;
}

bool MemorySpace::blockedQueue_push_back(PCB* pcb, const std::vector<int> blocked_request){

	/* mutex on blocked queue. */

	std::unique_lock<std::mutex> guard(*MemorySpace::mutex_blockedOperation);

	bool status =  MemorySpace::values_push_back<PCB>( 
			MemorySpace::mutex_blockedQueue,
			MemorySpace::blockedQueue,
			MemorySpace::blockedQueueMaxLength,
			pcb);

	Item* item = new Item();
	item->id = pcb->PID;
	item->request = blocked_request;

	/* push to an "Item" queue, for ModuleBanker's convenience. */
	bool status_item = MemorySpace::values_push_back<Item>( 
			MemorySpace::mutex_blockedQueue,
			MemorySpace::blockedItemQueue,
			MemorySpace::blockedQueueMaxLength,
			item);

	if( status && status_item ){
		MemorySpace::blockedQueueCurrentLength ++;
	}
	else{
		utils::log("crit","MemorySpace","Blocked queue and Blocked item queue not align.");
	}

	guard.unlock();

	return status;
}


bool MemorySpace::runningQueue_push_back(PCB* pcb){
	bool status =  MemorySpace::values_push_back<PCB>( 
			MemorySpace::mutex_runningQueue,
			MemorySpace::runningQueue,
			MemorySpace::runningQueueMaxLength,
			pcb);
	if( status ){
		MemorySpace::runningQueueCurrentLength ++;
	}
	return status;
}


template<typename Type>
std::vector<Type*> MakeVector(){
	utils::log("info","MemorySpace","Initializing task queue...");
	std::vector<Type*> v;
	return v;
}

/* shares the same mutex, between push_back and pull
	this means:
	1. vector being written, non shall pull
	2. vector being read, non shall write

   returns actually copied length.


   Why does "push" increase CurrentLength,
   while "pull" doesn't decrease it?

   Because "pull" doesn't necessarily want to delete something,
   could be just taking a look, or sort it, or anything.

*/
std::vector<PCB*> MemorySpace::readyQueue_pull(int length){
	std::vector<PCB*> ret;

	ret = MemorySpace::values_pop_index<PCB>( 
			MemorySpace::mutex_readyQueue,
			MemorySpace::readyQueue,
			0,
			length-1);
	return ret;
}


std::vector<PCB*> MemorySpace::runningQueue_pull(int length){
	// should return only when with Core=1
	std::vector<PCB*> ret;

	ret = MemorySpace::values_pop_index<PCB>( 
			MemorySpace::mutex_runningQueue,
			MemorySpace::runningQueue,
			0,
			length-1);

	return ret;
}


// private static members initialization
std::vector<PCB*> MemorySpace::readyQueue = MakeVector<PCB>();
std::vector<PCB*> MemorySpace::runningQueue = MakeVector<PCB>();
std::vector<PCB*> MemorySpace::blockedQueue = MakeVector<PCB>();
std::vector<Item*> MemorySpace::blockedItemQueue = MakeVector<Item>();


// pointers to mutex,
// to make pushing-back into a vector thread-safe.
std::mutex * MemorySpace::mutex_readyQueue = new std::mutex();
std::mutex * MemorySpace::mutex_runningQueue = new std::mutex();
std::mutex * MemorySpace::mutex_blockedQueue = new std::mutex();
std::mutex * MemorySpace::mutex_blockedOperation = new std::mutex();


int MemorySpace::runningQueueCurrentLength = 0;
int MemorySpace::readyQueueCurrentLength   = 0;
int MemorySpace::blockedQueueCurrentLength = 0;




PCB* MemorySpace::blockedQueue_pop_id( int id ){
	// DO NOT LOCK WITH MUTEX
	/*
	 *	only called in CodeExecutor::reviveBlocked().
	 *	the calling function already locks the mutex
	 *
	 * */
	PCB* ret = nullptr;
	for(int i=0; i<MemorySpace::blockedQueue.size(); i++){
		if(MemorySpace::blockedQueue[i]->PID == id){
			ret = MemorySpace::blockedQueue[ i ];

			MemorySpace::blockedQueue.erase( MemorySpace::blockedQueue.begin() + i );
			MemorySpace::blockedItemQueue.erase( MemorySpace::blockedItemQueue.begin() + i );
		}
	}
	return ret;
}
