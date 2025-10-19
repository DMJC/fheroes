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

#include "dialog_game_settings.h"

#include <cassert>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "agg_image.h"
#include "dialog.h"
#include "dialog_audio.h"
#include "dialog_graphics_settings.h"
#include "dialog_hotkeys.h"
#include "dialog_language_selection.h"
#include "game_hotkeys.h"
#include "game_language.h"
#include "game_mainmenu_ui.h"
#include "icn.h"
#include "image.h"
#include "localevent.h"
#include "math_base.h"
#include "screen.h"
#include "settings.h"
#include "translations.h"
#include "ui_button.h"
#include "ui_dialog.h"
#include "ui_language.h"
#include "ui_option_item.h"
#include "ui_window.h"

namespace
{
    enum class SelectedWindow : int
    {
        Configuration,
        Language,
        Graphics,
        AudioSettings,
        HotKeys,
        CursorType,
        TextSupportMode,
        UpdateSettings,
        Exit
    };

    const fheroes::Size offsetBetweenOptions{ 92, 110 };

    const fheroes::Point optionOffset{ 20, 31 };
    const int32_t optionWindowSize{ 65 };

    const fheroes::Rect languageRoi{ optionOffset.x, optionOffset.y, optionWindowSize, optionWindowSize };
    const fheroes::Rect graphicsRoi{ optionOffset.x + offsetBetweenOptions.width, optionOffset.y, optionWindowSize, optionWindowSize };
    const fheroes::Rect audioRoi{ optionOffset.x + offsetBetweenOptions.width * 2, optionOffset.y, optionWindowSize, optionWindowSize };
    const fheroes::Rect hotKeyRoi{ optionOffset.x, optionOffset.y + offsetBetweenOptions.height, optionWindowSize, optionWindowSize };
    const fheroes::Rect cursorTypeRoi{ optionOffset.x + offsetBetweenOptions.width, optionOffset.y + offsetBetweenOptions.height, optionWindowSize, optionWindowSize };
    const fheroes::Rect textSupportModeRoi{ optionOffset.x + offsetBetweenOptions.width * 2, optionOffset.y + offsetBetweenOptions.height, optionWindowSize,
                                             optionWindowSize };

    void drawLanguage( const fheroes::Rect & optionRoi )
    {
        const fheroes::SupportedLanguage currentLanguage = fheroes::getLanguageFromAbbreviation( Settings::Get().getGameLanguage() );
        fheroes::LanguageSwitcher languageSwitcher( currentLanguage );

        fheroes::drawOption( optionRoi, fheroes::AGG::GetICN( ICN::SPANEL, 18 ), _( "Language" ), fheroes::getLanguageName( currentLanguage ),
                              fheroes::UiOptionTextWidth::THREE_ELEMENTS_ROW );
    }

    void drawGraphics( const fheroes::Rect & optionRoi )
    {
        fheroes::drawOption( optionRoi, fheroes::AGG::GetICN( ICN::GAME_OPTION_ICON, 1 ), _( "Graphics" ), _( "Settings" ),
                              fheroes::UiOptionTextWidth::THREE_ELEMENTS_ROW );
    }

    void drawAudioOptions( const fheroes::Rect & optionRoi )
    {
        fheroes::drawOption( optionRoi, fheroes::AGG::GetICN( ICN::SPANEL, 1 ), _( "Audio" ), _( "Settings" ), fheroes::UiOptionTextWidth::THREE_ELEMENTS_ROW );
    }

    void drawHotKeyOptions( const fheroes::Rect & optionRoi )
    {
        fheroes::drawOption( optionRoi, fheroes::AGG::GetICN( ICN::GAME_OPTION_ICON, 0 ), _( "Hot Keys" ), _( "Configure" ),
                              fheroes::UiOptionTextWidth::THREE_ELEMENTS_ROW );
    }

    void drawCursorTypeOptions( const fheroes::Rect & optionRoi )
    {
        if ( Settings::Get().isMonochromeCursorEnabled() ) {
            fheroes::drawOption( optionRoi, fheroes::AGG::GetICN( ICN::SPANEL, 20 ), _( "Mouse Cursor" ), _( "Black & White" ),
                                  fheroes::UiOptionTextWidth::THREE_ELEMENTS_ROW );
        }
        else {
            fheroes::drawOption( optionRoi, fheroes::AGG::GetICN( ICN::SPANEL, 21 ), _( "Mouse Cursor" ), _( "Color" ),
                                  fheroes::UiOptionTextWidth::THREE_ELEMENTS_ROW );
        }
    }

    void drawTextSupportModeOptions( const fheroes::Rect & optionRoi )
    {
        if ( Settings::Get().isTextSupportModeEnabled() ) {
            fheroes::drawOption( optionRoi, fheroes::AGG::GetICN( ICN::CSPANEL, 4 ), _( "Text Support" ), _( "On" ), fheroes::UiOptionTextWidth::THREE_ELEMENTS_ROW );
        }
        else {
            fheroes::drawOption( optionRoi, fheroes::AGG::GetICN( ICN::SPANEL, 9 ), _( "Text Support" ), _( "Off" ), fheroes::UiOptionTextWidth::THREE_ELEMENTS_ROW );
        }
    }

    SelectedWindow showConfigurationWindow()
    {
        fheroes::Display & display = fheroes::Display::instance();

        fheroes::StandardWindow background( 289, 272, true, display );

        const fheroes::Rect windowRoi = background.activeArea();

        const Settings & conf = Settings::Get();
        const bool isEvilInterface = conf.isEvilInterfaceEnabled();

        fheroes::Button buttonOk;
        const int buttonOkIcnId = isEvilInterface ? ICN::BUTTON_SMALL_OKAY_EVIL : ICN::BUTTON_SMALL_OKAY_GOOD;
        background.renderButton( buttonOk, buttonOkIcnId, 0, 1, { 0, 11 }, fheroes::StandardWindow::Padding::BOTTOM_CENTER );

        fheroes::ImageRestorer emptyDialogRestorer( display, windowRoi.x, windowRoi.y, windowRoi.width, windowRoi.height );

        const fheroes::Rect windowLanguageRoi( languageRoi + windowRoi.getPosition() );
        const fheroes::Rect windowGraphicsRoi( graphicsRoi + windowRoi.getPosition() );
        const fheroes::Rect windowAudioRoi( audioRoi + windowRoi.getPosition() );
        const fheroes::Rect windowHotKeyRoi( hotKeyRoi + windowRoi.getPosition() );
        const fheroes::Rect windowCursorTypeRoi( cursorTypeRoi + windowRoi.getPosition() );
        const fheroes::Rect windowTextSupportModeRoi( textSupportModeRoi + windowRoi.getPosition() );

        const auto drawOptions = [&windowLanguageRoi, &windowGraphicsRoi, &windowAudioRoi, &windowHotKeyRoi, &windowCursorTypeRoi, &windowTextSupportModeRoi]() {
            drawLanguage( windowLanguageRoi );
            drawGraphics( windowGraphicsRoi );
            drawAudioOptions( windowAudioRoi );
            drawHotKeyOptions( windowHotKeyRoi );
            drawCursorTypeOptions( windowCursorTypeRoi );
            drawTextSupportModeOptions( windowTextSupportModeRoi );
        };

        drawOptions();

        display.render( background.totalArea() );

        bool isTextSupportModeEnabled = conf.isTextSupportModeEnabled();

        LocalEvent & le = LocalEvent::Get();
        while ( le.HandleEvents() ) {
            buttonOk.drawOnState( le.isMouseLeftButtonPressedAndHeldInArea( buttonOk.area() ) );

            if ( le.MouseClickLeft( buttonOk.area() ) || Game::HotKeyCloseWindow() ) {
                break;
            }
            if ( le.MouseClickLeft( windowLanguageRoi ) ) {
                return SelectedWindow::Language;
            }
            if ( le.MouseClickLeft( windowGraphicsRoi ) ) {
                return SelectedWindow::Graphics;
            }
            if ( le.MouseClickLeft( windowAudioRoi ) ) {
                return SelectedWindow::AudioSettings;
            }
            if ( le.MouseClickLeft( windowHotKeyRoi ) ) {
                return SelectedWindow::HotKeys;
            }
            if ( le.MouseClickLeft( windowCursorTypeRoi ) ) {
                return SelectedWindow::CursorType;
            }
            if ( le.MouseClickLeft( windowTextSupportModeRoi ) ) {
                return SelectedWindow::TextSupportMode;
            }

            if ( le.isMouseRightButtonPressedInArea( windowLanguageRoi ) ) {
                fheroes::showStandardTextMessage( _( "Select Game Language" ), _( "Change the language of the game." ), 0 );
            }
            else if ( le.isMouseRightButtonPressedInArea( windowGraphicsRoi ) ) {
                fheroes::showStandardTextMessage( _( "Graphics" ), _( "Change the graphics settings of the game." ), 0 );
            }
            else if ( le.isMouseRightButtonPressedInArea( windowAudioRoi ) ) {
                fheroes::showStandardTextMessage( _( "Audio" ), _( "Change the audio settings of the game." ), 0 );
            }
            else if ( le.isMouseRightButtonPressedInArea( windowHotKeyRoi ) ) {
                fheroes::showStandardTextMessage( _( "Hot Keys" ), _( "Check and configure all the hot keys present in the game." ), 0 );
            }
            else if ( le.isMouseRightButtonPressedInArea( windowCursorTypeRoi ) ) {
                fheroes::showStandardTextMessage( _( "Mouse Cursor" ), _( "Toggle colored cursor on or off. This is only an aesthetic choice." ), 0 );
            }
            else if ( le.isMouseRightButtonPressedInArea( windowTextSupportModeRoi ) ) {
                fheroes::showStandardTextMessage( _( "Text Support" ), _( "Toggle text support mode to output extra information about windows and events in the game." ),
                                                   0 );
            }
            else if ( le.isMouseRightButtonPressedInArea( buttonOk.area() ) ) {
                fheroes::showStandardTextMessage( _( "Okay" ), _( "Exit this menu." ), 0 );
            }

            // Text support mode can be toggled using a global hotkey, we need to properly reflect this change in the UI
            if ( isTextSupportModeEnabled != conf.isTextSupportModeEnabled() ) {
                isTextSupportModeEnabled = conf.isTextSupportModeEnabled();

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
    void openGameSettings()
    {
        drawMainMenuScreen();
        fheroes::Display::instance().render();

        Settings & conf = Settings::Get();

        bool saveConfiguration = false;

        SelectedWindow windowType = SelectedWindow::Configuration;
        while ( windowType != SelectedWindow::Exit ) {
            switch ( windowType ) {
            case SelectedWindow::Configuration:
                windowType = showConfigurationWindow();
                break;
            case SelectedWindow::Language: {
                const std::vector<SupportedLanguage> supportedLanguages = getSupportedLanguages();

                if ( supportedLanguages.size() > 1 ) {
                    selectLanguage( supportedLanguages, getLanguageFromAbbreviation( conf.getGameLanguage() ), true );
                }
                else {
                    assert( supportedLanguages.front() == SupportedLanguage::English );

                    conf.setGameLanguage( getLanguageAbbreviation( SupportedLanguage::English ) );

                    fheroes::showStandardTextMessage( "Attention", "Your version of Heroes of Might and Magic II does not support any other languages than English.",
                                                       Dialog::OK );
                }

                windowType = SelectedWindow::UpdateSettings;
                break;
            }
            case SelectedWindow::Graphics:
                saveConfiguration |= fheroes::openGraphicsSettingsDialog( []() { drawMainMenuScreen(); } );
                windowType = SelectedWindow::Configuration;
                break;
            case SelectedWindow::AudioSettings:
                saveConfiguration |= Dialog::openAudioSettingsDialog( false );
                windowType = SelectedWindow::Configuration;
                break;
            case SelectedWindow::HotKeys:
                fheroes::openHotkeysDialog();
                windowType = SelectedWindow::Configuration;
                break;
            case SelectedWindow::CursorType:
                conf.setMonochromeCursor( !conf.isMonochromeCursorEnabled() );
                windowType = SelectedWindow::UpdateSettings;
                break;
            case SelectedWindow::TextSupportMode:
                conf.setTextSupportMode( !conf.isTextSupportModeEnabled() );
                windowType = SelectedWindow::UpdateSettings;
                break;
            case SelectedWindow::UpdateSettings:
                saveConfiguration = true;
                windowType = SelectedWindow::Configuration;
                break;
            default:
                return;
            }
        }

        if ( saveConfiguration ) {
            conf.Save( Settings::configFileName );
        }
    }
}
