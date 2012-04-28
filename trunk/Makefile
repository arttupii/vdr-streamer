CXX=g++
CXXFLAGS=-g -pthread
LIBS=-lm

streamer: main.o CSocket.o CHttpRequestHandler.o configfile/configfile.o CVdrLinks.o
	$(CXX) $(CXXFLAGS) $(LIBS) $^ -o $@
	
.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@
