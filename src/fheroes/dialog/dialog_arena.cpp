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

#include <cstdint>
#include <string>

#include "agg_image.h"
#include "cursor.h"
#include "dialog.h" // IWYU pragma: associated
#include "game_hotkeys.h"
#include "icn.h"
#include "image.h"
#include "localevent.h"
#include "math_base.h"
#include "screen.h"
#include "settings.h"
#include "skill.h"
#include "translations.h"
#include "ui_button.h"
#include "ui_dialog.h"
#include "ui_text.h"

namespace
{
    void InfoSkillClear( const fheroes::Rect & rect1, const fheroes::Rect & rect2, const fheroes::Rect & rect3 )
    {
        fheroes::Display & display = fheroes::Display::instance();

        fheroes::Blit( fheroes::AGG::GetICN( ICN::XPRIMARY, 0 ), display, rect1.x, rect1.y );
        fheroes::Blit( fheroes::AGG::GetICN( ICN::XPRIMARY, 1 ), display, rect2.x, rect2.y );
        fheroes::Blit( fheroes::AGG::GetICN( ICN::XPRIMARY, 2 ), display, rect3.x, rect3.y );
    }

    void InfoSkillSelect( int skill, const fheroes::Rect & rect1, const fheroes::Rect & rect2, const fheroes::Rect & rect3 )
    {
        fheroes::Display & display = fheroes::Display::instance();

        switch ( skill ) {
        case Skill::Primary::ATTACK:
            fheroes::Blit( fheroes::AGG::GetICN( ICN::XPRIMARY, 4 ), display, rect1.x, rect1.y );
            break;
        case Skill::Primary::DEFENSE:
            fheroes::Blit( fheroes::AGG::GetICN( ICN::XPRIMARY, 5 ), display, rect2.x, rect2.y );
            break;
        case Skill::Primary::POWER:
            fheroes::Blit( fheroes::AGG::GetICN( ICN::XPRIMARY, 6 ), display, rect3.x, rect3.y );
            break;
        default:
            break;
        }
    }

    int InfoSkillNext( int skill )
    {
        switch ( skill ) {
        case Skill::Primary::ATTACK:
            return Skill::Primary::DEFENSE;
        case Skill::Primary::DEFENSE:
            return Skill::Primary::POWER;
        default:
            break;
        }

        return Skill::Primary::UNKNOWN;
    }

    int InfoSkillPrev( int skill )
    {
        switch ( skill ) {
        case Skill::Primary::POWER:
            return Skill::Primary::DEFENSE;
        case Skill::Primary::DEFENSE:
            return Skill::Primary::ATTACK;
        default:
            break;
        }

        return Skill::Primary::UNKNOWN;
    }
}

int Dialog::SelectSkillFromArena()
{
    fheroes::Display & display = fheroes::Display::instance();
    const int system = Settings::Get().isEvilInterfaceEnabled() ? ICN::SYSTEME : ICN::SYSTEM;

    // setup cursor
    const CursorRestorer cursorRestorer( true, Cursor::POINTER );

    fheroes::Text title( _( "Arena" ), fheroes::FontType::normalYellow() );

    fheroes::Text textbox(
        _( "You enter the arena and face a pack of vicious lions. You handily defeat them, to the wild cheers of the crowd. Impressed by your skill, the aged trainer of gladiators agrees to train you in a skill of your choice." ),
        fheroes::FontType::normalWhite() );
    const fheroes::Sprite & sprite = fheroes::AGG::GetICN( ICN::XPRIMARY, 0 );
    const int spacer = 10;

    const Dialog::FrameBox box( title.height( fheroes::boxAreaWidthPx ) + textbox.height( fheroes::boxAreaWidthPx ) + 2 * spacer + sprite.height() + 15, true );

    const fheroes::Rect & box_rt = box.GetArea();
    fheroes::Point dst_pt( box_rt.x, box_rt.y );

    title.draw( dst_pt.x, dst_pt.y + 2, fheroes::boxAreaWidthPx, display );
    dst_pt.y += title.height( fheroes::boxAreaWidthPx ) + spacer;

    textbox.draw( dst_pt.x, dst_pt.y + 2, fheroes::boxAreaWidthPx, display );
    dst_pt.y += textbox.height( fheroes::boxAreaWidthPx ) + spacer;

    int res = Skill::Primary::ATTACK;

    const int spacingX = ( box_rt.width - sprite.width() * 3 ) / 4;

    fheroes::Rect rect1( dst_pt.x + spacingX, dst_pt.y, sprite.width(), sprite.height() );
    fheroes::Rect rect2( rect1.x + sprite.width() + spacingX, dst_pt.y, sprite.width(), sprite.height() );
    fheroes::Rect rect3( rect2.x + sprite.width() + spacingX, dst_pt.y, sprite.width(), sprite.height() );

    InfoSkillClear( rect1, rect2, rect3 );
    InfoSkillSelect( res, rect1, rect2, rect3 );

    // info texts
    const int32_t skillTextWidth = 60;

    fheroes::Text text( Skill::Primary::String( Skill::Primary::ATTACK ), fheroes::FontType::smallWhite() );
    dst_pt.x = rect1.x + ( rect1.width - skillTextWidth ) / 2;
    dst_pt.y = rect1.y + rect1.height + 5;
    text.draw( dst_pt.x, dst_pt.y + 2, skillTextWidth, display );

    text.set( Skill::Primary::String( Skill::Primary::DEFENSE ), fheroes::FontType::smallWhite() );
    dst_pt.x = rect2.x + ( rect2.width - skillTextWidth ) / 2;
    dst_pt.y = rect2.y + rect2.height + 5;
    text.draw( dst_pt.x, dst_pt.y + 2, skillTextWidth, display );

    text.set( Skill::Primary::String( Skill::Primary::POWER ), fheroes::FontType::smallWhite() );
    dst_pt.x = rect3.x + ( rect3.width - skillTextWidth ) / 2;
    dst_pt.y = rect3.y + rect3.height + 5;
    text.draw( dst_pt.x, dst_pt.y + 2, skillTextWidth, display );

    // buttons
    dst_pt.x = box_rt.x + ( box_rt.width - fheroes::AGG::GetICN( system, 1 ).width() ) / 2;
    dst_pt.y = box_rt.y + box_rt.height - fheroes::AGG::GetICN( system, 1 ).height();
    fheroes::Button buttonOk( dst_pt.x, dst_pt.y, system, 1, 2 );

    LocalEvent & le = LocalEvent::Get();

    buttonOk.draw();
    display.render();

    // message loop
    while ( le.HandleEvents() ) {
        bool redraw = false;

        buttonOk.drawOnState( le.isMouseLeftButtonPressedAndHeldInArea( buttonOk.area() ) );

        if ( Game::HotKeyPressEvent( Game::HotKeyEvent::DEFAULT_LEFT ) && Skill::Primary::UNKNOWN != InfoSkillPrev( res ) ) {
            res = InfoSkillPrev( res );
            redraw = true;
        }
        else if ( Game::HotKeyPressEvent( Game::HotKeyEvent::DEFAULT_RIGHT ) && Skill::Primary::UNKNOWN != InfoSkillNext( res ) ) {
            res = InfoSkillNext( res );
            redraw = true;
        }
        else if ( le.MouseClickLeft( rect1 ) ) {
            res = Skill::Primary::ATTACK;
            redraw = true;
        }
        else if ( le.MouseClickLeft( rect2 ) ) {
            res = Skill::Primary::DEFENSE;
            redraw = true;
        }
        else if ( le.MouseClickLeft( rect3 ) ) {
            res = Skill::Primary::POWER;
            redraw = true;
        }
        else if ( le.isMouseRightButtonPressedInArea( rect1 ) ) {
            fheroes::PrimarySkillDialogElement( Skill::Primary::ATTACK, "" ).showPopup( Dialog::ZERO );
        }
        else if ( le.isMouseRightButtonPressedInArea( rect2 ) ) {
            fheroes::PrimarySkillDialogElement( Skill::Primary::DEFENSE, "" ).showPopup( Dialog::ZERO );
        }
        else if ( le.isMouseRightButtonPressedInArea( rect3 ) ) {
            fheroes::PrimarySkillDialogElement( Skill::Primary::POWER, "" ).showPopup( Dialog::ZERO );
        }

        if ( redraw ) {
            InfoSkillClear( rect1, rect2, rect3 );
            InfoSkillSelect( res, rect1, rect2, rect3 );
            display.render();
        }

        if ( Game::HotKeyPressEvent( Game::HotKeyEvent::DEFAULT_OKAY ) || le.MouseClickLeft( buttonOk.area() ) )
            break;
    }

    return res;
}
