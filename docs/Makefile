###########################################################################
#   fheroes: https://github.com/ihhub/fheroes                           #
#   Copyright (C) 2022 - 2025                                             #
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

LIN_DATA := $$HOME/.local/share/fheroes
LIN_CONF := $$HOME/.config/fheroes
UNIX_DATA := $$HOME/.fheroes

SEDEX := /^\#_SG/,/^\#_EG/ d

ifneq ($(FHEROES_DATA),)
SEDEX := /^\#_SG/,/^\#_EG/ { /^\#_[SE]G/d; s:@FHEROES_DATA@:$(FHEROES_DATA):}
endif
ifeq ($(shell uname), Linux)
SEDEX := $(SEDEX); s:@data@:$(LIN_DATA):; s:@config@:$(LIN_CONF):
else
SEDEX := $(SEDEX); /@data@/,/^This path/ {s:@data@:$(UNIX_DATA):; /^This path/d}
SEDEX := $(SEDEX); /@config@/,/^This path/ {s:@config@:$(UNIX_DATA):; /^This path/d}
endif

.PHONY: all clean

all:
	sed -e '$(SEDEX)' fheroes.6.in > fheroes.6

clean:
	-rm -f fheroes.6
