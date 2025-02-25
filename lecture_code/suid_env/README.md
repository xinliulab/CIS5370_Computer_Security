

ls -l /etc/shadow

passwd

ls -l /usr/bin/passwd

sudo cat /etc/shadow

sudo cat /etc/passwd

whereis ls

gcc cap_leak.c -o cap_leak
sudo chown root cap_leak
sudo chmod 4755 cap_leak
ls -l cap_leak

sudo touch /etc/zzz
sudo chmod 666 /etc/zzz
nano /etc/zzz
sudo chmod 644 /etc/zzz
sudo chown root:root /etc/zzz

./cap_leak
id
exec 3>&-
