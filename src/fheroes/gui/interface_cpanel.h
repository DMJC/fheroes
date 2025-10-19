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
#include <memory>

#include "game_mode.h"
#include "math_base.h"

namespace fheroes
{
    class Sprite;
}

namespace Interface
{
    class AdventureMap;

    class ControlPanel final : protected fheroes::Rect
    {
    public:
        explicit ControlPanel( AdventureMap & );
        ControlPanel( const ControlPanel & ) = delete;

        ~ControlPanel() = default;

        ControlPanel & operator=( const ControlPanel & ) = delete;

        void SetPos( int32_t, int32_t );
        void ResetTheme();
        fheroes::GameMode QueueEventProcessing() const;

        const fheroes::Rect & GetArea() const;

        // Do not call this method directly, use Interface::AdventureMap::redraw() instead to avoid issues in the "no interface" mode.
        // The name of this method starts from _ on purpose to do not mix with other public methods.
        void _redraw() const;

    private:
        AdventureMap & interface;

        // We do not want to make a copy of images but to store just references to them.
        struct Buttons
        {
            Buttons( const fheroes::Sprite & radar_, const fheroes::Sprite & icons_, const fheroes::Sprite & buttons_, const fheroes::Sprite & status_,
                     const fheroes::Sprite & end_ )
                : radar( radar_ )
                , icons( icons_ )
                , buttons( buttons_ )
                , status( status_ )
                , end( end_ )
            {}

            const fheroes::Sprite & radar;
            const fheroes::Sprite & icons;
            const fheroes::Sprite & buttons;
            const fheroes::Sprite & status;
            const fheroes::Sprite & end;
        };

        std::unique_ptr<Buttons> _buttons;

        fheroes::Rect rt_radar;
        fheroes::Rect rt_icons;
        fheroes::Rect rt_buttons;
        fheroes::Rect rt_status;
        fheroes::Rect rt_end;
    };
}
