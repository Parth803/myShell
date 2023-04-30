echo
echo ------------------------------------------
echo This script tests builtins (cd, pwd, exit)
echo

echo use pwd to get current directory and store it in a file
pwd > out/cwd.txt
echo

echo cd to parent
cd ..

echo cwd:
pwd
echo

echo cd with wildcard (will cd to tests subdirectory)
cd ./*/t*ts

echo cwd:
pwd
echo

cd ..

echo cd no arguments (will cd to HOME)
cd

echo cwd:
pwd
echo

echo cd to root
cd /

echo cwd:
pwd
echo 

echo cd to HOME using Tilde (also will work with ~/)
cd ~

echo cwd:
pwd
echo

echo cd piped with pwd
cd / | pwd
echo

echo cd to non existent directory (will cause Error)
cd non_existent_dir
echo

echo cd to non accessible directory (will cause Error)
cd /common/home/ejx2
echo

echo cd with more than 1 argument (will cause Error)
cd / ~/ arg1
echo

echo pwd with 1 argument or more (will cause Error)
pwd arg1
echo


echo exiting before EOF (number of following arguments doesn't matter)
exit arg1 arg2

echo this should not print
