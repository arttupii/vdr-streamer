/*
 * CVdrLinks.h
 *
 *  Created on: Apr 24, 2012
 *      Author: arttu
 */

#ifndef CVDRLINKS_H_
#define CVDRLINKS_H_
#include <string>
#include <map>
#include <list>
using namespace std;

typedef struct{
	string link;
	string name;
	string fileName;
} VdrLink;

class CVdrLinks {
private:
	CVdrLinks();
public:
	void update();
	list<VdrLink> get();
	string getLink(string fileName);
	virtual ~CVdrLinks();
	static CVdrLinks* instance();
private:

	list<VdrLink> links;
};

#endif /* CVDRLINKS_H_ */
