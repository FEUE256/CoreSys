# Get Started

1. Build CoreSys (See more in the README.md that is in the root of this project)
3. If Software, go to the title "Software" or if otherwise go to the title "Hardware".

## Software

Requirements:
- See in docs/req.md

1. Open WSL or the terminal
2. Change Folder
```bash
cd Path/to/CoreSys/build/master
```
3. Run Script
```bash
make qemu
```
(Qemu come work when this text is removed now it do NOT work)

## Hardware
WARNING: THIS WILL WIPE THE WHOLE DRIVE
Note: Run diskwriter.run with sudo
Note: When you unmount the disk files can be corrupted

1. Open WSL or the terminal
2. Change Folder
```bash
cd Path/to/CoreSys/bin/Release/Diskwriter
```
3. List disks
```bash
lsblk
```
See which disk it is by checking its disk size. Remember the characters under the title "NAME" for the disk. Check if it is mounted under the title "MOUNTPOINTS" for the disk if that is blank, go to step 5 else go to step 4.
4. Unmount the disk
```bash
umount -f /mnt/NAME
```
5. Flash the device
```bash
sudo ./diskwriter.run ../CoreSys/boot.img NAME
```

(boot.img come work when this text is removed now it do NOT work)
