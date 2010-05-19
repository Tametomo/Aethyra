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
########################################################################

# Checks for various distros
isDebianBased='/etc/debian_version'
isFedoraBased='/etc/fedora-release'
numberOfCPUs="`cat /proc/cpuinfo | grep processor | wc -l`"
checkDependencies='false';
toolchain='gnu';
runTest='false';

##
## Functions called within the script.
##

# Ensures that all of the required files for building are present
check_dependencies()
{
    local packages='';
    echo "Checking to ensure that required packages are installed."
    if [ -e $isFedoraBased ];then
        packages="g++ gettext guichan-devel libcurl-devel libtool libxml2-devel \
                 make physfs-devel SDL-devel SDL_gfx-devel SDL_image-devel \
                 SDL_mixer-devel SDL_net-devel SDL_ttf-devel xorg-x11-devel \
                 zlib-devel"

        if [ $toolchain = 'gnu' ];then
            packages="$packages automake autoconf"
        else
            packages="$packages cmake"
        fi 
        su - -c "yum install $packages"
    elif [ -e $isDebianBased ];then
        packages="g++ gettext git git-core libcurl4-openssl-dev libgl1-mesa-dev \
                  libglu1-mesa-dev libguichan-dev libphysfs-dev libsdl1.2-dev \
                  libsdl-gfx1.2-dev libsdl-image1.2-dev libsdl-mixer1.2-dev \
                  libsdl-net1.2-dev libsdl-ttf2.0-dev libtool libx11-dev \
                  libxml2-dev";

        if [ $toolchain = 'gnu' ];then
            packages="$packages automake autoconf";
        else
            packages="$packages cmake";
        fi

        # Ubuntu derived. Needs to use sudo by default.
        if [ "$( cat /etc/issue | grep -i -c 'buntu' )" -gt 0 ];then
            sudo apt-get install $packages;
        # Normal Debian derivative, which don't use sudo by default.
        else
            echo "Please input your password to proceed."
            su - -c "apt-get install $packages"
        fi
    else
        echo "Error: No support for package checking for your current distro. "
        echo "Please help add support for your distro by supplying some."
        echo
    fi
}

# Uses the gnu makefile toolchain for building
use_gnu_toolchain()
{
    echo "Generating build information using aclocal, autoheader, automake and autoconf."

    # Regenerate configuration files
    autoreconf -i
    echo
    echo "Now configuring Aethyra"

    ./configure
    finish_install
}

# Uses the cmake build toolchain for buinding
use_cmake_toolchain()
{
    echo "Generating build information using cmake."
    cmake .

    finish_install
}

# Completes commonly shared install options
finish_install()
{
    echo
    echo "Now running make"

    make -j $numberOfCPUs
    strip src/aethyra

    echo
    echo "Now running make install. Please input your root password, if prompted to.";

    if [ "$( cat /etc/issue | grep -i -c 'buntu' )" -gt 0 ];then
        sudo make install;
    else
        su -c "make install";
    fi
}

# Tests whether the build was successful or not.
test_application()
{
    echo
    echo "Now testing Aethyra";
    echo "-------------------"
    echo "If the application fails to launch, please report this on the forums ";
    echo "or on IRC, with your architecture, distro information, and log file.";
    echo
    aethyra
}

# Parses the options passed through the command line using getopts.
# args: $1 - "$@"
get_options()
{
    local opt=''

    while getopts "cCdDhHtT:" opt
    do
        # If you add any new options here, please add them to get_help() also
        case $opt in
            C|c) toolchain='cmake'
                ;;
            D|d) checkDependencies='true'
                ;;
            T|t) runTest='true'
                ;;
            *) get_help
                ;;
        esac
    done
}

# Displays a nicely formatted help file to let users know what options can be
# used.
get_help()
{
    echo "Aethyra autobuild script";
    echo "\tCopyright (C) 2008-2010  Aethyra Development Team"
    echo
    echo "License GPLv2+: GNU GPL version 2 or later "
    echo "\t<http://gnu.org/licenses/gpl2.html>"
    echo
    echo "This is free software: you are free to change and redistribute it."
    echo "There is NO WARRANTY, to the extent permitted by law."
    echo
    echo "Options: "
    echo "    -c -C: Use CMake toolchain"
    echo "    -d -D: Checks to see if you have all needed dependencies."
    echo "    -h,-H: Displays this help file"
    echo "    -t,-T: Test the currently installed build"
    exit 1;
}

##
## All code after this is the main block code.
##

# Parse all command line options
get_options "$@"

if [ "$checkDependencies" != 'false' ];then
    check_dependencies;
fi

if [ "$toolchain" = 'gnu' ];then
    use_gnu_toolchain;
else
    use_cmake_toolchain;
fi

if [ "$runTest" != 'false' ];then
    test_application;
fi

echo
echo "Autobuilding completed"
echo

exit 1;

