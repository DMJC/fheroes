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
#include <utility>

#include "agg_image.h"
#include "castle.h" // IWYU pragma: associated
#include "cursor.h"
#include "dialog.h"
#include "game_hotkeys.h"
#include "heroes.h"
#include "icn.h"
#include "image.h"
#include "localevent.h"
#include "mageguild.h"
#include "math_base.h"
#include "screen.h"
#include "settings.h"
#include "spell.h"
#include "spell_storage.h"
#include "translations.h"
#include "ui_button.h"
#include "ui_dialog.h"
#include "ui_mage_guild.h"
#include "ui_text.h"

namespace
{
    const int32_t bottomBarOffsetY = 461;
}

void Castle::_openMageGuild( const Heroes * hero ) const
{
    fheroes::Display & display = fheroes::Display::instance();

    // setup cursor
    const CursorRestorer cursorRestorer( true, Cursor::POINTER );

    const fheroes::ImageRestorer restorer( display, ( display.width() - fheroes::Display::DEFAULT_WIDTH ) / 2,
                                            ( display.height() - fheroes::Display::DEFAULT_HEIGHT ) / 2, fheroes::Display::DEFAULT_WIDTH,
                                            fheroes::Display::DEFAULT_HEIGHT );

    const fheroes::Point cur_pt( restorer.x(), restorer.y() );
    fheroes::Point dst_pt( cur_pt.x, cur_pt.y );

    const bool isEvilInterface = Settings::Get().isEvilInterfaceEnabled();

    fheroes::Blit( fheroes::AGG::GetICN( isEvilInterface ? ICN::STONEBAK_EVIL : ICN::STONEBAK, 0 ), display, cur_pt.x, cur_pt.y );

    // status bar
    const int32_t exitWidth = fheroes::AGG::GetICN( ICN::BUTTON_GUILDWELL_EXIT, 0 ).width();

    dst_pt.x = cur_pt.x;
    dst_pt.y = cur_pt.y + bottomBarOffsetY;

    // The original ICN::WELLXTRA image does not have a yellow outer frame.
    const fheroes::Sprite & bottomBar = fheroes::AGG::GetICN( ICN::SMALLBAR, 0 );
    const int32_t barHeight = bottomBar.height();
    // ICN::SMALLBAR image's first column contains all black pixels. This should not be drawn.
    fheroes::Copy( bottomBar, 1, 0, display, dst_pt.x, dst_pt.y, fheroes::Display::DEFAULT_WIDTH / 2, barHeight );
    fheroes::Copy( bottomBar, bottomBar.width() - fheroes::Display::DEFAULT_WIDTH / 2 + exitWidth - 1, 0, display, dst_pt.x + fheroes::Display::DEFAULT_WIDTH / 2,
                    dst_pt.y, fheroes::Display::DEFAULT_WIDTH / 2 - exitWidth + 1, barHeight );

    // text bar
    const char * textAlternative;
    if ( hero == nullptr || !hero->HaveSpellBook() ) {
        textAlternative = _( "The above spells are available here." );
    }
    else {
        textAlternative = _( "The spells the hero can learn have been added to their book." );
    }
    fheroes::Text statusText( textAlternative, fheroes::FontType::normalWhite() );
    statusText.draw( cur_pt.x + ( fheroes::Display::DEFAULT_WIDTH - exitWidth ) / 2 - statusText.width() / 2, cur_pt.y + 464, display );

    const int guildLevel = GetLevelMageGuild();

    fheroes::renderMageGuildBuilding( _race, guildLevel, cur_pt );

    const bool haveLibraryCapability = HaveLibraryCapability();
    const bool hasLibrary = isLibraryBuilt();

    std::array<std::unique_ptr<fheroes::SpellsInOneRow>, 5> spellRows;

    for ( size_t levelIndex = 0; levelIndex < spellRows.size(); ++levelIndex ) {
        const int32_t spellsLevel = static_cast<int32_t>( levelIndex ) + 1;
        const int32_t count = MageGuild::getMaxSpellsCount( spellsLevel, haveLibraryCapability );

        SpellStorage spells = GetMageGuild().GetSpells( guildLevel, hasLibrary, spellsLevel );
        spells.resize( count, Spell::NONE );

        spellRows[levelIndex] = std::make_unique<fheroes::SpellsInOneRow>( std::move( spells ) );

        spellRows[levelIndex]->setPosition( { cur_pt.x + 250, cur_pt.y + 365 - 90 * static_cast<int32_t>( levelIndex ) } );
        spellRows[levelIndex]->redraw( display );
    }

    fheroes::Button buttonExit( cur_pt.x + fheroes::Display::DEFAULT_WIDTH - exitWidth, cur_pt.y + bottomBarOffsetY, ICN::BUTTON_GUILDWELL_EXIT, 0, 1 );

    buttonExit.draw();

    display.render();

    LocalEvent & le = LocalEvent::Get();

    // message loop
    while ( le.HandleEvents() ) {
        if ( le.MouseClickLeft( buttonExit.area() ) || Game::HotKeyCloseWindow() ) {
            break;
        }

        buttonExit.drawOnState( le.isMouseLeftButtonPressedAndHeldInArea( buttonExit.area() ) );

        spellRows[0]->queueEventProcessing( false ) || spellRows[1]->queueEventProcessing( false ) || spellRows[2]->queueEventProcessing( false )
            || spellRows[3]->queueEventProcessing( false ) || spellRows[4]->queueEventProcessing( false );

        if ( le.isMouseRightButtonPressedInArea( buttonExit.area() ) ) {
            fheroes::showStandardTextMessage( _( "Exit" ), _( "Exit this menu." ), Dialog::ZERO );
        }
    }
}
