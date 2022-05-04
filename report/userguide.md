## How to configure DNS in linux?

At first, you need to run this program `sudo`, i.e. as root.

It's *a little* complicate to configure this DNSrelay program. **You may need to stop the firewall in linux, which may cause some security problem.**

Try following:

1. Solve bind error.

```
sudo vim /etc/systemd/resolved.conf
```

**DNS=127.0.0.1**
**DNSStubListener=no** 
> uncomment and change the DNS= value in this file to the DNS server you want to use (e.g.`127.0.0.1` to use this local proxy. 1.1.1.1 to use the Cloudflare DNS, etc.). So as DNSStubListener.

then

```
systemctl restart systemd-resolved
systemctl enable systemd-resolved
```

If the commands above don't work fine, try also:

```
sudo ln -sf /run/systemd/resolve/resolv.conf /etc/resolv.conf
```

Now port 53 should now be free on your Ubuntu system, try  `sudo lsof -i :53`.

Try connect to the Ethernet, the program will output
`[Serving]`

2. Solve DNS messing up

You may also need to

```
sudo vim /etc/resolv.conf
```

and add `nameserver 127.0.0.1` at the top.

You don't need to comment the following DNS, just put 127.0.0.1 first.

3. If you use Wifi

You can enter "wifi-setting", choose your wifi and set DNS=127.0.0.1

4. If you use proxy

In the browser or something else, you may need to close the proxy, especially TUN mode.

5. tips

Reboot may help a lot whenever meeting a messing problem.

## Recover

```
(backup)
mv /etc/resolv.conf  /etc/resolv.conf.bak
ln -s /run/systemd/resolve/resolv.conf /etc/
```

You can undo changes by following steps:

```
sudo vim /etc/systemd/resolved.conf

#DNS=
#DNSStubListener=no

sudo rm /etc/resolv.conf
```

Restart the systemd-resolved or reboot the system. 