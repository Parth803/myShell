echo
echo --------------------------------------
echo This script tests pathname executables
echo

echo run an executable that is in cwd
./hello
echo

echo run an executable that is in cwd with wild card in arguments
./helloarg *.c b c
echo

cd ..
echo run an executable that is in different directory
pwd
./*/hello
echo

echo run an executable that is in different directory with arguments
./*/helloarg a b c
echo

echo run an executable that is in different directory with multiple wildcards and/or tilde
*/hello*rg ran with multiple wildcards
~/*/*/hello*arg ran with tilde and multiple wildcards
echo

echo run an executable with piping and wildcards
*/hello*rg ran with multiple wildcards | cat
echo

echo run an executable even if exit is in first pipe
exit|*/hello 
