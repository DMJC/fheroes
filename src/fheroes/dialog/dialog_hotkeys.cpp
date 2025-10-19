/***************************************************************************
 *   fheroes: https://github.com/ihhub/fheroes                           *
 *   Copyright (C) 2022 - 2025                                             *
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

#include "dialog_hotkeys.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "agg_image.h"
#include "cursor.h"
#include "dialog.h"
#include "game_hotkeys.h"
#include "icn.h"
#include "image.h"
#include "interface_list.h"
#include "localevent.h"
#include "math_base.h"
#include "screen.h"
#include "settings.h"
#include "tools.h"
#include "translations.h"
#include "ui_button.h"
#include "ui_constants.h"
#include "ui_dialog.h"
#include "ui_text.h"
#include "ui_window.h"

namespace
{
    const int32_t keyDescriptionLength = 300;
    const int32_t hotKeyLength = 120;

    class HotKeyElement : public fheroes::DialogElement
    {
    public:
        HotKeyElement( const fheroes::Key key, fheroes::Image & output )
            : _restorer( output, 0, 0, 0, 0 )
            , _key( key )
        {
            // Text always occupies the whole width of the dialog.
            _area = { fheroes::boxAreaWidthPx, fheroes::getFontHeight( fheroes::FontSize::NORMAL ) };
        }

        ~HotKeyElement() override = default;

        void draw( fheroes::Image & output, const fheroes::Point & offset ) const override
        {
            _restorer.restore();

            fheroes::MultiFontText text;
            text.add( fheroes::Text{ _( "Hotkey: " ), fheroes::FontType::normalYellow() } );
            text.add( fheroes::Text{ StringUpper( KeySymGetName( _key ) ), fheroes::FontType::normalWhite() } );

            _restorer.update( offset.x, offset.y, fheroes::boxAreaWidthPx, text.height( fheroes::boxAreaWidthPx ) );

            text.draw( offset.x, offset.y, fheroes::boxAreaWidthPx, output );
        }

        void processEvents( const fheroes::Point & /*offset*/ ) const override
        {
            const LocalEvent & le = LocalEvent::Get();
            if ( le.isAnyKeyPressed() ) {
                _key = le.getPressedKeyValue();
                _keyChanged = true;
            }
        }

        // Never call this method as a custom image has nothing to popup.
        void showPopup( const int /*buttons*/ ) const override
        {
            assert( 0 );
        }

        bool update( fheroes::Image & output, const fheroes::Point & offset ) const override
        {
            if ( _keyChanged ) {
                _keyChanged = false;
                draw( output, offset );
                return true;
            }

            return false;
        }

        void reset()
        {
            _restorer.reset();
        }

        fheroes::Key getKey() const
        {
            return _key;
        }

    private:
        mutable fheroes::ImageRestorer _restorer;
        mutable fheroes::Key _key;
        mutable bool _keyChanged{ false };
    };

    class HotKeyList : public Interface::ListBox<std::pair<Game::HotKeyEvent, Game::HotKeyCategory>>
    {
    public:
        using Interface::ListBox<std::pair<Game::HotKeyEvent, Game::HotKeyCategory>>::ListBox;

        using Interface::ListBox<std::pair<Game::HotKeyEvent, Game::HotKeyCategory>>::ActionListSingleClick;
        using Interface::ListBox<std::pair<Game::HotKeyEvent, Game::HotKeyCategory>>::ActionListPressRight;
        using Interface::ListBox<std::pair<Game::HotKeyEvent, Game::HotKeyCategory>>::ActionListDoubleClick;

        void RedrawItem( const std::pair<Game::HotKeyEvent, Game::HotKeyCategory> & hotKeyEvent, int32_t offsetX, int32_t offsetY, bool current ) override
        {
            fheroes::Display & display = fheroes::Display::instance();

            const fheroes::FontType fontType = current ? fheroes::FontType::normalYellow() : fheroes::FontType::normalWhite();

            offsetY += 2;

            fheroes::Text name( _( Game::getHotKeyEventNameByEventId( hotKeyEvent.first ) ), fontType );
            name.fitToOneRow( keyDescriptionLength );
            name.draw( offsetX + 4, offsetY, display );

            fheroes::Text hotkey( Game::getHotKeyNameByEventId( hotKeyEvent.first ), fontType );
            hotkey.fitToOneRow( hotKeyLength );
            hotkey.draw( offsetX + keyDescriptionLength + 9, offsetY, hotKeyLength, display );
        }

        void RedrawBackground( const fheroes::Point & /* unused */ ) override
        {
            _listBackground->restore();
        }

        void ActionCurrentUp() override
        {
            // Do nothing.
        }

        void ActionCurrentDn() override
        {
            // Do nothing.
        }

        void ActionListSingleClick( std::pair<Game::HotKeyEvent, Game::HotKeyCategory> & /*unused*/ ) override
        {
            // Do nothing.
        }

        void ActionListPressRight( std::pair<Game::HotKeyEvent, Game::HotKeyCategory> & hotKeyEvent ) override
        {
            fheroes::MultiFontText title;

            title.add( fheroes::Text{ _( "Category: " ), fheroes::FontType::normalYellow() } );
            title.add( fheroes::Text{ _( Game::getHotKeyCategoryName( hotKeyEvent.second ) ), fheroes::FontType::normalWhite() } );
            title.add( fheroes::Text{ "\n\n", fheroes::FontType::normalWhite() } );
            title.add( fheroes::Text{ _( "Event: " ), fheroes::FontType::normalYellow() } );
            title.add( fheroes::Text{ _( Game::getHotKeyEventNameByEventId( hotKeyEvent.first ) ), fheroes::FontType::normalWhite() } );
            title.add( fheroes::Text{ "\n\n", fheroes::FontType::normalWhite() } );
            title.add( fheroes::Text{ _( "Hotkey: " ), fheroes::FontType::normalYellow() } );
            title.add( fheroes::Text{ Game::getHotKeyNameByEventId( hotKeyEvent.first ), fheroes::FontType::normalWhite() } );

            fheroes::showMessage( fheroes::Text{}, title, Dialog::ZERO );
        }

        void ActionListDoubleClick( std::pair<Game::HotKeyEvent, Game::HotKeyCategory> & hotKeyEvent ) override
        {
            HotKeyElement hotKeyUI( Game::getHotKeyForEvent( hotKeyEvent.first ), fheroes::Display::instance() );

            // Okay and Cancel events are special cases as they are used in dialogs. By default we need to disable these events to allow to be set any key for an event.
            // Global events (that work on all screens) must be disabled as well.
            const fheroes::Key okayEventKey = Game::getHotKeyForEvent( Game::HotKeyEvent::DEFAULT_OKAY );
            const fheroes::Key cancelEventKey = Game::getHotKeyForEvent( Game::HotKeyEvent::DEFAULT_CANCEL );
            const fheroes::Key fullscreenEventKey = Game::getHotKeyForEvent( Game::HotKeyEvent::GLOBAL_TOGGLE_FULLSCREEN );
            const fheroes::Key textSupportModeEventKey = Game::getHotKeyForEvent( Game::HotKeyEvent::GLOBAL_TOGGLE_TEXT_SUPPORT_MODE );

            Game::setHotKeyForEvent( Game::HotKeyEvent::DEFAULT_OKAY, fheroes::Key::NONE );
            Game::setHotKeyForEvent( Game::HotKeyEvent::DEFAULT_CANCEL, fheroes::Key::NONE );
            Game::setHotKeyForEvent( Game::HotKeyEvent::GLOBAL_TOGGLE_FULLSCREEN, fheroes::Key::NONE );
            Game::setHotKeyForEvent( Game::HotKeyEvent::GLOBAL_TOGGLE_TEXT_SUPPORT_MODE, fheroes::Key::NONE );

            fheroes::MultiFontText title;

            title.add( fheroes::Text{ _( "Category: " ), fheroes::FontType::normalYellow() } );
            title.add( fheroes::Text{ _( Game::getHotKeyCategoryName( hotKeyEvent.second ) ), fheroes::FontType::normalWhite() } );
            title.add( fheroes::Text{ "\n\n", fheroes::FontType::normalWhite() } );
            title.add( fheroes::Text{ _( "Event: " ), fheroes::FontType::normalYellow() } );
            title.add( fheroes::Text{ _( Game::getHotKeyEventNameByEventId( hotKeyEvent.first ) ), fheroes::FontType::normalWhite() } );

            const int returnValue = fheroes::showMessage( fheroes::Text{}, title, Dialog::OK | Dialog::CANCEL, { &hotKeyUI } );

            Game::setHotKeyForEvent( Game::HotKeyEvent::DEFAULT_OKAY, okayEventKey );
            Game::setHotKeyForEvent( Game::HotKeyEvent::DEFAULT_CANCEL, cancelEventKey );
            Game::setHotKeyForEvent( Game::HotKeyEvent::GLOBAL_TOGGLE_FULLSCREEN, fullscreenEventKey );
            Game::setHotKeyForEvent( Game::HotKeyEvent::GLOBAL_TOGGLE_TEXT_SUPPORT_MODE, textSupportModeEventKey );

            // To avoid UI issues we need to reset restorer manually.
            hotKeyUI.reset();

            if ( returnValue == Dialog::CANCEL ) {
                return;
            }

            Game::setHotKeyForEvent( hotKeyEvent.first, hotKeyUI.getKey() );
            Game::HotKeySave();
        }

        void initListBackgroundRestorer( fheroes::Rect roi )
        {
            _listBackground = std::make_unique<fheroes::ImageRestorer>( fheroes::Display::instance(), roi.x, roi.y, roi.width, roi.height );
        }

    private:
        std::unique_ptr<fheroes::ImageRestorer> _listBackground;
    };
}

namespace fheroes
{
    void openHotkeysDialog()
    {
        // Setup cursor.
        const CursorRestorer cursorRestorer( true, ::Cursor::POINTER );

        fheroes::Display & display = fheroes::Display::instance();

        // Dialog height is capped with the current screen height minus 100 pixels.
        fheroes::StandardWindow background( keyDescriptionLength + hotKeyLength + 8 + 75, std::min( display.height() - 100, 520 ), true, display );

        const fheroes::Rect roi( background.activeArea() );
        const fheroes::Rect listRoi( roi.x + 24, roi.y + 37, keyDescriptionLength + hotKeyLength + 8, roi.height - 75 );

        const fheroes::Text title( _( "Hot Keys:" ), fheroes::FontType::normalYellow() );
        title.draw( roi.x + ( roi.width - title.width() ) / 2, roi.y + 16, display );

        // We divide the list: action description and binded hot-key.
        background.applyTextBackgroundShading( { listRoi.x, listRoi.y, keyDescriptionLength + 8, listRoi.height } );
        background.applyTextBackgroundShading( { listRoi.x + keyDescriptionLength + 8, listRoi.y, hotKeyLength, listRoi.height } );

        const bool isEvilInterface = Settings::Get().isEvilInterfaceEnabled();

        // Prepare OKAY button and render its shadow.
        fheroes::Button buttonOk;
        const int buttonOkIcn = isEvilInterface ? ICN::BUTTON_SMALL_OKAY_EVIL : ICN::BUTTON_SMALL_OKAY_GOOD;
        background.renderButton( buttonOk, buttonOkIcn, 0, 1, { 0, 7 }, StandardWindow::Padding::BOTTOM_CENTER );

        HotKeyList listbox( roi.getPosition() );
        listbox.initListBackgroundRestorer( listRoi );
        listbox.SetAreaItems( { listRoi.x, listRoi.y + 3, listRoi.width - 3, listRoi.height - 4 } );

        int32_t scrollbarOffsetX = roi.x + roi.width - 35;
        background.renderScrollbarBackground( { scrollbarOffsetX, listRoi.y, listRoi.width, listRoi.height }, isEvilInterface );

        const int32_t topPartHeight = 19;
        const int listIcnId = isEvilInterface ? ICN::SCROLLE : ICN::SCROLL;
        ++scrollbarOffsetX;

        listbox.SetScrollButtonUp( listIcnId, 0, 1, { scrollbarOffsetX, listRoi.y + 1 } );
        listbox.SetScrollButtonDn( listIcnId, 2, 3, { scrollbarOffsetX, listRoi.y + listRoi.height - 15 } );
        listbox.setScrollBarArea( { scrollbarOffsetX + 2, listRoi.y + topPartHeight, 10, listRoi.height - 2 * topPartHeight } );
        listbox.setScrollBarImage( fheroes::AGG::GetICN( listIcnId, 4 ) );
        listbox.SetAreaMaxItems( ( listRoi.height - 7 ) / fheroes::getFontHeight( fheroes::FontSize::NORMAL ) );

        std::vector<std::pair<Game::HotKeyEvent, Game::HotKeyCategory>> hotKeyEvents = Game::getAllHotKeyEvents();

        listbox.SetListContent( hotKeyEvents );
        listbox.updateScrollBarImage();
        listbox.Redraw();

        display.render( background.totalArea() );

        LocalEvent & le = LocalEvent::Get();
        while ( le.HandleEvents() ) {
            buttonOk.drawOnState( le.isMouseLeftButtonPressedAndHeldInArea( buttonOk.area() ) );

            listbox.QueueEventProcessing();

            if ( le.MouseClickLeft( buttonOk.area() ) || Game::HotKeyCloseWindow() ) {
                return;
            }

            if ( le.isMouseRightButtonPressedInArea( buttonOk.area() ) ) {
                fheroes::showStandardTextMessage( _( "Okay" ), _( "Exit this menu." ), Dialog::ZERO );

                continue;
            }

            if ( !listbox.IsNeedRedraw() ) {
                continue;
            }

            listbox.Redraw();
            display.render( roi );
        }
    }
}
