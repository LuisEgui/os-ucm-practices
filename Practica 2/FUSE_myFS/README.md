## About

**myFUSE** is a custom FUSE framework for the Operating System subject in the Universidad Complutense de Madrid.

## Usage

myFUSE is written in C and uses gcc and fuse (version 26). You can built it like this:

```sh
make all
```

All documentation is located in the *doc/html/index.html* file or *doc/man/* folder.

An example for man documentation would be:
```sh
man -l myFS.c.3
```

To run a test, first you need to create a mount directory, i.e:
```sh
mkdir mount-point
```

Then,  give execution permissions to *test1.sh script, like:
```sh
chmod +x test1.sh
```

Then, you can run the test like follows:
```sh
./test1.sh
```

To execute myFuse, run the example output command from:
```sh
./fs-fuse
```

To test its functionallity, go to another terminal and check the output from the first terminal after doing:
```sh
ls /mount-point/
cat /mount-point/file1.txt
echo "a new file" >> /mount-point/file3.txt
```
