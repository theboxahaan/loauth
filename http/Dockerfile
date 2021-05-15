FROM gcc:4.9
WORKDIR /http
COPY . .
EXPOSE 7001
RUN gcc -w bhserver.c -o bhserver
ENV PATH /http:$PATH
# ENTRYPOINT ["/bhserver" "7001"]
CMD ["./bhserver", "7001"]

