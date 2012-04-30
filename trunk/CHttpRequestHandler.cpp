/*
 * CHttpRequestHandler.cpp
 *
 *  Created on: Apr 22, 2012
 *      Author: arttu
 */

#include "CHttpRequestHandler.h"
#include "CSocket.h"
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include "CVdrLinks.h"
//#include <syslog.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include "configfile/configfile.h"
#include "CCommon.h"

//pid_t popen2(const char *command, int *infp, int *outfp);

CHttpRequestHandler::CHttpRequestHandler(int socket) {
	// TODO Auto-generated constructor stub
	this->socket=socket;

	content_type_map[".html"]="text/html; charset=UTF-8";
	content_type_map[".htm"]="text/html; charset=UTF-8";
	content_type_map[".txt"]="text/plain; charset=UTF-8";
	content_type_map[".jpg"]="image/jpeg";
	content_type_map[".png"]="image/png";
	content_type_map[".js"]="text/javascript";
	content_type_map[".m3u"]=="text/plain; charset=UTF-8";

	allowed_files = get_allowed_files();

	video_type=".stream";
}

CHttpRequestHandler::~CHttpRequestHandler() {
	// TODO Auto-generated destructor stub
}

bool CHttpRequestHandler::getLine2(string &line, char &lastChar, char c)
{
	if(c=='\b')
	{
		if(line.length()>0)
		{
			line=line.substr(0,line.length()-1);
		}
	}
	else if( c=='\n' || c=='\r' )  //\r\n
	{
		if((lastChar=='\r'&&c=='\n') || (lastChar!='\r'&&c=='\n'))
		{
			lastChar=0;
			return true;
		}
	}
	else
	{
		line+=c;
	}
	lastChar=c;
	return false;
}

string CHttpRequestHandler::getContenType(string file)
{
	string sep = ".txt";
	if(file.rfind(".")!=string::npos)
	{
		sep = file.substr(file.rfind("."));
	}

	map<string,string>::iterator it = content_type_map.find(sep);
	if(it!=content_type_map.end())
	{
		return it->second;
	}

	return "text/plain; charset=UTF-8";
}
map<string,bool>  CHttpRequestHandler::get_allowed_files()
{
	map<string,bool>  ret;
	const char *dir_path="./www";
	vector<string> filelist;
	CCommon::get_file_list(filelist, dir_path);
	for(int i=0;i<filelist.size();i++)
	{
		printf("%s\n", filelist[i].c_str());
		ret[filelist[i]]=true;
	}
	return ret;
}
string CHttpRequestHandler::getVirtualFile(string file)
{
	stringstream data;
	printf("DEBUG %s \n", file.c_str());
	if(file=="/channels.txt")
	{
		list<VdrLink> links = CVdrLinks::instance()->get();
		list<VdrLink>::iterator it = links.begin();
		for(;it!=links.end();it++)
		{
			data<<(*it).name<<";"<<(*it).fileName<<"\n";
		}
	}
	if(file=="/playlist.m3u")
	{
		list<VdrLink> links = CVdrLinks::instance()->get();
		list<VdrLink>::iterator it = links.begin();
		data<<"#EXTM3U\n";
		int i=1;
                for(;it!=links.end();it++)
                {
                        data<<"#EXTINF:-1,"<<i<<" "<<(*it).name<<"\n";
			if(params["serverUrl"].empty())
			{
				data<<"http://"<<host<<"/"<<(*it).fileName<<"\n";
			}
			else
			{
				 data<<params["serverUrl"]<<"/"<<(*it).fileName<<"\n";
			}
			i++;
                }
	}
	return data.str();
}
int CHttpRequestHandler::send_from_socket_to_another(int from, int to)
{
	char buffer[4*1024];
	int size = receive_from_socket(from, buffer, sizeof(buffer), true);
	if(size==0)
	{
		//syslog(0,"send_from_socket_to_another(from=%d, to=%d): close socket", from, to);
		return -1;
	}
	else if(size<0 && errno!=EAGAIN)
	{
		//syslog(0,"send_from_socket_to_another(from=%d, to=%d): close socket %s", from, to, strerror(errno));
		return -1;
	}
	else if(size>0)
	{
		return send_to_socket(to, buffer, size);
	}
	return 0;
}

bool CHttpRequestHandler::startStreamScript(const char input[], int port, const char pid_file[])
{
	char buf[200];
	snprintf(buf, sizeof(buf),"/bin/sh stream_script.sh %s %d %s", input, port, pid_file);
	if(system(buf)==0)
		return true;
	return false;
}

bool CHttpRequestHandler::handleStreamFile(string file)
{
	if(file.length()<2) return false;
	string link = CVdrLinks::instance()->getLink(file.substr(1));
	if(link.empty()==false)
	{
		//string channel = file.substr(1,index-1);
		


		printf("VDR Stream: link %s\n", link.c_str());
		if(link.empty()) return false;

			stringstream pid_file;
			pid_file<<"pid_"<<this;
			stringstream cmd;
			cmd<<"/bin/sh stream_script.sh "<<link<<" "<<pid_file.str();

			//stringstream resp;
			//resp<<"HTTP/1.0 200 OK\r\n";
			//resp<<"Content-Type: video/"<<video_type<<"\r\n\r\n";

			//send_to_socket(socket, resp.str().c_str(), resp.str().length());

			printf("Open pipe\n");

			FILE *pipef = popen(cmd.str().c_str(), "r");

			int d = fileno(pipef);
			fcntl(d, F_SETFL, O_NONBLOCK);

			printf("pipe on auki \n");
			char buffer[1024*16];

			int loop=0;

			ssize_t  min=99999, max=0, k=0,kt=0;
			ssize_t  i=0;
			while(true)
			{
				ssize_t r = read(d, buffer, sizeof(buffer));
				if (r == -1 && errno == EAGAIN)
				{
					loop++;
					usleep(1000);
				}
				else if (r > 0)
				{
					loop=0;
					if(send_to_socket(socket, buffer, r)<=0) break;
					if(min>r) min = r;
					if(max<r) max = r;
					i++;
					kt+=r;
					if(i==1000)
					{
						i=0;
						k=kt/1000;
						kt=0;
					}
				}
				else
				{
					break;
				}
				if(loop>10000) break;

				int size = receive_from_socket(socket, buffer, sizeof(buffer), true);
				if(size==0)
				{
					break;
				}
				else if(size<0 && errno!=EAGAIN)
				{
					break;
				}
				else if(size>0)
				{
					printf("Saatu jottain???\n");
				}

			}
			printf("min %d, max%d, k=%d\n", min,max,k);
			cmd.str("");
			cmd<<"kill `cat "<<pid_file.str()<<"` ; rm "<<pid_file.str();
			system(cmd.str().c_str());
			pclose(pipef);


			//kill(pid, SIGKILL);
			//system(cmd.str().c_str());


			printf("Pipe close\n");

		/*
		stringstream pid;
		pid<<"pid_"<<this;
		startStreamScript(link.c_str(), 8086, pid.str().c_str());
		sleep(5);

		int vlc_socket = create_client("127.0.0.1", 8086);

		stringstream req;
		req<<"GET / HTTP/1.0\r\n";
		req<<"Host: 127.0.0.1\r\n";
		req<<"Connection: close\r\n";
		req<<"User-Agent: streamer\r\n";
		req<<"Accept-Charset: ISO-8859-1,UTF-8;q=0.7,*;q=0.7\r\n";
		req<<"Cache-Control: no-cache\r\n";
		req<<"Accept-Language: de,en;q=0.7,en-us;q=0.3\r\n\r\n";

		send_to_socket(vlc_socket, req.str().c_str(), req.str().length());

		while (vlc_socket>=0)
		{
			int ret;
			bool sleep=false;
			ret = send_from_socket_to_another(socket, vlc_socket);
			if(ret<0) break;
			if(ret==0) sleep=true;

			ret = send_from_socket_to_another(vlc_socket, socket);
			if(ret<0) break;
			if(ret==0) sleep=true;

			if(sleep) usleep(10000);
		}

		//kill(pSubProcess, SIGKILL);
		stringstream cmd;
		cmd<<"kill `cat "<<pid.str()<<"` ; rm "<<pid.str();
		system(cmd.str().c_str());
		printf("close socket + %s\n", cmd.str().c_str());
		close(vlc_socket);
		return true;
		*/
		return true;
	}
	return false;
}
void CHttpRequestHandler::sendFile(string file)
{
	file = urlDecoding(file);
	if(handleStreamFile(file)) return;

	string virtualFile=getVirtualFile(file);
	FILE * pFile=NULL;
	unsigned int lSize=0;

	if(virtualFile.empty())
	{
		if(file=="/") file = "/index.html";
		if(allowed_files.find(file)!=allowed_files.end())
		{
			if(file=="/") file = "/index.html";
			file = "./www/" + file;

			pFile = fopen ( file.c_str() , "rb" );
			if(pFile)
			{
				// obtain file size:
				fseek (pFile , 0 , SEEK_END);
				lSize = ftell (pFile);
				rewind (pFile);
			}
		}
		else
		{
			printf("Not allowed file %s\n", file.c_str());
		}
	}
	stringstream header;
	if(pFile|| (virtualFile.empty()==false) )
	{
		header<<"HTTP/1.1 200 OK\r\n";
		header<<"Server: Streamer\r\n";

		header<<"Content-Length: "<<(virtualFile.empty()? lSize : virtualFile.size())<<"\r\n";
		header<<"Connection: close\r\n";
		if(virtualFile.empty())
		{
			header<<"Content-Type: "<<getContenType(file)<<"\r\n";
		}
		else
		{
			header<<"Content-Type: "<<"text/plain; charset=UTF-8"<<"\r\n";
		}
		header<<"\r\n";
	}
	else
	{
		header<<"HTTP/1.1 404 Not Found\r\n";
		header<<"Server: Streamer\r\n";
		header<<"Content-Length: 0\r\n";
		header<<"Connection: close\r\n";
		header<<"\r\n";
	}
	send_to_socket(socket, header.str().c_str(), header.str().length());

	if(!virtualFile.empty())
	{
		int i=0;
		while(!virtualFile.empty())
		{
			int size=0;
			if(virtualFile.length()<=1024) size=1024;
			else size=virtualFile.length();
			if(size==0) break;

			string tmp = virtualFile.substr(0,size);
			virtualFile = virtualFile.substr(size);
			if(send_to_socket(socket, tmp.c_str(), tmp.length())<=0) break;
		}

	}
	else
	{
		if(pFile)
		{
			char buffer[1024];
			size_t read;
			while((read=fread(buffer,1,sizeof(buffer),pFile))>0)
			{
				if(send_to_socket(socket, buffer, read)<=0) break;
			}
			// terminate
			fclose (pFile);
		}
	}
}
void CHttpRequestHandler::clearParsetHeaderInfo()
{
	post_line="";
	get_line="";
	content_length=0;
	content.str("");
	host="";
	params.clear();	
}
void CHttpRequestHandler::handleGetPost()
{
	string line="";
	if(!get_line.empty())
	{
		line=get_line;
	} else if(!post_line.empty())
	{
		line=post_line;
	}
	else return;
	printf("LINE %s\n", line.c_str());
	size_t params_index = line.find("?");
	if(params_index!=string::npos)
	{
		string params = line.substr(params_index+1);
		params=params.substr(0, params.find(" "));
		printf("Params: \"%s\"\n", params.c_str());

		while(!params.empty())
		{
			string name="";
			string value="";
			size_t i1=params.find('=');
			
			if(i1==string::npos) break;

			name= urlDecoding(params.substr(0,i1));
			
			if(i1>=params.length()-1)
				params="";
			else 
				params=params.substr(i1+1);
			
			i1=params.find("&");
			if(i1==string::npos) i1 = params.length();
			value= urlDecoding(params.substr(0,i1));
			if(i1==params.length())
			 	params="";
			else
			 	params=params.substr(i1+1);
			
			this->params[name]=value;
			printf("name: \"%s\", value:\"%s\"  \n", name.c_str(), value.c_str());
		}
	}

	size_t file_index=line.find(" ");
	if(file_index!=string::npos)
	{
		string file=line.substr(file_index+1);
		size_t file_end_index = file.find("?");
		if(file_end_index==string::npos)
		{
			file_end_index=file.rfind(" ");
		}
		file=file.substr(0,file_end_index);
		printf("Get file: \"%s\"\n", file.c_str());
		sendFile(file);
	}
}
void CHttpRequestHandler::handle()
{
	char buffer[1024];
	size_t size;
	string line="";
	char lastchar=0;
	clearParsetHeaderInfo();
	while((size=receive_from_socket(socket, buffer, sizeof(buffer)))>0)
	{
		for(int i=0;i<size;i++)
		{
			if(this->getLine2(line, lastchar, buffer[i]))
			{
				if(line=="exit"||line=="quit"||line=="bye")
				{
					return;
				}
				else if(line.find("GET ")==0)
				{
					printf("Get=%s\n", line.c_str());
					get_line = line;
				}
				else if(line.find("POST ")==0)
				{
					printf("Post=%s\n", line.c_str());
					post_line = line;
				}
				else if(line.find("Content-Length: ")==0)
				{
					printf("%s\n", line.c_str());
					if(sscanf(line.c_str() ,"%*s %d%*s", &content_length))
					{
						printf("Content-Length parsed. length = %u\n", content_length);
					}
				}
				else if(line.find("Host: ")==0)
				{
					host=line.substr(6);
					printf("Host: %s\n", host.c_str());
				}
				else if (line.empty()) //http header handled
				{
					printf("Handle http request\n");

					if(content_length>0)
					{
						size_t content_received=0;
						char c_buf[1024];
						while(content_received<content_length)
						{
							size_t s = receive_from_socket(socket, c_buf, sizeof(c_buf));
							content_received+=s;
							if(s>0)
							{
								content<<string(c_buf, s);
							}
							else
							{
								printf("Connection lost during content fetching\n");
								return;
							}
						}
						printf("Content received from client successfully\n");
					}
					handleGetPost();
					printf("Handle http request end\n");
					clearParsetHeaderInfo();
					printf("END");
					return;
				}
				line="";
			}
		}
	}
	return;
}

string CHttpRequestHandler::urlDecoding(string url)
{
	string tmp="";
	for(int i=0;i<url.length();i++)
	{
		if(url[i]=='%')
		{
			i++;
			if(i+3<url.length())
			{
				char ascii[] = {url[i],url[i+1],0};
				int c;
				sscanf(ascii,"%X", &c);
				i+=1;
				tmp+=(char)c;
			}
		}
		else
		{
			if(url[i]=='+')
				tmp+=' ';
			else
				tmp+=url[i];
		}
	}
	return tmp;
}


/*
#define READ 0
#define WRITE 1

pid_t popen2(const char *command, int *readpipe, int *writepipe)
{
    int pipefd[2];
    pid_t pid;

    if (pipe(pipefd) != 0)
        return -1;

    pid = fork();

    if (pid < 0)
        return pid;
    else if (pid == 0)
    {
    	dup2(pipefd[WRITE], STDOUT_FILENO);
    	close(pipefd[READ]);

    	system(command);
        //execl("/bin/sh", "sh", "-c", command, NULL);
        perror("execl");

        close(pipefd[WRITE]);
        exit(1);
    }

    if(readpipe==NULL) close(pipefd[READ]);
    else *readpipe = pipefd[READ];
    if(writepipe==NULL) close(pipefd[WRITE]);
    else *writepipe=pipefd[WRITE];
    return pid;
}*/
