FROM gcc:4.9
WORKDIR /home/Desktop/Code/server/http
COPY . .
EXPOSE 7001
RUN gcc -o bhserver bhserver.c
ENV PATH /home/Desktop/Code/server/http:$PATH
ENTRYPOINT ["bhserver", "7001"]
