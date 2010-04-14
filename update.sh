#!/bin/bash
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
####  Auto updating script/initializer, which chainloads into the autobuilder
####  script. Provided as an easy way in which users can keep their client
####  up-to-date if compiling from source.
########################################################################
if [ -n "$1" ]; then
    SCRIPT_PATH="$1";
    cd $SCRIPT_PATH;
else
    SCRIPT_PATH="${BASH_SOURCE[0]}";

    if([ -h "${SCRIPT_PATH}" ]) then
        while([ -h "${SCRIPT_PATH}" ]) do SCRIPT_PATH=`readlink "${SCRIPT_PATH}"`; done
    fi

    pushd . > /dev/null
    cd `dirname ${SCRIPT_PATH}` > /dev/null
    SCRIPT_PATH=`pwd`;
    popd  > /dev/null
fi

# Go to the Aethyra directory, and search for a .git directory
if [ -e $SCRIPT_PATH/.git ]; then
    # Change the current URL if it doesn't match the current repo location
    # NOTE: Developers who use this script in their development branches should
    #       change this from the read-only address to the push address to ensure
    #       that they can still push after using this script
    git remote set-url origin git://github.com/Tametomo/Aethyra.git;
    # Issue the git update command
    git pull;
    # Run the autobuild script
    ./autobuild.sh;
    echo "Aethyra has been updated."
    exit 1;
else
    # Make this directory a git repo
    git init;
    # Link the newly created git repo to the official Aethyra repository
    git remote add origin git://github.com/Tametomo/Aethyra.git;
    # Start tracking the origin remote
    git fetch origin;
    # Link the origin remote to the master branch
    git checkout --track -b master origin/master
    # Update the new repo to the current git head
    git pull;
    # Run the autobuild script
    ./autobuild.sh
    echo "Congratulations on installing Aethyra! Enjoy!"
    exit 1;
fi
