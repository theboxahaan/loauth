FROM gcc:4.9
WORKDIR http/
COPY . .
EXPOSE 7001
RUN make server
ENTRYPOINT ./build/bhserver 7001
