FROM prosody/prosody:latest
USER root
RUN apt-get update && apt-get install -y \
	vim \
	libxml2 \
	libxml2-dev \
	gcc \
	telnet \
	lua5.2-dev
COPY mod_tls.lua /usr/lib/prosody/modules/
WORKDIR /staging/
COPY libnativefunc.c present_cbc.h authModule.h xmppclient.c . 
RUN gcc -fPIC -o libnativefunc.o -c libnativefunc.c -I/usr/include/lua5.2 && gcc libnativefunc.o -shared -o /usr/lib/prosody/util/libnativefunc.so

RUN gcc -w  xmppclient.c -o xmppclient -I/usr/include/libxml2 -lxml2
