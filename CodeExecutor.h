#ifndef INCLUDE_CODE_EXECUTOR
#define INCLUDE_CODE_EXECUTOR

#include <string>
#include <vector>
#include <thread>
#include "utils.h"
#include "DataStructures.h"
#include "SystemStatus.h"
#include "MemorySpace.h"
#include "ModuleBanker.h"

class CodeExecutor{
	public:
		bool parseAndRun( std::string line_code, PCB* pcb );
		void run();


	private:
		static bool CMP;


		/* stores variable into PCB's spacePointer(vector) */
		/* if varname exists, then update it's value. */
		static std::vector<int> err_request;
		static inline void log( std::string tag, std::string msg ){
			utils::log( tag, "C.E", msg );
		}

		static inline void reviveBlocked(){
			std::unique_lock<std::mutex> guard(*MemorySpace::mutex_blockedOperation);

			/*
			 * Why mutex?
			 * 
			 * - process 1 release, launch thread to revive blocked.
			 *   - during revive, appendReadyQueue() could take time.
			 * - process 2 running, but then blocked,
			 *   - also needs blockedQueue
			 *
			 * - process 2 release, while process 1 is still being released.
			 *   ( revive blocked takes time )
			 * - they shouldn't be reviving blocked queue at the same time.
			 *
			 * check blocked Queue.
			 * Try ModuleBanker::solveRequest();
			 *
			 * true:	blocked --> ready
			 * false:	leave it be.
			 *
			 * */
			bool printed_msg_once = false;
			for(int i=0; i<MemorySpace::blockedItemQueue.size(); i++){
				Item* _tmp_item = MemorySpace::blockedItemQueue[i];
				if( !printed_msg_once ){
					CodeExecutor::log("revv","Dispatched blocked process reviver...");
				}
				/* print "Dispatched blocked ..." only once
				 * for a clean output.
				 * */
				printed_msg_once = true;
				if( ModuleBanker::solveRequest(*_tmp_item, false) ){
					CodeExecutor::log("revv","Blocked process goes back to ready.");
					MemorySpace::blockedQueueCurrentLength --;
					PCB* blocked_to_ready = MemorySpace::blockedQueue_pop_id( _tmp_item->id );

					// launches thread(s) here, so it won't take much time.
					std::thread thread_append( CodeExecutor::appendReadyQueue, blocked_to_ready );
					thread_append.join();
				}
			}
			guard.unlock();
		}

		static inline void appendReadyQueue( PCB* pcb ){
			SystemStatus::appendingReadyCount ++;
			while( MemorySpace::readyQueueCurrentLength < MemorySpace::readyQueueMaxLength ){
				if( MemorySpace::readyQueue_push_back( pcb ) ){
					break;
				}
			}
			SystemStatus::appendingReadyCount --;
		}

		static inline bool hasChuncks(const std::vector<std::string> &code, int expected_chuncks){
			return code.size() == expected_chuncks;
		}

};

#endif
