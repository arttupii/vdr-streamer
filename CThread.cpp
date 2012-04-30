/*
 * CThread.cpp
 *
 *  Created on: 30.4.2012
 *      Author: arttu
 */

#include "CThread.h"

void *start_thread( void *ptr )
{
	CThread *t = (CThread*)ptr;
	t->running = true;
	t->run();
	t->running = false;
}
CThread::CThread()
{
	running=false;
}

CThread::~CThread()
{
	stop();
}

void CThread::start()
{
	int iret1 = pthread_create( &thread, NULL, start_thread, (void*) this);
}
void CThread::stop()
{
	if(running==true)
	{
		pthread_cancel(thread);
		join();
	}
}
void CThread::join()
{
	pthread_join(thread, NULL);
}
bool CThread::isRunning()
{
	return running;
}
