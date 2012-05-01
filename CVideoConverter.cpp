/*
 * CVideoConverter.cpp
 *
 *  Created on: 30.4.2012
 *      Author: arttu
 */

#include "CVideoConverter.h"
#include <sstream>
#include "CCommon.h"
#include "configfile/configfile.h"
#include <pthread.h>
#include <semaphore.h>

sem_t task_sem;

void *runUpdateVdrFilesTask(void *vc)
{
	while(1)
	{
		((CVideoConverter*)vc)->updateVideoInfoFromVdrDir();
		sleep(60);
	}
	pthread_exit(NULL);
}

CVideoConverter::CVideoConverter() {
	// TODO Auto-generated constructor stub
	id_counter=0;
	int max_count_video_tasks = 2;
	vdr_video_folder = "/home/video";
	ConfigFile::instance()->get_value("vdr_video_folder", vdr_video_folder);
	ConfigFile::instance()->get_value("max_count_video_tasks", max_count_video_tasks);
	updateVideoInfoFromVdrDir();
	pthread_t thread;
	int rc = pthread_create(&thread, NULL, runUpdateVdrFilesTask, (void *)this);
	if (rc)
	{
		printf("ERROR; return code from pthread_create() is %d\n", rc);
	}

	sem_init(&task_sem, 0, max_count_video_tasks);
}

CVideoConverter::~CVideoConverter() {
	// TODO Auto-generated destructor stub
}

CVideoConverter *CVideoConverter::instance()
{
	static CVideoConverter *p = 0;
	if(p==0)
	{
		p = new CVideoConverter();
	}
	return p;
}

typedef struct{
	CVideoConverter *vc;
	string folder;
	string id;
} TaskParams;


void *runTask(void *_params)
{
	TaskParams *params = (TaskParams*)_params;
	sem_wait(&task_sem);
	params->vc->setVideoConvertingStatus(params->id, "ONGOING");
	sleep(30);
	params->vc->setVideoConvertingStatus(params->id, "DONE");

	sem_post(&task_sem);
	pthread_exit(NULL);
}

string CVideoConverter::startVideoConverting(string id)
{
	CQuard quard(mutex);
	list<TaskInfo>::iterator it=findTaskInfo(id);
	if(it!=tasks.end())
	{
		if((*it).status=="ONGOING" || (*it).status=="WAITING" )
		{
			return "already ongoing";
		}
		printf("Converting started... %s\n", it->folder.c_str());
		TaskParams *params = new TaskParams;
		(*it).status="WAITING";
		params->folder=(*it).folder;
		params->id=id;
		params->vc=this;
		pthread_t thread;
		int rc = pthread_create(&thread, NULL, runTask, (void *)params);
		if (rc)
		{
			printf("ERROR; return code from pthread_create() is %d\n", rc);
		}
		return "ok";
	}
	else
	{
		printf("Unkown id %s... Cannot start converting\n", id.c_str());
	}
	return "invalid id";
}

void CVideoConverter::setVideoConvertingStatus(string id, string status)
{
	CQuard quard(mutex);
	list<TaskInfo>::iterator it=findTaskInfo(id);
	if(it!=tasks.end())
	{
		(*it).status=status;
	}
}

list<TaskInfo>::iterator CVideoConverter::findTaskInfo(string id)
{
	list<TaskInfo>::iterator it=tasks.begin();

	for(;it!=tasks.end();it++)
	{
		if((*it).id==id)
		{
			return it;
		}
	}
	return tasks.end();
}

void CVideoConverter::updateVideoInfoFromVdrDir()
{
	vector<string> filelist;
	CCommon::get_file_list(filelist, vdr_video_folder.c_str());

	const string infofile = "/info";
	for(int i=0;i<filelist.size();i++)
	{
		string file = filelist[i];
		ssize_t info_index = file.find(infofile);

		if( (info_index + infofile.length()) == file.length() )
		{
			string folder = file.substr(0,info_index);
			vector<string> videoInfo = getInfo(vdr_video_folder + file);
			if(!videoInfo.empty())
			{
				CQuard quard(mutex);

				string channel = videoInfo[0];
				string info = videoInfo[1];
				string name = videoInfo[2];
				string description = videoInfo[3];
				string status = "IDLE";

				list<TaskInfo>::iterator it=tasks.begin();
				bool found=false;
				for(;it!=tasks.end();it++)
				{
					if((*it).folder==folder)
					{
						found=true;
						break;
					}
				}
				if(found==false)
				{
					TaskInfo t;
					t.channel=channel;
					t.name=name;
					t.info=info;
					t.description=description;
					t.folder=folder;
					t.status=status;
					t.taskRunning=false;

					stringstream id;
					id<<id_counter;
					id_counter++;
					t.id=id.str();
					tasks.push_back(t);
				}
			}
		}
	}
}

string CVideoConverter::getStatus()
{
	CQuard quard(mutex);
	stringstream data;
	list<TaskInfo>::iterator it=tasks.begin();
	for(;it!=tasks.end();it++)
	{
		TaskInfo ti = *it;

		data<<ti.channel<<";"<<ti.info<<";"<<ti.name<<";"<<ti.description<<";"<<ti.id<<";"<<ti.status<<"\n";
	}

	return data.str();
}

vector<string> CVideoConverter::getInfo(string file)
{
	string tmp="";
	ifstream in;
	in.open(file.c_str());

	if (in.is_open())
	{
		 while (!in.eof()) {
			string x;
			getline(in,x);
		    tmp+=x;
		 }
	}
	in.close();

	int x,y;
	vector<string> v;

	x=tmp.find(" ",3)+1;
	y=tmp.find("E ");
	if(x==string::npos || y==string::npos) return v;
	string channel = tmp.substr(x, y-x);

	x=tmp.find("E ")+2;
	y=tmp.find("T ");
	if(x==string::npos || y==string::npos) return v;
	string info = tmp.substr(x, y-x);


	x=tmp.find("T ")+2;
	y=tmp.find("D ");
	if(x==string::npos || y==string::npos) return v;
	string name = tmp.substr(x, y-x);

	x=tmp.find("D ")+2;
	y=tmp.find("X ");
	if(x==string::npos || y==string::npos) return v;
	string description = tmp.substr(x, y-x);

	v.push_back(converInfoString(channel));
	v.push_back(converInfoString(info));
	v.push_back(converInfoString(name));
	v.push_back(converInfoString(description));

	return v;
}

string CVideoConverter::converInfoString(string x)
{
	for(size_t i=0;i<x.length();i++)
	{
		switch(x[i])
		{
		case '\n':
		case ';':
		case ':':
		case '\r':
			x[i]=' ';
		break;
		default:
			break;
		}
	}
	return x;
}


