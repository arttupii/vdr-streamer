
#include"CSocket.h"
using namespace std;

#include <iostream>
#include <string>
#include <string.h>
// Required by for routine
#include <sys/types.h>
#include <unistd.h>

#include <stdlib.h>   // Declaration for exit()
//#include <syslog.h>
using namespace std;
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

#include "configfile/configfile.h"
#include "CHttpRequestHandler.h"

#include "CVdrLinks.h"
#include "CVideoConverter.h"

#include <list>
#include <sys/wait.h>
#define CONFIG_FILE "vdr-streamer.conf"

void termination_handler (int signum)
{
	printf("EXIT\n");
	exit(0);
}

#include"CCommon.h"

int main()
{

	if(!ConfigFile::instance()->open_configFile(CONFIG_FILE))
	{
		//syslog(0,"Cannot open config file, %s", CONFIG_FILE);
		printf("Cannot open config file, %s", CONFIG_FILE);
	}

	printf("vdr-streamer starter\n");
	CVideoConverter::startTask();

	struct sigaction new_action, old_action;

	  /* Set up the structure to specify the new action. */
	  new_action.sa_handler = termination_handler;
	  sigemptyset (&new_action.sa_mask);
	  new_action.sa_flags = 0;

	  sigaction (SIGINT, NULL, &old_action);
	  if (old_action.sa_handler != SIG_IGN)
	    sigaction (SIGINT, &new_action, NULL);
	  sigaction (SIGHUP, NULL, &old_action);
	  if (old_action.sa_handler != SIG_IGN)
	    sigaction (SIGHUP, &new_action, NULL);
	  sigaction (SIGTERM, NULL, &old_action);
	  if (old_action.sa_handler != SIG_IGN)
	    sigaction (SIGTERM, &new_action, NULL);



	CVdrLinks::instance()->update();

	string sin_addr="0.0.0.0";
	ConfigFile::instance()->get_value("in_addr", sin_addr);
	printf("in_addr --> %s\n", sin_addr.c_str());

	int port=50000;
	ConfigFile::instance()->get_value("server port", port);
	int s = create_server(sin_addr, port);
	int c;
	list<pid_t> children;
	printf("in_addr --> %s     %d\n", sin_addr.c_str(), port);
	while((c=wait_connection(s))>0)
	{
		pid_t pid = fork();
		if(pid==0)
		{
			close_socket(s);
			printf("Client connected %d", (int)c);
			CHttpRequestHandler rec(c);
			rec.handle();
			close_socket(c);
			exit(0);
		}
		close_socket(c);

		if(pid>0)
		{
			children.push_back(pid);
		}
		list<pid_t>::iterator it = children.begin();
		for(;it!=children.end();it++)
		{
			int status;
			pid_t rep = waitpid((*it), &status, WNOHANG);
			if(rep!=0)
			{
				it=children.erase(it);
			}
		}
	}
	close_socket(s);
}
