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

#include "dialog_interface_settings.h"

#include <cstdint>
#include <string>
#include <utility>

#include "agg_image.h"
#include "cursor.h"
#include "game_hotkeys.h"
#include "icn.h"
#include "image.h"
#include "localevent.h"
#include "math_base.h"
#include "screen.h"
#include "settings.h"
#include "translations.h"
#include "ui_button.h"
#include "ui_constants.h"
#include "ui_dialog.h"
#include "ui_option_item.h"

namespace
{
    enum class SelectedWindow : int
    {
        Configuration,
        InterfaceType,
        InterfacePresence,
        CursorType,
        Exit
    };

    const fheroes::Size offsetBetweenOptions{ 118, 110 };
    const fheroes::Point optionOffset{ 69, 47 };
    const int32_t optionWindowSize{ 65 };

    const fheroes::Rect interfaceTypeRoi{ optionOffset.x, optionOffset.y, optionWindowSize, optionWindowSize };
    const fheroes::Rect interfacePresenceRoi{ optionOffset.x + offsetBetweenOptions.width, optionOffset.y, optionWindowSize, optionWindowSize };
    const fheroes::Rect cursorTypeRoi{ optionOffset.x, optionOffset.y + offsetBetweenOptions.height, optionWindowSize, optionWindowSize };
    const fheroes::Rect scrollSpeedRoi{ optionOffset.x + offsetBetweenOptions.width, optionOffset.y + offsetBetweenOptions.height, optionWindowSize, optionWindowSize };

    void drawInterfacePresence( const fheroes::Rect & optionRoi )
    {
        // Interface show/hide state.
        const Settings & conf = Settings::Get();
        const bool isHiddenInterface = conf.isHideInterfaceEnabled();
        const bool isEvilInterface = conf.isEvilInterfaceEnabled();
        const fheroes::Sprite & interfaceStateIcon
            = isHiddenInterface ? fheroes::AGG::GetICN( ICN::ESPANEL, 4 ) : fheroes::AGG::GetICN( ICN::SPANEL, isEvilInterface ? 17 : 16 );

        std::string value;
        if ( isHiddenInterface ) {
            value = _( "Hide" );
        }
        else {
            value = _( "Show" );
        }

        fheroes::drawOption( optionRoi, interfaceStateIcon, _( "Interface" ), std::move( value ), fheroes::UiOptionTextWidth::TWO_ELEMENTS_ROW );
    }

    SelectedWindow showConfigurationWindow( bool & saveConfiguration )
    {
        fheroes::Display & display = fheroes::Display::instance();

        Settings & conf = Settings::Get();
        const bool isEvilInterface = conf.isEvilInterfaceEnabled();
        const fheroes::Sprite & dialog = fheroes::AGG::GetICN( ( isEvilInterface ? ICN::ESPANBKG_EVIL : ICN::ESPANBKG ), 0 );
        const fheroes::Sprite & dialogShadow = fheroes::AGG::GetICN( ( isEvilInterface ? ICN::CSPANBKE : ICN::CSPANBKG ), 1 );

        const fheroes::Point dialogOffset( ( display.width() - dialog.width() ) / 2, ( display.height() - dialog.height() ) / 2 );
        const fheroes::Point shadowOffset( dialogOffset.x - fheroes::borderWidthPx, dialogOffset.y );

        const fheroes::ImageRestorer restorer( display, shadowOffset.x, shadowOffset.y, dialog.width() + fheroes::borderWidthPx,
                                                dialog.height() + fheroes::borderWidthPx );
        const fheroes::Rect windowRoi{ dialogOffset.x, dialogOffset.y, dialog.width(), dialog.height() };

        fheroes::Blit( dialogShadow, display, windowRoi.x - fheroes::borderWidthPx, windowRoi.y + fheroes::borderWidthPx );
        fheroes::Blit( dialog, display, windowRoi.x, windowRoi.y );

        fheroes::ImageRestorer emptyDialogRestorer( display, windowRoi.x, windowRoi.y, windowRoi.width, windowRoi.height );

        const fheroes::Rect windowInterfaceTypeRoi( interfaceTypeRoi + windowRoi.getPosition() );
        const fheroes::Rect windowInterfacePresenceRoi( interfacePresenceRoi + windowRoi.getPosition() );
        const fheroes::Rect windowCursorTypeRoi( cursorTypeRoi + windowRoi.getPosition() );
        const fheroes::Rect windowScrollSpeedRoi( scrollSpeedRoi + windowRoi.getPosition() );

        const auto drawOptions = [&conf, &windowInterfaceTypeRoi, &windowInterfacePresenceRoi, &windowCursorTypeRoi, &windowScrollSpeedRoi]() {
            drawInterfaceType( windowInterfaceTypeRoi, conf.getInterfaceType() );
            drawInterfacePresence( windowInterfacePresenceRoi );
            drawCursorType( windowCursorTypeRoi, conf.isMonochromeCursorEnabled() );
            drawScrollSpeed( windowScrollSpeedRoi, conf.ScrollSpeed() );
        };

        drawOptions();

        const fheroes::Point buttonOffset( 112 + windowRoi.x, 252 + windowRoi.y );
        fheroes::Button buttonOk( buttonOffset.x, buttonOffset.y, isEvilInterface ? ICN::BUTTON_SMALL_OKAY_EVIL : ICN::BUTTON_SMALL_OKAY_GOOD, 0, 1 );

        buttonOk.draw();

        const auto refreshWindow = [&drawOptions, &emptyDialogRestorer, &buttonOk, &display]() {
            emptyDialogRestorer.restore();
            drawOptions();
            buttonOk.draw();
            display.render( emptyDialogRestorer.rect() );
        };

        display.render();

        bool isFullScreen = fheroes::engine().isFullScreen();

        LocalEvent & le = LocalEvent::Get();
        while ( le.HandleEvents() ) {
            buttonOk.drawOnState( le.isMouseLeftButtonPressedAndHeldInArea( buttonOk.area() ) );

            if ( le.MouseClickLeft( buttonOk.area() ) || Game::HotKeyCloseWindow() ) {
                break;
            }
            if ( le.MouseClickLeft( windowInterfaceTypeRoi ) ) {
                return SelectedWindow::InterfaceType;
            }
            if ( le.MouseClickLeft( windowInterfacePresenceRoi ) ) {
                return SelectedWindow::InterfacePresence;
            }
            if ( le.MouseClickLeft( windowCursorTypeRoi ) ) {
                return SelectedWindow::CursorType;
            }

            if ( le.MouseClickLeft( windowScrollSpeedRoi ) ) {
                saveConfiguration = true;
                conf.SetScrollSpeed( ( conf.ScrollSpeed() + 1 ) % ( SCROLL_SPEED_VERY_FAST + 1 ) );
                refreshWindow();

                continue;
            }
            if ( le.isMouseWheelUpInArea( windowScrollSpeedRoi ) ) {
                saveConfiguration = true;
                conf.SetScrollSpeed( conf.ScrollSpeed() + 1 );
                refreshWindow();

                continue;
            }
            if ( le.isMouseWheelDownInArea( windowScrollSpeedRoi ) ) {
                saveConfiguration = true;
                conf.SetScrollSpeed( conf.ScrollSpeed() - 1 );
                refreshWindow();

                continue;
            }

            if ( le.isMouseRightButtonPressedInArea( windowInterfaceTypeRoi ) ) {
                fheroes::showStandardTextMessage( _( "Interface Type" ), _( "Toggle the type of interface you want to use." ), 0 );
            }
            else if ( le.isMouseRightButtonPressedInArea( windowInterfacePresenceRoi ) ) {
                fheroes::showStandardTextMessage( _( "Interface" ), _( "Toggle interface visibility." ), 0 );
            }
            else if ( le.isMouseRightButtonPressedInArea( windowCursorTypeRoi ) ) {
                fheroes::showStandardTextMessage( _( "Mouse Cursor" ), _( "Toggle colored cursor on or off. This is only an aesthetic choice." ), 0 );
            }
            if ( le.isMouseRightButtonPressedInArea( windowScrollSpeedRoi ) ) {
                fheroes::showStandardTextMessage( _( "Scroll Speed" ), _( "Sets the speed at which you scroll the window." ), 0 );
            }
            else if ( le.isMouseRightButtonPressedInArea( buttonOk.area() ) ) {
                fheroes::showStandardTextMessage( _( "Okay" ), _( "Exit this menu." ), 0 );
            }

            // Fullscreen mode can be toggled using a global hotkey, we need to properly reflect this change in the UI
            if ( isFullScreen != fheroes::engine().isFullScreen() ) {
                isFullScreen = fheroes::engine().isFullScreen();

                emptyDialogRestorer.restore();
                drawOptions();

                display.render( emptyDialogRestorer.rect() );
            }
        }

        return SelectedWindow::Exit;
    }
}

namespace fheroes
{
    bool openInterfaceSettingsDialog( const std::function<void()> & updateUI )
    {
        const CursorRestorer cursorRestorer( true, ::Cursor::POINTER );

        Settings & conf = Settings::Get();

        bool saveConfiguration = false;

        SelectedWindow windowType = SelectedWindow::Configuration;
        while ( windowType != SelectedWindow::Exit ) {
            switch ( windowType ) {
            case SelectedWindow::Configuration:
                windowType = showConfigurationWindow( saveConfiguration );
                break;
            case SelectedWindow::InterfaceType:
                if ( conf.getInterfaceType() == InterfaceType::DYNAMIC ) {
                    conf.setInterfaceType( InterfaceType::GOOD );
                }
                else if ( conf.getInterfaceType() == InterfaceType::GOOD ) {
                    conf.setInterfaceType( InterfaceType::EVIL );
                }
                else {
                    conf.setInterfaceType( InterfaceType::DYNAMIC );
                }
                updateUI();
                saveConfiguration = true;

                windowType = SelectedWindow::Configuration;
                break;
            case SelectedWindow::InterfacePresence:
                conf.setHideInterface( !conf.isHideInterfaceEnabled() );
                updateUI();
                saveConfiguration = true;

                windowType = SelectedWindow::Configuration;
                break;
            case SelectedWindow::CursorType:
                conf.setMonochromeCursor( !conf.isMonochromeCursorEnabled() );
                saveConfiguration = true;

                windowType = SelectedWindow::Configuration;
                break;
            default:
                return saveConfiguration;
            }
        }

        return saveConfiguration;
    }
}
