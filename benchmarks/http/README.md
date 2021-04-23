## `http` `client-server` setup
-----------

To locally build both `bhclient` and `bhserver` 
```
make
```

Then launch `bhserver` using `./build/bhserver <port 7001>`
and the client `bhclient` using `./build/bhclient <port 7001> <S> <size of payload 100>`

## To deploy `bhserver` in a container
```bash
$ docker build -t loauth .
$ docker run -p 7001:7001 -it loauth:latest
```

### Todos 
1. Free dynamically allocated memory 
2. Get container logs to work properly
3. Fix `docker-compose.yml`
4. Do the same for `xmpp` client-server.

