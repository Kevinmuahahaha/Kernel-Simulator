#ifndef INCLUDE_DATA_STRUCTURES
#define INCLUDE_DATA_STRUCTURES
#include <vector>
#include <string>
#include <utility>
#include "utils.h"
class Code{
	public:
		Code(const std::vector<std::string> code_sequence);
		Code();
		std::vector<std::string> codeSequence;
};

class DataSpace{
	public:
		/* "varname","string value" */
		std::vector<std::pair<std::string,std::string>> variablePairs;

		void inline print(){
			// for testing only
			for(int i=0; i<this->variablePairs.size(); i++){
				utils::log("print","test",variablePairs[i].first + " : " + variablePairs[i].second);
			}
		}

		void inline storeVariableWithName( std::string var_name, std::string var_content ){
			for(int i=0; i<variablePairs.size(); i++){
				if( variablePairs[i].first == var_name ){
					/* Update variable instead of creating new ones. */
					variablePairs[i].second = var_content;
					return;
				}
			}

			std::pair<std::string, std::string> stored_pair;
			stored_pair.first = var_name;
			stored_pair.second = var_content;
			this->variablePairs.push_back( stored_pair );
		}
		inline const std::string* getContentByName( std::string var_name ){
			std::string* pointer_to_string = nullptr;
			for(int i=0; i<this->variablePairs.size(); i++){
				if( this->variablePairs[i].first == var_name ){
					pointer_to_string = &this->variablePairs[i].second;
				}
			}
			// will return "nullptr" when nothing was found.
			// pointer should't be deleted, one should just copy it's content.
			return pointer_to_string;
		}
};

class PCB{
	public:
		PCB();
		int PID;
		int currentInstructionIndex;
		int priority;
		bool saved_register_CMP;
		Code* codePointer;
		DataSpace* spacePointer;

		/* delete codePointer, delete spacePointer */
		~PCB();

		void print();
};

class Item{
	public:
		int id;
		std::vector<int> request;
};

#endif
