## gcc -w httpclient.c -o httpclient -L/usr/lib -lssl -lcrypto
all: bhclient.c bhserver.c
	gcc -w bhserver.c -o bhserver
	gcc -w bhclient.c -o bhclient

clean: 
	$(RM) bhserver
	$(RM) bhclient
	
