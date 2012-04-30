/*
 * CThread.h
 *
 *  Created on: 30.4.2012
 *      Author: arttu
 */

#ifndef CTHREAD_H_
#define CTHREAD_H_
#include <pthread.h>

class CThread {
public:
	CThread();
	virtual ~CThread();
	virtual void run()=0;
	void start();
	void stop();
	void join();
	bool isRunning();
private:
	pthread_t thread;
	bool running;
	void friend *start_thread( void *ptr );
};

#endif /* CTHREAD_H_ */
