#!/usr/bin/env bash

###########################################################################
#   fheroes: https://github.com/ihhub/fheroes                           #
#   Copyright (C) 2025                                                    #
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
#   This program is distributed in the hope that it will be useful,       #
#   but WITHOUT ANY WARRANTY; without even the implied warranty of        #
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
#   GNU General Public License for more details.                          #
#                                                                         #
#   You should have received a copy of the GNU General Public License     #
#   along with this program; if not, write to the                         #
#   Free Software Foundation, Inc.,                                       #
#   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             #
###########################################################################

# This script cleans up fheroes application bundle files from the user's home directory.
# It removes:
#   - Preferences: ~/Library/Preferences/fheroes
#   - Data files and save games: ~/Library/Application Support/fheroes
# The script will show what will be removed and ask for confirmation unless -y/--yes is provided.
# It exits early if no target directories exist.

set -e

PLATFORM_NAME="$(uname 2> /dev/null)"
if [[ "${PLATFORM_NAME}" != "Darwin" ]]; then
    echo "This script must be run on a macOS host"
    exit 1
fi

# Parse command line arguments
AUTO_CONFIRM=0
while [[ $# -gt 0 ]]; do
    case $1 in
        -y|--yes)
            AUTO_CONFIRM=1
            shift
            ;;
        *)
            echo "Unknown option: $1"
            echo "Usage: $0 [-y|--yes]"
            exit 1
            ;;
    esac
done

# Check if either target directory exists
if [[ ! -d "${HOME}/Library/Preferences/fheroes" ]] && [[ ! -d "${HOME}/Library/Application Support/fheroes" ]]; then
    echo "No fheroes app bundle directories found to clean up"
    exit 0
fi

echo "The following directories will be removed:"
echo "1. ${HOME}/Library/Preferences/fheroes"
[[ -d "${HOME}/Library/Preferences/fheroes" ]] && echo "   - Contains $(find "${HOME}/Library/Preferences/fheroes" -type f | wc -l) files"

echo "2. ${HOME}/Library/Application Support/fheroes"
[[ -d "${HOME}/Library/Application Support/fheroes" ]] && echo "   - Contains $(find "${HOME}/Library/Application Support/fheroes" -type f | wc -l) files"

echo ""
echo -e "\033[1mThis will remove all your fheroes data files, settings and savegames.\033[0m"
if [[ $AUTO_CONFIRM -eq 0 ]]; then
    read -p "Are you sure you want to remove these directories? (y/N): " -r
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo "Files will not be removed"
        exit 0
    fi
fi

echo "Removing preferences directory..."
rm -rf "${HOME}/Library/Preferences/fheroes"

echo "Removing application support directory..."
rm -rf "${HOME}/Library/Application Support/fheroes"

echo "Cleanup complete"
