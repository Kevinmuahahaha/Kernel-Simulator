#ifndef INCLUDE_HIGH_LEVEL_SCHEDULER
#define INCLUDE_HIGH_LEVEL_SCHEDULER

#include <string>
#include <vector>
#include "utils.h"
#include "DataStructures.h"

class HighLevelScheduler{
	public:

		Code* loadCode( std::string file_path );

		/* utilities for a rather well-thought-out simulation. */
		bool configIntegrityCheck(std::string file_path);
		bool codeIntegrityCheck(const Code* code);


		/*
		 * parse "Task" configuration file
		 * create PCB, then append them to readyQueue
		 *
		 * */
		void parseConfig(std::string file_path);

	private:
		static void inline log(std::string status, std::string msg){
			utils::log(status,"H.L.S",msg);
		}
		bool taskIntegrityCheck(const std::vector<std::string> &config_strings);
		std::vector<int> calculateMaxResource( const Code* code );


};

#endif
