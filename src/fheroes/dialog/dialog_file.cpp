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

#include "cursor.h"
#include "dialog.h" // IWYU pragma: associated
#include "game_hotkeys.h"
#include "game_interface.h"
#include "game_io.h"
#include "game_mode.h"
#include "icn.h"
#include "localevent.h"
#include "screen.h"
#include "settings.h"
#include "translations.h"
#include "ui_button.h"
#include "ui_dialog.h"
#include "ui_window.h"

namespace
{
    fheroes::GameMode selectFileOption()
    {
        // setup cursor
        const CursorRestorer cursorRestorer( true, Cursor::POINTER );

        fheroes::Display & display = fheroes::Display::instance();

        auto & config = Settings::Get();
        const bool isEvilInterface = config.isEvilInterfaceEnabled();
        const int bigButtonsICN = isEvilInterface ? ICN::BUTTONS_FILE_DIALOG_EVIL : ICN::BUTTONS_FILE_DIALOG_GOOD;
        fheroes::ButtonGroup optionButtons( bigButtonsICN );
        fheroes::StandardWindow background( optionButtons, false, 0, display );

        const fheroes::ButtonBase & newGameButton = optionButtons.button( 0 );
        const fheroes::ButtonBase & loadGameButton = optionButtons.button( 1 );
        fheroes::ButtonBase & restartGameButton = optionButtons.button( 2 );
        const fheroes::ButtonBase & saveGameButton = optionButtons.button( 3 );
        const fheroes::ButtonBase & quickSaveButton = optionButtons.button( 4 );
        const fheroes::ButtonBase & quitButton = optionButtons.button( 5 );

        // For now this button is disabled.
        restartGameButton.disable();

        background.renderSymmetricButtons( optionButtons, 0, false );

        fheroes::Button buttonCancel;

        background.renderButton( buttonCancel, isEvilInterface ? ICN::BUTTON_SMALL_CANCEL_EVIL : ICN::BUTTON_SMALL_CANCEL_GOOD, 0, 1, { 0, 11 },
                                 fheroes::StandardWindow::Padding::BOTTOM_CENTER );

        display.render( background.totalArea() );

        fheroes::GameMode result = fheroes::GameMode::QUIT_GAME;

        LocalEvent & le = LocalEvent::Get();

        // dialog menu loop
        while ( le.HandleEvents() ) {
            optionButtons.drawOnState( le );
            buttonCancel.drawOnState( le.isMouseLeftButtonPressedAndHeldInArea( buttonCancel.area() ) );

            if ( le.MouseClickLeft( newGameButton.area() ) || Game::HotKeyPressEvent( Game::HotKeyEvent::MAIN_MENU_NEW_GAME ) ) {
                if ( Interface::AdventureMap::Get().EventNewGame() == fheroes::GameMode::NEW_GAME ) {
                    result = fheroes::GameMode::NEW_GAME;
                    break;
                }
            }
            else if ( le.MouseClickLeft( loadGameButton.area() ) || Game::HotKeyPressEvent( Game::HotKeyEvent::MAIN_MENU_LOAD_GAME ) ) {
                if ( Interface::AdventureMap::Get().EventLoadGame() == fheroes::GameMode::LOAD_GAME ) {
                    result = fheroes::GameMode::LOAD_GAME;
                    break;
                }
            }
            else if ( restartGameButton.isEnabled() && le.MouseClickLeft( restartGameButton.area() ) ) {
                // TODO: restart the campaign here.
                fheroes::showStandardTextMessage( _( "Restart Game" ), "This option is under construction.", Dialog::OK );
                result = fheroes::GameMode::CANCEL;
                break;
            }
            else if ( le.MouseClickLeft( saveGameButton.area() ) || Game::HotKeyPressEvent( Game::HotKeyEvent::WORLD_SAVE_GAME ) ) {
                // Special case: since we show a window about file saving we don't want to display the current dialog anymore.
                background.hideWindow();

                return Interface::AdventureMap::Get().EventSaveGame();
            }
            else if ( le.MouseClickLeft( quickSaveButton.area() ) ) {
                if ( !Game::QuickSave() ) {
                    fheroes::showStandardTextMessage( "", _( "There was an issue during saving." ), Dialog::OK );
                }

                result = fheroes::GameMode::CANCEL;
                break;
            }

            if ( le.MouseClickLeft( quitButton.area() ) || Game::HotKeyPressEvent( Game::HotKeyEvent::MAIN_MENU_QUIT ) ) {
                if ( Interface::AdventureMap::EventExit() == fheroes::GameMode::QUIT_GAME ) {
                    result = fheroes::GameMode::QUIT_GAME;
                    break;
                }
            }
            else if ( le.MouseClickLeft( buttonCancel.area() ) || Game::HotKeyCloseWindow() ) {
                result = fheroes::GameMode::CANCEL;
                break;
            }

            if ( le.isMouseRightButtonPressedInArea( newGameButton.area() ) ) {
                fheroes::showStandardTextMessage( _( "New Game" ), _( "Start a single or multi-player game." ), Dialog::ZERO );
            }
            else if ( le.isMouseRightButtonPressedInArea( loadGameButton.area() ) ) {
                fheroes::showStandardTextMessage( _( "Load Game" ), _( "Load a previously saved game." ), Dialog::ZERO );
            }
            else if ( le.isMouseRightButtonPressedInArea( restartGameButton.area() ) ) {
                fheroes::showStandardTextMessage( _( "Restart Game" ), _( "Restart the scenario." ), Dialog::ZERO );
            }
            else if ( le.isMouseRightButtonPressedInArea( saveGameButton.area() ) ) {
                fheroes::showStandardTextMessage( _( "Save Game" ), _( "Save the current game." ), Dialog::ZERO );
            }
            else if ( le.isMouseRightButtonPressedInArea( quickSaveButton.area() ) ) {
                fheroes::showStandardTextMessage( _( "Quick Save" ), _( "Save the current game without name selection." ), Dialog::ZERO );
            }
            else if ( le.isMouseRightButtonPressedInArea( quitButton.area() ) ) {
                fheroes::showStandardTextMessage( _( "Quit" ), _( "Quit out of Heroes of Might and Magic II." ), Dialog::ZERO );
            }
            else if ( le.isMouseRightButtonPressedInArea( buttonCancel.area() ) ) {
                fheroes::showStandardTextMessage( _( "Cancel" ), _( "Exit this menu without doing anything." ), Dialog::ZERO );
            }
        }

        return result;
    }
}

fheroes::GameMode Dialog::FileOptions()
{
    const fheroes::GameMode result = selectFileOption();
    return result;
}
