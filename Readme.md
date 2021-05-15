# Benchmark Comparison between XMPP and HTTP Protocol Using Docker.

## Directories:
```bash
.
├── http
│   ├── bhclient.c
│   ├── bhserver.c
│   ├── docker-compose.yml
│   ├── Dockerfile
│   ├── Makefile
│   └── script.sh
├── Readme.md
└── xmpp
    ├── docker-compose.yml
    ├── Dockerfile
    ├── Makefile
    ├── script.sh
    ├── xmppclient.c
    └── xmppserver.c
```
---
*If needed then change the IP adreess according to your docker config, define in C file. Then execute the `make` command.*

---
## Run The Servers
Both The Servers run by these following commands:
1. `sudo docker-compose build`
2. `sudo docker-compose up`
---
## Run The HTTP Client
Format:
  `./bhclient <PORT_NO> <METHOD> <BENCHMARK_ROUND> <MSG_LENGTH>`

* *Methods:*
    1. `S` for sending data.
    2. `L` for receiving only.

Exmaple:
`./bhclient 7001 S 10 10`

---
## Run The XMMP Client
Format:
  `./xmppclient <PID> <METHOD> <BENCHMARK_ROUND> <MSG_LENGTH>`

* *Methods:*
    1. `pub` for publishing data.
    2. `sub` for receiving only.

Exmaple:
`./xmppclient name@172.19.0.2 pub 10 10`

---
*`script.sh` used for execute multiple client at a time.*