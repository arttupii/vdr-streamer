CXX=g++
CXXFLAGS=-lrt -g -std=c++0x 
LIBS=-lm 

OBS= main.o \
     CSocket.o \
     CHttpRequestHandler.o \
     configfile/configfile.o \
     CVdrLinks.o \
     CCommon.o \
     CVideoConverter.o \
     CMutex.o 

streamer: $(OBS) 
	$(CXX) $(CXXFLAGS) $(LIBS) $^ -o $@
	
.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBS)

