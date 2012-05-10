CXX=g++
CXXFLAGS=-g -std=c++0x 
LIBS= -lrt 

OBS= main.o \
     CSocket.o \
     CHttpRequestHandler.o \
     configfile/configfile.o \
     CVdrLinks.o \
     CCommon.o \
     CVideoConverter.o \
     CMutex.o 

streamer: $(OBS) 
	$(CXX)  $^ $(LIBS) -o $@
	
.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBS)

