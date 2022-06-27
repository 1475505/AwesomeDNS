## 6.28
!!!ATTENTION!!

- if you need to use jyy's `threads.h`, you need to add `-lpthread` in compiling (config Makefike)

## 5.4

May need to reorder(big/little endian) when set bit in header.info

<-- for writing report-->:

- Need to bind DNS via UDP instead of TCP in branch `main`.
- Need to close the proxy.

## sudo debug in vscode

[ref](https://stackoverflow.com/questions/40033311/how-to-debug-programs-with-sudo-in-vscode)

```
sudo visudo

user_name(eg. ll) ALL=(ALL) NOPASSWD:/usr/bin/gdb

```

restart the shell.

```
touch .gdbroot
vim .gdbroot

sudo /usr/bin/gdb "$@" 

(launch.json)
"miDebuggerPath": "/home/ll/.gdbroot",
```