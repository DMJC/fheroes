/***************************************************************************
 *   fheroes: https://github.com/ihhub/fheroes                           *
 *   Copyright (C) 2024 - 2025                                             *
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

#include "editor_options.h"

#include <cassert>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "agg_image.h"
#include "cursor.h"
#include "dialog.h"
#include "dialog_audio.h"
#include "dialog_graphics_settings.h"
#include "dialog_hotkeys.h"
#include "dialog_language_selection.h"
#include "editor_interface.h"
#include "game_hotkeys.h"
#include "game_language.h"
#include "icn.h"
#include "interface_base.h"
#include "localevent.h"
#include "math_base.h"
#include "render_processor.h"
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
    enum class DialogAction : uint8_t
    {
        Configuration,
        Language,
        Graphics,
        AudioSettings,
        HotKeys,
        Animation,
        Passabiility,
        UpdateSettings,
        InterfaceType,
        CursorType,
        UpdateScrollSpeed,
        IncreaseScrollSpeed,
        DecreaseScrollSpeed,
        Close
    };

    const fheroes::Size offsetBetweenOptions{ 92, 110 };
    const fheroes::Point optionOffset{ 20, 31 };
    const int32_t optionWindowSize{ 65 };

    const fheroes::Rect languageRoi{ optionOffset.x, optionOffset.y, optionWindowSize, optionWindowSize };
    const fheroes::Rect graphicsRoi{ optionOffset.x + offsetBetweenOptions.width, optionOffset.y, optionWindowSize, optionWindowSize };
    const fheroes::Rect audioRoi{ optionOffset.x + offsetBetweenOptions.width * 2, optionOffset.y, optionWindowSize, optionWindowSize };
    const fheroes::Rect hotKeyRoi{ optionOffset.x, optionOffset.y + offsetBetweenOptions.height, optionWindowSize, optionWindowSize };
    const fheroes::Rect animationRoi{ optionOffset.x + offsetBetweenOptions.width, optionOffset.y + offsetBetweenOptions.height, optionWindowSize, optionWindowSize };
    const fheroes::Rect passabilityRoi{ optionOffset.x + offsetBetweenOptions.width * 2, optionOffset.y + offsetBetweenOptions.height, optionWindowSize,
                                         optionWindowSize };
    const fheroes::Rect interfaceTypeRoi{ optionOffset.x, optionOffset.y + offsetBetweenOptions.height * 2, optionWindowSize, optionWindowSize };
    const fheroes::Rect cursorTypeRoi{ optionOffset.x + offsetBetweenOptions.width, optionOffset.y + offsetBetweenOptions.height * 2, optionWindowSize,
                                        optionWindowSize };
    const fheroes::Rect scrollSpeedRoi{ optionOffset.x + offsetBetweenOptions.width * 2, optionOffset.y + offsetBetweenOptions.height * 2, optionWindowSize,
                                         optionWindowSize };

    void drawLanguage( const fheroes::Rect & optionRoi )
    {
        const fheroes::SupportedLanguage currentLanguage = fheroes::getLanguageFromAbbreviation( Settings::Get().getGameLanguage() );
        const fheroes::LanguageSwitcher languageSwitcher( currentLanguage );

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

    void drawAnimationOptions( const fheroes::Rect & optionRoi )
    {
        if ( Settings::Get().isEditorAnimationEnabled() ) {
            fheroes::drawOption( optionRoi, fheroes::AGG::GetICN( ICN::ESPANEL, 1 ), _( "Animation" ), _( "On" ), fheroes::UiOptionTextWidth::THREE_ELEMENTS_ROW );
        }
        else {
            fheroes::drawOption( optionRoi, fheroes::AGG::GetICN( ICN::ESPANEL, 0 ), _( "Animation" ), _( "Off" ), fheroes::UiOptionTextWidth::THREE_ELEMENTS_ROW );
        }
    }

    void drawPassabilityOptions( const fheroes::Rect & optionRoi )
    {
        if ( Settings::Get().isEditorPassabilityEnabled() ) {
            fheroes::drawOption( optionRoi, fheroes::AGG::GetICN( ICN::ESPANEL, 5 ), _( "Passability" ), _( "Show" ), fheroes::UiOptionTextWidth::THREE_ELEMENTS_ROW );
        }
        else {
            fheroes::drawOption( optionRoi, fheroes::AGG::GetICN( ICN::ESPANEL, 4 ), _( "Passability" ), _( "Hide" ), fheroes::UiOptionTextWidth::THREE_ELEMENTS_ROW );
        }
    }

    DialogAction openEditorOptionsDialog()
    {
        fheroes::Display & display = fheroes::Display::instance();

        fheroes::StandardWindow background( 289, 382, true, display );

        const fheroes::Rect windowRoi = background.activeArea();

        const fheroes::Rect windowLanguageRoi( languageRoi + windowRoi.getPosition() );
        const fheroes::Rect windowGraphicsRoi( graphicsRoi + windowRoi.getPosition() );
        const fheroes::Rect windowAudioRoi( audioRoi + windowRoi.getPosition() );
        const fheroes::Rect windowHotKeyRoi( hotKeyRoi + windowRoi.getPosition() );
        const fheroes::Rect windowAnimationRoi( animationRoi + windowRoi.getPosition() );
        const fheroes::Rect windowPassabilityRoi( passabilityRoi + windowRoi.getPosition() );
        const fheroes::Rect windowInterfaceTypeRoi( interfaceTypeRoi + windowRoi.getPosition() );
        const fheroes::Rect windowCursorTypeRoi( cursorTypeRoi + windowRoi.getPosition() );
        const fheroes::Rect windowScrollSpeedRoi( scrollSpeedRoi + windowRoi.getPosition() );

        const auto drawOptions = [&windowLanguageRoi, &windowGraphicsRoi, &windowAudioRoi, &windowHotKeyRoi, &windowAnimationRoi, &windowPassabilityRoi,
                                  &windowInterfaceTypeRoi, &windowCursorTypeRoi, &windowScrollSpeedRoi]() {
            const Settings & conf = Settings::Get();

            drawLanguage( windowLanguageRoi );
            drawGraphics( windowGraphicsRoi );
            drawAudioOptions( windowAudioRoi );
            drawHotKeyOptions( windowHotKeyRoi );
            drawAnimationOptions( windowAnimationRoi );
            drawPassabilityOptions( windowPassabilityRoi );
            drawInterfaceType( windowInterfaceTypeRoi, conf.getInterfaceType() );
            drawCursorType( windowCursorTypeRoi, conf.isMonochromeCursorEnabled() );
            drawScrollSpeed( windowScrollSpeedRoi, conf.ScrollSpeed() );
        };

        drawOptions();

        const Settings & conf = Settings::Get();
        const bool isEvilInterface = conf.isEvilInterfaceEnabled();

        fheroes::Button buttonOk;
        const int buttonOkIcnId = isEvilInterface ? ICN::BUTTON_SMALL_OKAY_EVIL : ICN::BUTTON_SMALL_OKAY_GOOD;
        background.renderButton( buttonOk, buttonOkIcnId, 0, 1, { 0, 11 }, fheroes::StandardWindow::Padding::BOTTOM_CENTER );

        // Render the whole screen as interface type or resolution could have been changed.
        display.render();

        LocalEvent & le = LocalEvent::Get();
        while ( le.HandleEvents() ) {
            buttonOk.drawOnState( le.isMouseLeftButtonPressedAndHeldInArea( buttonOk.area() ) );

            if ( le.MouseClickLeft( buttonOk.area() ) || Game::HotKeyCloseWindow() ) {
                break;
            }
            if ( le.MouseClickLeft( windowLanguageRoi ) ) {
                return DialogAction::Language;
            }
            if ( le.MouseClickLeft( windowGraphicsRoi ) ) {
                return DialogAction::Graphics;
            }
            if ( le.MouseClickLeft( windowAudioRoi ) ) {
                return DialogAction::AudioSettings;
            }
            if ( le.MouseClickLeft( windowHotKeyRoi ) ) {
                return DialogAction::HotKeys;
            }
            if ( le.MouseClickLeft( windowAnimationRoi ) ) {
                return DialogAction::Animation;
            }
            if ( le.MouseClickLeft( windowPassabilityRoi ) ) {
                return DialogAction::Passabiility;
            }
            if ( le.MouseClickLeft( windowInterfaceTypeRoi ) ) {
                return DialogAction::InterfaceType;
            }
            if ( le.MouseClickLeft( windowCursorTypeRoi ) ) {
                return DialogAction::CursorType;
            }
            if ( le.MouseClickLeft( windowScrollSpeedRoi ) ) {
                return DialogAction::UpdateScrollSpeed;
            }
            if ( le.isMouseWheelUpInArea( windowScrollSpeedRoi ) ) {
                return DialogAction::IncreaseScrollSpeed;
            }
            if ( le.isMouseWheelDownInArea( windowScrollSpeedRoi ) ) {
                return DialogAction::DecreaseScrollSpeed;
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
            else if ( le.isMouseRightButtonPressedInArea( windowAnimationRoi ) ) {
                fheroes::showStandardTextMessage( _( "Animation" ), _( "Toggle animation of the objects." ), 0 );
            }
            else if ( le.isMouseRightButtonPressedInArea( windowPassabilityRoi ) ) {
                fheroes::showStandardTextMessage( _( "Passability" ), _( "Toggle display of objects' passability." ), 0 );
            }
            else if ( le.isMouseRightButtonPressedInArea( windowInterfaceTypeRoi ) ) {
                fheroes::showStandardTextMessage( _( "Interface Type" ), _( "Toggle the type of interface you want to use." ), 0 );
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
        }

        return DialogAction::Close;
    }
}

namespace Editor
{
    void openEditorSettings()
    {
        const CursorRestorer cursorRestorer( true, Cursor::POINTER );

        // We should write to the configuration file only once to avoid extra I/O operations.
        bool saveConfiguration = false;
        Settings & conf = Settings::Get();

        auto redrawEditor = [&conf]() {
            Interface::EditorInterface & editorInterface = Interface::EditorInterface::Get();

            // Since the radar interface has a restorer we must redraw it first to avoid the restorer doing something nasty.
            editorInterface.redraw( Interface::REDRAW_RADAR );

            uint32_t redrawOptions = Interface::REDRAW_ALL;
            if ( conf.isEditorPassabilityEnabled() ) {
                redrawOptions |= Interface::REDRAW_PASSABILITIES;
            }

            editorInterface.redraw( redrawOptions & ( ~Interface::REDRAW_RADAR ) );
        };

        auto rebuildEditor = [&redrawEditor]() {
            Interface::EditorInterface::Get().reset();

            redrawEditor();
        };

        DialogAction action = DialogAction::Configuration;

        while ( action != DialogAction::Close ) {
            switch ( action ) {
            case DialogAction::Configuration:
                action = openEditorOptionsDialog();
                break;
            case DialogAction::Language: {
                const std::vector<fheroes::SupportedLanguage> supportedLanguages = fheroes::getSupportedLanguages();

                if ( supportedLanguages.size() > 1 ) {
                    selectLanguage( supportedLanguages, fheroes::getLanguageFromAbbreviation( conf.getGameLanguage() ), true );
                }
                else {
                    assert( supportedLanguages.front() == fheroes::SupportedLanguage::English );

                    conf.setGameLanguage( fheroes::getLanguageAbbreviation( fheroes::SupportedLanguage::English ) );

                    fheroes::showStandardTextMessage( "Attention", "Your version of Heroes of Might and Magic II does not support any other languages than English.",
                                                       Dialog::OK );
                }

                redrawEditor();
                saveConfiguration = true;
                action = DialogAction::Configuration;
                break;
            }
            case DialogAction::Graphics:
                saveConfiguration |= fheroes::openGraphicsSettingsDialog( rebuildEditor );

                action = DialogAction::Configuration;
                break;
            case DialogAction::AudioSettings:
                saveConfiguration |= Dialog::openAudioSettingsDialog( false );

                action = DialogAction::Configuration;
                break;
            case DialogAction::HotKeys:
                fheroes::openHotkeysDialog();

                action = DialogAction::Configuration;
                break;
            case DialogAction::Animation:
                conf.setEditorAnimation( !conf.isEditorAnimationEnabled() );
                saveConfiguration = true;

                if ( conf.isEditorAnimationEnabled() ) {
                    fheroes::RenderProcessor::instance().startColorCycling();
                }
                else {
                    fheroes::RenderProcessor::instance().stopColorCycling();
                }

                action = DialogAction::Configuration;
                break;
            case DialogAction::Passabiility:
                conf.setEditorPassability( !conf.isEditorPassabilityEnabled() );
                saveConfiguration = true;

                redrawEditor();

                action = DialogAction::Configuration;
                break;
            case DialogAction::InterfaceType:
                if ( conf.getInterfaceType() == InterfaceType::DYNAMIC ) {
                    conf.setInterfaceType( InterfaceType::GOOD );
                }
                else if ( conf.getInterfaceType() == InterfaceType::GOOD ) {
                    conf.setInterfaceType( InterfaceType::EVIL );
                }
                else {
                    conf.setInterfaceType( InterfaceType::DYNAMIC );
                }
                rebuildEditor();
                saveConfiguration = true;

                action = DialogAction::Configuration;
                break;
            case DialogAction::CursorType:
                conf.setMonochromeCursor( !conf.isMonochromeCursorEnabled() );
                saveConfiguration = true;

                action = DialogAction::Configuration;
                break;
            case DialogAction::UpdateScrollSpeed:
                conf.SetScrollSpeed( ( conf.ScrollSpeed() + 1 ) % ( SCROLL_SPEED_VERY_FAST + 1 ) );
                saveConfiguration = true;

                action = DialogAction::Configuration;
                break;
            case DialogAction::IncreaseScrollSpeed:
                conf.SetScrollSpeed( conf.ScrollSpeed() + 1 );
                saveConfiguration = true;

                action = DialogAction::Configuration;
                break;
            case DialogAction::DecreaseScrollSpeed:
                conf.SetScrollSpeed( conf.ScrollSpeed() - 1 );
                saveConfiguration = true;

                action = DialogAction::Configuration;
                break;
            default:
                break;
            }
        }

        if ( saveConfiguration ) {
            conf.Save( Settings::configFileName );
        }
    }
}
