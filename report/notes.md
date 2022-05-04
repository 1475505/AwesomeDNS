This is the UDP version of the DNSrelay.

# Linux Commands
To see if the server program is working, run
```
sudo netstat -apn|grep :53
//or
sudo netstat -apn|grep main
```

You can connect tp the server by
```
nc -u 127.0.0.1 53
```

You can also use /output/client for testing.

Then you can send and receive.

# Project Files

/src:

- config.c : Config debuginfo/serverIp/DNSfile.
- DNS.h : the structure of DNS
- Socket.c: A wrapper of socket.h(from web)
- utils.c : ip2hex

/output:

- main: the DNSserver program executable file
- test: just for unit-debug
- client: a client program, send message to 127.0.0.1:53.

# Some Net cmds in Linux

To see which progress is using the port, run

```
sudo lsof -i :53
```

