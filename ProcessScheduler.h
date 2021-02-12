#ifndef INCLUDE_PROCESS_SCHEDULER
#define INCLUDE_PROCESS_SCHEDULER

#include <vector>
#include <algorithm> 
#include "SystemStatus.h"
#include "MemorySpace.h"
#include "DataStructures.h"
#include "utils.h"


class ProcessScheduler{
	public:
		void run();
	private:
		static std::vector<PCB*> mainQueue;
		static std::vector<PCB*> makePCBVector();
		static inline bool compair_priority(const PCB* p1,const PCB* p2)
		{
			return p1->priority > p2->priority;
		}

};

#endif
