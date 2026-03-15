/***************************************************************************
 *   fheroes2: https://github.com/ihhub/fheroes2                           *
 *   Copyright (C) 2019 - 2025                                             *
 *                                                                         *
 *   Free Heroes2 Engine: http://sourceforge.net/projects/fheroes2         *
 *   Copyright (C) 2009 by Andrey Afletdinov <fheroes2@gmail.com>          *
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
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "agg.h"
#include "agg_image.h"
#include "audio.h"
#include "audio_manager.h"
#include "cursor.h"
#include "dialog.h"
#include "dialog_selectscenario.h"
#include "difficulty.h"
#include "game.h" // IWYU pragma: associated
#include "game_hotkeys.h"
#include "game_interface.h"
#include "game_mainmenu_ui.h"
#include "game_mode.h"
#include "icn.h"
#include "image.h"
#include "localevent.h"
#include "logging.h"
#include "maps_fileinfo.h"
#include "math_base.h"
#include "math_tools.h"
#include "mus.h"
#include "player_info.h"
#include "players.h"
#include "screen.h"
#include "settings.h"
#include "system.h"
#include "tools.h"
#include "translations.h"
#include "ui_button.h"
#include "ui_dialog.h"
#include "ui_text.h"
#include "ui_tool.h"
#include "ui_window.h"
#include "world.h"

namespace
{
    void outputNewGameInTextSupportMode()
    {
        START_TEXT_SUPPORT_MODE
        COUT( "Select Map for New Game\n" )

        COUT( "Press " << Game::getHotKeyNameByEventId( Game::HotKeyEvent::MAIN_MENU_SELECT_MAP ) << " to select a map." )
        COUT( "Press " << Game::getHotKeyNameByEventId( Game::HotKeyEvent::DEFAULT_CANCEL ) << " to close the dialog and return to the Main Menu." )
        COUT( "Press " << Game::getHotKeyNameByEventId( Game::HotKeyEvent::DEFAULT_OKAY ) << " to start the chosen map." )
    }

    void showCurrentlySelectedMapInfoInTextSupportMode( const Maps::FileInfo & mapInfo )
    {
        START_TEXT_SUPPORT_MODE
        COUT( "Currently selected map:\n" )
        COUT( mapInfo.getSummary() )
    }

    void updatePlayers( Players & players, const int humanPlayerCount )
    {
        if ( humanPlayerCount < 2 )
            return;

        int foundHumans = 0;

        for ( size_t i = 0; i < players.size(); ++i ) {
            if ( players[i]->isControlHuman() ) {
                ++foundHumans;
                if ( players[i]->isControlAI() )
                    players[i]->SetControl( CONTROL_HUMAN );
            }

            if ( foundHumans == humanPlayerCount )
                break;
        }
    }

    void DrawScenarioStaticInfo( const fheroes2::Rect & rt )
    {
        fheroes2::Display & display = fheroes2::Display::instance();

        const fheroes2::FontType normalWhiteFont = fheroes2::FontType::normalWhite();

        // text scenario
        fheroes2::Text text( _( "Scenario:" ), normalWhiteFont );
        text.draw( rt.x, rt.y + 9, rt.width, display );

        // text game difficulty
        text.set( _( "Game Difficulty:" ), normalWhiteFont );
        text.draw( rt.x, rt.y + 59, rt.width, display );

        // text opponents
        text.set( _( "Opponents:" ), normalWhiteFont );
        text.draw( rt.x, rt.y + 164, rt.width, display );

        // text class
        text.set( _( "Class:" ), normalWhiteFont );
        text.draw( rt.x, rt.y + 248, rt.width, display );
    }

    void RedrawMapTitle( const Settings & conf, const fheroes2::Rect & maxRoi, const fheroes2::Rect & centeredRoi )
    {
        const auto & info = conf.getCurrentMapInfo();
        fheroes2::Text text{ info.name, fheroes2::FontType::normalWhite(), info.getSupportedLanguage() };

        if ( maxRoi.width <= 0 ) {
            return;
        }

        if ( centeredRoi.width > 0 && text.width() > centeredRoi.width ) {
            text.fitToOneRow( maxRoi.width );
            text.draw( maxRoi.x + ( maxRoi.width - text.width() ), maxRoi.y + 3, text.width(), fheroes2::Display::instance() );
        }
        else if ( centeredRoi.width > 0 ) {
            text.draw( centeredRoi.x, centeredRoi.y + 3, centeredRoi.width, fheroes2::Display::instance() );
        }
        else {
            text.fitToOneRow( maxRoi.width );
            text.draw( maxRoi.x, maxRoi.y + 3, maxRoi.width, fheroes2::Display::instance() );
        }
    }

    void RedrawDifficultyInfo( const fheroes2::Point & dst )
    {
        const int32_t width = 77;
        const int32_t height = 69;

        for ( int32_t current = Difficulty::EASY; current <= Difficulty::IMPOSSIBLE; ++current ) {
            const int32_t offset = width * current;
            int32_t normalSpecificOffset = 0;
            // Add offset shift because the original difficulty icons have irregular spacing.
            if ( current == Difficulty::NORMAL ) {
                normalSpecificOffset = 1;
            }

            const fheroes2::Text text( Difficulty::String( current ), fheroes2::FontType::smallWhite() );
            text.draw( dst.x + 31 + offset + normalSpecificOffset - ( text.width() / 2 ), dst.y + height, fheroes2::Display::instance() );
        }
    }

    fheroes2::Rect RedrawRatingInfo( const fheroes2::Point & offset, int32_t width_ )
    {
        std::string str( _( "Rating %{rating}%" ) );
        StringReplace( str, "%{rating}", Game::GetRating() );

        const fheroes2::Text text( str, fheroes2::FontType::normalWhite() );
        const int32_t y = offset.y + 372;
        text.draw( offset.x, y, width_, fheroes2::Display::instance() );

        const int32_t textX = ( width_ > text.width() ) ? offset.x + ( width_ - text.width() ) / 2 : 0;

        return { textX, y, text.width(), text.height() };
    }

    // HoMM1-specific scenario selection screen.
    // Uses NEWGAME.BMP as the right panel background, NEWGAME.ICN for UI elements,
    // and BTNNEWGM.ICN for OKAY/CANCEL buttons.
    //
    // NEWGAME.ICN frame layout (29 frames):
    //   0       : 251×20  scenario name bar
    //   1-4     : 65×65   chess pieces (Easy, Normal, Hard, Expert) — unselected
    //   5-8     : 65×65   chess pieces (Easy, Normal, Hard, Expert) — selected (brighter)
    //   9       : 65×65   extra variant
    //   10-13   : 59×44   balance scales (Fool, Easy, Average, Hard) — AI strength icons
    //   14-17   : 59×44   balance scales — alternate states
    //   18      : 249×20  scenario list row bar
    //   19      : 24×20   scroll arrow up
    //   20      : 24×20   scroll arrow down
    //   21-22   : 96×25   small buttons (unused here)
    //   23-24   : 96×25   small buttons (unused here)
    //   25      : 71×71   color gem / king-of-hill indicator
    //   26      : 65×65   extra large icon
    //   27-28   : 44×39   small icons
    //
    // BTNNEWGM.ICN frame layout (8 frames, 127×63 each):
    //   0/1     : OKAY released / pressed
    //   2/3     : CANCEL released / pressed
    // Player colors available for selection in HoMM1 (index 0-3 cycles Red→Yellow→Green→Blue).
    struct H1ColorInfo
    {
        const char * name;
        uint8_t r, g, b;
    };
    static constexpr H1ColorInfo h1Colors[] = {
        { "Red",    200,  30,  30 },
        { "Yellow", 220, 200,  20 },
        { "Green",   30, 180,  30 },
        { "Blue",    30,  80, 220 },
    };

    fheroes2::GameMode ChooseHoMM1Map( MapsFileInfoList & maps, const int /*humanPlayerCount*/ )
    {
        assert( !maps.empty() );

        fheroes2::Display & display = fheroes2::Display::instance();

        // Draw the HEROES.BMP background only — NOT H1PANEL, which contains the main menu
        // Standard/Campaign buttons that would bleed over our scenario panel.
        {
            const fheroes2::Sprite & bg = fheroes2::AGG::GetICN( ICN::HEROES, 0 );
            display.fill( 0 );
            if ( !bg.empty() ) {
                const int32_t bgX = ( display.width() - bg.width() ) / 2;
                const int32_t bgY = ( display.height() - bg.height() ) / 2;
                fheroes2::Copy( bg, 0, 0, display, bgX, bgY, bg.width(), bg.height() );
            }
        }

        // Right panel: NEWGAME.BMP (322×459) — contains all static art including pre-drawn
        // chess pieces, balance scales, color gem slot, and king-of-hill area.
        const fheroes2::Sprite & panel = fheroes2::AGG::GetICN( ICN::H1NEWGAME_BMP, 0 );
        const int32_t panelW = panel.empty() ? 322 : panel.width();
        const int32_t panelH = panel.empty() ? 459 : panel.height();
        const int32_t panelX = display.width() - panelW;
        const int32_t panelY = ( display.height() - panelH ) / 2;

        // Default selection: AES31000.MAP (first scenario in HoMM1).
        int selectedMap = 0;
        for ( int i = 0; i < static_cast<int>( maps.size() ); ++i ) {
            const std::string base = StringLower( System::GetFileName( maps[i].filename ) );
            if ( base == "aes31000.map" ) {
                selectedMap = i;
                break;
            }
        }
        int scrollOffset = selectedMap;
        int difficulty   = 1; // 0=Easy, 1=Normal, 2=Hard, 3=Expert
        int aiStrength   = 2; // 0=Fool, 1=Easy, 2=Average, 3=Hard
        int colorIndex   = 0; // cycles Red→Yellow→Green→Blue
        bool kingOfHill  = false;

        // --- Difficulty chess pieces — NEWGAME.BMP has them pre-drawn; we only add the selection border ---
        constexpr int32_t chessW       = 65;
        constexpr int32_t chessSpacing = 6;
        constexpr int32_t chessRowW    = 4 * chessW + 3 * chessSpacing;
        const int32_t chessStartX = panelX + ( panelW - chessRowW ) / 2;
        const int32_t chessY      = panelY + 48;

        // --- AI balance scale label positions (scales are pre-rendered in the BMP) ---
        constexpr int32_t scaleW       = 59;
        constexpr int32_t scaleH       = 44;
        constexpr int32_t scaleSpacing = 6;
        constexpr int32_t nOpponents   = 3;
        constexpr int32_t scaleRowW    = nOpponents * scaleW + ( nOpponents - 1 ) * scaleSpacing;
        const int32_t scaleStartX = panelX + ( panelW - scaleRowW ) / 2;
        const int32_t scaleY      = panelY + 148;

        // --- Color gem and King-of-Hill click areas ---
        constexpr int32_t gemW = 50;
        constexpr int32_t gemH = 50;
        const int32_t gemX = panelX + 40;
        const int32_t gemY = panelY + 228;
        const fheroes2::Rect gemRect{ gemX, gemY, gemW, gemH };

        constexpr int32_t kohW = 50;
        constexpr int32_t kohH = 50;
        const int32_t kohX = panelX + 168;
        const int32_t kohY = panelY + 228;
        const fheroes2::Rect kohRect{ kohX, kohY, kohW, kohH };

        // --- OKAY / CANCEL buttons (BTNNEWGM.ICN frames 0/1 and 2/3, 127×63) ---
        constexpr int32_t btnW = 127;
        constexpr int32_t btnH = 63;
        const int32_t btnY       = panelY + panelH - btnH - 10;
        const int32_t btnOkX     = panelX + panelW / 2 - btnW - 5;
        const int32_t btnCancelX = panelX + panelW / 2 + 5;

        fheroes2::Button btnOk    ( btnOkX,     btnY, ICN::H1BTNNEWGM, 0, 1 );
        fheroes2::Button btnCancel( btnCancelX, btnY, ICN::H1BTNNEWGM, 2, 3 );

        // --- Scenario list area ---
        constexpr int32_t arrowW    = 24;
        constexpr int32_t arrowH    = 20;
        constexpr int32_t itemH     = 16;
        constexpr int32_t listMarginX = 8;
        const int32_t listAreaX   = panelX + listMarginX;
        const int32_t listAreaY   = panelY + 310;
        const int32_t listAreaW   = panelW - listMarginX * 2 - arrowW - 2;
        const int32_t listAreaH   = btnY - listAreaY - 4;
        const int32_t visibleItems = std::max( 1, listAreaH / itemH );
        const int32_t arrowX      = listAreaX + listAreaW + 2;

        const fheroes2::Rect arrowUpRect  { arrowX, listAreaY,          arrowW, arrowH };
        const fheroes2::Rect arrowDownRect{ arrowX, listAreaY + arrowH, arrowW, arrowH };

        static const char * aiLabels[]   = { "Fool", "Easy", "Average", "Hard" };
        static const int    diffValues[] = { Difficulty::EASY, Difficulty::NORMAL, Difficulty::HARD, Difficulty::EXPERT };

        fheroes2::Rect chessRects[4];
        for ( int i = 0; i < 4; ++i )
            chessRects[i] = { chessStartX + i * ( chessW + chessSpacing ), chessY, chessW, chessW };

        fheroes2::Rect scaleRects[nOpponents];
        for ( int i = 0; i < nOpponents; ++i )
            scaleRects[i] = { scaleStartX + i * ( scaleW + scaleSpacing ), scaleY, scaleW, scaleH };

        const auto redrawAll = [&]() {
            // Repaint panel background (restores pre-drawn chess pieces, scales, gem, etc.).
            if ( !panel.empty() )
                fheroes2::Blit( panel, display, panelX, panelY );

            // --- Difficulty: NEWGAME.BMP has chess pieces pre-drawn; only add red border on selected ---
            for ( int i = 0; i < 4; ++i ) {
                if ( i == difficulty ) {
                    const int32_t cx = chessStartX + i * ( chessW + chessSpacing );
                    fheroes2::DrawRect( display, { cx - 2, chessY - 2, chessW + 4, chessW + 4 }, fheroes2::GetColorId( 200, 20, 20 ) );
                }
            }

            // --- AI strength (scales pre-drawn in BMP; draw label below each scale) ---
            for ( int i = 0; i < nOpponents; ++i ) {
                const int32_t sx = scaleStartX + i * ( scaleW + scaleSpacing );
                fheroes2::Text lbl( aiLabels[aiStrength], fheroes2::FontType::smallWhite() );
                lbl.draw( sx + ( scaleW - lbl.width() ) / 2, scaleY + scaleH + 4, display );
            }

            // --- Color gem: overlay a colored filled square over the pre-drawn gem slot ---
            {
                const H1ColorInfo & col = h1Colors[colorIndex];
                const uint8_t colId = fheroes2::GetColorId( col.r, col.g, col.b );
                fheroes2::Fill( display, gemX + 5, gemY + 5, gemW - 10, gemH - 10, colId );
                fheroes2::Text colLbl( col.name, fheroes2::FontType::smallWhite() );
                colLbl.draw( gemX + ( gemW - colLbl.width() ) / 2, gemY + gemH + 2, display );
            }

            // --- King of Hill: draw a colored indicator box ---
            {
                const uint8_t kohColorId = kingOfHill
                    ? fheroes2::GetColorId( h1Colors[colorIndex].r, h1Colors[colorIndex].g, h1Colors[colorIndex].b )
                    : fheroes2::GetColorId( 80, 80, 80 );
                fheroes2::Fill( display, kohX + 5, kohY + 5, kohW - 10, kohH - 10, kohColorId );
            }

            // --- Scenario list ---
            // Draw a dark background for the list area.
            fheroes2::Fill( display, listAreaX, listAreaY, listAreaW, listAreaH, fheroes2::GetColorId( 20, 20, 40 ) );

            const int mapCount = static_cast<int>( maps.size() );
            for ( int row = 0; row < visibleItems; ++row ) {
                const int mapIdx = scrollOffset + row;
                if ( mapIdx >= mapCount )
                    break;
                const int32_t rowY = listAreaY + row * itemH;
                // Highlight selected row.
                if ( mapIdx == selectedMap ) {
                    fheroes2::Fill( display, listAreaX, rowY, listAreaW, itemH, fheroes2::GetColorId( 60, 60, 120 ) );
                }
                fheroes2::Text rowText( maps[mapIdx].name, fheroes2::FontType::smallWhite() );
                rowText.fitToOneRow( listAreaW - 2 );
                rowText.draw( listAreaX + 2, rowY + 2, display );
            }

            // Scroll arrows.
            const fheroes2::Sprite & arrowUp   = fheroes2::AGG::GetICN( ICN::H1NEWGAME_ICN, 19 );
            const fheroes2::Sprite & arrowDown = fheroes2::AGG::GetICN( ICN::H1NEWGAME_ICN, 20 );
            if ( !arrowUp.empty() )
                fheroes2::Blit( arrowUp,   display, arrowX, listAreaY );
            if ( !arrowDown.empty() )
                fheroes2::Blit( arrowDown, display, arrowX, listAreaY + arrowH );

            btnOk.draw();
            btnCancel.draw();
        };

        redrawAll();
        fheroes2::validateFadeInAndRender();

        Settings & conf = Settings::Get();
        LocalEvent & le = LocalEvent::Get();

        while ( le.HandleEvents() ) {
            btnOk.drawOnState( le.isMouseLeftButtonPressedAndHeldInArea( btnOk.area() ) );
            btnCancel.drawOnState( le.isMouseLeftButtonPressedAndHeldInArea( btnCancel.area() ) );

            bool changed = false;

            // Difficulty: click a chess piece position to select.
            for ( int i = 0; i < 4; ++i ) {
                if ( le.MouseClickLeft( chessRects[i] ) && difficulty != i ) {
                    difficulty = i;
                    changed = true;
                }
            }

            // AI strength: click any scale area to cycle.
            for ( int i = 0; i < nOpponents; ++i ) {
                if ( le.MouseClickLeft( scaleRects[i] ) ) {
                    aiStrength = ( aiStrength + 1 ) % 4;
                    changed = true;
                }
            }

            // Color gem: cycle through Red→Yellow→Green→Blue.
            if ( le.MouseClickLeft( gemRect ) ) {
                colorIndex = ( colorIndex + 1 ) % 4;
                changed = true;
            }

            // King of Hill: toggle.
            if ( le.MouseClickLeft( kohRect ) ) {
                kingOfHill = !kingOfHill;
                changed = true;
            }

            // Scenario list: click a row to select.
            for ( int row = 0; row < visibleItems; ++row ) {
                const int mapIdx = scrollOffset + row;
                if ( mapIdx >= static_cast<int>( maps.size() ) )
                    break;
                const fheroes2::Rect rowRect{ listAreaX, listAreaY + row * itemH, listAreaW, itemH };
                if ( le.MouseClickLeft( rowRect ) && selectedMap != mapIdx ) {
                    selectedMap = mapIdx;
                    changed = true;
                }
            }

            // Scroll arrows.
            if ( le.MouseClickLeft( arrowUpRect ) && scrollOffset > 0 ) {
                --scrollOffset;
                changed = true;
            }
            if ( le.MouseClickLeft( arrowDownRect )
                 && scrollOffset + visibleItems < static_cast<int>( maps.size() ) ) {
                ++scrollOffset;
                changed = true;
            }

            if ( changed ) {
                redrawAll();
                display.render();
            }

            if ( le.MouseClickLeft( btnOk.area() ) || Game::HotKeyPressEvent( Game::HotKeyEvent::DEFAULT_OKAY ) ) {
                conf.setCurrentMapInfo( maps[selectedMap] );
                conf.SetGameType( Game::TYPE_STANDARD );
                conf.SetGameDifficulty( diffValues[difficulty] );
                conf.GetPlayers().SetStartGame();
                return fheroes2::GameMode::START_GAME;
            }

            if ( le.MouseClickLeft( btnCancel.area() ) || Game::HotKeyPressEvent( Game::HotKeyEvent::DEFAULT_CANCEL ) ) {
                return fheroes2::GameMode::MAIN_MENU;
            }
        }

        return fheroes2::GameMode::MAIN_MENU;
    }

    fheroes2::GameMode ChooseNewMap( MapsFileInfoList & lists, const int humanPlayerCount )
    {
        assert( !lists.empty() );

        // setup cursor
        const CursorRestorer cursorRestorer( true, Cursor::POINTER );

        fheroes2::Display & display = fheroes2::Display::instance();

        Settings & conf = Settings::Get();
        const bool isEvilInterface = conf.isEvilInterfaceEnabled();

        fheroes2::drawMainMenuScreen();

        fheroes2::StandardWindow background( 388, 397, true, display );

        const fheroes2::Rect roi( background.activeArea() );

        const fheroes2::Point pointDifficultyInfo( roi.x + 8, roi.y + 79 );
        const fheroes2::Point pointOpponentInfo( roi.x + 8, roi.y + 181 );
        const fheroes2::Point pointClassInfo( roi.x + 8, roi.y + 265 );

        const fheroes2::Sprite & scenarioBox = fheroes2::AGG::GetICN( isEvilInterface ? ICN::METALLIC_BORDERED_TEXTBOX_EVIL : ICN::METALLIC_BORDERED_TEXTBOX_GOOD, 0 );

        // If the ICN is missing (e.g. HoMM1 data), fall back to a plain rect spanning the dialog.
        const fheroes2::Rect scenarioBoxRoi = ( scenarioBox.width() > 0 )
            ? fheroes2::Rect( roi.x + ( roi.width - scenarioBox.width() ) / 2, roi.y + 24, scenarioBox.width(), scenarioBox.height() )
            : fheroes2::Rect( roi.x + 4, roi.y + 24, roi.width - 8, 25 );

        if ( scenarioBox.width() > 0 ) {
            fheroes2::Copy( scenarioBox, 0, 0, display, scenarioBoxRoi );
            fheroes2::addGradientShadow( scenarioBox, display, scenarioBoxRoi.getPosition(), { -5, 5 } );
        }

        const fheroes2::Sprite & difficultyCursor = fheroes2::AGG::GetICN( ICN::NGEXTRA, 62 );

        const int32_t difficultyCursorWidth = difficultyCursor.width();
        const int32_t difficultyCursorHeight = difficultyCursor.height();

        // Difficulty selection areas vector.
        std::vector<fheroes2::Rect> coordDifficulty;
        coordDifficulty.reserve( 5 );

        coordDifficulty.emplace_back( roi.x + 8, roi.y + 78, difficultyCursorWidth, difficultyCursorHeight );
        coordDifficulty.emplace_back( roi.x + 85, roi.y + 78, difficultyCursorWidth, difficultyCursorHeight );
        coordDifficulty.emplace_back( roi.x + 161, roi.y + 78, difficultyCursorWidth, difficultyCursorHeight );
        coordDifficulty.emplace_back( roi.x + 238, roi.y + 78, difficultyCursorWidth, difficultyCursorHeight );
        coordDifficulty.emplace_back( roi.x + 315, roi.y + 78, difficultyCursorWidth, difficultyCursorHeight );

        const int32_t buttonSelectWidth = fheroes2::AGG::GetICN( ICN::BUTTON_MAP_SELECT_GOOD, 0 ).width();

        fheroes2::Button buttonSelectMaps( scenarioBoxRoi.x + scenarioBoxRoi.width - 6 - buttonSelectWidth, scenarioBoxRoi.y + 5,
                                           isEvilInterface ? ICN::BUTTON_MAP_SELECT_EVIL : ICN::BUTTON_MAP_SELECT_GOOD, 0, 1 );
        buttonSelectMaps.draw();

        fheroes2::Button buttonOk;
        fheroes2::Button buttonCancel;

        const fheroes2::Point buttonOffset( 20, 6 );

        const int buttonOkIcn = isEvilInterface ? ICN::BUTTON_SMALL_OKAY_EVIL : ICN::BUTTON_SMALL_OKAY_GOOD;
        background.renderButton( buttonOk, buttonOkIcn, 0, 1, buttonOffset, fheroes2::StandardWindow::Padding::BOTTOM_LEFT );

        const int buttonCancelIcn = isEvilInterface ? ICN::BUTTON_SMALL_CANCEL_EVIL : ICN::BUTTON_SMALL_CANCEL_GOOD;
        background.renderButton( buttonCancel, buttonCancelIcn, 0, 1, buttonOffset, fheroes2::StandardWindow::Padding::BOTTOM_RIGHT );

        const Maps::FileInfo & mapInfo = [&lists, &conf = std::as_const( conf )]() {
            const Maps::FileInfo & currentMapinfo = conf.getCurrentMapInfo();

            if ( currentMapinfo.filename.empty() ) {
                return lists.front();
            }

            // Make sure that the current map actually exists in the map's list
            const auto iter = std::find_if( lists.begin(), lists.end(), [&currentMapinfo]( const Maps::FileInfo & info ) {
                return info.name == currentMapinfo.name && info.filename == currentMapinfo.filename;
            } );
            if ( iter == lists.end() ) {
                return lists.front();
            }

            return *iter;
        }();

        Players & players = conf.GetPlayers();

        showCurrentlySelectedMapInfoInTextSupportMode( mapInfo );
        conf.setCurrentMapInfo( mapInfo );
        updatePlayers( players, humanPlayerCount );

        // Load players parameters saved from the previous call of the scenario info dialog.
        Game::LoadPlayers( mapInfo.filename, players );

        Interface::PlayersInfo playersInfo;
        playersInfo.UpdateInfo( players, pointOpponentInfo, pointClassInfo );

        DrawScenarioStaticInfo( roi );
        RedrawDifficultyInfo( pointDifficultyInfo );

        const int icnIndex = isEvilInterface ? 1 : 0;

        // Draw difficulty icons.
        for ( int i = 0; i < 5; ++i ) {
            const fheroes2::Sprite & icon = fheroes2::AGG::GetICN( ICN::DIFFICULTY_ICON_EASY + i, icnIndex );
            fheroes2::Copy( icon, 0, 0, display, coordDifficulty[i] );
            fheroes2::addGradientShadow( icon, display, { coordDifficulty[i].x, coordDifficulty[i].y }, { -5, 5 } );
        }

        // We calculate the allowed text width according to the select button's width while ensuring symmetric placement of the map title.
        const int32_t boxBorder = 6;
        const int32_t overallBoxTextAreaWidth = ( scenarioBoxRoi.width - ( 2 * boxBorder ) );
        const int32_t maxTextAreaWidth = std::max( 1, overallBoxTextAreaWidth - buttonSelectWidth );

        const fheroes2::Rect maxTextRoi{ scenarioBoxRoi.x + boxBorder, scenarioBoxRoi.y + 5, maxTextAreaWidth, 19 };

        const int32_t halfBoxTextAreaWidth = overallBoxTextAreaWidth / 2;
        const int32_t rightSideAvailableTextWidth
            = ( halfBoxTextAreaWidth > buttonSelectWidth ) ? ( halfBoxTextAreaWidth - buttonSelectWidth ) : ( buttonSelectWidth - halfBoxTextAreaWidth );

        const fheroes2::Rect centeredTextRoi{ scenarioBoxRoi.x + boxBorder + buttonSelectWidth, scenarioBoxRoi.y + 5, 2 * rightSideAvailableTextWidth, 19 };

        // Set up restorers.
        fheroes2::ImageRestorer mapTitleArea( display, maxTextRoi.x, maxTextRoi.y, maxTextRoi.width, maxTextRoi.height );
        fheroes2::ImageRestorer opponentsArea( display, roi.x, pointOpponentInfo.y, roi.width, 65 );
        fheroes2::ImageRestorer classArea( display, roi.x, pointClassInfo.y, roi.width, 69 );
        fheroes2::ImageRestorer handicapArea( display, roi.x, pointClassInfo.y + 69, roi.width, 31 );
        fheroes2::ImageRestorer ratingArea( display, buttonOk.area().x + buttonOk.area().width, buttonOk.area().y,
                                            roi.width - buttonOk.area().width - buttonCancel.area().width - 20 * 2, buttonOk.area().height );

        // Map name
        RedrawMapTitle( conf, maxTextRoi, centeredTextRoi );

        playersInfo.RedrawInfo( false );

        fheroes2::Rect ratingRoi = RedrawRatingInfo( roi.getPosition(), roi.width );

        fheroes2::MovableSprite levelCursor( difficultyCursor );
        const int32_t levelCursorOffset = 3;

        switch ( Game::getDifficulty() ) {
        case Difficulty::EASY:
            levelCursor.setPosition( coordDifficulty[0].x - levelCursorOffset, coordDifficulty[0].y - levelCursorOffset );
            break;
        case Difficulty::NORMAL:
            levelCursor.setPosition( coordDifficulty[1].x - levelCursorOffset, coordDifficulty[1].y - levelCursorOffset );
            break;
        case Difficulty::HARD:
            levelCursor.setPosition( coordDifficulty[2].x - levelCursorOffset, coordDifficulty[2].y - levelCursorOffset );
            break;
        case Difficulty::EXPERT:
            levelCursor.setPosition( coordDifficulty[3].x - levelCursorOffset, coordDifficulty[3].y - levelCursorOffset );
            break;
        case Difficulty::IMPOSSIBLE:
            levelCursor.setPosition( coordDifficulty[4].x - levelCursorOffset, coordDifficulty[4].y - levelCursorOffset );
            break;
        default:
            // Did you add a new difficulty mode? Add the corresponding case above!
            assert( 0 );
            break;
        }

        levelCursor.redraw();

        fheroes2::validateFadeInAndRender();

        fheroes2::GameMode result = fheroes2::GameMode::QUIT_GAME;

        outputNewGameInTextSupportMode();

        LocalEvent & le = LocalEvent::Get();

        while ( true ) {
            if ( !le.HandleEvents( true, true ) ) {
                if ( Interface::AdventureMap::EventExit() == fheroes2::GameMode::QUIT_GAME ) {
                    fheroes2::fadeOutDisplay();

                    return fheroes2::GameMode::QUIT_GAME;
                }

                continue;
            }

            // press button
            buttonSelectMaps.drawOnState( le.isMouseLeftButtonPressedAndHeldInArea( buttonSelectMaps.area() ) );
            buttonOk.drawOnState( le.isMouseLeftButtonPressedAndHeldInArea( buttonOk.area() ) );
            buttonCancel.drawOnState( le.isMouseLeftButtonPressedAndHeldInArea( buttonCancel.area() ) );

            // click select
            if ( HotKeyPressEvent( Game::HotKeyEvent::MAIN_MENU_SELECT_MAP ) || le.MouseClickLeft( buttonSelectMaps.area() ) ) {
                const Maps::FileInfo * fi = Dialog::SelectScenario( lists, false );
                if ( lists.empty() ) {
                    // This can happen if all maps have been deleted.
                    result = fheroes2::GameMode::MAIN_MENU;
                    break;
                }

                // The previous dialog might still have a pressed button event. We have to clean the state.
                le.reset();

                if ( fi && fi->filename != conf.getCurrentMapInfo().filename ) {
                    showCurrentlySelectedMapInfoInTextSupportMode( *fi );

                    // The map is changed. Update the map data and do default initialization of players.
                    conf.setCurrentMapInfo( *fi );

                    mapTitleArea.restore();
                    RedrawMapTitle( conf, maxTextRoi, centeredTextRoi );

                    opponentsArea.restore();
                    classArea.restore();
                    handicapArea.restore();
                    ratingArea.restore();

                    updatePlayers( players, humanPlayerCount );
                    playersInfo.UpdateInfo( players, pointOpponentInfo, pointClassInfo );

                    playersInfo.resetSelection();
                    playersInfo.RedrawInfo( false );

                    ratingRoi = RedrawRatingInfo( roi.getPosition(), roi.width );
                    levelCursor.setPosition( coordDifficulty[Game::getDifficulty()].x - levelCursorOffset,
                                             coordDifficulty[Game::getDifficulty()].y - levelCursorOffset ); // From 0 to 4, see: Difficulty enum
                }
                display.render();

                outputNewGameInTextSupportMode();
            }
            else if ( Game::HotKeyPressEvent( Game::HotKeyEvent::DEFAULT_CANCEL ) || le.MouseClickLeft( buttonCancel.area() ) ) {
                result = fheroes2::GameMode::MAIN_MENU;
                break;
            }

            if ( Game::HotKeyPressEvent( Game::HotKeyEvent::DEFAULT_OKAY ) || le.MouseClickLeft( buttonOk.area() ) ) {
                DEBUG_LOG( DBG_GAME, DBG_INFO, "select maps: " << conf.getCurrentMapInfo().filename << ", difficulty: " << Difficulty::String( Game::getDifficulty() ) )
                result = fheroes2::GameMode::START_GAME;

                // Fade-out screen before starting a scenario.
                fheroes2::fadeOutDisplay();
                break;
            }

            if ( le.MouseClickLeft( roi ) ) {
                const int32_t index = GetRectIndex( coordDifficulty, le.getMouseCursorPos() );

                // select difficulty
                if ( 0 <= index ) {
                    levelCursor.setPosition( coordDifficulty[index].x - levelCursorOffset, coordDifficulty[index].y - levelCursorOffset );
                    levelCursor.redraw();
                    Game::saveDifficulty( index );
                    ratingArea.restore();
                    ratingRoi = RedrawRatingInfo( roi.getPosition(), roi.width );

                    display.render( roi );
                }
                // playersInfo
                else if ( playersInfo.QueueEventProcessing() ) {
                    opponentsArea.restore();
                    classArea.restore();
                    handicapArea.restore();
                    playersInfo.RedrawInfo( false );

                    display.render( roi );
                }
            }
            else if ( ( le.isMouseWheelUp() || le.isMouseWheelDown() ) && playersInfo.QueueEventProcessing() ) {
                playersInfo.resetSelection();
                opponentsArea.restore();
                classArea.restore();
                handicapArea.restore();
                playersInfo.RedrawInfo( false );

                display.render( roi );
            }

            if ( le.isMouseRightButtonPressedInArea( roi ) ) {
                if ( le.isMouseRightButtonPressedInArea( buttonSelectMaps.area() ) ) {
                    fheroes2::showStandardTextMessage( _( "Scenario" ), _( "Click here to select which scenario to play." ), Dialog::ZERO );
                }
                else if ( 0 <= GetRectIndex( coordDifficulty, le.getMouseCursorPos() ) ) {
                    fheroes2::showStandardTextMessage(
                        _( "Game Difficulty" ),
                        _( "This lets you change the starting difficulty at which you will play. Higher difficulty levels start you off with fewer resources, and at the higher settings, give extra resources to the computer." ),
                        Dialog::ZERO );
                }
                else if ( le.isMouseRightButtonPressedInArea( ratingRoi ) ) {
                    fheroes2::showStandardTextMessage(
                        _( "Difficulty Rating" ),
                        _( "The difficulty rating reflects a combination of various settings for your game. This number will be applied to your final score." ),
                        Dialog::ZERO );
                }
                else if ( le.isMouseRightButtonPressedInArea( buttonOk.area() ) ) {
                    fheroes2::showStandardTextMessage( _( "Okay" ), _( "Click to accept these settings and start a new game." ), Dialog::ZERO );
                }
                else if ( le.isMouseRightButtonPressedInArea( buttonCancel.area() ) ) {
                    fheroes2::showStandardTextMessage( _( "Cancel" ), _( "Click to return to the main menu." ), Dialog::ZERO );
                }
                else {
                    playersInfo.QueueEventProcessing();
                }
            }
        }

        // Save the changes players parameters before closing this dialog.
        Game::SavePlayers( conf.getCurrentMapInfo().filename, players );

        return result;
    }

    fheroes2::GameMode LoadNewMap()
    {
        Settings & conf = Settings::Get();

        conf.GetPlayers().SetStartGame();

        const Maps::FileInfo & mapInfo = conf.getCurrentMapInfo();

        if ( mapInfo.version == GameVersion::SUCCESSION_WARS || mapInfo.version == GameVersion::PRICE_OF_LOYALTY ) {
            if ( world.LoadMapMP2( mapInfo.filename, ( mapInfo.version == GameVersion::SUCCESSION_WARS ) ) ) {
                return fheroes2::GameMode::START_GAME;
            }

            fheroes2::drawMainMenuScreen();
            fheroes2::showStandardTextMessage( _( "Warning" ), _( "The map is corrupted or doesn't exist." ), Dialog::OK );
            return fheroes2::GameMode::MAIN_MENU;
        }

        if ( mapInfo.version == GameVersion::HOMM1 ) {
            // Strip "[AGG]" prefix to get the AGG-internal filename
            const std::string aggKey = mapInfo.filename.substr( 5 ); // remove "[AGG]"
            if ( world.loadHoMM1Map( AGG::getDataFromAggFile( aggKey, false ) ) ) {
                return fheroes2::GameMode::START_GAME;
            }

            fheroes2::drawMainMenuScreen();
            fheroes2::showStandardTextMessage( _( "Warning" ), _( "The map is corrupted or doesn't exist." ), Dialog::OK );
            return fheroes2::GameMode::MAIN_MENU;
        }

        assert( mapInfo.version == GameVersion::RESURRECTION );
        if ( world.loadResurrectionMap( mapInfo.filename ) ) {
            return fheroes2::GameMode::START_GAME;
        }

        fheroes2::drawMainMenuScreen();
        fheroes2::showStandardTextMessage( _( "Warning" ), _( "The map is corrupted or doesn't exist." ), Dialog::OK );
        return fheroes2::GameMode::MAIN_MENU;
    }
}

fheroes2::GameMode Game::SelectScenario( const uint8_t humanPlayerCount )
{
    assert( humanPlayerCount >= 1 && humanPlayerCount <= 6 );

    AudioManager::PlayMusicAsync( MUS::MAINMENU, Music::PlaybackMode::RESUME_AND_PLAY_INFINITE );

    MapsFileInfoList maps = Maps::getAllMapFileInfos( false, humanPlayerCount );
    if ( maps.empty() ) {
        fheroes2::showStandardTextMessage( _( "Warning" ), _( "No maps available!" ), Dialog::OK );
        return fheroes2::GameMode::MAIN_MENU;
    }

    // Use HoMM1-specific scenario selection if all maps are HoMM1 format.
    const bool allHoMM1 = std::all_of( maps.begin(), maps.end(), []( const Maps::FileInfo & fi ) {
        return fi.version == GameVersion::HOMM1;
    } );

    // We must release UI resources for this window before loading a new map. That's why all UI logic is in a separate function.
    const fheroes2::GameMode result = allHoMM1 ? ChooseHoMM1Map( maps, humanPlayerCount ) : ChooseNewMap( maps, humanPlayerCount );
    if ( result != fheroes2::GameMode::START_GAME ) {
        return result;
    }

    return LoadNewMap();
}

int32_t Game::GetStep4Player( const int32_t currentId, const int32_t width, const int32_t totalCount )
{
    return currentId * width * maxNumOfPlayers / totalCount + ( width * ( maxNumOfPlayers - totalCount ) / ( 2 * totalCount ) );
}
