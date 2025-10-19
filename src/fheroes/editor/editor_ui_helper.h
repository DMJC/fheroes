/***************************************************************************
 *   fheroes: https://github.com/ihhub/fheroes                           *
 *   Copyright (C) 2024 - 2025                                             *
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

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "color.h"
#include "math_base.h"
#include "ui_tool.h"

struct Funds;

namespace fheroes
{
    class Image;
}

namespace Editor
{
    class Checkbox
    {
    public:
        Checkbox( const int32_t x, const int32_t y, const PlayerColor boxColor, const bool checked, fheroes::Image & output );

        Checkbox( Checkbox && other ) = delete;
        ~Checkbox() = default;
        Checkbox( Checkbox & ) = delete;
        Checkbox & operator=( const Checkbox & ) = delete;

        const fheroes::Rect & getRect() const
        {
            return _area;
        }

        PlayerColor getColor() const
        {
            return _color;
        }

        bool toggle();

    private:
        const PlayerColor _color{ PlayerColor::NONE };
        fheroes::Rect _area;
        fheroes::MovableSprite _checkmark;
    };

    void createColorCheckboxes( std::vector<std::unique_ptr<Checkbox>> & list, const PlayerColorsSet availableColors, const PlayerColorsSet selectedColors,
                                const int32_t boxOffsetX, const int32_t boxOffsetY, fheroes::Image & output );

    fheroes::Rect drawCheckboxWithText( fheroes::MovableSprite & checkSprite, std::string str, fheroes::Image & output, const int32_t posX, const int32_t posY,
                                         const bool isEvil, const int32_t maxWidth );

    void renderResources( const Funds & resources, const fheroes::Rect & roi, fheroes::Image & output, std::array<fheroes::Rect, 7> & resourceRoi );

    std::string getDateDescription( const int32_t day );
}
