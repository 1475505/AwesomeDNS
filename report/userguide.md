## How to configure DNS in linux?

```
sudo vim /etc/systemd/resolved.conf
DNS=
```
then
```
systemctl restart systemd-resolved
systemctl enable systemd-resolved
```
```(backup)
mv /etc/resolv.conf  /etc/resolv.conf.bak
ln -s /run/systemd/resolve/resolv.conf /etc/
```