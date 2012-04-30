
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

#include <pthread.h>

#include "CVdrLinks.h"
#include "CVideoConverter.h"

pid_t start_sub_process(string cmd)
{
	pid_t pID = fork();
	if (pID == 0)                // child
	{
		system(cmd.c_str());
		exit(0);
	}
	else if (pID < 0)            // failed to fork
	{
		cerr << "Failed to fork" << endl;

	}
	
      // Code only executed by parent process
	return pID;
}
                



#define CONFIG_FILE "vdr-streamer.conf"

void *serverClient(void *socket)
{
	long c = (long)socket;
	printf("Client connected %d", c);
	CHttpRequestHandler rec(c);
	rec.handle();
	//sleep(1);
	close_socket(c);
	pthread_exit(NULL);
}

void termination_handler (int signum)
{
	printf("EXIT\n");
	exit(0);
}



int main()
{
	CVideoConverter::instance();
	CVdrLinks::instance();


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

	//openlog("TESTI", LOG_NOWAIT, LOG_USER);

	if(!ConfigFile::instance()->open_configFile(CONFIG_FILE))
	{
		//syslog(0,"Cannot open config file, %s", CONFIG_FILE);
		printf("Cannot open config file, %s", CONFIG_FILE);
	}

	CVdrLinks::instance()->update();

	string sin_addr="0.0.0.0";
	ConfigFile::instance()->get_value("in_addr", sin_addr);
	printf("in_addr --> %s\n", sin_addr.c_str());

	int port=50000;
	ConfigFile::instance()->get_value("server port", port);
	int s = create_server(sin_addr, port);
	//syslog(0,"Server created successfully %d", s);
	int c;
	while((c=wait_connection(s))>0)
	{
		pthread_t thread;
		pthread_create(&thread, NULL, serverClient, (void *)c);
	}
	close_socket(s);
}
