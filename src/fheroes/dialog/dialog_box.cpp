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

#include <algorithm>
#include <cstdint>
#include <memory>

#include "agg_image.h"
#include "dialog.h" // IWYU pragma: associated
#include "icn.h"
#include "image.h"
#include "screen.h"
#include "settings.h"

namespace
{
    const int32_t windowWidth = 288; // this is measured value
    const int32_t buttonHeight = 40;
    const int32_t activeAreaHeight = 35;

    int32_t topHeight( const bool isEvilInterface )
    {
        const fheroes::Sprite & image = fheroes::AGG::GetICN( isEvilInterface ? ICN::BUYBUILE : ICN::BUYBUILD, 0 );
        return image.height();
    }

    int32_t bottomHeight( const bool isEvilInterface )
    {
        const fheroes::Sprite & image = fheroes::AGG::GetICN( isEvilInterface ? ICN::BUYBUILE : ICN::BUYBUILD, 2 );
        return image.height();
    }

    int32_t leftWidth( const bool isEvilInterface )
    {
        const int icnId = isEvilInterface ? ICN::BUYBUILE : ICN::BUYBUILD;

        const fheroes::Sprite & image3 = fheroes::AGG::GetICN( icnId, 3 );
        const fheroes::Sprite & image4 = fheroes::AGG::GetICN( icnId, 4 );
        const fheroes::Sprite & image5 = fheroes::AGG::GetICN( icnId, 5 );

        int32_t widthLeft = image3.width();
        widthLeft = std::max( widthLeft, image4.width() );
        widthLeft = std::max( widthLeft, image5.width() );

        return widthLeft;
    }

    int32_t rightWidth( const bool isEvilInterface )
    {
        const int icnId = isEvilInterface ? ICN::BUYBUILE : ICN::BUYBUILD;

        const fheroes::Sprite & image0 = fheroes::AGG::GetICN( icnId, 0 );
        const fheroes::Sprite & image1 = fheroes::AGG::GetICN( icnId, 1 );
        const fheroes::Sprite & image2 = fheroes::AGG::GetICN( icnId, 2 );

        int32_t widthRight = image0.width();
        widthRight = std::max( widthRight, image1.width() );
        widthRight = std::max( widthRight, image2.width() );

        return widthRight;
    }

    int32_t overallWidth( const bool isEvilInterface )
    {
        return leftWidth( isEvilInterface ) + rightWidth( isEvilInterface );
    }

    int32_t leftOffset( const bool isEvilInterface )
    {
        return leftWidth( isEvilInterface ) - windowWidth / 2;
    }
}

Dialog::NonFixedFrameBox::NonFixedFrameBox( int height, int startYPos, bool showButtons )
    : _middleFragmentCount( 0 )
    , _middleFragmentHeight( 0 )
{
    if ( showButtons )
        height += buttonHeight;

    const bool evil = Settings::Get().isEvilInterfaceEnabled();
    _middleFragmentCount = ( height <= 2 * activeAreaHeight ? 0 : 1 + ( height - 2 * activeAreaHeight ) / activeAreaHeight );
    _middleFragmentHeight = height <= 2 * activeAreaHeight ? 0 : height - 2 * activeAreaHeight;
    const int32_t height_top_bottom = topHeight( evil ) + bottomHeight( evil );

    area.width = fheroes::boxAreaWidthPx;
    area.height = activeAreaHeight + activeAreaHeight + _middleFragmentHeight;

    fheroes::Display & display = fheroes::Display::instance();
    const int32_t leftSideOffset = leftOffset( evil );

    _position.x = ( display.width() - windowWidth ) / 2 - leftSideOffset;
    _position.y = startYPos;

    if ( startYPos < 0 ) {
        _position.y = ( ( display.height() - _middleFragmentHeight ) / 2 ) - topHeight( evil );
    }

    _restorer.reset( new fheroes::ImageRestorer( display, _position.x, _position.y, overallWidth( evil ), height_top_bottom + _middleFragmentHeight ) );

    area.x = _position.x + ( windowWidth - fheroes::boxAreaWidthPx ) / 2 + leftSideOffset;
    area.y = _position.y + ( topHeight( evil ) - activeAreaHeight );

    redraw();
}

void Dialog::NonFixedFrameBox::redraw()
{
    const bool isEvilInterface = Settings::Get().isEvilInterfaceEnabled();
    const int buybuild = isEvilInterface ? ICN::BUYBUILE : ICN::BUYBUILD;

    const int32_t overallLeftWidth = leftWidth( isEvilInterface );

    // right-top
    const fheroes::Sprite & part0 = fheroes::AGG::GetICN( buybuild, 0 );
    // left-top
    const fheroes::Sprite & part4 = fheroes::AGG::GetICN( buybuild, 4 );

    fheroes::Display & display = fheroes::Display::instance();

    fheroes::Blit( part4, display, _position.x + overallLeftWidth - part4.width(), _position.y );
    fheroes::Blit( part0, display, _position.x + overallLeftWidth, _position.y );

    _position.y += part4.height();

    const int32_t posBeforeMiddle = _position.y;
    int32_t middleLeftHeight = _middleFragmentHeight;
    for ( uint32_t i = 0; i < _middleFragmentCount; ++i ) {
        const int32_t chunkHeight = middleLeftHeight >= activeAreaHeight ? activeAreaHeight : middleLeftHeight;
        // left-middle
        const fheroes::Sprite & sl = fheroes::AGG::GetICN( buybuild, 5 );
        fheroes::Blit( sl, 0, 10, display, _position.x + overallLeftWidth - sl.width(), _position.y, sl.width(), chunkHeight );

        // right-middle
        const fheroes::Sprite & sr = fheroes::AGG::GetICN( buybuild, 1 );
        fheroes::Blit( sr, 0, 10, display, _position.x + overallLeftWidth, _position.y, sr.width(), chunkHeight );

        middleLeftHeight -= chunkHeight;
        _position.y += chunkHeight;
    }

    _position.y = posBeforeMiddle + _middleFragmentHeight;

    // right-bottom
    const fheroes::Sprite & part2 = fheroes::AGG::GetICN( buybuild, 2 );
    // left-bottom
    const fheroes::Sprite & part6 = fheroes::AGG::GetICN( buybuild, 6 );

    fheroes::Blit( part6, display, _position.x + overallLeftWidth - part6.width(), _position.y );
    fheroes::Blit( part2, display, _position.x + overallLeftWidth, _position.y );
}

Dialog::NonFixedFrameBox::~NonFixedFrameBox()
{
    _restorer->restore();

    fheroes::Display::instance().render( _restorer->rect() );
}

int32_t Dialog::NonFixedFrameBox::getButtonAreaHeight()
{
    return buttonHeight;
}
