#include "ProcessScheduler.h"
#include "DataStructures.h"
#include <vector>
#include <string>

std::vector<PCB*> ProcessScheduler::makePCBVector(){
	std::vector<PCB*> ret;
	return ret;
}

std::vector<PCB*> ProcessScheduler::mainQueue = makePCBVector();


void ProcessScheduler::run(){
	bool last_check = false;
	while( SystemStatus::isSystemRunning ){
		/* if system is still running ... */
		while( SystemStatus::isRunningEmpty ){
			/* if there's an empty slot in RunningQueue, 
			 * sort and append it. */

			std::vector<PCB*> queue = MemorySpace::readyQueue_pull( MemorySpace::readyQueueMaxLength );

			/* empty Running queue =
			 * empty slot in Ready queue ( soon )
			 *
			 * H.L.S may append one more job at this point.
			 * ( if there's any left )
			 * */

			for( int i=0; i<queue.size(); i++ ){
				ProcessScheduler::mainQueue.push_back( queue[i] );
			}

			if( mainQueue.size() > 0 ){
				std::sort( mainQueue.begin(), mainQueue.end(), compair_priority );

				MemorySpace::runningQueue_push_back( mainQueue[0] );
				utils::log("add","P.S","Appended running queue. PID: " + std::to_string(mainQueue[0]->PID));

				SystemStatus::isRunningEmpty = false;

				mainQueue.erase( mainQueue.begin() ); 
				
				MemorySpace::readyQueueCurrentLength --;
				// remove from readyQueue, because it's running
				continue;
			}

			if ( mainQueue.size() < 1
					&& SystemStatus::isNoJobLeft 
					&& SystemStatus::appendingReadyCount < 1 
					&& last_check  
					&& MemorySpace::blockedQueueCurrentLength < 1 
					&& MemorySpace::readyQueueCurrentLength < 1
					&& SystemStatus::isRunningEmpty
					&& !SystemStatus::isRevivingBlockedProcess
					){

				SystemStatus::isNoReadyLeft = true;

				utils::log("bye","P.S","Nothing left. End of my job.");

				return;
				// this thread should stop here.
			}

			if ( mainQueue.size() < 1
					&& SystemStatus::isNoJobLeft 
					&& SystemStatus::appendingReadyCount < 1 
					&& MemorySpace::blockedQueueCurrentLength < 1
					&& MemorySpace::readyQueueCurrentLength < 1
					){
				/*
				 * empty Job queue.
				 * empty Ready queue.
				 * no one is appending it.
				 * nothing left.
				 *
				 * There's nothing left other than
				 * the one in Running queue.
				 *
				 * */

				last_check = true;

				/*
				 * assume there're 2 jobs,
				 * 1 was appended, then taken away by ProcessScheduler.
				 * Before ProcessScheduler done using job 1
				 * job 2 was appended to Ready queue,
				 * and isNoJobLeft = true
				 *
				 * After appending to Running queue,
				 * mainQueue has 0 job left, and H.L.S is also retired.
				 * Check for one last time, if there's any "ready" left.
				 *
				 * Why the fuzz?
				 * Because "sort" ready, and "append" ready
				 * shouldn't be synced.
				 * H.L.S should "append" whenever there's a (to-be) slot.
				 * ProcessScheduler should "sort" whenever Running queue is available.
				 *
				 * aka. there's shouldn't be a mutex
				 * on "sort/append"-ing the Ready queue.
				 *
				 * */
			}

		}
	}
}
