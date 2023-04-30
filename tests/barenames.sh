echo
echo ---------------------------------------
echo This script tests barenames executables
echo

echo run an executable that is in cwd
hello
echo

echo run an executable that is in cwd with arguments
helloarg a b c
echo

echo run ls (barename executable) that is in one of the 6 directories that we check with wildcards in arguments
ls te*ts/*name*.sh
echo

echo run echo (barename executable) that is in one of the 6 directories that we check with wildcards in barename
ech*o h*l*.c
echo

echo run an executable with piping
ls | sort > out/sortedFiles.txt
cat < out/sortedFiles.txt | grep mysh > out/grepedFiles.txt
