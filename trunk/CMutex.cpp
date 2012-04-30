/*
 * CMutex.cpp
 *
 *  Created on: 30.4.2012
 *      Author: arttu
 */

#include "CMutex.h"

CMutex::CMutex() {
	mutex = PTHREAD_MUTEX_INITIALIZER;
}

CMutex::~CMutex() {

}

void CMutex::lock()
{
	pthread_mutex_lock( &mutex );
}
void CMutex::unlock()
{
	pthread_mutex_unlock( &mutex );
}
