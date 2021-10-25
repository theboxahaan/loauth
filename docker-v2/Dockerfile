FROM prosody/prosody:latest
USER root
RUN apt-get update && apt-get install -y \
	vim \
	libxml2 \
	libxml2-dev \
	gcc \
	telnet \
	lua5.2-dev
RUN chmod 666 /etc/prosody/certs/localhost.key && chmod 666 /etc/prosody/certs/localhost.crt

