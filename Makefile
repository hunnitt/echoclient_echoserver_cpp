all: echoclient echoserver

echoclient: echoclient.cpp
	g++ -g -o echoclient echoclient.cpp 

echoserver: echoserver.cpp
	g++ -g -o echoserver echoserver.cpp

clean:
	rm -f echoclient
	rm -f echoserver
