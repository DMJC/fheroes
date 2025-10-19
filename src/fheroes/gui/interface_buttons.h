/***************************************************************************
 *   fheroes: https://github.com/ihhub/fheroes                           *
 *   Copyright (C) 2019 - 2025                                             *
 *                                                                         *
 *   Free Heroes2 Engine: http://sourceforge.net/projects/fheroes         *
 *   Copyright (C) 2009 by Andrey Afletdinov <fheroes@gmail.com>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#pragma once

#include <cstdint>

#include "game_mode.h"
#include "interface_border.h"
#include "math_base.h"
#include "ui_button.h"

namespace Interface
{
    class AdventureMap;

    class ButtonsPanel final : public BorderWindow
    {
    public:
        explicit ButtonsPanel( AdventureMap & baseInterface )
            : BorderWindow( { 0, 0, 144, 72 } )
            , _interface( baseInterface )
        {
            // Do nothing.
        }

        ButtonsPanel( const ButtonsPanel & ) = delete;

        ~ButtonsPanel() override = default;

        ButtonsPanel & operator=( const ButtonsPanel & ) = delete;

        void SetPos( int32_t x, int32_t y ) override;
        void SavePosition() override;
        void setRedraw() const;

        fheroes::GameMode queueEventProcessing();

        // Do not call this method directly, use Interface::AdventureMap::redraw() instead to avoid issues in the "no interface" mode.
        // The name of this method starts from _ on purpose to do not mix with other public methods.
        void _redraw();

    private:
        void _setButtonStatus();

        AdventureMap & _interface;

        fheroes::Button _buttonNextHero;
        fheroes::Button _buttonHeroMovement;
        fheroes::Button _buttonKingdom;
        fheroes::Button _buttonSpell;
        fheroes::Button _buttonEndTurn;
        fheroes::Button _buttonAdventure;
        fheroes::Button _buttonFile;
        fheroes::Button _buttonSystem;

        fheroes::Rect _nextHeroRect;
        fheroes::Rect _heroMovementRect;
        fheroes::Rect _kingdomRect;
        fheroes::Rect _spellRect;
        fheroes::Rect _endTurnRect;
        fheroes::Rect _adventureRect;
        fheroes::Rect _fileRect;
        fheroes::Rect _systemRect;
    };
}
