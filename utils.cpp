#define LINUX // cross-patform note


#include <sstream>
#include <iostream>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <vector>
#include "SystemStatus.h"

#ifdef LINUX
#include <unistd.h>
#endif

#ifdef WINDOWS
#include <windows.h>
#endif

#include "utils.h"


void utils::log(std::string status, std::string class_name, std::string msg, bool enabled){
	if( enabled ){
		auto time = std::chrono::steady_clock::now();
		std::stringstream stream;
		stream << "[" << std::chrono::duration_cast<std::chrono::nanoseconds>(time.time_since_epoch()).count()
			<< "] | " << std::setw(7) << "["+status+"]" << " | " << std::setw(14) << "["+class_name+"]" << " | " << msg << std::endl;
		std::cout << stream.str() << std::flush;

	}
}



void utils::csleep(int sleep_mili_seconds)
{
#ifdef LINUX
	usleep(sleep_mili_seconds * 1000);   
	// usleep takes sleep time in us (1 millionth of a second)
#endif

#ifdef WINDOWS
	Sleep(sleep_mili_seconds);
#endif
}


bool utils::loadSystemwideConfig( std::string filepath ){

	std::ifstream input_file( filepath );
	std::vector<std::vector<std::string>> configs;
	std::string line;
	if( input_file.is_open() ){
		for(std::string line; getline(input_file,line); ){
			utils::trim(line);
			if(line.length() < 1){
				continue;
			}
			else{
				std::vector<std::string> splited_string = utils::split(line,":");
				if(splited_string.size() < 2){
					utils::log("bad","utils","Bad config at line: "+line);
					return false;
				}
				else{
					configs.push_back( splited_string );
				}
			}
		}
		input_file.close();
	}
	else{
		utils::log("bad","utils","System-wide configuration file not found: " + filepath);
		return false;
	}

	bool hasBadValues = false;

	for(int i=0; i<configs.size(); i++){
		// examine each line of configs
		utils::trim(configs[i][0]);
		utils::trim(configs[i][1]);

		if(configs[i][0] == "CODE_DELAY"){
			int value = -1;
			if( !utils::stoi(configs[i][1], &value) ){
				utils::log("bad","utils","Bad value found in system-wide config file. At \"CODE_DELAY\"");
				hasBadValues = true;
			}
			else{
				SystemStatus::sleepMiliSeconds = value;
			}
		}
		else if(configs[i][0] == "RESOURCE"){
			int resource_count = 0;
			int _tmp_integer = 0;
			std::vector<int> resource_list;
			std::vector<std::string> resource_numbers = utils::split(configs[i][1]);
			for(int res_index=0; res_index<resource_numbers.size(); res_index++){

				if( !utils::stoi(resource_numbers[res_index], &_tmp_integer) ){
					utils::log("bad","utils","Bad value found in system-wide config file. At \"RESOURCE\"");
					hasBadValues = true;
					break;
				}
				else{
					// utils::log("test","utils",std::to_string(_tmp_integer));
					resource_list.push_back( _tmp_integer );
					SystemStatus::supportedResourceCount ++;
				}

			}
			if( !hasBadValues ){
				SystemStatus::available = resource_list;
				SystemStatus::max_available = resource_list;
			}
		}
		else if(configs[i][0] == "MAX_TIME_CHUNCK"){
			int _tmp_integer = 0;
			if( utils::stoi(configs[i][1], &_tmp_integer) ){
				SystemStatus::maxTimeChunck = _tmp_integer;
			}
			else{
				utils::log("bad","utils","Bad value found in system-wide config file. At \"MAX_TIME_CHUNCK\"");
				hasBadValues = true;
			}
		}
		else if(configs[i][0] == "DISPLAY_CODE"){
			if( configs[i][1] == "true" ){
				SystemStatus::displayCode = true;
			}
			else if( configs[i][1] == "false" ){
				SystemStatus::displayCode = false;
			}
			else{
				utils::log("bad","utils","Bad value found in system-wide config file. At \"DISPLAY_CODE\"");
				hasBadValues = true;
			}
		}
		else if(configs[i][0] == "DISPLAY_DISK"){
			if( configs[i][1] == "true" ){
				SystemStatus::displayDisk = true;
			}
			else if( configs[i][1] == "false" ){
				SystemStatus::displayDisk = false;
			}
			else{
				utils::log("bad","utils","Bad value found in system-wide config file. At \"DISPLAY_DISK\"");
				hasBadValues = true;
			}
		}
		else if(configs[i][0] == "DISPLAY_FILESYSTEM_CUE"){
			if( configs[i][1] == "true" ){
				SystemStatus::displayFileSystemCue = true;
			}
			else if( configs[i][1] == "false" ){
				SystemStatus::displayFileSystemCue = false;
			}
			else{
				utils::log("bad","utils","Bad value found in system-wide config file. At \"DISPLAY_FILESYSTEM_CUE\"");
				hasBadValues = true;
			}
		}
		else if(configs[i][0] == "DISPLAY_FAT_TABLE"){
			if( configs[i][1] == "true" ){
				SystemStatus::displayFATTable = true;
			}
			else if( configs[i][1] == "false" ){
				SystemStatus::displayFATTable = false;
			}
			else{
				utils::log("bad","utils","Bad value found in system-wide config file. At \"DISPLAY_FAT_TABLE\"");
				hasBadValues = true;
			}
		}
		else if(configs[i][0] == "DISPLAY_TRACK"){
			if( configs[i][1] == "true" ){
				SystemStatus::displayTrack = true;
			}
			else if( configs[i][1] == "false" ){
				SystemStatus::displayTrack = false;
			}
			else{
				utils::log("bad","utils","Bad value found in system-wide config file. At \"DISPLAY_TRACK\"");
				hasBadValues = true;
			}
		}
		else if(configs[i][0] == "TRACK_ALGORITHM"){
			if( configs[i][1] == "SSTF" ){
				SystemStatus::trackAlgorithm = "SSTF";
			}
			else if( configs[i][1] == "SCAN" ){
				SystemStatus::trackAlgorithm = "SCAN";
			}
			else if( configs[i][1] == "FCFS" ){
				SystemStatus::trackAlgorithm = "FCFS";
			}
			else{
				hasBadValues = true;
				utils::log("bad","utils","Bad value found in system-wide config file. At \"TRACK_ALGORITHM\"");
			}
		}
		else{
			utils::log("bad","utils","Bad config at line: "+line + ", option not recognized.");
			// return false;  <--- not gonna exit when unrecognized system-wide config found.
		}
	}

	if( hasBadValues ){
		utils::log("bad","utils","System-wide configuration file contains bad value(s).");
		return false;
	}
	else{
		utils::log("load","utils","System status loaded.");
		return true;
	}

}
