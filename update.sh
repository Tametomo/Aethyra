#!/bin/bash
#automated script to update aethyra from git
#author=Blame <blame@aethyra.com>
#last modified 2009-03-28 Tametomo
# test for the aethyra client folder 
cd $0;
if [ -e $0/.git ]; then
	#issue the git update command
	git pull;
	#issue the autobuild command 
	./autobuild.sh;
	echo "Aethyra has been updated"
	exit 1;
else
	#make this directory a git repo
	git init;
	#link the empty repo to the aethyra client data
	git remote add origin git://gitorious.org/tmw/aethyra.git;
	#start tracking the origin remote
	git fetch origin;
	#link the origin remote to the master branch
	git checkout --track -b master origin/master
	#issue the git update command
	git pull;
	#issue the auto build script to compile the client
	./autobuild.sh
	echo "congratulations on installing aethyra enjoy"
	exit 1;
fi
