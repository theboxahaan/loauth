## sudo apt-get install libxml2-dev libxml2-doc

all: xmppclient.c xmppserver.c
	gcc -w xmppclient.c -I/usr/include/libxml2 -lxml2 -o xmppclient
	gcc -w xmppserver.c -I/usr/include/libxml2 -lxml2 -o xmppserver

clean: 
	$(RM) xmppserver
	$(RM) xmppclient
	
