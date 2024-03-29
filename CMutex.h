/*
 * CMutex.h
 *
 *  Created on: 30.4.2012
 *      Author: arttu
 */

#ifndef CMUTEX_H_
#define CMUTEX_H_
#include <pthread.h>

class CMutex {
public:
	CMutex();
	virtual ~CMutex();
	void lock();
	void unlock();
private:
	pthread_mutex_t mutex;
};

class CQuard
{
public:
	CQuard(CMutex &_m)
	{
		this->m=&_m;
		m->lock();
	}
	CQuard(CMutex *_m)
	{
		this->m=_m;
		m->lock();
	}
	~CQuard()
	{
		m->unlock();
	}
private:
	CMutex *m;
};
#endif /* CMUTEX_H_ */
