#!/bin/sh
########################################################################
####  Copyright (C) Aethyra Development Team 2008-2010
####  This program is free software; you can redistribute it and/or modify it under
####  the terms of the GNU General Public License as published by the Free Software
####  Foundation; either version 2 of the License, or (at your option) any later version.
####
####  This program is distributed in the hope that it will be useful, but WITHOUT
####  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
####  FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
####
####  Get the full text of the GPL here:
####  http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
########################################################################
####  Auto building script to help simplify installing Aethyra from source code
####  for several different distros.
####
####  TODO: Functionalize some of these checks
########################################################################

# Checks for various distros
isDebianBased='/etc/debian_version'
isFedoraBased='/etc/fedora-release'

# Ensures that all of the required files for building are present
echo "Checking to ensure that required packages are installed."
if [ -e $isFedoraBased ];then
    su - -c "yum install g++ automake autoconf libtool make guichan-devel \
             physfs-devel SDL_image-devel SDL_ttf-devel SDL_net-devel \
             SDL_mixer-devel SDL-devel zlib-devel libcurl-devel libxml2-devel \
             SDL_gfx-devel gettext xorg-x11-devel"
elif [ -e $isDebianBased ];then
    # Ubuntu derived. Needs to use sudo by default.
    if [ "$( cat /etc/issue | grep -i -c 'buntu' )" -gt 0 ];then
        sudo apt-get install build-essential automake autoconf libtool cvs
        sudo apt-get install autoconf gettext git git-core g++ libx11-dev
        sudo apt-get install libsdl1.2-dev libsdl-image1.2-dev libsdl-net1.2-dev
        sudo apt-get install libsdl-mixer1.2-dev libsdl-gfx1.2-4 libsdl-gfx1.2-dev
        sudo apt-get install libsdl-ttf2.0-0 libsdl-ttf2.0-dev libguichan-dev
        sudo apt-get install libphysfs-dev libcurl4-openssl-dev libgl1-mesa-dev 
        sudo apt-get install libglu1-mesa-dev liballegro4.2-dev libxml2-dev
    # Normal Debian derivative, which don't use sudo by default.
    else
        echo "Please input your password to proceed."
        su - -c "apt-get install build-essential automake autoconf libtool cvs \
                 autoconf gettext git git-core g++ libx11-dev libsdl1.2-dev \
                 libsdl-ttf2.0-0 libsdl-ttf2.0-dev libsdl-gfx1.2-4 \
                 libsdl-image1.2-dev libsdl-net1.2-dev libsdl-mixer1.2-dev \
                 libphysfs-dev libcurl4-openssl-dev libgl1-mesa-dev \
                 libglu1-mesa-dev liballegro4.2-dev libxml2-dev libguichan-dev \
                 libsdl-gfx1.2-dev"
    fi
else
        echo "Error: No support for package checking for your current distro. "
        echo "Please help add support for your distro by supplying some."
        echo
fi

echo "Generating build information using aclocal, autoheader, automake and autoconf."

# Regenerate configuration files
autoreconf -i
echo
echo "Now configuring Aethyra"

./configure

echo
echo "Now running make"

make
strip src/aethyra

echo
echo "Now running make install. Please input your root password, if prompted to.";

if [ "$( cat /etc/issue | grep -i -c 'buntu' )" -gt 0 ];then
    sudo make install;
else
    su -c "make install";
fi

echo
echo "Building done. Now launching Aethyra to check if it works. If it doesn't "
echo "work, please report so on the forums or on IRC, with your architecture "
echo "and distro information."
echo
aethyra
exit 1;

