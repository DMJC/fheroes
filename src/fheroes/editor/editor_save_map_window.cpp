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

#include "editor_save_map_window.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <ctime>
#include <iterator>
#include <memory>
#include <optional>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "agg_image.h"
#include "color.h"
#include "cursor.h"
#include "dialog.h"
#include "game_hotkeys.h"
#include "game_language.h"
#include "game_string.h"
#include "icn.h"
#include "image.h"
#include "interface_list.h"
#include "localevent.h"
#include "logging.h"
#include "maps.h"
#include "maps_fileinfo.h"
#include "math_base.h"
#include "screen.h"
#include "settings.h"
#include "system.h"
#include "translations.h"
#include "ui_button.h"
#include "ui_dialog.h"
#include "ui_keyboard.h"
#include "ui_language.h"
#include "ui_text.h"
#include "ui_tool.h"
#include "ui_window.h"

namespace
{
    // This constant sets the maximum displayed file name width. This value affects the dialog horizontal size.
    const int32_t maxFileNameWidth = 300;

    const std::string mapFileExtension = ".fh2m";

    class FileInfoListBox : public Interface::ListBox<Maps::FileInfo>
    {
    public:
        using Interface::ListBox<Maps::FileInfo>::ActionListDoubleClick;
        using Interface::ListBox<Maps::FileInfo>::ActionListSingleClick;
        using Interface::ListBox<Maps::FileInfo>::ActionListPressRight;

        using ListBox::ListBox;

        void RedrawItem( const Maps::FileInfo & info, int32_t posX, int32_t posY, bool current ) override;
        void RedrawBackground( const fheroes::Point & dst ) override;

        void ActionCurrentUp() override;
        void ActionCurrentDn() override;
        void ActionListDoubleClick( Maps::FileInfo & info ) override;
        void ActionListSingleClick( Maps::FileInfo & info ) override;

        void ActionListPressRight( Maps::FileInfo & info ) override
        {
            if ( info.version != GameVersion::RESURRECTION ) {
                // The Editor doesn't support original map formats so you are trying to do some nonsence hacks.
                assert( 0 );
                return;
            }

            const fheroes::Text header( info.name, fheroes::FontType::normalYellow(), info.mainLanguage );

            const Settings & conf = Settings::Get();
            const fheroes::SupportedLanguage gameLanguage = fheroes::getLanguageFromAbbreviation( conf.getGameLanguage() );

            std::vector<std::pair<fheroes::LocalizedString, fheroes::FontType>> strings;

            strings.emplace_back( fheroes::LocalizedString( _( "Map Type:\n" ), gameLanguage ), fheroes::FontType::normalYellow() );
            strings.emplace_back( fheroes::LocalizedString( _( "Resurrection" ), gameLanguage ), fheroes::FontType::normalWhite() );
            strings.emplace_back( fheroes::LocalizedString( _( "\n\nLanguage:\n" ), gameLanguage ), fheroes::FontType::normalYellow() );
            strings.emplace_back( fheroes::LocalizedString( fheroes::getLanguageName( info.mainLanguage ), gameLanguage ), fheroes::FontType::normalWhite() );
            strings.emplace_back( fheroes::LocalizedString( _( "\n\nSize: " ), gameLanguage ), fheroes::FontType::normalYellow() );
            strings.emplace_back( fheroes::LocalizedString( std::to_string( info.width ) + " x " + std::to_string( info.height ), gameLanguage ),
                                  fheroes::FontType::normalWhite() );
            strings.emplace_back( fheroes::LocalizedString( _( "\n\nDescription: " ), gameLanguage ), fheroes::FontType::normalYellow() );
            strings.emplace_back( fheroes::LocalizedString( info.description, info.mainLanguage ), fheroes::FontType::normalWhite() );
            strings.emplace_back( fheroes::LocalizedString( _( "\n\nLocation: " ), gameLanguage ), fheroes::FontType::smallYellow() );
            strings.emplace_back( fheroes::LocalizedString( info.filename, gameLanguage ), fheroes::FontType::smallWhite() );

            fheroes::showMessage( header, *fheroes::getLocalizedText( strings ), Dialog::ZERO );
        }

        void initListBackgroundRestorer( fheroes::Rect roi )
        {
            _listBackground = std::make_unique<fheroes::ImageRestorer>( fheroes::Display::instance(), roi.x, roi.y, roi.width, roi.height );
        }

        bool isDoubleClicked() const
        {
            return _isDoubleClicked;
        }

    private:
        bool _isDoubleClicked{ false };
        std::unique_ptr<fheroes::ImageRestorer> _listBackground;
    };

    void FileInfoListBox::RedrawItem( const Maps::FileInfo & info, int32_t posX, int32_t posY, bool current )
    {
        std::string mapFileName( System::GetFileName( info.filename ) );
        assert( !mapFileName.empty() );

        const fheroes::FontType font = current ? fheroes::FontType::normalYellow() : fheroes::FontType::normalWhite();

        fheroes::Display & display = fheroes::Display::instance();

        fheroes::Text fileNameText( std::move( mapFileName ), font );
        fileNameText.fitToOneRow( maxFileNameWidth - 40 );
        fileNameText.draw( posX + 44, posY + 2, display );

        const uint32_t racesCountIcnIndex = static_cast<uint32_t>( Color::Count( info.kingdomColors ) + 19 );
        const fheroes::Sprite & racesCount = fheroes::AGG::GetICN( ICN::REQUESTS, racesCountIcnIndex );
        fheroes::Copy( racesCount, 0, 0, display, posX + 6, posY, racesCount.width(), racesCount.height() );

        const uint32_t mapSizeIcnIndex = static_cast<uint32_t>( info.width / Maps::SMALL + 25 );
        const fheroes::Sprite & mapSize = fheroes::AGG::GetICN( ICN::REQUESTS, mapSizeIcnIndex );
        fheroes::Copy( mapSize, 0, 0, display, posX + 25, posY, mapSize.width(), mapSize.height() );
    }

    void FileInfoListBox::RedrawBackground( const fheroes::Point & /* unused */ )
    {
        _listBackground->restore();
    }

    void FileInfoListBox::ActionCurrentUp()
    {
        // Do nothing.
    }

    void FileInfoListBox::ActionCurrentDn()
    {
        // Do nothing.
    }

    void FileInfoListBox::ActionListDoubleClick( Maps::FileInfo & /*unused*/ )
    {
        _isDoubleClicked = true;
    }

    void FileInfoListBox::ActionListSingleClick( Maps::FileInfo & /*unused*/ )
    {
        // Do nothing.
    }
}

namespace Editor
{
    bool mapSaveSelectFile( std::string & fileName, std::string & mapName, const fheroes::SupportedLanguage language, const int32_t maxMapNameLength )
    {
        const CursorRestorer cursorRestorer( true, Cursor::POINTER );

        MapsFileInfoList lists = Maps::getResurrectionMapFileInfos( true, 0 );

        const int32_t listWidth = maxFileNameWidth + 9;
        const int32_t listHeightDeduction = 112 + 17;
        const int32_t listAreaOffsetY = 6;
        const int32_t listAreaHeightDeduction = 8;

        // If we don't have many map files, we reduce the maximum dialog height,
        // but not less than enough for 11 elements. We also limit the maximum list height to 22 lines.
        const int32_t listLineHeight = 2 + fheroes::getFontHeight( fheroes::FontSize::NORMAL );
        const int32_t estraDialogHeight = listAreaOffsetY + listAreaHeightDeduction + listHeightDeduction;
        const int32_t maxDialogHeight = listLineHeight * std::clamp( static_cast<int32_t>( lists.size() ), 11, 22 ) + estraDialogHeight;

        fheroes::Display & display = fheroes::Display::instance();

        const int32_t listItems = ( std::min( display.height() - 100, maxDialogHeight ) - estraDialogHeight ) / listLineHeight;
        const int32_t dialogHeight = listLineHeight * listItems + estraDialogHeight;

        // Dialog height is also capped with the current screen height.
        fheroes::StandardWindow background( listWidth + 75, dialogHeight, true, display );

        const fheroes::Rect area( background.activeArea() );
        const fheroes::Rect listRoi( area.x + 24, area.y + 37 + 17, listWidth, area.height - listHeightDeduction );
        const fheroes::Rect fileNameRoi( listRoi.x + 4, listRoi.y + listRoi.height + 14, maxFileNameWidth, 21 );

        const fheroes::Text header( _( "Save Map:" ), fheroes::FontType::normalYellow() );
        header.draw( area.x + ( area.width - header.width() ) / 2, area.y + 10, display );

        if ( fileName.empty() ) {
            fileName = "My map";
        }

        if ( mapName.empty() ) {
            mapName = fileName;
        }

        fheroes::Text mapNameText( mapName, fheroes::FontType::normalWhite(), language );
        const fheroes::Rect mapNameRoi( listRoi.x, area.y + 28, listRoi.width, mapNameText.height() + 4 );

        background.applyTextBackgroundShading( mapNameRoi );
        fheroes::ImageRestorer mapNameBackground( display, mapNameRoi.x, mapNameRoi.y, mapNameRoi.width, mapNameRoi.height );

        const int32_t maxMapNameTextWidth = mapNameRoi.width - 6;
        mapNameText.fitToOneRow( maxMapNameTextWidth );
        mapNameText.drawInRoi( mapNameRoi.x, mapNameRoi.y + 4, mapNameRoi.width, display, mapNameRoi );

        background.applyTextBackgroundShading( { listRoi.x, listRoi.y, maxFileNameWidth + 8, listRoi.height } );
        background.applyTextBackgroundShading( { listRoi.x, fileNameRoi.y - 2, maxFileNameWidth + 8, fileNameRoi.height } );

        // Prepare OKAY and CANCEL buttons and render their shadows.
        fheroes::Button buttonOk;
        fheroes::Button buttonCancel;

        background.renderOkayCancelButtons( buttonOk, buttonCancel );

        FileInfoListBox listbox( area.getPosition() );

        // Initialize list background restorer to use it in list method 'listbox.RedrawBackground()'.
        listbox.initListBackgroundRestorer( listRoi );

        listbox.SetAreaItems( { listRoi.x, listRoi.y + listAreaOffsetY, listRoi.width, listRoi.height - listAreaHeightDeduction } );

        const bool isEvilInterface = Settings::Get().isEvilInterfaceEnabled();

        int32_t scrollbarOffsetX = area.x + area.width - 35;
        background.renderScrollbarBackground( { scrollbarOffsetX, listRoi.y, listRoi.width, listRoi.height }, isEvilInterface );

        const int listIcnId = isEvilInterface ? ICN::SCROLLE : ICN::SCROLL;
        const int32_t topPartHeight = 19;
        ++scrollbarOffsetX;

        listbox.SetScrollButtonUp( listIcnId, 0, 1, { scrollbarOffsetX, listRoi.y + 1 } );
        listbox.SetScrollButtonDn( listIcnId, 2, 3, { scrollbarOffsetX, listRoi.y + listRoi.height - 15 } );
        listbox.setScrollBarArea( { scrollbarOffsetX + 2, listRoi.y + topPartHeight, 10, listRoi.height - 2 * topPartHeight } );
        listbox.setScrollBarImage( fheroes::AGG::GetICN( listIcnId, 4 ) );
        listbox.SetAreaMaxItems( listItems );
        listbox.SetListContent( lists );
        listbox.updateScrollBarImage();

        size_t charInsertPos = 0;

        charInsertPos = fileName.size();

        MapsFileInfoList::iterator it = lists.begin();
        for ( ; it != lists.end(); ++it ) {
            if ( System::GetStem( ( *it ).filename ) == fileName ) {
                break;
            }
        }

        if ( it != lists.end() ) {
            listbox.SetCurrent( std::distance( lists.begin(), it ) );
        }
        else {
            listbox.Unselect();
        }

        auto buttonOkDisabler = [&buttonOk, &fileName]() {
            if ( fileName.empty() && buttonOk.isEnabled() ) {
                buttonOk.disable();
                buttonOk.draw();
            }
            else if ( !fileName.empty() && buttonOk.isDisabled() ) {
                buttonOk.enable();
                buttonOk.draw();
            }
        };

        listbox.Redraw();

        fheroes::TextInputField textInput( fileNameRoi, false, false, display );
        textInput.draw( fileName + mapFileExtension, static_cast<int32_t>( charInsertPos ) );

        // Render a button to open the Virtual Keyboard window.
        const int buttonVirtualKBIcnID = isEvilInterface ? ICN::BUTTON_VIRTUAL_KEYBOARD_EVIL : ICN::BUTTON_VIRTUAL_KEYBOARD_GOOD;
        fheroes::Button buttonVirtualKB;
        background.renderButton( buttonVirtualKB, buttonVirtualKBIcnID, 0, 1, { 0, 7 }, fheroes::StandardWindow::Padding::BOTTOM_CENTER );

        display.render( background.totalArea() );

        const size_t maxFileNameLength = 255;
        std::string lastSelectedSaveFileName;

        const bool isInGameKeyboardRequired = System::isVirtualKeyboardSupported();

        LocalEvent & le = LocalEvent::Get();

        while ( le.HandleEvents() ) {
            buttonOk.drawOnState( le.isMouseLeftButtonPressedAndHeldInArea( buttonOk.area() ) );
            buttonCancel.drawOnState( le.isMouseLeftButtonPressedAndHeldInArea( buttonCancel.area() ) );
            buttonVirtualKB.drawOnState( le.isMouseLeftButtonPressedAndHeldInArea( buttonVirtualKB.area() ) );

            if ( le.MouseClickLeft( buttonCancel.area() ) || Game::HotKeyPressEvent( Game::HotKeyEvent::DEFAULT_CANCEL ) ) {
                return false;
            }

            if ( buttonOk.isEnabled()
                 && ( le.MouseClickLeft( buttonOk.area() ) || Game::HotKeyPressEvent( Game::HotKeyEvent::DEFAULT_OKAY ) || listbox.isDoubleClicked() ) ) {
                return true;
            }

            const int listId = listbox.getCurrentId();

            const bool listboxEvent = listbox.QueueEventProcessing();

            bool isListboxSelected = listbox.isSelected();

            bool needFileNameRedraw = ( listId != listbox.getCurrentId() );

            if ( le.MouseClickLeft( buttonVirtualKB.area() ) || ( isInGameKeyboardRequired && le.MouseClickLeft( fileNameRoi ) ) ) {
                {
                    // TODO: allow to use other languages once we add support of filesystem language support.
                    const fheroes::LanguageSwitcher switcher( fheroes::SupportedLanguage::English );
                    fheroes::openVirtualKeyboard( fileName, maxFileNameLength );
                }

                charInsertPos = fileName.size();
                listbox.Unselect();
                isListboxSelected = false;
                needFileNameRedraw = true;

                buttonOkDisabler();

                // Set the whole screen to redraw next time to properly restore image under the Virtual Keyboard dialog.
                display.updateNextRenderRoi( { 0, 0, display.width(), display.height() } );
            }
            else if ( !fileName.empty() && le.MouseClickLeft( fileNameRoi ) ) {
                // Do not allow to put cursor at the file extension part.
                const size_t newPos = std::min( textInput.getCursorInTextPosition( le.getMouseLeftButtonPressedPos() ), fileName.size() );

                if ( charInsertPos != newPos || isListboxSelected ) {
                    charInsertPos = newPos;

                    listbox.Unselect();
                    isListboxSelected = false;
                    needFileNameRedraw = true;
                }
            }
            else if ( le.MouseClickLeft( mapNameRoi ) ) {
                std::string editableMapName = mapName;
                const fheroes::Text body{ _( "Change Map Name" ), fheroes::FontType::normalWhite() };
                if ( Dialog::inputString( fheroes::Text{}, body, editableMapName, maxMapNameLength, false, language ) ) {
                    if ( editableMapName.empty() ) {
                        // Map should have a non empty name.
                        continue;
                    }

                    mapName = std::move( editableMapName );
                    mapNameText.set( mapName, fheroes::FontType::normalWhite(), language );
                    mapNameBackground.restore();
                    mapNameText.fitToOneRow( maxMapNameTextWidth );
                    mapNameText.drawInRoi( mapNameRoi.x, mapNameRoi.y + 4, mapNameRoi.width, display, mapNameRoi );

                    display.render( mapNameRoi );

                    continue;
                }
            }
            else if ( isListboxSelected && le.isKeyPressed( fheroes::Key::KEY_DELETE ) ) {
                listbox.SetCurrent( listId );
                listbox.Redraw();

                std::string msg( _( "Are you sure you want to delete file:" ) );
                msg.append( "\n\n" );
                msg.append( System::GetFileName( listbox.GetCurrent().filename ) );

                if ( Dialog::YES == fheroes::showStandardTextMessage( _( "Warning" ), msg, Dialog::YES | Dialog::NO ) ) {
                    if ( !System::Unlink( listbox.GetCurrent().filename ) ) {
                        ERROR_LOG( "Unable to delete file " << listbox.GetCurrent().filename )
                    }

                    listbox.RemoveSelected();

                    if ( lists.empty() ) {
                        isListboxSelected = false;
                        charInsertPos = 0;
                        fileName.clear();

                        buttonOk.disable();
                        buttonOk.draw();
                    }

                    listbox.updateScrollBarImage();
                    listbox.SetCurrent( std::max( listId - 1, 0 ) );
                }

                needFileNameRedraw = true;
            }
            else if ( !listboxEvent && le.isAnyKeyPressed() ) {
                const fheroes::Key pressedKey = le.getPressedKeyValue();
                if ( ( fileName.size() < maxFileNameLength || pressedKey == fheroes::Key::KEY_BACKSPACE || pressedKey == fheroes::Key::KEY_DELETE )
                     && pressedKey != fheroes::Key::KEY_UP && pressedKey != fheroes::Key::KEY_DOWN ) {
                    charInsertPos = InsertKeySym( fileName, charInsertPos, pressedKey, LocalEvent::getCurrentKeyModifiers() );

                    buttonOkDisabler();

                    listbox.Unselect();
                    isListboxSelected = false;
                    needFileNameRedraw = true;
                }
            }

            if ( le.isMouseRightButtonPressedInArea( buttonCancel.area() ) ) {
                fheroes::showStandardTextMessage( _( "Cancel" ), _( "Exit this menu without doing anything." ), Dialog::ZERO );
            }
            else if ( le.isMouseRightButtonPressedInArea( buttonOk.area() ) ) {
                fheroes::showStandardTextMessage( _( "Okay" ), _( "Click to save the current map." ), Dialog::ZERO );
            }
            else if ( le.isMouseRightButtonPressedInArea( buttonVirtualKB.area() ) ) {
                fheroes::showStandardTextMessage( _( "Open Virtual Keyboard" ), _( "Click to open the Virtual Keyboard dialog." ), Dialog::ZERO );
            }
            else if ( le.isMouseRightButtonPressedInArea( mapNameRoi ) ) {
                fheroes::MultiFontText message;
                message.add( fheroes::Text{ "\"", fheroes::FontType::normalWhite() } );
                message.add( fheroes::Text{ mapName, fheroes::FontType::normalWhite(), language } );
                message.add( fheroes::Text{ "\"\n\n", fheroes::FontType::normalWhite() } );
                message.add( fheroes::Text{ _( "Click to change your map name." ), fheroes::FontType::normalWhite() } );

                fheroes::showMessage( fheroes::Text{ _( "Map Name" ), fheroes::FontType::normalYellow() }, message, Dialog::ZERO );
            }

            const bool needRedrawListbox = listbox.IsNeedRedraw();

            // Text input blinking cursor render is done when the render of the filename (with cursor) is not planned.
            if ( !needFileNameRedraw && textInput.eventProcessing() ) {
                display.render( textInput.getCursorArea() );
            }

            if ( !needFileNameRedraw && !needRedrawListbox ) {
                continue;
            }

            if ( needFileNameRedraw ) {
                if ( isListboxSelected ) {
                    const std::string selectedFileName = System::GetStem( listbox.GetCurrent().filename );
                    if ( lastSelectedSaveFileName != selectedFileName ) {
                        lastSelectedSaveFileName = selectedFileName;
                        fileName = selectedFileName;
                        charInsertPos = fileName.size();

                        buttonOkDisabler();
                    }
                }
                else {
                    // Empty last selected map file name so that we can replace the input field's name if we select the same map file again.
                    lastSelectedSaveFileName.clear();
                }

                textInput.draw( fileName + mapFileExtension, static_cast<int32_t>( charInsertPos ) );
            }

            if ( needRedrawListbox ) {
                listbox.Redraw();
                display.render( area );
            }
            else {
                display.render( textInput.getOverallArea() );
            }
        }

        return false;
    }
}
