# Race Condition Vulnerability

## vulp.c

```
gcc vulp.c -o vulp
sudo chown root vulp          
sudo chmod 4755 vulp
ls -l vulp 
```

It should show the root privilege.

```
gcc attack_process.c -o attack_process
```


```
touch /tmp/myfile
chmod 666 /tmp/myfile
```
