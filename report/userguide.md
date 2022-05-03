## How to configure DNS in linux?

At first, you need to run this program `sudo`, i.e. as root.

(IMPORTANT)You may need to stop the firewall in linux, which may cause some securiity problrm.

```
sudo vim /etc/systemd/resolved.conf
```

**Then uncomment the DNS= line and the DNSStubListener= line.** 
- change the DNS= value in this file to the DNS server you want to use (e.g.`127.0.0.1` to use a local proxy, which points to this program. 1.1.1.1 to use the Cloudflare DNS, etc.), and also change the DNSStubListener= value from yes to `no`.

then

```
systemctl restart systemd-resolved
systemctl enable systemd-resolved
```

If the commands above doesn`t work, try:

```
sudo ln -sf /run/systemd/resolve/resolv.conf /etc/resolv.conf
```

Port 53 should now be free on your Ubuntu system, and you shouldn't be getting errors like "bind: address already in use" anymore. try  `sudo lsof -i :53`.


## Recover

```(backup)
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