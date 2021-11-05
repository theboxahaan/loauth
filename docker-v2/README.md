## Deploy `docker-v2` container
```bash
$ cd <loauth-root>
$ docker run --rm --name prosody -v $(pwd)/docker-v2/:/etc/prosody/:ro -p 5222:5222 -p 5582:5582 loauth/prosody
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
$ python dummy.py
```
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
