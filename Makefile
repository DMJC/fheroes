###########################################################################
#   fheroes: https://github.com/ihhub/fheroes                           #
#   Copyright (C) 2021 - 2025                                             #
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

# Options:
#
# FHEROES_STRICT_COMPILATION: build in strict compilation mode (turns warnings into errors)
# FHEROES_WITH_DEBUG: build in debug mode
# FHEROES_WITH_ASAN: build with UB Sanitizer and Address Sanitizer (small runtime overhead, incompatible with FHEROES_WITH_TSAN)
# FHEROES_WITH_TSAN: build with UB Sanitizer and Thread Sanitizer (large runtime overhead, incompatible with FHEROES_WITH_ASAN)
# FHEROES_WITH_IMAGE: build with SDL_image (requires libpng)
# FHEROES_WITH_SYSTEM_SMACKER: build with an external libsmacker instead of the bundled one
# FHEROES_WITH_TOOLS: build additional tools
# FHEROES_MACOS_APP_BUNDLE: create a Mac app bundle (only valid when building on macOS)
# FHEROES_DATA: set the built-in path to the fheroes data directory (e.g. /usr/share/fheroes)

PROJECT_NAME := fheroes
PROJECT_VERSION := $(file < version.txt)

.PHONY: all clean

all:
	$(MAKE) -C src/dist
	$(MAKE) -C files/lang
ifdef FHEROES_MACOS_APP_BUNDLE
	mkdir -p fheroes.app/Contents/MacOS
	mkdir -p fheroes.app/Contents/Resources/h2d
	mkdir -p fheroes.app/Contents/Resources/maps
	mkdir -p fheroes.app/Contents/Resources/translations
	cp files/data/*.h2d fheroes.app/Contents/Resources/h2d
	cp files/lang/*.mo fheroes.app/Contents/Resources/translations
	cp maps/*.fh2m fheroes.app/Contents/Resources/maps
	cp src/dist/fheroes/fheroes fheroes.app/Contents/MacOS
	cp src/resources/fheroes.icns fheroes.app/Contents/Resources
	sed -e "s/\$${MACOSX_BUNDLE_BUNDLE_NAME}/$(PROJECT_NAME)/" \
	    -e "s/\$${MACOSX_BUNDLE_BUNDLE_VERSION}/$(PROJECT_VERSION)/" \
	    -e "s/\$${MACOSX_BUNDLE_EXECUTABLE_NAME}/fheroes/" \
	    -e "s/\$${MACOSX_BUNDLE_GUI_IDENTIFIER}/org.fheroes.$(PROJECT_NAME)/" \
	    -e "s/\$${MACOSX_BUNDLE_ICON_FILE}/fheroes.icns/" \
	    -e "s/\$${MACOSX_BUNDLE_SHORT_VERSION_STRING}/$(PROJECT_VERSION)/" src/resources/Info.plist.in > fheroes.app/Contents/Info.plist
	dylibbundler -od -b -x fheroes.app/Contents/MacOS/fheroes -d fheroes.app/Contents/libs
else
	cp src/dist/fheroes/fheroes .
endif

clean:
	$(MAKE) -C src/dist clean
	$(MAKE) -C files/lang clean
	-rm -rf fheroes fheroes.app
