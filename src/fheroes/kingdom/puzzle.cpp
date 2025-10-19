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

#include "puzzle.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <functional>
#include <string>
#include <vector>

#include "agg_image.h"
#include "artifact_ultimate.h"
#include "audio.h"
#include "audio_manager.h"
#include "cursor.h"
#include "dialog.h"
#include "game_delays.h"
#include "game_hotkeys.h"
#include "game_interface.h"
#include "icn.h"
#include "image.h"
#include "interface_gamearea.h"
#include "interface_radar.h"
#include "localevent.h"
#include "logging.h"
#include "math_base.h"
#include "mus.h"
#include "rand.h"
#include "screen.h"
#include "serialize.h"
#include "settings.h"
#include "translations.h"
#include "ui_button.h"
#include "ui_constants.h"
#include "ui_dialog.h"
#include "ui_tool.h"
#include "ui_window.h"
#include "world.h"

namespace
{
    bool ClosedTilesExists( const Puzzle & pzl, const std::vector<uint8_t> & zone )
    {
        return std::any_of( zone.begin(), zone.end(), [&pzl]( const uint8_t tile ) { return !pzl.test( tile ); } );
    }

    void ZoneOpenFirstTiles( Puzzle & pzl, size_t & opens, const std::vector<uint8_t> & zone )
    {
        while ( opens ) {
            std::vector<uint8_t>::const_iterator it = zone.begin();
            while ( it != zone.end() && pzl.test( *it ) )
                ++it;

            if ( it != zone.end() ) {
                pzl.set( *it );
                --opens;
            }
            else
                break;
        }
    }

    void drawPuzzle( const Puzzle & pzl, const fheroes::Image & sf, int32_t dstx, int32_t dsty )
    {
        fheroes::Display & display = fheroes::Display::instance();

        // Immediately reveal the entire puzzle in developer mode
        if ( IS_DEVEL() ) {
            assert( sf.singleLayer() );

            fheroes::Copy( sf, 0, 0, display, dstx, dsty, sf.width(), sf.height() );

            return;
        }

        for ( size_t i = 0; i < pzl.size(); ++i ) {
            const fheroes::Sprite & piece = fheroes::AGG::GetICN( ICN::PUZZLE, static_cast<uint32_t>( i ) );

            fheroes::Blit( piece, display, dstx + piece.x() - fheroes::borderWidthPx, dsty + piece.y() - fheroes::borderWidthPx );
        }
    }

    bool revealPuzzle( const Puzzle & pzl, const fheroes::Image & sf, int32_t dstx, int32_t dsty, fheroes::Button & buttonExit,
                       const std::function<fheroes::Rect()> * drawControlPanel = nullptr )
    {
        // In developer mode, the entire puzzle should already be revealed
        if ( IS_DEVEL() ) {
            return false;
        }

        // The game area puzzle image should be single-layer.
        assert( sf.singleLayer() );

        fheroes::Display & display = fheroes::Display::instance();
        LocalEvent & le = LocalEvent::Get();

        const std::vector<Game::DelayType> delayTypes = { Game::PUZZLE_FADE_DELAY };
        Game::passAnimationDelay( Game::PUZZLE_FADE_DELAY );

        int alpha = 250;

        while ( alpha >= 0 && le.HandleEvents( Game::isDelayNeeded( delayTypes ) ) ) {
            buttonExit.drawOnState( le.isMouseLeftButtonPressedAndHeldInArea( buttonExit.area() ) );
            // If exit button was pressed before reveal animation is finished, return true to indicate early exit.
            if ( le.MouseClickLeft( buttonExit.area() ) || Game::HotKeyCloseWindow() ) {
                return true;
            }

            if ( le.isMouseRightButtonPressedInArea( buttonExit.area() ) ) {
                fheroes::showStandardTextMessage( _( "Exit" ), _( "Exit this menu." ), 0 );
            }

            if ( Game::validateAnimationDelay( Game::PUZZLE_FADE_DELAY ) ) {
                fheroes::Copy( sf, 0, 0, display, dstx, dsty, sf.width(), sf.height() );

                for ( size_t i = 0; i < pzl.size(); ++i ) {
                    const fheroes::Sprite & piece = fheroes::AGG::GetICN( ICN::PUZZLE, static_cast<uint32_t>( i ) );

                    uint8_t pieceAlpha = 255;
                    if ( pzl.test( i ) )
                        pieceAlpha = static_cast<uint8_t>( alpha );

                    fheroes::AlphaBlit( piece, display, dstx + piece.x() - fheroes::borderWidthPx, dsty + piece.y() - fheroes::borderWidthPx, pieceAlpha );
                }

                if ( drawControlPanel ) {
                    display.render( ( *drawControlPanel )() );
                }

                display.render( { dstx, dsty, sf.width(), sf.height() } );

                if ( alpha <= 0 ) {
                    break;
                }

                alpha -= 10;
                assert( alpha >= 0 );
            }
        }

        return false;
    }

    void ShowStandardDialog( const Puzzle & pzl, const fheroes::Image & sf )
    {
        fheroes::Display & display = fheroes::Display::instance();

        const bool isEvilInterface = Settings::Get().isEvilInterfaceEnabled();

        // Puzzle map is called only for the Adventure Map, not Editor.
        Interface::AdventureMap & adventureMapInterface = Interface::AdventureMap::Get();
        const fheroes::Rect & radarArea = adventureMapInterface.getRadar().GetArea();

        fheroes::ImageRestorer back( display, fheroes::borderWidthPx, fheroes::borderWidthPx, sf.width(), sf.height() );
        fheroes::ImageRestorer radarRestorer( display, radarArea.x, radarArea.y, radarArea.width, radarArea.height );

        fheroes::fadeOutDisplay( back.rect(), false );

        fheroes::Copy( fheroes::AGG::GetICN( ( isEvilInterface ? ICN::EVIWPUZL : ICN::VIEWPUZL ), 0 ), 0, 0, display, radarArea );
        display.updateNextRenderRoi( radarArea );

        const int exitButtonIcnID = ( isEvilInterface ? ICN::BUTTON_SMALL_EXIT_EVIL : ICN::BUTTON_SMALL_EXIT_GOOD );
        fheroes::Button buttonExit( radarArea.x + 32, radarArea.y + radarArea.height - 37, exitButtonIcnID, 0, 1 );
        buttonExit.setPosition( radarArea.x + ( radarArea.width - buttonExit.area().width ) / 2, buttonExit.area().y );
        fheroes::addGradientShadow( fheroes::AGG::GetICN( exitButtonIcnID, 0 ), display, buttonExit.area().getPosition(), { -5, 5 } );

        buttonExit.draw();

        drawPuzzle( pzl, sf, fheroes::borderWidthPx, fheroes::borderWidthPx );

        display.updateNextRenderRoi( radarArea );

        fheroes::fadeInDisplay( back.rect(), false );

        const bool earlyExit = revealPuzzle( pzl, sf, fheroes::borderWidthPx, fheroes::borderWidthPx, buttonExit );

        LocalEvent & le = LocalEvent::Get();

        while ( !earlyExit && le.HandleEvents() ) {
            buttonExit.drawOnState( le.isMouseLeftButtonPressedAndHeldInArea( buttonExit.area() ) );
            if ( le.MouseClickLeft( buttonExit.area() ) || Game::HotKeyCloseWindow() ) {
                break;
            }

            if ( le.isMouseRightButtonPressedInArea( buttonExit.area() ) ) {
                fheroes::showStandardTextMessage( _( "Exit" ), _( "Exit this menu." ), 0 );
            }
        }

        // Fade from puzzle map to adventure map.
        fheroes::fadeOutDisplay( back.rect(), false );
        back.restore();
        radarRestorer.restore();
        display.updateNextRenderRoi( radarArea );
        fheroes::fadeInDisplay( back.rect(), false );

        adventureMapInterface.getGameArea().SetUpdateCursor();
    }

    void ShowExtendedDialog( const Puzzle & pzl, const fheroes::Image & sf )
    {
        fheroes::Display & display = fheroes::Display::instance();

        // Puzzle map is called only for the Adventure Map, not Editor.
        Interface::AdventureMap & adventureMapInterface = Interface::AdventureMap::Get();
        Interface::GameArea & gameArea = adventureMapInterface.getGameArea();
        const fheroes::Rect & gameAreaRoi = gameArea.GetROI();
        const Interface::Radar & radar = adventureMapInterface.getRadar();
        const fheroes::Rect & radarArea = radar.GetArea();

        const fheroes::ImageRestorer radarRestorer( display, radarArea.x, radarArea.y, radarArea.width, radarArea.height );

        const fheroes::StandardWindow border( gameAreaRoi.x + ( gameAreaRoi.width - sf.width() ) / 2, gameAreaRoi.y + ( gameAreaRoi.height - sf.height() ) / 2,
                                               sf.width(), sf.height(), false );

        const fheroes::Rect & puzzleArea = border.activeArea();

        fheroes::Image background( puzzleArea.width, puzzleArea.height );

        const Settings & conf = Settings::Get();
        const bool isEvilInterface = conf.isEvilInterfaceEnabled();
        const bool isHideInterface = conf.isHideInterfaceEnabled();

        if ( isEvilInterface ) {
            background.fill( fheroes::GetColorId( 80, 80, 80 ) );
        }
        else {
            background.fill( fheroes::GetColorId( 128, 64, 32 ) );
        }

        fheroes::Copy( background, 0, 0, display, puzzleArea );

        const int exitButtonIcnID = ( isEvilInterface ? ICN::BUTTON_SMALL_EXIT_EVIL : ICN::BUTTON_SMALL_EXIT_GOOD );
        fheroes::Button buttonExit( radarArea.x + 32, radarArea.y + radarArea.height - 37, exitButtonIcnID, 0, 1 );
        buttonExit.setPosition( radarArea.x + ( radarArea.width - buttonExit.area().width ) / 2, buttonExit.area().y );
        fheroes::addGradientShadow( fheroes::AGG::GetICN( exitButtonIcnID, 0 ), display, buttonExit.area().getPosition(), { -5, 5 } );

        const fheroes::Rect & radarRect = radar.GetRect();

        const std::function<fheroes::Rect()> drawControlPanel = [&display, isEvilInterface, isHideInterface, &radarRect, &radarArea, &buttonExit, exitButtonIcnID]() {
            if ( isHideInterface ) {
                Dialog::FrameBorder::RenderRegular( radarRect );
            }

            fheroes::Copy( fheroes::AGG::GetICN( ( isEvilInterface ? ICN::EVIWPUZL : ICN::VIEWPUZL ), 0 ), 0, 0, display, radarArea );
            display.updateNextRenderRoi( radarArea );

            buttonExit.draw();
            fheroes::addGradientShadow( fheroes::AGG::GetICN( exitButtonIcnID, 0 ), display, buttonExit.area().getPosition(), { -5, 5 } );

            return radarRect;
        };

        drawPuzzle( pzl, sf, puzzleArea.x, puzzleArea.y );
        drawControlPanel();

        display.updateNextRenderRoi( border.totalArea() );
        fheroes::fadeInDisplay( border.activeArea(), true );

        const bool earlyExit = revealPuzzle( pzl, sf, puzzleArea.x, puzzleArea.y, buttonExit, isHideInterface ? &drawControlPanel : nullptr );

        LocalEvent & le = LocalEvent::Get();

        while ( le.HandleEvents() && !earlyExit ) {
            buttonExit.drawOnState( le.isMouseLeftButtonPressedAndHeldInArea( buttonExit.area() ) );
            if ( le.MouseClickLeft( buttonExit.area() ) || Game::HotKeyCloseWindow() ) {
                break;
            }

            if ( le.isMouseRightButtonPressedInArea( buttonExit.area() ) ) {
                fheroes::showStandardTextMessage( _( "Exit" ), _( "Exit this menu." ), 0 );
            }
        }

        fheroes::fadeOutDisplay( border.activeArea(), true );

        gameArea.SetUpdateCursor();
    }
}

Puzzle::Puzzle()
{
    Rand::Shuffle( zone1_order );
    Rand::Shuffle( zone2_order );
    Rand::Shuffle( zone3_order );
    Rand::Shuffle( zone4_order );
}

Puzzle & Puzzle::operator=( const char * str )
{
    while ( str && *str ) {
        *this <<= 1;
        if ( *str == 0x31 )
            set( 0 );
        ++str;
    }

    return *this;
}

void Puzzle::Update( uint32_t open_obelisk, uint32_t total_obelisk )
{
    const uint32_t open_puzzle = open_obelisk * numOfPuzzleTiles / total_obelisk;
    size_t need_puzzle = open_puzzle > count() ? open_puzzle - count() : 0;

    if ( need_puzzle && ClosedTilesExists( *this, zone1_order ) )
        ZoneOpenFirstTiles( *this, need_puzzle, zone1_order );

    if ( need_puzzle && ClosedTilesExists( *this, zone2_order ) )
        ZoneOpenFirstTiles( *this, need_puzzle, zone2_order );

    if ( need_puzzle && ClosedTilesExists( *this, zone3_order ) )
        ZoneOpenFirstTiles( *this, need_puzzle, zone3_order );

    if ( need_puzzle && ClosedTilesExists( *this, zone4_order ) )
        ZoneOpenFirstTiles( *this, need_puzzle, zone4_order );
}

void Puzzle::ShowMapsDialog() const
{
    const fheroes::Image & sf = world.GetUltimateArtifact().GetPuzzleMapSurface();
    if ( sf.empty() )
        return;

    const fheroes::Display & display = fheroes::Display::instance();

    // Set the cursor image. After this dialog the Game Area will be shown, so it does not require a cursor restorer.
    Cursor::Get().SetThemes( Cursor::POINTER );

    // restore the original music on exit
    const AudioManager::MusicRestorer musicRestorer;

    AudioManager::PlayMusic( MUS::PUZZLE, Music::PlaybackMode::PLAY_ONCE );

    if ( display.isDefaultSize() && !Settings::Get().isHideInterfaceEnabled() ) {
        ShowStandardDialog( *this, sf );
    }
    else {
        ShowExtendedDialog( *this, sf );
    }
}

OStreamBase & operator<<( OStreamBase & stream, const Puzzle & pzl )
{
    stream << pzl.to_string<char, std::char_traits<char>, std::allocator<char>>();

    stream << static_cast<uint8_t>( pzl.zone1_order.size() );
    for ( const uint8_t tile : pzl.zone1_order ) {
        stream << tile;
    }

    stream << static_cast<uint8_t>( pzl.zone2_order.size() );
    for ( const uint8_t tile : pzl.zone2_order ) {
        stream << tile;
    }

    stream << static_cast<uint8_t>( pzl.zone3_order.size() );
    for ( const uint8_t tile : pzl.zone3_order ) {
        stream << tile;
    }

    stream << static_cast<uint8_t>( pzl.zone4_order.size() );
    for ( const uint8_t tile : pzl.zone4_order ) {
        stream << tile;
    }

    return stream;
}

IStreamBase & operator>>( IStreamBase & stream, Puzzle & pzl )
{
    std::string str;

    stream >> str;
    pzl = str.c_str();

    uint8_t size{ 0 };

    stream >> size;
    pzl.zone1_order.resize( size );
    for ( uint8_t i = 0; i < size; ++i ) {
        stream >> pzl.zone1_order[i];
    }

    stream >> size;
    pzl.zone2_order.resize( size );
    for ( uint8_t i = 0; i < size; ++i ) {
        stream >> pzl.zone2_order[i];
    }

    stream >> size;
    pzl.zone3_order.resize( size );
    for ( uint8_t i = 0; i < size; ++i ) {
        stream >> pzl.zone3_order[i];
    }

    stream >> size;
    pzl.zone4_order.resize( size );
    for ( uint8_t i = 0; i < size; ++i ) {
        stream >> pzl.zone4_order[i];
    }

    return stream;
}
