/***************************************************************************
 *   fheroes: https://github.com/ihhub/fheroes                           *
 *   Copyright (C) 2021 - 2025                                             *
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

#include <cstddef>
#include <cstdint>
#include <vector>

#include "math_base.h"
#include "settings.h"

enum class PlayerColor : uint8_t;

namespace Interface
{
    class BaseInterface;
}

enum class ViewWorldMode : int32_t
{
    OnlyVisible = 0, // Only show what is currently not under fog of war

    ViewArtifacts = 1,
    ViewMines = 2,
    ViewResources = 3,
    ViewHeroes = 4,
    ViewTowns = 5,

    ViewAll = 6,
};

class ViewWorld
{
public:
    static void ViewWorldWindow( const PlayerColor color, const ViewWorldMode mode, Interface::BaseInterface & interface );

    class ZoomROIs
    {
    public:
        ZoomROIs( const ZoomLevel zoomLevel, const fheroes::Point & centerInPixels, const fheroes::Rect & visibleScreenInPixels, const size_t zoomLevels );

        bool zoomIn( const bool cycle );
        bool zoomOut( const bool cycle );
        bool ChangeCenter( const fheroes::Point & centerInPixels );

        const fheroes::Rect & GetROIinPixels() const
        {
            return _roiForZoomLevels[static_cast<uint8_t>( _zoomLevel )];
        }

        fheroes::Rect GetROIinTiles() const;

        ZoomLevel getZoomLevel() const
        {
            return _zoomLevel;
        }

        const fheroes::Point & getCenter() const
        {
            return _center;
        }

    private:
        void _updateZoomLevels();

        bool _updateCenter();
        bool _changeZoom( const ZoomLevel newLevel );

        ZoomLevel _zoomLevel{ ZoomLevel::ZoomLevel1 };
        fheroes::Point _center;
        std::vector<fheroes::Rect> _roiForZoomLevels;
        fheroes::Rect _visibleROI;
    };
};
