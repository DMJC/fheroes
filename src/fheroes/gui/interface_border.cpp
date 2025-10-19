/***************************************************************************
 *   fheroes: https://github.com/ihhub/fheroes                           *
 *   Copyright (C) 2019 - 2024                                             *
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

#include "interface_border.h"

#include <algorithm>

#include "agg_image.h"
#include "cursor.h"
#include "icn.h"
#include "image.h"
#include "localevent.h"
#include "screen.h"
#include "settings.h"
#include "ui_constants.h"
#include "ui_tool.h"

namespace
{
    void repeatPattern( const fheroes::Image & in, int32_t inX, int32_t inY, int32_t inWidth, int32_t inHeight, fheroes::Image & out, int32_t outX, int32_t outY,
                        int32_t width, int32_t height )
    {
        if ( inX < 0 || inY < 0 || outX < 0 || outY < 0 || inWidth <= 0 || inHeight <= 0 || width <= 0 || height <= 0 )
            return;

        const int32_t countX = width / inWidth;
        const int32_t countY = height / inHeight;
        const int32_t restWidth = width % inWidth;
        const int32_t restHeight = height % inHeight;

        for ( int32_t y = 0; y < countY; ++y ) {
            for ( int32_t x = 0; x < countX; ++x ) {
                fheroes::Blit( in, inX, inY, out, outX + x * inWidth, outY + y * inHeight, inWidth, inHeight );
            }
            if ( restWidth != 0 ) {
                fheroes::Blit( in, inX, inY, out, outX + width - restWidth, outY + y * inHeight, restWidth, inHeight );
            }
        }
        if ( restHeight != 0 ) {
            for ( int32_t x = 0; x < countX; ++x ) {
                fheroes::Blit( in, inX, inY, out, outX + x * inWidth, outY + height - restHeight, inWidth, restHeight );
            }
            if ( restWidth != 0 ) {
                fheroes::Blit( in, inX, inY, out, outX + width - restWidth, outY + height - restHeight, restWidth, restHeight );
            }
        }
    }
}

void Interface::GameBorderRedraw( const bool viewWorldMode )
{
    const Settings & conf = Settings::Get();
    if ( conf.isHideInterfaceEnabled() && !viewWorldMode )
        return;

    const bool isEvilInterface = conf.isEvilInterfaceEnabled();

    fheroes::Display & display = fheroes::Display::instance();

    const int32_t displayWidth = display.width();
    const int32_t displayHeight = display.height();
    const int32_t extraDisplayWidth = displayWidth - fheroes::Display::DEFAULT_WIDTH;
    const int32_t extraDisplayHeight = displayHeight - fheroes::Display::DEFAULT_HEIGHT;

    const int32_t topRepeatCount = extraDisplayWidth > 0 ? extraDisplayWidth / fheroes::tileWidthPx : 0;
    const int32_t topRepeatWidth = ( topRepeatCount + 1 ) * fheroes::tileWidthPx;

    const int32_t vertRepeatCount = extraDisplayHeight > 0 ? extraDisplayHeight / fheroes::tileWidthPx : 0;
    const int32_t iconsCount = vertRepeatCount > 3 ? 8 : ( vertRepeatCount < 3 ? 4 : 7 );

    const int32_t vertRepeatHeight = ( vertRepeatCount + 1 ) * fheroes::tileWidthPx;
    const int32_t vertRepeatHeightTop = ( iconsCount - 3 ) * fheroes::tileWidthPx;
    const int32_t vertRepeatHeightBottom = vertRepeatHeight - vertRepeatHeightTop;

    const int32_t topPadWidth = extraDisplayWidth % fheroes::tileWidthPx;

    // top and bottom padding is split in two halves around the repeated "tiles"
    const int32_t topPadWidthLeft = topPadWidth / 2;
    const int32_t topPadWidthRight = topPadWidth - topPadWidthLeft;

    int32_t bottomTileWidth;
    int32_t bottomRepeatCount;
    if ( isEvilInterface ) {
        bottomTileWidth = 7; // width of a single bone piece
        bottomRepeatCount = extraDisplayWidth > 0 ? extraDisplayWidth / bottomTileWidth : 0;
    }
    else {
        bottomTileWidth = fheroes::tileWidthPx;
        bottomRepeatCount = topRepeatCount;
    }
    const int32_t bottomRepeatWidth = ( bottomRepeatCount + 1 ) * bottomTileWidth;

    const int32_t bottomPadWidth = extraDisplayWidth % bottomTileWidth;
    const int32_t bottomPadWidthLeft = bottomPadWidth / 2;
    const int32_t bottomPadWidthRight = bottomPadWidth - bottomPadWidthLeft;

    const int32_t vertPadHeight = extraDisplayHeight % fheroes::tileWidthPx;

    fheroes::Rect srcrt;
    fheroes::Point dstpt;
    const fheroes::Sprite & icnadv = fheroes::AGG::GetICN( isEvilInterface ? ICN::ADVBORDE : ICN::ADVBORD, 0 );

    // TOP BORDER
    srcrt.x = 0;
    srcrt.y = 0;
    srcrt.width = isEvilInterface ? 153 : 193;
    srcrt.height = fheroes::borderWidthPx;
    dstpt.x = srcrt.x;
    dstpt.y = srcrt.y;
    fheroes::Blit( icnadv, srcrt.x, srcrt.y, display, dstpt.x, dstpt.y, srcrt.width, srcrt.height );
    srcrt.x += srcrt.width;

    srcrt.width = 6;
    dstpt.x = srcrt.x;
    repeatPattern( icnadv, srcrt.x, srcrt.y, srcrt.width, srcrt.height, display, dstpt.x, dstpt.y, srcrt.width + topPadWidthLeft, fheroes::borderWidthPx );
    dstpt.x += srcrt.width + topPadWidthLeft;
    srcrt.x += srcrt.width;

    srcrt.width = isEvilInterface ? 64 : 24;
    fheroes::Blit( icnadv, srcrt.x, srcrt.y, display, dstpt.x, dstpt.y, srcrt.width, srcrt.height );
    dstpt.x += srcrt.width;
    srcrt.x += srcrt.width;

    srcrt.width = fheroes::tileWidthPx;
    repeatPattern( icnadv, srcrt.x, srcrt.y, srcrt.width, srcrt.height, display, dstpt.x, dstpt.y, topRepeatWidth, fheroes::borderWidthPx );
    dstpt.x += topRepeatWidth;
    srcrt.x += fheroes::tileWidthPx;

    srcrt.width = isEvilInterface ? 65 : 25;
    fheroes::Blit( icnadv, srcrt.x, srcrt.y, display, dstpt.x, dstpt.y, srcrt.width, srcrt.height );
    dstpt.x += srcrt.width;
    srcrt.x += srcrt.width;

    srcrt.width = 6;
    repeatPattern( icnadv, srcrt.x, srcrt.y, srcrt.width, srcrt.height, display, dstpt.x, dstpt.y, srcrt.width + topPadWidthRight, fheroes::borderWidthPx );
    dstpt.x += srcrt.width + topPadWidthRight;
    srcrt.x += srcrt.width;

    srcrt.width = icnadv.width() - srcrt.x;
    fheroes::Blit( icnadv, srcrt.x, srcrt.y, display, dstpt.x, dstpt.y, srcrt.width, srcrt.height );

    // LEFT BORDER
    srcrt.x = 0;
    srcrt.y = fheroes::borderWidthPx;
    srcrt.width = fheroes::borderWidthPx;
    srcrt.height = 255 - fheroes::borderWidthPx;
    dstpt.x = srcrt.x;
    dstpt.y = srcrt.y;
    fheroes::Blit( icnadv, srcrt.x, srcrt.y, display, dstpt.x, dstpt.y, srcrt.width, srcrt.height );
    dstpt.y += srcrt.height;
    srcrt.y += srcrt.height;

    if ( isEvilInterface ) {
        srcrt.height = fheroes::tileWidthPx;
        repeatPattern( icnadv, srcrt.x, srcrt.y, srcrt.width, srcrt.height, display, dstpt.x, dstpt.y, fheroes::borderWidthPx, vertRepeatHeightTop );
        dstpt.y += vertRepeatHeightTop;
        srcrt.y += fheroes::tileWidthPx;

        srcrt.width = fheroes::borderWidthPx;
        srcrt.height = 35;
        fheroes::Blit( icnadv, srcrt.x, srcrt.y, display, dstpt.x, dstpt.y, srcrt.width, srcrt.height );
        dstpt.y += srcrt.height;
        srcrt.y += srcrt.height;

        srcrt.height = 6;
        repeatPattern( icnadv, srcrt.x, srcrt.y, srcrt.width, srcrt.height, display, dstpt.x, dstpt.y, fheroes::borderWidthPx, srcrt.height + vertPadHeight );
        dstpt.y += srcrt.height + vertPadHeight;
        srcrt.y += srcrt.height;

        srcrt.height = 103;
        fheroes::Blit( icnadv, srcrt.x, srcrt.y, display, dstpt.x, dstpt.y, srcrt.width, srcrt.height );
        dstpt.y += srcrt.height;
        srcrt.y += srcrt.height;

        srcrt.height = fheroes::tileWidthPx;
        fheroes::Blit( icnadv, srcrt.x, srcrt.y, display, dstpt.x, dstpt.y, srcrt.width, srcrt.height );
        dstpt.y += fheroes::tileWidthPx;

        repeatPattern( icnadv, srcrt.x, srcrt.y, srcrt.width, srcrt.height, display, dstpt.x, dstpt.y, fheroes::borderWidthPx, vertRepeatHeightBottom );
        dstpt.y += vertRepeatHeightBottom;
        srcrt.y += fheroes::tileWidthPx;
    }
    else {
        srcrt.height = fheroes::tileWidthPx;
        repeatPattern( icnadv, srcrt.x, srcrt.y, srcrt.width, srcrt.height, display, dstpt.x, dstpt.y, fheroes::borderWidthPx, vertRepeatHeight );
        dstpt.y += vertRepeatHeight;
        srcrt.y += fheroes::tileWidthPx;

        srcrt.height = 125;
        fheroes::Blit( icnadv, srcrt.x, srcrt.y, display, dstpt.x, dstpt.y, srcrt.width, srcrt.height );
        dstpt.y += srcrt.height;
        srcrt.y += srcrt.height;

        srcrt.height = 4;
        repeatPattern( icnadv, srcrt.x, srcrt.y, srcrt.width, srcrt.height, display, dstpt.x, dstpt.y, fheroes::borderWidthPx, srcrt.height + vertPadHeight );
        dstpt.y += srcrt.height + vertPadHeight;
        srcrt.y += srcrt.height;
    }

    srcrt.height = icnadv.height() - fheroes::borderWidthPx - srcrt.y;
    fheroes::Blit( icnadv, srcrt.x, srcrt.y, display, dstpt.x, dstpt.y, srcrt.width, srcrt.height );

    // MIDDLE BORDER
    srcrt.x = icnadv.width() - fheroes::radarWidthPx - 2 * fheroes::borderWidthPx;
    srcrt.y = fheroes::borderWidthPx;
    srcrt.width = fheroes::borderWidthPx;
    srcrt.height = 255 - fheroes::borderWidthPx;
    dstpt.x = displayWidth - fheroes::radarWidthPx - 2 * fheroes::borderWidthPx;
    dstpt.y = srcrt.y;
    fheroes::Blit( icnadv, srcrt.x, srcrt.y, display, dstpt.x, dstpt.y, srcrt.width, srcrt.height );
    dstpt.y += srcrt.height;
    srcrt.y += srcrt.height;

    srcrt.height = fheroes::tileWidthPx;
    repeatPattern( icnadv, srcrt.x, srcrt.y, srcrt.width, srcrt.height, display, dstpt.x, dstpt.y, fheroes::borderWidthPx, vertRepeatHeightTop );
    dstpt.y += vertRepeatHeightTop;
    srcrt.y += fheroes::tileWidthPx;

    srcrt.height = isEvilInterface ? 35 : 50;
    fheroes::Blit( icnadv, srcrt.x, srcrt.y, display, dstpt.x, dstpt.y, srcrt.width, srcrt.height );

    // hide embranchment
    if ( viewWorldMode ) {
        fheroes::Rect fixrt( 478, isEvilInterface ? 137 : 345, 3, isEvilInterface ? 15 : 20 );
        fheroes::Point fixpt( dstpt.x + 14, dstpt.y + 18 );
        fheroes::Blit( icnadv, fixrt.x, fixrt.y, display, fixpt.x, fixpt.y, fixrt.width, fixrt.height );
    }

    dstpt.y += srcrt.height;
    srcrt.y += srcrt.height;

    if ( isEvilInterface ) {
        srcrt.height = 6;
        repeatPattern( icnadv, srcrt.x, srcrt.y, srcrt.width, srcrt.height, display, dstpt.x, dstpt.y, fheroes::borderWidthPx, srcrt.height + vertPadHeight );
        dstpt.y += srcrt.height + vertPadHeight;
        srcrt.y += srcrt.height;

        srcrt.height = 103;
        fheroes::Blit( icnadv, srcrt.x, srcrt.y, display, dstpt.x, dstpt.y, srcrt.width, srcrt.height );
        dstpt.y += srcrt.height;
        srcrt.y += srcrt.height;

        srcrt.height = fheroes::tileWidthPx;
        fheroes::Blit( icnadv, srcrt.x, srcrt.y, display, dstpt.x, dstpt.y, srcrt.width, srcrt.height );
        dstpt.y += fheroes::tileWidthPx;

        repeatPattern( icnadv, srcrt.x, srcrt.y, srcrt.width, srcrt.height, display, dstpt.x, dstpt.y, fheroes::borderWidthPx, vertRepeatHeightBottom );
        dstpt.y += vertRepeatHeightBottom;
        srcrt.y += fheroes::tileWidthPx;
    }
    else {
        srcrt.height = fheroes::tileWidthPx;
        fheroes::Blit( icnadv, srcrt.x, srcrt.y, display, dstpt.x, dstpt.y, srcrt.width, srcrt.height );
        dstpt.y += fheroes::tileWidthPx;

        repeatPattern( icnadv, srcrt.x, srcrt.y, srcrt.width, srcrt.height, display, dstpt.x, dstpt.y, fheroes::borderWidthPx, vertRepeatHeightBottom );
        dstpt.y += vertRepeatHeightBottom;
        srcrt.y += fheroes::tileWidthPx;

        srcrt.height = 43;
        fheroes::Blit( icnadv, srcrt.x, srcrt.y, display, dstpt.x, dstpt.y, srcrt.width, srcrt.height );
        dstpt.y += srcrt.height;
        srcrt.y += srcrt.height;

        srcrt.height = 8; // middle border is special on good interface due to all the green leaves
        repeatPattern( icnadv, srcrt.x, srcrt.y, srcrt.width, srcrt.height, display, dstpt.x, dstpt.y, fheroes::borderWidthPx, srcrt.height + vertPadHeight );
        dstpt.y += srcrt.height + vertPadHeight;
        srcrt.y += srcrt.height;
    }

    srcrt.height = icnadv.height() - fheroes::borderWidthPx - srcrt.y;
    fheroes::Blit( icnadv, srcrt.x, srcrt.y, display, dstpt.x, dstpt.y, srcrt.width, srcrt.height );

    // RIGHT BORDER
    srcrt.x = icnadv.width() - fheroes::borderWidthPx;
    srcrt.y = fheroes::borderWidthPx;
    srcrt.width = fheroes::borderWidthPx;
    srcrt.height = 255 - fheroes::borderWidthPx;
    dstpt.x = displayWidth - fheroes::borderWidthPx;
    dstpt.y = srcrt.y;
    fheroes::Blit( icnadv, srcrt.x, srcrt.y, display, dstpt.x, dstpt.y, srcrt.width, srcrt.height );
    dstpt.y += srcrt.height;
    srcrt.y += srcrt.height;

    srcrt.height = fheroes::tileWidthPx;
    repeatPattern( icnadv, srcrt.x, srcrt.y, srcrt.width, srcrt.height, display, dstpt.x, dstpt.y, fheroes::borderWidthPx, vertRepeatHeightTop );
    dstpt.y += vertRepeatHeightTop;
    srcrt.y += fheroes::tileWidthPx;

    srcrt.height = isEvilInterface ? 35 : 50;
    fheroes::Blit( icnadv, srcrt.x, srcrt.y, display, dstpt.x, dstpt.y, srcrt.width, srcrt.height );

    // hide embranchment
    if ( viewWorldMode ) {
        fheroes::Rect fixrt( 624, isEvilInterface ? 139 : 345, 3, isEvilInterface ? 15 : 20 );
        fheroes::Point fixpt( dstpt.x, dstpt.y + 18 );
        fheroes::Blit( icnadv, fixrt.x, fixrt.y, display, fixpt.x, fixpt.y, fixrt.width, fixrt.height );
    }

    dstpt.y += srcrt.height;
    srcrt.y += srcrt.height;

    if ( isEvilInterface ) {
        srcrt.height = 6;
        repeatPattern( icnadv, srcrt.x, srcrt.y, srcrt.width, srcrt.height, display, dstpt.x, dstpt.y, fheroes::borderWidthPx, srcrt.height + vertPadHeight );
        dstpt.y += srcrt.height + vertPadHeight;
        srcrt.y += srcrt.height;

        srcrt.height = 103;
        fheroes::Blit( icnadv, srcrt.x, srcrt.y, display, dstpt.x, dstpt.y, srcrt.width, srcrt.height );
        dstpt.y += srcrt.height;
        srcrt.y += srcrt.height;

        srcrt.height = fheroes::tileWidthPx;
        fheroes::Blit( icnadv, srcrt.x, srcrt.y, display, dstpt.x, dstpt.y, srcrt.width, srcrt.height );
        dstpt.y += fheroes::tileWidthPx;

        repeatPattern( icnadv, srcrt.x, srcrt.y, srcrt.width, srcrt.height, display, dstpt.x, dstpt.y, fheroes::borderWidthPx, vertRepeatHeightBottom );
        dstpt.y += vertRepeatHeightBottom;
        srcrt.y += fheroes::tileWidthPx;
    }
    else {
        srcrt.height = fheroes::tileWidthPx;
        fheroes::Blit( icnadv, srcrt.x, srcrt.y, display, dstpt.x, dstpt.y, srcrt.width, srcrt.height );
        dstpt.y += fheroes::tileWidthPx;

        repeatPattern( icnadv, srcrt.x, srcrt.y, srcrt.width, srcrt.height, display, dstpt.x, dstpt.y, fheroes::borderWidthPx, vertRepeatHeightBottom );
        dstpt.y += vertRepeatHeightBottom;
        srcrt.y += fheroes::tileWidthPx;

        srcrt.height = 43;
        fheroes::Blit( icnadv, srcrt.x, srcrt.y, display, dstpt.x, dstpt.y, srcrt.width, srcrt.height );
        dstpt.y += srcrt.height;
        srcrt.y += srcrt.height;

        srcrt.height = 4;
        repeatPattern( icnadv, srcrt.x, srcrt.y, srcrt.width, srcrt.height, display, dstpt.x, dstpt.y, fheroes::borderWidthPx, srcrt.height + vertPadHeight );
        dstpt.y += srcrt.height + vertPadHeight;
        srcrt.y += srcrt.height;
    }

    srcrt.height = icnadv.height() - fheroes::borderWidthPx - srcrt.y;
    fheroes::Blit( icnadv, srcrt.x, srcrt.y, display, dstpt.x, dstpt.y, srcrt.width, srcrt.height );

    // BOTTOM BORDER
    srcrt.x = 0;
    srcrt.y = icnadv.height() - fheroes::borderWidthPx;
    srcrt.width = isEvilInterface ? 129 : 193;
    srcrt.height = fheroes::borderWidthPx;
    dstpt.x = srcrt.x;
    dstpt.y = displayHeight - fheroes::borderWidthPx;
    fheroes::Blit( icnadv, srcrt.x, srcrt.y, display, dstpt.x, dstpt.y, srcrt.width, srcrt.height );
    dstpt.x += srcrt.width;
    srcrt.x += srcrt.width;

    srcrt.width = 6;
    repeatPattern( icnadv, srcrt.x, srcrt.y, srcrt.width, srcrt.height, display, dstpt.x, dstpt.y, srcrt.width + bottomPadWidthLeft, fheroes::borderWidthPx );
    dstpt.x += srcrt.width + bottomPadWidthLeft;
    srcrt.x += srcrt.width;

    srcrt.width = isEvilInterface ? 90 : 24;
    fheroes::Blit( icnadv, srcrt.x, srcrt.y, display, dstpt.x, dstpt.y, srcrt.width, srcrt.height );
    dstpt.x += srcrt.width;
    srcrt.x += srcrt.width;

    srcrt.width = bottomTileWidth;
    repeatPattern( icnadv, srcrt.x, srcrt.y, srcrt.width, srcrt.height, display, dstpt.x, dstpt.y, bottomRepeatWidth, fheroes::borderWidthPx );
    dstpt.x += bottomRepeatWidth;
    srcrt.x += srcrt.width;

    srcrt.width = isEvilInterface ? 86 : 25; // evil bottom border is asymmetric
    fheroes::Blit( icnadv, srcrt.x, srcrt.y, display, dstpt.x, dstpt.y, srcrt.width, srcrt.height );
    dstpt.x += srcrt.width;
    srcrt.x += srcrt.width;

    srcrt.width = 6;
    repeatPattern( icnadv, srcrt.x, srcrt.y, srcrt.width, srcrt.height, display, dstpt.x, dstpt.y, srcrt.width + bottomPadWidthRight, fheroes::borderWidthPx );
    dstpt.x += srcrt.width + bottomPadWidthRight;
    srcrt.x += srcrt.width;

    srcrt.width = icnadv.width() - srcrt.x;
    fheroes::Blit( icnadv, srcrt.x, srcrt.y, display, dstpt.x, dstpt.y, srcrt.width, srcrt.height );

    // ICON BORDER
    srcrt.x = icnadv.width() - fheroes::radarWidthPx - fheroes::borderWidthPx;
    srcrt.y = fheroes::radarWidthPx + fheroes::borderWidthPx;
    srcrt.width = fheroes::radarWidthPx;
    srcrt.height = fheroes::borderWidthPx;
    dstpt.x = displayWidth - fheroes::radarWidthPx - fheroes::borderWidthPx;
    dstpt.y = srcrt.y;
    fheroes::Blit( icnadv, srcrt.x, srcrt.y, display, dstpt.x, dstpt.y, srcrt.width, srcrt.height );

    if ( !viewWorldMode ) {
        dstpt.y = srcrt.y + fheroes::borderWidthPx + iconsCount * 32;
        srcrt.y = srcrt.y + fheroes::borderWidthPx + 4 * 32;
        fheroes::Blit( icnadv, srcrt.x, srcrt.y, display, dstpt.x, dstpt.y, srcrt.width, srcrt.height );
    }
}

Interface::BorderWindow::BorderWindow( const fheroes::Rect & rt )
    : area( rt )
{}

const fheroes::Rect & Interface::BorderWindow::GetRect() const
{
    return Settings::Get().isHideInterfaceEnabled() && border.isValid() ? border.GetRect() : GetArea();
}

bool Interface::BorderWindow::isMouseCaptured()
{
    if ( !_isMouseCaptured ) {
        return false;
    }

    const LocalEvent & le = LocalEvent::Get();

    _isMouseCaptured = le.isMouseLeftButtonPressed();

    // Even if the mouse has just been released from the capture, consider it still captured at this
    // stage to ensure that events directly related to the release (for instance, releasing the mouse
    // button) will not be handled by other UI elements.
    return true;
}

void Interface::BorderWindow::Redraw() const
{
    Dialog::FrameBorder::RenderRegular( border.GetRect() );
}

void Interface::BorderWindow::SetPosition( const int32_t x, const int32_t y, const int32_t width, const int32_t height )
{
    area.width = width;
    area.height = height;

    SetPosition( x, y );
}

void Interface::BorderWindow::SetPosition( int32_t x, int32_t y )
{
    if ( Settings::Get().isHideInterfaceEnabled() ) {
        const fheroes::Display & display = fheroes::Display::instance();

        x = std::max( 0, std::min( x, display.width() - ( area.width + border.BorderWidth() * 2 ) ) );
        y = std::max( 0, std::min( y, display.height() - ( area.height + border.BorderHeight() * 2 ) ) );

        area.x = x + border.BorderWidth();
        area.y = y + border.BorderHeight();

        border.SetPosition( x, y, area.width, area.height );
        SavePosition();
    }
    else {
        area.x = x;
        area.y = y;
    }
}

void Interface::BorderWindow::captureMouse()
{
    const LocalEvent & le = LocalEvent::Get();

    if ( le.isMouseLeftButtonPressedInArea( GetRect() ) ) {
        _isMouseCaptured = true;
    }
    else {
        _isMouseCaptured = _isMouseCaptured && le.isMouseLeftButtonPressed();
    }
}

bool Interface::BorderWindow::QueueEventProcessing()
{
    LocalEvent & le = LocalEvent::Get();

    if ( !Settings::Get().isHideInterfaceEnabled() || !le.isMouseLeftButtonPressedInArea( border.GetTop() ) ) {
        return false;
    }

    // Reset the cursor when dragging, because if the window border is close to the edge of the game window,
    // then the mouse cursor could be changed to a scroll cursor.
    Cursor::Get().SetThemes( Cursor::POINTER );

    fheroes::Display & display = fheroes::Display::instance();

    const fheroes::Point & mp = le.getMouseCursorPos();
    const fheroes::Rect & pos = GetRect();

    fheroes::MovableSprite moveIndicator( pos.width, pos.height, pos.x, pos.y );
    moveIndicator.reset();
    fheroes::DrawBorder( moveIndicator, fheroes::GetColorId( 0xD0, 0xC0, 0x48 ), 6 );

    const int32_t ox = mp.x - pos.x;
    const int32_t oy = mp.y - pos.y;

    moveIndicator.setPosition( pos.x, pos.y );
    moveIndicator.redraw();
    display.render();

    while ( le.HandleEvents() && le.isMouseLeftButtonPressed() ) {
        if ( le.hasMouseMoved() ) {
            moveIndicator.setPosition( mp.x - ox, mp.y - oy );
            display.render();
        }
    }

    SetPos( mp.x - ox, mp.y - oy );

    return true;
}
