#!/bin/sh
# auto build script based on fedora but usable for other distros
#author=Blame <blame582@gmail.com> 
#last modified date 2009-03-28 Tametomo
# right now this should be ok for testing 
file=/etc/fedora-release
file1=/etc/lsb-release

# Ensures that all of the required files for building are present
if [ -e $file1 ];then
        echo "Checking to ensure that required packages are installed"
        echo
        sudo apt-get install build-essential automake autoconf libtool cvs
        sudo apt-get install autoconf gettext git git-core g++
        sudo apt-get install libsdl1.2-dev libsdl-image1.2-dev libsdl-net1.2-dev libsdl-mixer1.2-dev 
        sudo apt-get install libphysfs-dev libcurl4-openssl-dev libgl1-mesa-dev 
        sudo apt-get install libglu1-mesa-dev liballegro4.2-dev libxml2-dev
        sudo apt-get install libguichan-0.8.1-1 libguichan-sdl-0.8.1-1 libguichan-dev
        sudo apt-get install libsdl-ttf2.0-0 libsdl-ttf2.0-dev
fi

echo "Generating build information using aclocal, autoheader, automake and autoconf."
echo

# Regerate configuration files
autoreconf -i

echo
echo "Now you are ready to run ./configure"

./configure

echo
echo "now running make"

make

echo
echo "now running make install please input your root password";
# we must test for fedora first otherwise we get an error
if [ -e $file ]; then
	echo "Fedora found";
	su -c "make install";
	su -c "make clean";
	echo "Done";
	aethyra &
	exit 1;
elif [ -e $file1 ];then
	echo "ubuntu found";
	sudo make install;
	aethyra &
	exit 1;
else 
	echo "login as root and run make install"
fi
echo "done"

