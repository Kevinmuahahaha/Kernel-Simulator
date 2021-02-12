#include "DataStructures.h"
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include "ModuleBanker.h"
#include "utils.h"

PCB::PCB(){
	this->PID = -1;
	this->currentInstructionIndex = 0;
	this->saved_register_CMP = false;
	this->spacePointer = nullptr;
	this->codePointer = nullptr;
	/*
	 * DataSpace and CodePointer are initialized by H.L.S
	 *
	 * */
}

PCB::~PCB(){
	//utils::log("~","PCB","ID: " + std::to_string(this->PID) + ", CALLED DELETE");
	
	this->PID = -1;
	
	if( this->codePointer != nullptr ){
		delete this->codePointer;
	}
	if( this->spacePointer != nullptr ){
		delete this->spacePointer;
	}
	
}

void PCB::print(){
	std::stringstream stream;
	stream << "{"
		<< std::endl << "\tPID:" << std::dec << PID
		<< std::endl << "\tPRI:" << std::dec << priority
		<< std::endl << "}" << std::endl;
	std::cout << stream.str() << std::flush;
}

Code::Code(const std::vector<std::string> code_sequence){
	this->codeSequence = code_sequence;
}

Code::Code(){
}
