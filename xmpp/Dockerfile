FROM gcc:4.9
WORKDIR /xmpp
COPY . .
EXPOSE 7002
RUN gcc -w xmppserver.c -I/usr/include/libxml2 -lxml2 -o xmppserver
ENV PATH /xmpp:$PATH
# ENTRYPOINT ["xmppserver"]
CMD ["./xmppserver", "7002"]
