/***************************************************************************
 *   fheroes: https://github.com/ihhub/fheroes                           *
 *   Copyright (C) 2020 - 2025                                             *
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

#include "dialog_resolution.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "agg_image.h"
#include "cursor.h"
#include "dialog.h"
#include "embedded_image.h"
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
#include "ui_dialog.h"
#include "ui_text.h"
#include "ui_window.h"
#include "zzlib.h"

namespace
{
    const int32_t textAreaWidth = 270;
    const int32_t scrollBarAreaWidth = 48;
    const int32_t paddingLeftSide = 24;
    const int32_t resolutionItemHeight = 19;
    // TODO: this is a hack over partially incorrect text height calculation. Fix it later together with the Text classes.
    const int32_t textOffsetYCorrection = 6;
    const std::string middleText = " x ";

    std::pair<std::string, std::string> getResolutionStrings( const fheroes::ResolutionInfo & resolution )
    {
        if ( resolution.screenWidth != resolution.gameWidth && resolution.screenHeight != resolution.gameHeight ) {
            const int32_t integer = resolution.screenWidth / resolution.gameWidth;
            const int32_t fraction = resolution.screenWidth * 10 / resolution.gameWidth - 10 * integer;

            return std::make_pair( std::to_string( resolution.gameWidth ),
                                   std::to_string( resolution.gameHeight ) + " (x" + std::to_string( integer ) + "." + std::to_string( fraction ) + ')' );
        }

        return std::make_pair( std::to_string( resolution.gameWidth ), std::to_string( resolution.gameHeight ) );
    }

    class ResolutionList : public Interface::ListBox<fheroes::ResolutionInfo>
    {
    public:
        using Interface::ListBox<fheroes::ResolutionInfo>::ActionListSingleClick;
        using Interface::ListBox<fheroes::ResolutionInfo>::ActionListPressRight;
        using Interface::ListBox<fheroes::ResolutionInfo>::ActionListDoubleClick;

        explicit ResolutionList( const fheroes::Point & offset )
            : Interface::ListBox<fheroes::ResolutionInfo>( offset )
            , _isDoubleClicked( false )
        {
            // Do nothing.
        }

        void RedrawItem( const fheroes::ResolutionInfo & resolution, int32_t offsetX, int32_t offsetY, bool current ) override
        {
            const fheroes::FontType fontType = current ? fheroes::FontType::normalYellow() : fheroes::FontType::normalWhite();

            const auto [leftText, rightText] = getResolutionStrings( resolution );
            const int32_t middleTextSize = fheroes::Text( middleText, fontType ).width();
            const int32_t leftTextSize = fheroes::Text( leftText, fontType ).width();

            const fheroes::Text resolutionText( leftText + middleText + rightText, fontType );

            const int32_t textOffsetX = offsetX + textAreaWidth / 2 - leftTextSize - middleTextSize / 2;
            const int32_t textOffsetY = offsetY + ( resolutionItemHeight - resolutionText.height() + textOffsetYCorrection ) / 2;

            resolutionText.draw( textOffsetX, textOffsetY, fheroes::Display::instance() );
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

        void ActionListSingleClick( fheroes::ResolutionInfo & /*unused*/ ) override
        {
            // Do nothing.
        }

        void ActionListPressRight( fheroes::ResolutionInfo & resolution ) override
        {
            const auto [leftText, rightText] = getResolutionStrings( resolution );

            if ( resolution.gameHeight == resolution.screenHeight && resolution.gameWidth == resolution.screenWidth ) {
                fheroes::showStandardTextMessage( leftText + middleText + rightText, _( "Select this game resolution." ), Dialog::ZERO );
            }
            else {
                const int32_t integer = resolution.screenWidth / resolution.gameWidth;
                const int32_t fraction = resolution.screenWidth * 10 / resolution.gameWidth - 10 * integer;

                std::string scaledResolutionText = _(
                    "Selecting this resolution will set a resolution that is scaled from the original resolution (%{resolution}) by multiplying it with the number "
                    "in the brackets (%{scale}).\n\nA resolution with a clean integer number (2.0x, 3.0x etc.) will usually look better because the pixels are upscaled "
                    "evenly in both horizontal and vertical directions." );

                StringReplace( scaledResolutionText, "%{resolution}", std::to_string( resolution.gameWidth ) + middleText + std::to_string( resolution.gameHeight ) );
                StringReplace( scaledResolutionText, "%{scale}", std::to_string( integer ) + "." + std::to_string( fraction ) );

                fheroes::showStandardTextMessage( leftText + middleText + rightText, scaledResolutionText, Dialog::ZERO );
            }
        }

        void ActionListDoubleClick( fheroes::ResolutionInfo & /*unused*/ ) override
        {
            _isDoubleClicked = true;
        }

        bool isDoubleClicked() const
        {
            return _isDoubleClicked;
        }

        void initListBackgroundRestorer( fheroes::Rect roi )
        {
            _listBackground = std::make_unique<fheroes::ImageRestorer>( fheroes::Display::instance(), roi.x, roi.y, roi.width, roi.height );
        }

    private:
        bool _isDoubleClicked;
        std::unique_ptr<fheroes::ImageRestorer> _listBackground;
    };

    void RedrawInfo( const fheroes::Point & dst, const fheroes::ResolutionInfo & resolution, fheroes::Image & output )
    {
        if ( resolution.gameWidth > 0 && resolution.gameHeight > 0 ) {
            const fheroes::FontType fontType = fheroes::FontType::normalYellow();

            const auto [leftText, rightText] = getResolutionStrings( resolution );
            const int32_t middleTextSize = fheroes::Text( middleText, fontType ).width();
            const int32_t leftTextSize = fheroes::Text( leftText, fontType ).width();

            const int32_t textOffsetX = dst.x + textAreaWidth / 2 - leftTextSize - middleTextSize / 2;

            const fheroes::Text resolutionText( leftText + middleText + rightText, fontType );
            resolutionText.draw( textOffsetX, dst.y + ( resolutionItemHeight - resolutionText.height() + textOffsetYCorrection ) / 2, output );
        }
    }

    fheroes::ResolutionInfo getNewResolution()
    {
        // setup cursor
        const CursorRestorer cursorRestorer( true, Cursor::POINTER );

        const int32_t listHeightDeduction = 112;
        const int32_t listAreaOffsetY = 3;
        const int32_t listAreaHeightDeduction = 4;

        std::vector<fheroes::ResolutionInfo> resolutions = fheroes::engine().getAvailableResolutions();

        // If we don't have many resolutions, we reduce the maximum dialog height,
        // but not less than enough for 11 elements.
        // We also limit the maximum list height to 22 lines.
        const int32_t maxDialogHeight = fheroes::getFontHeight( fheroes::FontSize::NORMAL ) * std::clamp( static_cast<int32_t>( resolutions.size() ), 11, 22 )
                                        + listAreaOffsetY + listAreaHeightDeduction + listHeightDeduction;

        fheroes::Display & display = fheroes::Display::instance();

        // Dialog height is also capped with the current screen height.
        fheroes::StandardWindow background( paddingLeftSide + textAreaWidth + scrollBarAreaWidth + 3, std::min( display.height() - 100, maxDialogHeight ), true,
                                             display );

        const fheroes::Rect roi( background.activeArea() );
        const fheroes::Rect listRoi( roi.x + paddingLeftSide, roi.y + 37, textAreaWidth, roi.height - listHeightDeduction );

        // We divide the list: resolution list and selected resolution.
        const fheroes::Rect selectedResRoi( listRoi.x, listRoi.y + listRoi.height + 12, listRoi.width, 21 );
        background.applyTextBackgroundShading( selectedResRoi );
        background.applyTextBackgroundShading( { listRoi.x, listRoi.y, listRoi.width, listRoi.height } );

        fheroes::ImageRestorer selectedResBackground( fheroes::Display::instance(), selectedResRoi.x, selectedResRoi.y, listRoi.width, selectedResRoi.height );

        // Prepare OKAY and CANCEL buttons and render their shadows.
        fheroes::Button buttonOk;
        fheroes::Button buttonCancel;
        background.renderOkayCancelButtons( buttonOk, buttonCancel );

        ResolutionList listBox( roi.getPosition() );

        listBox.initListBackgroundRestorer( listRoi );

        listBox.SetAreaItems( { listRoi.x, listRoi.y + 3, listRoi.width - 3, listRoi.height - 4 } );

        const bool isEvilInterface = Settings::Get().isEvilInterfaceEnabled();

        int32_t scrollbarOffsetX = roi.x + roi.width - 35;
        background.renderScrollbarBackground( { scrollbarOffsetX, listRoi.y, listRoi.width, listRoi.height }, isEvilInterface );

        const int32_t topPartHeight = 19;
        ++scrollbarOffsetX;

        const int listIcnId = isEvilInterface ? ICN::SCROLLE : ICN::SCROLL;
        listBox.SetScrollButtonUp( listIcnId, 0, 1, { scrollbarOffsetX, listRoi.y + 1 } );
        listBox.SetScrollButtonDn( listIcnId, 2, 3, { scrollbarOffsetX, listRoi.y + listRoi.height - 15 } );
        listBox.setScrollBarArea( { scrollbarOffsetX + 2, listRoi.y + topPartHeight, 10, listRoi.height - 2 * topPartHeight } );
        listBox.setScrollBarImage( fheroes::AGG::GetICN( listIcnId, 4 ) );
        listBox.SetAreaMaxItems( ( listRoi.height - 7 ) / fheroes::getFontHeight( fheroes::FontSize::NORMAL ) );
        listBox.SetListContent( resolutions );
        listBox.updateScrollBarImage();

        const fheroes::ResolutionInfo currentResolution{ display.width(), display.height(), display.screenSize().width, display.screenSize().height };

        fheroes::ResolutionInfo selectedResolution;
        for ( size_t i = 0; i < resolutions.size(); ++i ) {
            if ( resolutions[i] == currentResolution ) {
                listBox.SetCurrent( i );
                selectedResolution = listBox.GetCurrent();
                break;
            }
        }

        listBox.Redraw();

        RedrawInfo( selectedResRoi.getPosition(), selectedResolution, display );

        const fheroes::Text title( _( "Select Game Resolution:" ), fheroes::FontType::normalYellow() );
        title.draw( roi.x + ( roi.width - title.width() ) / 2, roi.y + 16, display );

        display.render( background.totalArea() );

        LocalEvent & le = LocalEvent::Get();
        while ( le.HandleEvents() ) {
            buttonOk.drawOnState( le.isMouseLeftButtonPressedAndHeldInArea( buttonOk.area() ) );
            buttonCancel.drawOnState( le.isMouseLeftButtonPressedAndHeldInArea( buttonCancel.area() ) );

            const int listId = listBox.getCurrentId();
            listBox.QueueEventProcessing();
            const bool needRedraw = listId != listBox.getCurrentId();

            if ( ( buttonOk.isEnabled() && le.MouseClickLeft( buttonOk.area() ) ) || Game::HotKeyPressEvent( Game::HotKeyEvent::DEFAULT_OKAY )
                 || listBox.isDoubleClicked() ) {
                if ( listBox.isSelected() ) {
                    break;
                }
            }
            else if ( le.MouseClickLeft( buttonCancel.area() ) || Game::HotKeyPressEvent( Game::HotKeyEvent::DEFAULT_CANCEL ) ) {
                selectedResolution = {};
                break;
            }

            if ( le.isMouseRightButtonPressedInArea( buttonCancel.area() ) ) {
                fheroes::showStandardTextMessage( _( "Cancel" ), _( "Exit this menu without doing anything." ), Dialog::ZERO );
            }
            else if ( le.isMouseRightButtonPressedInArea( buttonOk.area() ) ) {
                fheroes::showStandardTextMessage( _( "Okay" ), _( "Click to apply the selected resolution." ), Dialog::ZERO );
            }

            if ( !listBox.IsNeedRedraw() ) {
                continue;
            }

            if ( needRedraw ) {
                selectedResolution = listBox.GetCurrent();
                selectedResBackground.restore();
                RedrawInfo( selectedResRoi.getPosition(), selectedResolution, display );
            }

            listBox.Redraw();
            display.render( roi );
        }

        return selectedResolution;
    }
}

namespace Dialog
{
    bool SelectResolution()
    {
        const fheroes::ResolutionInfo selectedResolution = getNewResolution();
        if ( selectedResolution == fheroes::ResolutionInfo{} ) {
            return false;
        }

        fheroes::Display & display = fheroes::Display::instance();
        const fheroes::ResolutionInfo currentResolution{ display.width(), display.height(), display.screenSize().width, display.screenSize().height };

        if ( selectedResolution.gameWidth > 0 && selectedResolution.gameHeight > 0 && selectedResolution.screenWidth >= selectedResolution.gameWidth
             && selectedResolution.screenHeight >= selectedResolution.gameHeight && selectedResolution != currentResolution ) {
            display.setResolution( selectedResolution );

#if !defined( MACOS_APP_BUNDLE )
            const fheroes::Image & appIcon = Compression::CreateImageFromZlib( 32, 32, iconImage, sizeof( iconImage ), true );
            fheroes::engine().setIcon( appIcon );
#endif

            return true;
        }

        return false;
    }
}
