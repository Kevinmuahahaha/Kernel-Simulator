#include <thread>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <iostream> //testing
#include "utils.h"
#include "MemorySpace.h"
#include "DataStructures.h"
#include "SystemStatus.h"
#include "HighLevelScheduler.h"
#include "ModuleBanker.h"
#include "ProcessScheduler.h"
#include "CodeExecutor.h"
#include "FileSystem.h"
#include "Module_DiskAccess.h"

using namespace std;

void log(string status, string msg){
	utils::log(status,"main",msg);
}

void ShutdownTimer(){
	// log("info","System will shut down in 5 seconds.");
	for(int i=0 ; i< 10;i++){
		//utils::csleep(5000);
	}
	//SystemStatus::isSystemRunning = false;
}

int main(void){

	// windows Chinese charachter fix
	// system("chcp 65001");
	// comment the line above when compiling on linux.

	log("start", "program is starting...");

	if( !utils::loadSystemwideConfig("systemwide_config.txt") ){
		log("bad", "Bad System-wide config. Abort.");
		log("end","program ends.");
		return -1;
	}

	Module_DiskAccess init;
	
	HighLevelScheduler hls;
	string file_path = "config.txt";

	ProcessScheduler ps;
	CodeExecutor ce;

	if( hls.configIntegrityCheck(file_path) ){

		//hls.parseConfig( file_path );

		thread thread_ProcessScheduler( &ProcessScheduler::run, ps );
		thread thread_CodeExecutor( &CodeExecutor::run, ce );
	
		thread thread_ShutdownTimer( ShutdownTimer );
		thread thread_HLS( &HighLevelScheduler::parseConfig, hls, file_path);


		thread_ShutdownTimer.join();
		thread_HLS.join();
		thread_ProcessScheduler.join();
		thread_CodeExecutor.join();

	}


	log("bye", "Ready:   " + std::to_string( MemorySpace::readyQueueCurrentLength ) );
	log("bye", "Blocked: " + std::to_string( MemorySpace::blockedQueueCurrentLength ) );
	log("bye", "Running: " + std::to_string( MemorySpace::runningQueueCurrentLength ) );
	log("bye", "pendd:   " + std::to_string( SystemStatus::appendingReadyCount ) );

	
	ModuleBanker::showdata();
	

	log("end","program ends.");

	getchar();

	return 0;
}
