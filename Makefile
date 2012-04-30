CXX=g++
CXXFLAGS=-g -pthread
LIBS=-lm

OBS= main.o \
     CSocket.o \
     CHttpRequestHandler.o \
     configfile/configfile.o \
     CVdrLinks.o \
     CCommon.o

streamer: $(OBS) 
	$(CXX) $(CXXFLAGS) $(LIBS) $^ -o $@
	
.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBS)

