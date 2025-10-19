/***************************************************************************
 *   fheroes: https://github.com/ihhub/fheroes                           *
 *   Copyright (C) 2023 - 2025                                             *
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
#include <vector>

#include "color.h"
#include "math_base.h"

class Heroes;

namespace fheroes
{
    class Image;
    struct ObjectRenderingInfo;
}

namespace Interface
{
    class GameArea;
}

namespace MP2
{
    enum ObjectIcnType : uint8_t;
}

namespace Maps
{
    class Tile;
    struct ObjectPart;

    void redrawEmptyTile( fheroes::Image & dst, const fheroes::Point & mp, const Interface::GameArea & area );

    void redrawFlyingGhostsOnMap( fheroes::Image & dst, const fheroes::Point & pos, const Interface::GameArea & area, const bool isEditor );
    void redrawTopLayerObject( const Tile & tile, fheroes::Image & dst, const bool isPuzzleDraw, const fheroes::Point & pos, const Interface::GameArea & area,
                               const ObjectPart & part );

    void drawFog( const Tile & tile, fheroes::Image & dst, const Interface::GameArea & area );

    void redrawPassable( const Tile & tile, fheroes::Image & dst, const PlayerColorsSet friendColors, const Interface::GameArea & area, const bool isEditor );

    void redrawBottomLayerObjects( const Tile & tile, fheroes::Image & dst, bool isPuzzleDraw, const Interface::GameArea & area, const uint8_t level );

    void drawByObjectIcnType( const Tile & tile, fheroes::Image & output, const Interface::GameArea & area, const MP2::ObjectIcnType objectIcnType );

    std::vector<fheroes::ObjectRenderingInfo> getMonsterSpritesPerTile( const Tile & tile, const bool isEditorMode );
    std::vector<fheroes::ObjectRenderingInfo> getMonsterShadowSpritesPerTile( const Tile & tile, const bool isEditorMode );
    std::vector<fheroes::ObjectRenderingInfo> getBoatSpritesPerTile( const Tile & tile );
    std::vector<fheroes::ObjectRenderingInfo> getBoatShadowSpritesPerTile( const Tile & tile );
    std::vector<fheroes::ObjectRenderingInfo> getMineGuardianSpritesPerTile( const Tile & tile );
    std::vector<fheroes::ObjectRenderingInfo> getHeroSpritesPerTile( const Heroes & hero );
    std::vector<fheroes::ObjectRenderingInfo> getHeroShadowSpritesPerTile( const Heroes & hero );
    std::vector<fheroes::ObjectRenderingInfo> getEditorHeroSpritesPerTile( const Tile & tile );

    const fheroes::Image & getTileSurface( const Tile & tile );
}
