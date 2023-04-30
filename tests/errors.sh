echo
echo ------------------------------------------------
echo This script tests errors for specific situations
echo

echo output redirection in first subcommand with pipe (will cause Error)
echo foo > bar | cat
echo

echo input redirection in second subcommand with pipe (will cause Error)
echo foo | cat < bar
echo

echo multiple output redirections (will cause Error)
echo foo > bar > baz
echo

echo multiple input redirections (will cause Error)
cat < foo < bar
echo

echo ambiguous paths/files for pathname using wildcards (will cause Error)
~/*
echo

echo ambiguous files for barename using wildcards (will cause Error)
hello*
echo

echo multiple files for arguments using wildcards (will cause Error)
cd ~/*
echo
echo ------------------------------------------------
echo
