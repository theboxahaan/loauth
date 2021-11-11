## Build container
```bash
$ cd <loauth-root/docker-v2>
$ docker build --tag loauth/prosody:latest --no-cache .
```

## Deploy `docker-v2` container
```bash
$ cd <loauth-root>
$ docker run --rm --name prosody -v $(pwd)/docker-v2/:/etc/prosody/:ro \
-v $(pwd)/docker-v2/libnativefunc.so:/usr/lib/prosody/util/libnativefunc.so \
-v $(pwd)/docker-v2/mod_pls.lua:/usr/lib/prosody/modules/mod_pls.lua \
-v $(pwd)/docker-v2/mod_tls.lua:/usr/lib/prosody/modules/mod_tls.lua \
-v $(pwd)/docker-v2/mod_saslauth.lua:/usr/lib/prosody/modules/mod_saslauth.lua \
-p 5222:5222 -p 5582:5582 \
loauth/prosody
```
--------------
> **This step is not required to be done explicitly.
> `prosodyctl` does this in the `Dockerfile`**

### Add user `admin@localhost` to the server
```bash
$ docker exec -it prosody bash
```
- Use the `telnet` API to add users
```bash
$ telnet localhost 5582

# add user 
> user:create("admin@localhost","123")
```

## Using `client-v2`
Currently implements `SASL` using `SCRAM-SHA-1`

```bash
$ python client-v2.py --host <HOST URI> --port <PORT>
```
Deafult values are `localhost` and `5222` respectively.

It should print out a **success** message
```
b"<success xmlns='urn:ietf:params:xml:ns:xmpp-sasl'>dj1od2dlUk5ESGg4ckswNEJNRU91TVZIdzl1RjA9</success>"
```

-----------------

## Testing with `profanity`
```bash
$ profanity

# test profanity w/ SASL
> /connect admin@localhost server localhost port 5222 tls disable
```
