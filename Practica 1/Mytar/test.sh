#!/bin/bash
DIR = "/bin/Mytar"

if [ -e $DIR/mytar] && [-x $DIR/mytar]; then
 if[ -d $DIR/tmp]; then
  rm -rf ~$DIR/tmp
 else
   mkdir tmp
   cd tmp
   touch file1.txt
   echo "Hello world!" > file1.txt
   touch file2.txt
   head -10 /etc/passwd > file2.txt
   touch file3.dat
   head -c 1024 /dev/urandom
   ./mytar -c -f filetar.mtar file1.txt file2.txt file3.dat
   mkdir out
   cp filetar.mtar out
   cd out
   ../ mytar -x -f ../filetar.mtar
   diff file1.txt ../file1.txt
   diff file2.txt ../file2.txt
   diff file3.dat ../file3.dat
else
 echo " No puede ejecutar el programa
fi


mkdir tmp
cd tmp
