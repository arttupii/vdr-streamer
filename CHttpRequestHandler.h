/*
 * CHttpRequestHandler.h
 *
 *  Created on: Apr 22, 2012
 *      Author: arttu
 */

#ifndef CHTTPREQUESTHANDLER_H_
#define CHTTPREQUESTHANDLER_H_
#include <string>
#include <sstream>
#include <map>
#include <string>
#include <list>
#include <stdlib.h>

using namespace std;

class CHttpRequestHandler {
public:
	CHttpRequestHandler(int socket);
	virtual ~CHttpRequestHandler();

	void handle();
private:
	bool handleStreamFile(string file);
	string urlDecoding(string url);

	string getVirtualFile(string file);

	bool getLine2(string &line, char &lastChar, char c);
	int socket;

	void clearParsetHeaderInfo();

	void handleGetPost();
	int send_from_socket_to_another(int from, int to);
	bool startStreamScript(const char input[], int port, const char pid_file[]);

	void sendFile(string file);
	string getContenType(string file);
	map<string,bool>  get_allowed_files();

	map<string, string> content_type_map;
	map<string,bool> allowed_files;

	string post_line;
	string get_line;
	int content_length;
	stringstream content;
	string host;

	string video_type;
};


#endif /* CHTTPREQUESTHANDLER_H_ */
