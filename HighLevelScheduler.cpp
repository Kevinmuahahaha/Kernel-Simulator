#include "HighLevelScheduler.h"
#include "utils.h"
#include "DataStructures.h"
#include "SystemStatus.h"
#include "MemorySpace.h"
#include "ModuleBanker.h"
#include <string>
#include <fstream>
#include <vector>
#include <cctype>
#include <locale>
#include <utility>

std::vector<int> HighLevelScheduler::calculateMaxResource( const Code* code ){
	std::vector<int> ret_vector;
	std::vector<std::string> splited_string;
	std::string::size_type sz;
	int vector_index = -1;
	int split_index = -1;
	for(int i=0; i<SystemStatus::supportedResourceCount; i++){
		ret_vector.push_back(0); /*init resource list*/
	}
	for(int i=0; i<code->codeSequence.size(); i++){
		std::string tmp_str = code->codeSequence[i];
		std::string details;
		if( tmp_str.rfind("REQ",0) == 0 ){
			details = tmp_str.substr( 3, tmp_str.length() );
			utils::trim(details);
			splited_string = utils::split( details );
			for(int split_index=0; split_index<splited_string.size(); split_index++ ){
				std::string current = splited_string[split_index];

				if( std::isalpha(current[0]) ){
					/*
					 *	if current section is a Charachter
					 *	then, record it's corresponding value.
					 *
					 *	if not, this section will be skipped.
					 *
					 * */
					vector_index = std::toupper(current[0]) - 'A';
					ret_vector[ vector_index ] += std::stoi( splited_string[split_index+1],  &sz );
				}

			}
			
		}
	}
	return ret_vector;
}

Code* HighLevelScheduler::loadCode( std::string file_path ){
	HighLevelScheduler::log("load","Loading code: " + file_path);
	Code* ret_code = new Code();

	std::ifstream input_file(file_path);
	int line_count = 0;
	if( input_file.is_open() ){
		for(std::string line; getline(input_file,line); ){
			/* check each line */
			line_count ++;
			utils::trim(line);


			if( line.find(';') != std::string::npos ){
				/* if there's a comment, then remove the comment */
				line = line.erase( line.find(';') , line.length() );
			}
			utils::trim(line);

			/* each instruction has at least 3 chars. 
			 * Or it should be a jump tag.*/
			if(  line.find_last_of(':') != line.length() -1 && line.length() < 3  || line.length() < 1 ){
				// not a tag, and with chars fewer than 3
				continue;
			}

			ret_code->codeSequence.push_back(line);

		}
		input_file.close();
	}

	return ret_code;
}

bool HighLevelScheduler::configIntegrityCheck(std::string file_path){
	HighLevelScheduler::log("check","Checking config file integrity: "+file_path);
	std::ifstream input_file(file_path);
	int line_count = -1;
	std::vector<int> encapsulation_check;
	std::vector<std::string> task_check;
	if( input_file.is_open() ){
		for(std::string line; getline(input_file,line); ){
			/* check each line */
			utils::trim(line);
			line_count ++;

			if(line_count == 0 && line != "#config"){
				HighLevelScheduler::log("bad","First line of the config should start with \"#config\"");
				return false;
			}

			if(line.rfind(";",0) == 0){
				continue;
			}


			if( line.rfind("#task",0) == 0 ){
				if(encapsulation_check.size()>0){
					HighLevelScheduler::log("bad","Consecutive \"task\" tags is not supported.");
					return false;
				}
				else{
					encapsulation_check.push_back(line_count);
				}
			}
			else if( line.rfind("#ends",0) == 0 ){
				if(encapsulation_check.size()>0){
					/* Check a task's syntax when it's the "ends" */
					if(! HighLevelScheduler::taskIntegrityCheck(task_check) ){
						HighLevelScheduler::log("bad","Syntax error for task at line: "
								+std::to_string(encapsulation_check[encapsulation_check.size()-1]));
						return false;
					}
					else{
						// current task is OK
						task_check.clear();
					}
					encapsulation_check.pop_back();
				}
				else{
					HighLevelScheduler::log("bad","Tag mismatch at line: "+std::to_string(line_count)+" -> "+line);
					return false;
				}
			}
			else if( encapsulation_check.size() > 0 ){
				/* check inside of a task */
				if(line.length() > 0){
					task_check.push_back(line);
				}
			}
		}
	}
	else{
		HighLevelScheduler::log("bad","Configuration file: "+file_path+", not found.");
		return false;
	}


	input_file.close();
	return true;
}

bool HighLevelScheduler::codeIntegrityCheck(const Code* code){
	HighLevelScheduler::log("check","Checking code integrity...");
	/*
	 *	A code's first line should be "#codefile"
	 *	and at least one line.
	 *
	 * */
	if( code->codeSequence.size() < 2  || code->codeSequence[0] != "#codefile" ){
		HighLevelScheduler::log("bad","Code file should start with \"#codefile\" and have contents.");
		return false;
	}

	bool instructionSupported = false;
	int line_last_index = -1;
	std::locale local_digit;
	for(int i=1; i<code->codeSequence.size(); i++){
		instructionSupported = false;
		line_last_index = code->codeSequence[i].length() -1;
		/*
		 * if it's not a jump tag
		 * check if the instruction is available.
		 *
		 * */
		if(code->codeSequence[i][ line_last_index ] != ':'){
			std::string instruction = code->codeSequence[i].substr(0,3);
			for(int i_check=0; i_check<SystemStatus::supportedInstructions.size(); i_check++){
				if( SystemStatus::supportedInstructions[i_check] == instruction){
					instructionSupported = true;
				}
				if( instruction == "REQ" ){
					// check syntax for REQ
					std::vector<std::string> req_string = utils::split( code->codeSequence[i] );

					for( int req_index = 1; req_index < req_string.size(); req_index++ ){
						if( req_index%2 == 1 && !std::isalpha(req_string[req_index][0] ) ){
							/*
							 * odd index points to resource name (e.g. A)
							 * Otherwise, the syntax is wrong.
							 *
							 * */
							HighLevelScheduler::log("bad","Bad syntax for REQ at: " + code->codeSequence[i]);
							return false;
						}

						else if( req_index%2 == 0 ){

							for( int digit_check_index=0; digit_check_index<req_string[req_index].length(); digit_check_index ++ ){

								if( !std::isdigit(req_string[req_index][digit_check_index], local_digit) ){
									HighLevelScheduler::log("bad","Bad syntax for REQ at: " + code->codeSequence[i]);
									return false;
								}
							}
							/*
							 * even index points to resource amount (e.g. 50)
							 * Otherwise, the syntax is wrong.
							 *
							 * */
						}
						else if( std::isalpha(req_string[req_index][0]) ){
							int req_charachter_index = req_string[req_index][0] - 'A' + 1;
							if( req_charachter_index > SystemStatus::supportedResourceCount 
									|| req_charachter_index < 0 ){
								HighLevelScheduler::log("bad","Unsupported resource at: " + code->codeSequence[i]);
								return false;
							}
						}
					}
				}
			}
		}
		else{
			/* Skip jump tag */
			instructionSupported = true;
		}

		if(!instructionSupported){
			HighLevelScheduler::log("bad","Unsupported instruction at line: " + code->codeSequence[i]);
			return false;
		}
	}



	return true;
}

/* parseConfig creates PCB, 
 * should only be run when config file passes configIntegrityCheck */
void HighLevelScheduler::parseConfig(std::string file_path){
	HighLevelScheduler::log("good","Config OK.");
	HighLevelScheduler::log("load","Loading tasks from configuration file: " + file_path);

	std::ifstream input_file(file_path);
	int line_count = 0;


	if( input_file.is_open() ){
		//int memory = -1;
		std::string memory = "";
		std::string code_path = "";
		std::string priority = "";
		std::string::size_type sz;

		for(std::string line; getline(input_file,line); ){
			if( !SystemStatus::isSystemRunning ){
				break;
			}

			utils::trim(line);
			if( line.find(';') != std::string::npos ){
				/* if there's a comment, then remove the comment */
				line = line.erase( line.find(';') , line.length() );
			}
			utils::trim(line);
			if(line.length() < 1 || line == "#config"){
				continue;
			}

			/* if is memory, if is code */
			if( line.rfind("memory",0) == 0){
				memory = line.substr( line.find(':') +1, line.length() );
				utils::trim(memory);
			}
			else if( line.rfind("code",0) == 0){
				code_path = line.substr( line.find(':')+1, line.length() );
				utils::trim(code_path);
			}
			else if( line.rfind("priority",0) == 0){
				priority = line.substr( line.find(':')+1, line.length() );
				utils::trim(code_path);
			}

			if(line == "#ends"){
				/*
				HighLevelScheduler::log("test",memory);
				HighLevelScheduler::log("test",code_path);
				HighLevelScheduler::log("test",priority);
				*/
				PCB* pcb = new PCB();
				pcb->PID = SystemStatus::PIDCount++;

				Code* code = loadCode( code_path );

				if( codeIntegrityCheck(code) ){
					// update resource table
					// avaiable has been initialized in SystemStatus.cpp
					//
					// check if max > max_available
					bool _hasBadResource = false;
					std::vector<int> maxResources = this->calculateMaxResource(code);
					for(int index_max_res=0; index_max_res<SystemStatus::max_available.size(); index_max_res ++){
						if(maxResources[index_max_res] > SystemStatus::max_available[index_max_res]){
							HighLevelScheduler::log("bad","Process having bad amount of resource.");
							_hasBadResource = true;
						}
					}
					if(_hasBadResource){
						delete pcb;
						continue;
					}

					HighLevelScheduler::log("load","Creating PCB...");
					// lock table, incase the banker needs access to it.
					std::unique_lock<std::mutex> guard(*SystemStatus::tableOperation);

					std::pair<int,std::vector<int>> tmp_push;
					tmp_push.first = pcb->PID;
					tmp_push.second = maxResources;

					SystemStatus::Max.push_back( tmp_push );
					SystemStatus::need.push_back( tmp_push );

					for(int tmp_push_index=0; tmp_push_index<tmp_push.second.size(); tmp_push_index++){
						tmp_push.second[tmp_push_index] = 0;
						// allocation by default, has { 0, 0, 0 ... }
					}
					SystemStatus::allocation.push_back( tmp_push );


					// unlock it, when we're done using it.
					guard.unlock();
					// ModuleBanker::showdata();
				}
				else{
					HighLevelScheduler::log("bad","Code integrity check failed. Task won\'t be loaded.");
					delete pcb;
					continue;
				}

				pcb->codePointer = code;
				DataSpace* space = new DataSpace();
				pcb->spacePointer = space;

				pcb->priority = std::stoi( priority, &sz );

				/* 
				 * push, only when current length < max 
				 * stop pushing, only when pushing is successful.
				 *
				 * */
				while(true){
					if( MemorySpace::readyQueueCurrentLength < MemorySpace::readyQueueMaxLength ){
						/*
						 * thread 1: revive blocked PCBs
						 * thread 2: H.L.S appends PCB
						 *
						 * These two threads might, at one point,
						 * think that there's 1 empty slot in Running queue.
						 *
						 * But in fact, only one of them is gonna get it.
						 * The one failed to get the slot,
						 * will continue the loop.
						 *
						 * */
						if( MemorySpace::readyQueue_push_back(pcb) ){
							break;
						}
					}
				}

				pcb->print();
				HighLevelScheduler::log("load","Ready queue appended.");

				// create PCB
				// update Banker's tables
			}


		}
		SystemStatus::isNoJobLeft = true;
		HighLevelScheduler::log("bye","No task left. My job ends here.");
		input_file.close();
	}
}

/* 
 * Checks config file, 
 * then try to load each task, 
 * (after checking their codes specified in each task)
 * 	by calling codeIntegrityCheck()
 *
 */
bool HighLevelScheduler::taskIntegrityCheck(const std::vector<std::string> &config_strings){
	std::vector<std::string> mandatory;
	/* put required sections down here, codes below will fill the blank. */
	mandatory.push_back("memory");
	mandatory.push_back("code");
	mandatory.push_back("priority");
	std::vector<std::string> mandatoryArgs;
	std::string delimiter = ":";

	for(int i=0; i < config_strings.size(); i++){

		std::string tmp;
		tmp = config_strings[i];

		// solve memory without ':'

		for(int tags_check = 0; tags_check < mandatory.size(); tags_check++){
			if(tmp.rfind(mandatory[tags_check],0) == 0){
				/* Parse name:value, store "value" into "mandatoryArgs" */

				if( tmp.find(':') == std::string::npos ){
					HighLevelScheduler::log("bad","Config section: " 
							+ mandatory[tags_check] 
							+ ", missing delimiter.");
					return false;
				}

				tmp = tmp.substr( tmp.find(delimiter) +1, tmp.length() -1) ;
				if( tmp.find(';') != std::string::npos ){
					/* if there's a comment, then remove the comment */
					tmp = tmp.erase( tmp.find(';') , tmp.length()-1 );
				}
				utils::trim(tmp);

				if( tmp.length() == 0 ){
					HighLevelScheduler::log("bad","Mandatory tag: " 
							+ mandatory[tags_check] 
							+ ", needs to be filled.");
					return false;
				}

				mandatoryArgs.push_back(tmp);
			}
		}
	}

	if(mandatoryArgs.size() != mandatory.size()){

		HighLevelScheduler::log("bad","Insufficient information for creating task.");

		HighLevelScheduler::log("bad","Required: " 
				+ std::to_string(mandatory.size()) 
				+ ", Gets: " 
				+ std::to_string(mandatoryArgs.size()));

		return false;
	}
	else{
		return true;
	}

}
