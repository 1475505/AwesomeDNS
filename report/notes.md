To see if the server program is working, run
```
sudo netstat -apn|grep 5300
```

# Project Files

/src:

- config.c : Config debuginfo/serverIp/DNSfile.
- DNS.h : the structure of DNS
- Socket.c: A wrapper of socket.h(from web)
- utils.c : ip2hex

/output:

- main: the DNSserver program executable file
- test: just for uint debug
- client: a client program, send message to PORT.