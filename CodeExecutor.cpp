#include "CodeExecutor.h"
#include "Module_DiskAccess.h"
#include "ModuleBanker.h"
#include "FileSystem.h"
#include "MemorySpace.h"
#include "SystemStatus.h"
#include "DataStructures.h"
#include "utils.h"
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <cctype>
#include <locale>
#include <cstddef>



bool CodeExecutor::CMP = false; // stores result of CMP

std::vector<int> CodeExecutor::err_request = utils::makeIntVector();

bool CodeExecutor::parseAndRun( std::string line_code, PCB* running_pcb ){

	if( line_code == "#codefile" ){
		// skip beginning
		return true;
	}

	/*
	 *
	 *	false:	
	 *		- err_request should be updated
	 *		- blocked
	 *	true:	
	 *		- banker granted resources.
	 *
	 * */
	std::locale locale_digit;
	std::string::size_type sz;
	std::vector<std::string> code = utils::split( line_code );
	std::string instruction = code[0];

	if( instruction == "REQ" ){
		// ModuleBanker
		int character_index = -1;
		std::vector<int> request_vector( SystemStatus::supportedResourceCount );

		for(int req_index = 1; req_index < code.size(); req_index++){
			if( std::isalpha(code[req_index][0]) && req_index % 2 == 1 ){
				character_index = code[req_index][0] - 'A';
			}
			else if( std::isdigit(code[req_index][0], locale_digit) ){
				request_vector[ character_index ] += std::stoi( code[req_index], &sz );
			}
			else{
				CodeExecutor::log("crit","Bad request found. This shouldn\'t have happened.");
			}
		}

		Item solve_item;
		solve_item.id  = running_pcb->PID;
		solve_item.request = request_vector;
		
		/* convert request vector to string, for printing */
		std::string request_in_string;
		for(int indexx=0; indexx<request_vector.size(); indexx++){
			request_in_string += " " + std::to_string(request_vector[indexx]);
		}
		
		CodeExecutor::log("req","PID: "+std::to_string(running_pcb->PID)+", requesting: "+request_in_string);


		std::unique_lock<std::mutex> guard(*SystemStatus::tableOperation);
		if( ModuleBanker::solveRequest( solve_item ) ){
			CodeExecutor::log("solve","Banker granted resource(s).");
			return true;
		}
		else{
			CodeExecutor::log("solve","Banker denied request. PID: " + std::to_string(running_pcb->PID)+ ", blocked");
			err_request = request_vector;
			return false;
		}
		guard.unlock();

	}
	else if( instruction == "DIE" ){
		/*
		 * Points to the last line.
		 *
		 * */
		running_pcb->currentInstructionIndex = running_pcb->codePointer->codeSequence.size();
	}
	else if( instruction == "NOP"){
		// Do nothing
	}
	else if( instruction == "MOV" ){
		// DiskAccess store
		// or makeVariable
		// if MOV to resource P, then it prints the variable

		/*
		 * (note: string means string variables, not "string")
		 * (note2: file  is a string variable, not "file path")
		 *
		 * "Wrong"
		 * MOV "content" "file name"
		 * MOV "string"  $store_string
		 *
		 * "Right"
		 * DEF ^FILE_NAME    "file name"
		 * DEF $STR_CONTENT  "content"
		 * MOV $STR_CONTENT ^FILE_NAME
		 *
		 * MOV $FROM_STRING $TO_STRING
		 *
		 * string to string
		 * string to file
		 * file   to file
		 * file   to string
		 *
		 * anything to device
		 *
		 * */

		if( code[1][0] == '$' && code[2][0] == '$' ){
			// string to string
			const string* _tmp_string = running_pcb->spacePointer->getContentByName( code[1] );
			if( _tmp_string != nullptr ){
				running_pcb->spacePointer->storeVariableWithName( code[2], *_tmp_string );
			}
			else{
				CodeExecutor::log("bad","Accessing undefined variable denied: " + code[2]);
			}
		}


		if( code[1][0] == '$' && code[2][0] == '^' ){
			
			const std::string* _tmp_string_pointer = running_pcb->spacePointer->getContentByName( code[1] );
			const std::string* _tmp_string_filename = running_pcb->spacePointer->getContentByName( code[2] );

			bool _filename_found = (_tmp_string_filename != nullptr);
			bool _string_found = (_tmp_string_pointer != nullptr);
			bool _write_status = false; 

			if( !_filename_found ){
				CodeExecutor::log("bad","File not found with: " + code[2]);
			}
			else if( !_string_found ){
				CodeExecutor::log("bad","Accessing undefined variable with: " + code[1]);
			}
			else{
				// found filename, found string
				_write_status = FileSystem::writeFile( *_tmp_string_filename, *_tmp_string_pointer);
			}

			if(!_write_status){
				CodeExecutor::log("bad","Disk not written.");
			}
			else{
				if( SystemStatus::displayFileSystemCue ){
					CodeExecutor::log("write","Content written into file with: " + code[2]);
				}
			}
		}

		if( code[1][0] == '^' && code[2][0] == '^' ){

			const std::string* _tmp_filename_source = running_pcb->spacePointer->getContentByName( code[1] );
			const std::string* _tmp_filename_destin = running_pcb->spacePointer->getContentByName( code[2] );
			
			bool both_file_variable_exist = false;

			if( _tmp_filename_source == nullptr ){
				CodeExecutor::log("bad","Source filename not found with: " + code[1]);
			}
			else if( _tmp_filename_destin == nullptr ){
				CodeExecutor::log("bad","Destination filename not found with: " + code[2]);
			}
			else{
				both_file_variable_exist = true;
			}

			if( both_file_variable_exist ){
				const std::string* _tmp_string_pointer = FileSystem::readFile( *_tmp_filename_source  );
				bool _delete_status = false;

				if( _tmp_string_pointer == nullptr ){
					CodeExecutor::log("bad","Couldn\'t read source file.");
				}
				else{
					_delete_status = FileSystem::deleteFile( *_tmp_filename_destin );
				}

				if( _tmp_string_pointer != nullptr && _delete_status ){
					bool _write_status = FileSystem::writeFile( *_tmp_filename_destin, *_tmp_string_pointer );
					if( !_write_status ){
						CodeExecutor::log("bad","File overwritten but fail to store. Content could be too big.");
					}
					else{
						if( SystemStatus::displayFileSystemCue ){
							CodeExecutor::log("copy","File copied from: " + *_tmp_filename_source + ", to: " + *_tmp_filename_destin);
						}
					}
				}
				else if( !_delete_status ){
					CodeExecutor::log("bad","Destination file doesn\'t exist.");
				}
			}

			// delete _tmp_string_pointer;
		}

		if( code[1][0] == '^' && code[2][0] == '$' ){

			const std::string* _tmp_string_pointer = running_pcb->spacePointer->getContentByName( code[2] );
			// check if the $string exists
			
			const std::string* _tmp_string_filename = running_pcb->spacePointer->getContentByName( code[1] );
			// check if the ^filename exists (just the name, not its content)

			const std::string* _tmp_string_pointer_file = nullptr ;
			// check if the ^filename contains any data

			if( _tmp_string_filename != nullptr ){
				_tmp_string_pointer_file = FileSystem::readFile( *_tmp_string_filename );
			}
			else{
				// else the filename wasn't defined.
				CodeExecutor::log("bad","Accessing undefined filename with: " + code[2]);
			}

			if( _tmp_string_pointer == nullptr ){
				CodeExecutor::log("bad","Accessing undefined variable: " + code[2]);
			}
			else if( _tmp_string_pointer_file == nullptr ){
				CodeExecutor::log("bad","File not found with: " + code[1]);
			}
			else{
				running_pcb->spacePointer->storeVariableWithName(code[2], *_tmp_string_pointer_file );
			}
		}


		if( code[1][0] == '$' && code[2] == "P" ){ // print
			const std::string* _tmp_string_pointer;
			_tmp_string_pointer = running_pcb->spacePointer->getContentByName(code[1]);
			if( _tmp_string_pointer != nullptr ){
				utils::log("print","resource", *_tmp_string_pointer);
			}
			else{
				CodeExecutor::log("bad","Accessing undefined variable: "+code[1]);
			}
		}
		else if( code[1][0] == '^' && code[2] == "P" ){
			CodeExecutor::log("bad","Can\'t move file to print. Move it to a var first.");
		}

	}
	else if( instruction == "DEL" ){
		// DiskAccess delete
		/*
		 * Ask file system, where is the 1st block of "filename"
		 * 	int FileSystem::findBlockByFileName( string filename );
		 *
		 * Use DiskAccess::Remove( int block );
		 *
		 * */

		const std::string* _tmp_string_pointer = running_pcb->spacePointer->getContentByName(code[1]);
		if( _tmp_string_pointer == nullptr ){
			CodeExecutor::log("bad","File not found with: "+code[1]);
		}
		else{
			if( !FileSystem::deleteFile( *_tmp_string_pointer ) ){
				CodeExecutor::log("bad","Couldn\'t delete file: " + code[1]);
				// display error no matter what
			}
			else{
				if( SystemStatus::displayFileSystemCue ){
					CodeExecutor::log("del", "File: "+code[1]+", deleted.");
					// display "ok" status only when asked to.
				}
			}
		}

	}
	else if( instruction == "DEF" ){
		// makeVariable
		/*
		 * $STRING_BEGINS_WITH_DOLLAR_SIGN
		 *
		 * ^FILE_BEGINS_WITH_A_POINTY_SIGN
		 *
		 * */
		std::string _tmp_string;
		bool start_recording = false;

		for(int i=0; i<line_code.length(); i++){
			if( start_recording ){
				if(line_code[i] == '\"'){
					start_recording = false;
					continue;
				}
				_tmp_string += line_code[i];
			}
			if(line_code[i] == '\"'){
				start_recording = true;
			}
		}
		running_pcb->spacePointer->storeVariableWithName( code[1], _tmp_string );
	}
	else if( instruction == "APP" ){
		// makeVariable, append string
		//
		// APP $Changed, $Unchanged
		/*
		 * find content by $STRING_NAME (from PCB->DataSpace)
		 * append, and store it back.
		 *
		 * */
		const std::string* changed_string_pointer;
		const std::string* unchanged_string_pointer;
		changed_string_pointer =   running_pcb->spacePointer->getContentByName(code[1]);
		unchanged_string_pointer = running_pcb->spacePointer->getContentByName(code[2]);

		std::string _tmp_string;


		if( changed_string_pointer == nullptr ){
			CodeExecutor::log("bad","Access to undefined string denied: "+code[1]);
		}
		else if ( unchanged_string_pointer == nullptr ){
			CodeExecutor::log("bad","Access to undefined string denied: "+code[2]);
		}
		else{
			_tmp_string = *changed_string_pointer + *unchanged_string_pointer;
			running_pcb->spacePointer->storeVariableWithName( code[1], _tmp_string );
		}
	}
	else if( instruction == "SNP" ){
		// Snip from tail if match
		//
		// SNP $Changed, $Unchanged
		/*
		 * find content by $STRING_NAME (from PCB->DataSpace)
		 * append, and store it back.
		 *
		 * */
		const std::string* changed_string_pointer;
		const std::string* unchanged_string_pointer;
		changed_string_pointer =   running_pcb->spacePointer->getContentByName(code[1]);
		unchanged_string_pointer = running_pcb->spacePointer->getContentByName(code[2]);

		std::string _tmp_string;


		if( changed_string_pointer == nullptr ){
			CodeExecutor::log("bad","Access to undefined string denied: "+code[1]);
		}
		else if ( unchanged_string_pointer == nullptr ){
			CodeExecutor::log("bad","Access to undefined string denied: "+code[2]);
		}
		else{
			/*
			 * if matched from right
			 * remove substring from right
			 *
			 * */
			std::size_t _tmp_found_index = changed_string_pointer->rfind( *unchanged_string_pointer );
			if( _tmp_found_index != std::string::npos 
					&& _tmp_found_index + (*unchanged_string_pointer).length()
				       	== (*changed_string_pointer).length() ){
				// found matching tail
				_tmp_string = *changed_string_pointer; // stores content
				_tmp_string.erase( _tmp_string.begin() + _tmp_found_index, _tmp_string.end() );
				running_pcb->spacePointer->storeVariableWithName( code[1], _tmp_string );
			}
			else{
			}
		}
	}

	else if( instruction == "CMP" ){
		const std::string* _left   = running_pcb->spacePointer->getContentByName(code[1]);
		const std::string* _right  = running_pcb->spacePointer->getContentByName(code[2]);
		if( _left == nullptr || _right == nullptr ){
			CodeExecutor::log("bad","Accessing undefined variable(s).");
		}
		else{
			CodeExecutor::CMP = (*_left == *_right);
		}
	}
	else if( instruction == "JMP" ){
		/*
		 * look through codeSequence, find tag's index(in the sequence).
		 * set code_index to that line.
		 *
		 * */
		std::string _tmp_tag = code[1];
		for(int sequence_index=0; 
				sequence_index<running_pcb->codePointer->codeSequence.size(); 
				sequence_index++){
			if( running_pcb->codePointer->codeSequence[sequence_index] == _tmp_tag  ){
				running_pcb->currentInstructionIndex = sequence_index - 1;
				/* sequence_index -1, because it gets incremented at the end of each loop */
			}
			/* note: 
			 *
			 * if there're repeating tags, 
			 * the index will jump to the lowest one in code sequence. 
			 *
			 * if no tags found, then this jump is ignored.
			 *
			 * */
		}
	}
	else if( instruction == "JEQ" ){
		if( CodeExecutor::CMP ){
			std::string _tmp_tag = code[1];
			for(int sequence_index=0; 
					sequence_index<running_pcb->codePointer->codeSequence.size(); 
					sequence_index++){
				if( running_pcb->codePointer->codeSequence[sequence_index] == _tmp_tag  ){
					running_pcb->currentInstructionIndex = sequence_index -1;
				}
			}
		}
	}
	else if( instruction == "JNE" ){
		if( !CodeExecutor::CMP ){
			std::string _tmp_tag = code[1];
			for(int sequence_index=0; 
					sequence_index<running_pcb->codePointer->codeSequence.size(); 
					sequence_index++){
				if( running_pcb->codePointer->codeSequence[sequence_index] == _tmp_tag  ){
					running_pcb->currentInstructionIndex = sequence_index -1;
				}
				else{
				}
			}
		}
	}
	else if( instruction[instruction.length()-1] == ':' ){
		// skip tag
	}
	else{
		CodeExecutor::log("crit","Unsupported instruction found. This shouldn\'t have happened. : " + instruction );
	}
	utils::csleep( SystemStatus::sleepMiliSeconds );
	return true;
}

void CodeExecutor::run(){
	bool last_pull = false;
	bool isBlocked = false;
	bool isTimesUp = false;
	int  pulled_size = -1;
	while( SystemStatus::isSystemRunning ){

		/*
		 *	pull a task from Running queue.
		 *	- reset timer
		 *
		 *	run each line,
		 *	check if time's up after each line.
		 *
		 *
		 *	(after running each line):
		 *	if ( line_counter > line_max ) "process ends"
		 *		- set runningEmpty = true
		 *		- release resource
		 *		- launch a thread,
		 *		append blocked to Ready queue.
		 *		- (break loop, and pull)
		 *	else if "block"
		 *		- block
		 *		- set runningEmpty = true
		 *		- (break loop, and pull)
		 *	else if "time's up"
		 *		- launch a thread, 
		 *		append it back to Ready queue.
		 *		- (break loop, and pull)
		 *	else	"not block/time's up/ends"
		 *		- read next line
		 *	finally
		 *		program counter ++
		 *
		 *	https://stackoverflow.com/questions/53687178/interrupting-instruction-in-the-middle-of-execution
		 *
		 *	timer restarts:
		 *	when "time's up" signal get's recognized.
		 *
		 * */
		while( !SystemStatus::isRunningEmpty ){
			std::vector<PCB*> pulled = MemorySpace::runningQueue_pull( MemorySpace::runningQueueMaxLength );
			pulled_size = pulled.size();


			if( !last_pull 
					&& pulled_size < 1 
					&& SystemStatus::isNoReadyLeft 
					&& SystemStatus::isNoJobLeft 
					&& SystemStatus::appendingReadyCount < 1 
					&& MemorySpace::readyQueueCurrentLength > 0
			  ){
				/*
				 * CodeExecutor grabs.
				 * Then ready queue appended the last PCB,
				 * and sets isNoReadyLeft = true.
				 *
				 * Then comes this check.
				 * 
				 * */
				CodeExecutor::log("pull", "Reaches last pull");
				// pull for the last time
				last_pull = true;
				continue;
			}


			// note that, when core > 1, pulled may contain multiple values.
			if( pulled_size < 1 && ( !SystemStatus::isNoReadyLeft || SystemStatus::appendingReadyCount > 0 )){
				/*
				 * Pulled nothing, but ready queue is still being appended.
				 * Could be ProcessScheduler lagging.
				 *
				 * */
				continue;
			}

			if( pulled_size > 0 && last_pull ){
				/*
				 * last pull not empty?
				 * Give it another go.
				 *
				 * */
				CodeExecutor::log("lag","Process Scheduler was lagging.");
				last_pull = false;
			}


			if( last_pull && pulled_size < 1 ){
				CodeExecutor::log("bye","My job ends here");
				SystemStatus::isSystemRunning = false;
				return;
			}



			PCB* running_pcb = pulled[0]; // only for core = 1
			CodeExecutor::CMP = running_pcb->saved_register_CMP; // reloads saved registers
			CodeExecutor::log("pull", "Pulled from running Queue. PID: " + std::to_string(running_pcb->PID));
			isTimesUp = false;
			isTimesUp = false;

			int running_pcb_max_line = running_pcb->codePointer->codeSequence.size();

			auto time_start = std::chrono::high_resolution_clock::now();
			for( ; running_pcb->currentInstructionIndex < running_pcb_max_line; running_pcb->currentInstructionIndex ++ ){

				if(SystemStatus::displayCode){
					CodeExecutor::log("code", running_pcb->codePointer->codeSequence[running_pcb->currentInstructionIndex]);
				}
				if ( parseAndRun( running_pcb->
							codePointer->
							codeSequence[running_pcb->currentInstructionIndex], running_pcb ) 
						== false ){

					if(running_pcb->currentInstructionIndex + 1 < running_pcb_max_line){
						/* block, only when there's instruction left. 
						 * otherwise just kill it.
						 * */

						MemorySpace::blockedQueue_push_back( running_pcb, err_request );
						// push PCB + Item

						isBlocked = true;
						running_pcb->currentInstructionIndex++;
						/* 
						 * Why currentInstructionIndex++ ?
						 *
						 * Because: after being blocked
						 * at one point, the PCB is revived due to a "release".
						 *
						 * function CodeExecutor::reviveBlocked():
						 * 	- runs the request instead
						 * 	- so the line "REQ" is skipped.
						 *
						 * */

						break;
					}
				}
				else{
					isBlocked = false;
				}

				auto time_ends = std::chrono::high_resolution_clock::now();
				long int elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(time_ends - time_start).count();
				// CodeExecutor::log("test", "Elapsed Time: " + std::to_string( elapsed ) + " milis.");


				if(running_pcb->currentInstructionIndex + 1 < running_pcb_max_line){
					/* set time's up only when there's still instruction left
					 * otherwise just kill it.
					 * */

					if( elapsed >= SystemStatus::maxTimeChunck ){
						isTimesUp  = true;

						CodeExecutor::log("time","Time\'s up for ID: "+std::to_string(running_pcb->PID));

						running_pcb->priority --;
						CodeExecutor::appendReadyQueue( running_pcb );
						/*
						 * excuse my bird brain.
						 *
						 * ProcessScheduler could fetch the signal "isRunningEmpty" first
						 * before this thread append the PCB back to ready queue
						 * consequently choosing the process with a lower priority.
						 * ( assumming this thread is somehow extra slow )
						 *
						 * Appending the ready thread here should be a serial bahaviour.
						 * DO NOT launch appendReadyQueue with a thread (here).
						 *
						std::thread thread_append( CodeExecutor::appendReadyQueue, running_pcb );
						thread_append.join();
						*/

						running_pcb->currentInstructionIndex ++;
						break;
					}
					else{
						isTimesUp = false;
					}
				}
			}

			/*
			 * Whatever the reason would be,
			 * when the code reaches here, it's either:
			 * 	blocked / time's up / process ends.
			 *
			 * Then there shall be a new running process.
			 * aka. ProcessScheduler may prepare beforehand.
			 * Thus set the "signal" isRunningEmpty to "true"
			 * and then finish the rest of the chores.
			 *
			 * */
			SystemStatus::isRunningEmpty = true;
			MemorySpace::runningQueueCurrentLength --;
			running_pcb->saved_register_CMP = CodeExecutor::CMP;

			/* 
			 *
			 * if not blocked, not interrupted,
			 * a process reachs here only because it's done.
			 *
			 * delete running_pcb;
			 *
			 * */
			if( running_pcb->currentInstructionIndex >= running_pcb_max_line ){

				int del = running_pcb->PID;
				Item release_item;
				release_item.id = running_pcb->PID;
				/* Disable output when recycling a dying process. */


				// release resource & delete table entry
				std::unique_lock<std::mutex> guard(*SystemStatus::tableOperation);
				ModuleBanker::releaseResource( release_item, false );
				guard.unlock();

				delete running_pcb;

				CodeExecutor::log("~","PID: "+std::to_string(del)+", Died.");

				/* 
				 * Whenever there's a trigger( resource release ), 
				 * check blocked queue. 
				 * But this routine shouldn't stop other tasks from running,
				 * thus spawning an extra thread for reviving.
				 *
				 * */
				
				SystemStatus::isRevivingBlockedProcess = true;
				CodeExecutor::reviveBlocked(); // thread isn't needed. This won't take long.
				SystemStatus::isRevivingBlockedProcess = false;
				/*
				std::thread thread_revive_blocked( CodeExecutor::reviveBlocked );
				thread_revive_blocked.join();
				*/

			}
			// else it's either blocked or went to ready queue.

			pulled.erase( pulled.begin() );

			pulled_size = pulled.size();
		}


		if( pulled_size < 1 
				&& SystemStatus::isNoReadyLeft 
				&& SystemStatus::isNoJobLeft 
				&& SystemStatus::appendingReadyCount < 1 
				&& MemorySpace::runningQueueCurrentLength < 1
				&& MemorySpace::blockedQueueCurrentLength < 1
		  ){
			CodeExecutor::log("bye","My job ends here");
			SystemStatus::isSystemRunning = false;
			return;
		}
		else{
			// else shit hits the fan
			/*
			CodeExecutor::log("check","Pulled: "+std::to_string(pulled_size));
			CodeExecutor::log("check","isNoReadyLeft: "+std::to_string(SystemStatus::isNoReadyLeft));
			CodeExecutor::log("check","isNoJobLeft: "+std::to_string(SystemStatus::isNoJobLeft));
			CodeExecutor::log("check","appending ready: "+std::to_string(SystemStatus::appendingReadyCount));
			CodeExecutor::log("check","running queue: "+std::to_string(MemorySpace::runningQueueCurrentLength));
			CodeExecutor::log("check","blocked queue: "+std::to_string(MemorySpace::blockedQueueCurrentLength));
			CodeExecutor::log("","");
			CodeExecutor::log("","");
			CodeExecutor::log("","");
			*/
		}


	}
}
