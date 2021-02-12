#include <iomanip>
#include <sstream>
#include "ModuleBanker.h"
#include "SystemStatus.h"
#include <iostream>
#include <utility>
#include <vector>

using namespace std;
#define FALSE 0
#define TRUE 1

int ModuleBanker::findIndexById(int id){
	for(int index=0; index<SystemStatus::Max.size(); index++){
		if( SystemStatus::Max[index].first == id ){
			return index;
		}
	}
	/* find index from max, because
	 * max, allocation, need are deleted at the same time.
	 *
	 * return -1 when nothing's found */
	return -1;
}

/*1. 接受request */
bool ModuleBanker::solveRequest(const Item &item, bool enabled_log )
{

	// std::unique_lock<std::mutex> guard(*vector_mutex);
	if(enabled_log){
		showdata();
	}
	stringstream stream;
	vector<int> request;
	for(int i=0;i<item.request.size();i++) request.push_back(item.request[i]);
	int id = item.id;
	int index = findIndexById(id);
	if(index<0)
	{
		ModuleBanker::log("crit","申请的进程id不在表中");
		return false;
	}

	for (int j = 0; j < SystemStatus::available.size(); j++)
	{
		if(request[j]>SystemStatus::need[index].second[j])
		{
			stream.str("");

			stream<<id ;
			stream<<"号进程";
			stream<<"申请的资源数>进程";
			stream<<id ;
			stream<<"号进程还需要";
			stream<<j;
			stream<<"类资源的资源量"<<endl;
			
			if(enabled_log){
				ModuleBanker::print( stream.str() );
			}

			return false;

		}
		else 

			if (request[j]>SystemStatus::available[j])
			{

				stream.str("");

				stream<<"进程";
				stream<<id;
				stream<<"申请的资源数大于系统可用";
				stream<<j;
				stream<<"类资源的资源量"<<endl;

				if(enabled_log){
					ModuleBanker::print( stream.str() );
				}
				return false;
			}
	}
	//changedata()符合条件，开始试分配资源
	for (int j = 0; j < request.size(); j++)
	{
		SystemStatus::available[j]=SystemStatus::available[j]-request[j];
		SystemStatus::allocation[index].second[j]=SystemStatus::allocation[index].second[j]+request[j];
		SystemStatus::need[index].second[j]=SystemStatus::need[index].second[j]-request[j];
	}

	//分配资源完，检测安全性				  
	if (chkerr(index))
	{
		//不安全，回收资源rstordata()，返回false;
		for (int j = 0; j < request.size(); j++)
		{
			SystemStatus::available[j]=SystemStatus::available[j]+request[j];
			SystemStatus::allocation[index].second[j]=SystemStatus::allocation[index].second[j]-request[j];
			SystemStatus::need[index].second[j]=SystemStatus::need[index].second[j]+request[j];
		}
		return false;
	}

	else{
		return true;//安全，返回ture
	}
}

/* 2. 释放资源 */
void ModuleBanker::releaseResource( const Item &item, bool enabled_log ){
	//	当进程结束后，释放所有 Allocation
	int id =item.id;
	int index = findIndexById(id);
	if(index<0)
	{
		ModuleBanker::log("crit","申请的进程id不在表中");
	}
	else
	{
		for (int j = 0; j < SystemStatus::available.size(); j++)
		{
			SystemStatus::available[j]=SystemStatus::available[j]+SystemStatus::allocation[index].second[j];//可用资源回收，available增加
			SystemStatus::allocation[index].second[j]=SystemStatus::allocation[index].second[j]-SystemStatus::allocation[index].second[j];//该进程的allocation清零
		}

		/* remove it's table entry */
		int delete_table_index = ModuleBanker::findIndexById( id );
		if( delete_table_index != -1 ){
			SystemStatus::Max.erase( SystemStatus::Max.begin() + delete_table_index );
			SystemStatus::allocation.erase( SystemStatus::allocation.begin() + delete_table_index );
			SystemStatus::need.erase( SystemStatus::need.begin() + delete_table_index );
		}

		ModuleBanker::log("done","任务完成，已经释放资源");
		if( enabled_log ){
			showdata();
		}

	}
}

//显示数组
void ModuleBanker::showdata(){
	int i,j;
	stringstream stream;
	stream << endl;
	stream<< setw(20) << "[Available]\n"<< endl;
	stream<< " 资源类别：";
	for(int resource_type_index=0; resource_type_index<SystemStatus::supportedResourceCount; resource_type_index++)
	{
		stream << setw(4) << (char)('A' + resource_type_index) << " | ";
	}
	stream << endl;
	stream<<" 资源数目：";
	for (j=0;j<SystemStatus::available.size();j++)
	{
		stream << setw(4) << SystemStatus::available[j] << " | ";
	}
	stream<<endl;
	stream<<endl;
	stream<< setw(20) << "[NEED]\n" << endl;
	stream<<" 资源类别：";

	for(int resource_type_index=0; resource_type_index<SystemStatus::supportedResourceCount; resource_type_index++)
	{
		stream << setw(4) << (char)('A' + resource_type_index) << " | ";
	}

	stream << endl;
	for (i=0;i<SystemStatus::need.size();i++) //need的行数
	{
		stream << setw(3) << SystemStatus::need[i].first<<"号进程："; //第i个索引的进程id

		for (j=0;j<SystemStatus::need[i].second.size();j++) //need资源的列数
		{
			stream << setw(4) << SystemStatus::need[i].second[j] << " | ";
		}
		stream<<endl;
	}
	stream<<endl;
	//stream<<"各进程已经得到的资源量：" << endl;
	stream<< setw(20) << "[Allocation]\n" << endl;
	stream<<" 资源类别：";

	for(int resource_type_index=0; resource_type_index<SystemStatus::supportedResourceCount; resource_type_index++)
	{
		stream << setw(4) << (char)('A' + resource_type_index) << " | ";
	}

	stream << endl;
	for (i = 0; i < SystemStatus::allocation.size(); i++)
	{
		stream<< setw(3) << SystemStatus::allocation[i].first <<"号进程：";
		for (j=0;j<SystemStatus::allocation[i].second.size();j++)
		{
			stream << setw(4) << SystemStatus::allocation[i].second[j] << " | ";
		}
		stream<<endl;
	}
	stream<<endl;
	ModuleBanker::print( stream.str() );
}


//安全性检查函数 输入进程的排队序号index
int ModuleBanker::chkerr(int index){
	vector<int> work;
	vector<int>	finish;
	vector<int> temp;
	stringstream stream;
	int i=0,j=0,p=0;
	for(i=0;i<SystemStatus::need.size() ;i++) finish.push_back(FALSE);//将所有的进程标志置位false
	for(int r=0;r<SystemStatus::available.size();r++) //首先将available的值赋予work
	{

		int m =SystemStatus::available[r];
		work.push_back(m);
	}

	i=index;//从申请资源的进程开始检查安全性
	while(i<SystemStatus::need.size()){
		if(finish[i]==FALSE){
			//满足条件释放资源，并从头开始扫描进程集合 
			while (p <SystemStatus::available.size() && SystemStatus::need[i].second[p]<=work[p] ) p++;
			if (p ==SystemStatus::available.size()) //如果三个资源都满足
			{
				for (int j = 0; j < SystemStatus::available.size(); j++)
				{
					work[j]=work[j]+SystemStatus::allocation[i].second[j];
				}
				finish[i]=TRUE;
				int id = SystemStatus::need[i].first;
				temp.push_back(id); //记录分配资源给进程的id
				i=0;

			}
			else i++;

			p=0;	
		}	
		else
		{
			i++;
		}
	}
	for ( i = 0; i < SystemStatus::need.size(); i++)
		if (finish[i]==FALSE)
		{
			ModuleBanker::log("bad","系统不安全！本次资源申请不成功！");
			return 1;
		}
	stream << endl;
	stream << "经安全性检查，系统安全，本次分配成功";
	stream << "本次安全序列：";
	stream << "进程id依次为";
	for (int i = 0; i < SystemStatus::need.size(); i++)
	{
		stream << temp[i] << "->";
	}
	stream << "\n" << endl;
	ModuleBanker::print( stream.str() );
	return 0;
}

