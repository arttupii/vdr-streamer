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
#include <sys/types.h>
#include <sys/wait.h>
#include <mqueue.h>
#include <stdlib.h>

#define POSIX_QUEUE "/Video_converter_queue"
#define PID_FILE "CONVERT_PID.txt"
sem_t task_sem;

void *runUpdateVdrFilesTask(void *vc)
{
	int i=0;
	while(1)
	{
		if(i>=(60/5))
		{
			CVideoConverter::updateVideos();
			i=0;
		}
		CVideoConverter::checkTaskStatus();
		sleep(5);
		i++;
	}
	pthread_exit(NULL);
}

CVideoConverter::CVideoConverter() {
	// TODO Auto-generated constructor stub
	id_counter=0;
	max_count_video_tasks = 2;
	vdr_video_folder = "/home/video2";
	video_output_folder = "www/video";
	ConfigFile::instance()->get_value("vdr_video_folder", vdr_video_folder);
	ConfigFile::instance()->get_value("max_count_video_tasks", max_count_video_tasks);
	ConfigFile::instance()->get_value("video_output_folder", video_output_folder);

	//updateVideoInfoFromVdrDir();
	pthread_t thread;
	int rc = pthread_create(&thread, NULL, runUpdateVdrFilesTask, NULL);
	if (rc)
	{
		printf("ERROR; return code from pthread_create() is %d\n", rc);
	}

	sem_init(&task_sem, 0, max_count_video_tasks);
}

CVideoConverter::~CVideoConverter() {
	// TODO Auto-generated destructor stub
}

void CVideoConverter::startTask()
{
	printf("CVideoConverter:: Try to start CVideoConverter task\n");

	{

	pid_t pid = fork();
	if(pid==0)
	{
		printf("CVideoConverter:: Creating CVideoConverter\n");
		CVideoConverter cv;
		cv.runTask();
		exit(1);
	}
	if(pid<0)
	{
		printf("CVideoConverter::Cannot create fork\n");
		exit(1);
	}
	}
}

typedef struct{
	CVideoConverter *vc;
	string source;
	string target_folder;
	string target_file;
	string id;
	string pid_file;
} TaskParams;

void CVideoConverter::startNewTask(string id)
{
	list<TaskInfo>::iterator it = findTaskInfo(id);
	if(it!=tasks.end())
	{
		pid_t pid = fork();
		if(pid==0)
		{
			stringstream cmd;
			cmd<<"rm -f "<<PID_FILE;
			system(cmd.str().c_str());
			cmd.str("");
			cmd<<"/bin/sh convert_script.sh \""<<(*it).task_source_folder<<"\" \""<<(*it).task_target_folder<<"\" \""<<(*it).task_target_file_name<<"\" "<<PID_FILE;
			printf("task cmd: %s \n", cmd.str().c_str());
			system(cmd.str().c_str());
			exit(0);
		}
		if(pid>0)
		{
			(*it).task_pid_child = pid;
			for(int i=0;i<5;i++)
			{
				string line;
				ifstream pidfile (PID_FILE);
				if (pidfile.is_open())
				{
					if( pidfile.good() )
					{
					  getline (pidfile,line);
					  (*it).task_pid==line;
					  printf("New task started, id=%s, pid=%s\n", id.c_str(), line.c_str());
					  (*it).task_status="ONGOING";
					  break;
					}
					pidfile.close();
				}
				else
				{
					printf("Cannot open pid file %s yet --> waiting\n", PID_FILE);
					sleep(1);
				}
			}
		}
	}
}
void CVideoConverter::runTask()
{
	struct mq_attr attr, old_attr;   // To store queue attributes
	mqd_t mqdes;             // Message queue descriptors
	unsigned int prio;               // Priority

	attr.mq_maxmsg = 5;
	attr.mq_msgsize = 300;
	attr.mq_flags = 0;

	//mq_unlink (POSIX_QUEUE);

	// Open a queue with the attribute structure
	printf("CVideoConverter::open posix queue\n");
	mqdes = mq_open (POSIX_QUEUE, O_RDWR|O_CREAT, 0664, &attr);
	if(mqdes>0)
	{
		printf("CVideoConverter::posix queue is open\n");
		mq_setattr (mqdes, &attr, &old_attr);
		// Now eat all of the messages
		char buffer[300];
		unsigned int prio;

		updateVideoInfoFromVdrDir();
		write_status_to_disk();

		printf("CVideoConverter::Goto while loop\n");
		ssize_t size;
		while( (size=mq_receive (mqdes, buffer, sizeof(buffer), &prio)) != -1)
		{
			bool update = false;
			map<int, string> params = split(string(buffer,size), ';');
			string msg = params[0];
			string id = params[1];

			//printf("CVideoConverter:: Received posix msg %s\n", string(buffer,size).c_str());

			if("CHECK_TASK_STATUS")
			{
			}

			if("UPDATE_VIDEOS")
			{
				updateVideoInfoFromVdrDir();
				update = true;
			}

			if(msg=="START")
			{
				update = true;
				printf("START\n");
				list<TaskInfo>::iterator it = findTaskInfo(id);
				if(it!=tasks.end())
				{
					if((*it).task_status!="ONGOING" && (*it).task_status!="WAITING" )
					{
						if(getOngoingTaskCount()>=max_count_video_tasks)
						{
							(*it).task_status="WAITING";
							waiting_list.push_back(id);
						}
						else
						{
							startNewTask(id);
						}
					}
					else
					{
						printf("CVideoConverter::Task is already ongoing\n");
					}
				}
				else
				{
					printf("CVideoConverter::Invalid task id %s\n", id.c_str());
				}
			}

			if(msg=="STOP")
			{
				update = true;
				list<TaskInfo>::iterator it = findTaskInfo(id);
				if(it!=tasks.end())
				{
					//write_status_to_disk();
					if((*it).task_status=="ONGOING")
					{
						int status;
						printf("CVideoConverter::Killing task\n");
						system((string("kill ") + (*it).task_pid).c_str());
						kill((*it).task_pid_child, SIGTERM);
						waitpid((*it).task_pid_child, &status, 0);
						printf("CVideoConverter::Task is killed\n");
						(*it).task_status="STOPPED";
						stringstream cmd;
						cmd<<"rm -f \""<<(*it).task_target_folder<<"/"<<(*it).task_target_file_name<<"\"* ";
						system(cmd.str().c_str());
						updateConvertedVideos();
					}
					if((*it).task_status=="WAITING")
					{
						(*it).task_status="STOPPED";
						waiting_list.remove(id);
					}
				}
			}
			if(msg=="DELETE")
			{
				printf("CVideoConverter::Delete video file %s\n", id.c_str());
				list<TaskInfo>::iterator it=convertedFiles.begin();
				for(;it!=convertedFiles.end();it++)
				{
					if((*it).link == id )
					{
						int index = id.rfind(".");
						string file;
						if(string::npos!=index)
						{
							file = id.substr(0,index);
						}
						else
						{
							file = id;
						}
						printf("CVideoConverter::Delete video file  ----> file %s\n", file.c_str());

						system(string(string("rm -f \"") + file + string("\"*")).c_str());
					}
				}
			}
			//task_pid_child

			//Looping tasks if them are ready
			list<TaskInfo>::iterator it=tasks.begin();
			for(;it!=tasks.end();it++)
			{
				if((*it).task_status=="ONGOING")
				{
					int status;
					pid_t p = waitpid((*it).task_pid_child, &status, WNOHANG);
					if(p>0)
					{
						update = true;
						(*it).task_status="DONE";
						stringstream cmd;
						cmd<<"cp -f \""<<(*it).task_info_file<<"\" \""<<(*it).task_target_folder<<"/"<<(*it).task_target_file_name<<".info\"";
						system(cmd.str().c_str());
						printf("%s\n", cmd.str().c_str());
						updateConvertedVideos();
					}
					
				}
			}


			while(getOngoingTaskCount()<max_count_video_tasks)
			{
				if(waiting_list.empty()==false)
				{
					startNewTask(waiting_list.front());
					waiting_list.pop_front();
				}
				else
				{
					break;
				}
			}

			if(update)
			{
				write_status_to_disk();
			}
		}
	}
	else
	{
		printf("CVideoConverter::Cannot open posix queue\n");
	}
}
string CVideoConverter::stopVideoConverting(string id)
{
	if(writeToPosixQueue(string("STOP;")+id)==0)
		return "ok";
	else
		return "error";
}
void CVideoConverter::updateVideos()
{
	writeToPosixQueue("UPDATE_VIDEOS");
}
void CVideoConverter::checkTaskStatus()
{
	CVideoConverter::writeToPosixQueue("CHECK_TASK_STATUS");
}
string CVideoConverter::startVideoConverting(string id)
{
	if(writeToPosixQueue(string("START;")+id)==0)
		return "ok";
	else
		return "error";
}
string CVideoConverter::deleteVideoFile(string file)
{
	if(writeToPosixQueue(string("DELETE;")+file)==0)
		return "ok";
	else
		return "error";
}
void CVideoConverter::setVideoConvertingStatus(string id, string status)
{
	list<TaskInfo>::iterator it=findTaskInfo(id);
	if(it!=tasks.end())
	{
		(*it).task_status=status;
	}
}
string CVideoConverter::getVideoConvertingStatus(string id)
{
	list<TaskInfo>::iterator it=findTaskInfo(id);
	if(it!=tasks.end())
	{
		return (*it).task_status;
	}
	return "";
}

list<TaskInfo>::iterator CVideoConverter::findTaskInfo(string id)
{
        list<TaskInfo>::iterator it=tasks.begin();

        for(;it!=tasks.end();it++)
        {
                if((*it).task_id==id)
                {
                        return it;
                }
        }
        return tasks.end();
}

int CVideoConverter::getOngoingTaskCount()
{
	int ret=0;
	list<TaskInfo>::iterator it=tasks.begin();

	for(;it!=tasks.end();it++)
	{
		if((*it).task_status=="ONGOING")
		{
			ret++;
		}
	}
	return ret;
}

void CVideoConverter::updateConvertedVideos()
{
	stringstream data;

	//printf("CVideoConverter::updateConvertedVideos()  folder=%s\n", video_output_folder.c_str());
	vector<string> filelist;
	CCommon::get_file_list(filelist, video_output_folder.c_str());

	const string infofile = ".info";
	for(int i=0;i<filelist.size();i++)
	{
		string file = filelist[i];
		ssize_t info_index = file.find(infofile);


		if( (info_index + infofile.length()) == file.length() )
		{
			string file_name = file.substr(0,info_index);
			vector<string> videoInfo = getInfo(video_output_folder + string("/") + file);
			if(!videoInfo.empty())
			{
				string channel = videoInfo[0];
				string info = videoInfo[1];
				string name = videoInfo[2];
				string description = videoInfo[3];

				for(int x=0;x<filelist.size();x++)
				{
					if(filelist[x].find(file_name)!=string::npos)
					{
						if(filelist[x].find(".info")==string::npos)
						{
							//printf("CVideoConverter::updateConvertedVideos() %s\n", filelist[x].c_str());
							string file = filelist[x];
							list<TaskInfo>::iterator it=convertedFiles.begin();
							bool found=false;
							for(;it!=convertedFiles.end();it++)
								{
								if((*it).link==file)
								{
									found=true;
									(*it).updated=true;
								}
							}
							if(found==false)
							{
								TaskInfo t;
								t.channel=channel;
								t.name=file;
								t.info=info;
								t.description=description;
								t.updated=true;
								t.link = video_output_folder + file;
								convertedFiles.push_back(t);
							}
							data<<name<<";"<<info<<";"<<name<<";"<<description<<";"<<video_output_folder + file;
							break;
						}
					}
				}

			}
		}
	}
	list<TaskInfo>::iterator it=convertedFiles.begin();
	for(;it!=convertedFiles.end();it++)
	{
		if((*it).updated==false)
		{
			it = convertedFiles.erase(it);
			continue;
		}
		(*it).updated=false;
	}

	FILE * pFile;
	pFile = fopen ("./www/converted_videolist.txt","w");
	if (pFile!=NULL)
	{
	    fputs (data.str().c_str(),pFile);
	    fclose (pFile);
	}
}
bool CVideoConverter::isTaskConverted(TaskInfo ti)
{
	list<TaskInfo>::iterator it=convertedFiles.begin();
	for(;it!=convertedFiles.end();it++)
	{
		if((*it).channel==ti.channel && (*it).name == ti.name && (*it).info==ti.info && (*it).description==ti.description )
		{
			return true;
		}
	}
	return false;
}
void CVideoConverter::updateVideoInfoFromVdrDir()
{
	updateConvertedVideos();
	//printf("CVideoConverter::updateVideoInfoFromVdrDir()\n");
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
						(*it).updated=true;

						if(!isTaskConverted((*it)))
						{
							if((*it).task_status!="ONGOING" && (*it).task_status!="WAITING")
							{
								(*it).task_status="IDLE";
							}
						}
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

					stringstream id;
					id<<id_counter;
					id_counter++;

					t.task_id=id.str();
					t.task_status=status;
					t.task_isRunning=false;
					t.task_source_folder = vdr_video_folder + string("/") + folder;
					t.task_target_folder = video_output_folder;
					t.task_target_file_name = t.task_id + "_" + t.name;
					t.task_pid="";
					t.task_pid_child=-1;
					t.task_info_file=vdr_video_folder + file;
					t.updated=true;

					if(isTaskConverted(t))
					{
						t.task_status="DONE";
					}

					tasks.push_back(t);
				}
			}
		}
	}
	list<TaskInfo>::iterator it=tasks.begin();
	for(;it!=tasks.end();it++)
	{
		if((*it).updated==false)
		{
			it = tasks.erase(it);
			continue;
		}
		(*it).updated=false;
	}
}


void CVideoConverter::write_status_to_disk()
{
	stringstream data;
	list<TaskInfo>::iterator it=tasks.begin();
	for(;it!=tasks.end();it++)
	{
		TaskInfo ti = *it;

		data<<ti.channel<<";"<<ti.info<<";"<<ti.name<<";"<<ti.description<<";"<<ti.task_id<<";"<<ti.task_status<<"\n";
	}

	data.str();

	FILE * pFile;
	pFile = fopen ("./www/videolist.txt","w");
	if (pFile!=NULL)
	{
	    fputs (data.str().c_str(),pFile);
	    fclose (pFile);
	}
	return;
}

vector<string> CVideoConverter::getInfo(string file)
{
	string tmp="";
	ifstream in;
	in.open(file.c_str());

	string channel, info, name, description;
	bool ok=false;
	if (in.is_open())
	{
		 while (!in.eof()) {
			string x;
			getline(in,x);
			switch(x[0])
			{
				case 'C':
					channel=&x.c_str()[2];
					break;
				case 'E':
					info=&x.c_str()[2];
					break;
				case 'T':
					name=&x.c_str()[2];
					ok=true;
					break;
				case 'D':
					description=&x.c_str()[2];
					break;
			}
		 }
	}
	in.close();

	vector<string> v;
	if(ok)
	{
		v.push_back(converInfoString(channel));
		v.push_back(converInfoString(info));
		v.push_back(converInfoString(name));
		v.push_back(converInfoString(description));
	}

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
		case '"':
			x[i]=' ';
		break;
		default:
			break;
		}
	}
	return x;
}

int CVideoConverter::writeToPosixQueue(string text)
{
	 //printf("Write to posix queue %s\n", text.c_str());
	  struct mq_attr attr, old_attr;   // To store queue attributes
	  mqd_t mqdes;             // Message queue descriptors
	  unsigned int prio=0;               // Priority

	  attr.mq_maxmsg = 5;
	  attr.mq_msgsize = 300;
	  attr.mq_flags = 0;

	  // Open a queue with the attribute structure
	  mqdes = mq_open (POSIX_QUEUE, O_RDWR|O_CREAT, 0664, &attr);
	  if(mqdes>0)
	  {
		 // mq_setattr  (mqdes, &attr, &old_attr);'
		  if (mq_send (mqdes, text.c_str(), text.length(), prio) == -1)
		  {
			  perror ("mq_send()");
			  return -1;
		  }
		  //printf("Send posix msg %s\n", text.c_str());
		  // Close all open message queue descriptors
		  mq_close (mqdes);
		  return 0;
	  }
	  else
	  {
		  printf("Cannot open posix queue %s\n", text.c_str());
	  }
	  return -1;
}

map<int, string> CVideoConverter::split(string text, char c)
{
	map<int, string> ret;
	int x=0;
	while(!text.empty())
	{
		ssize_t i = text.find(c);
		if(i==string::npos)
		{
			i = text.length();
			ret[x] = text.substr(0,i);
			break;
		}

		ret[x] = text.substr(0,i);
		text = text.substr(i+1);
		x++;

	}
	return ret;
}
