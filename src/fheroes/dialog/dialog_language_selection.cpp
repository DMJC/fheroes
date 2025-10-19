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

#include "dialog_language_selection.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <utility>

#include "agg_image.h"
#include "cursor.h"
#include "dialog.h"
#include "game_hotkeys.h"
#include "game_language.h"
#include "icn.h"
#include "image.h"
#include "interface_list.h"
#include "localevent.h"
#include "math_base.h"
#include "screen.h"
#include "settings.h"
#include "translations.h"
#include "ui_button.h"
#include "ui_dialog.h"
#include "ui_language.h"
#include "ui_text.h"
#include "ui_window.h"

namespace
{
    const int32_t verticalPaddingAreasHight = 30;
    const int32_t textAreaWidth = 270;
    const int32_t scrollBarAreaWidth = 48;
    const int32_t paddingLeftSide = 24;

    class LanguageList final : public Interface::ListBox<fheroes::SupportedLanguage>
    {
    public:
        using Interface::ListBox<fheroes::SupportedLanguage>::ActionListSingleClick;
        using Interface::ListBox<fheroes::SupportedLanguage>::ActionListPressRight;
        using Interface::ListBox<fheroes::SupportedLanguage>::ActionListDoubleClick;

        explicit LanguageList( const fheroes::Point & offset )
            : Interface::ListBox<fheroes::SupportedLanguage>( offset )
            , _isDoubleClicked( false )
        {
            // Do nothing.
        }

        void RedrawItem( const fheroes::SupportedLanguage & language, int32_t offsetX, int32_t offsetY, bool isSelected ) override
        {
            // This method is supposed to do rendering but since the language resource generation is too performance extensive,
            // we cache the information needed to render here and perform the actual rendering in the Redraw() method.
            _languagesRenderingInfo[fheroes::getCodePage( language )].emplace_back( language, offsetX, offsetY, isSelected );
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

        void ActionListSingleClick( fheroes::SupportedLanguage & /*language*/ ) override
        {
            // Do nothing.
        }

        void ActionListPressRight( fheroes::SupportedLanguage & /*language*/ ) override
        {
            // Do nothing.
        }

        void ActionListDoubleClick( fheroes::SupportedLanguage & /*language*/ ) override
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

        void Redraw() override
        {
            // Do not call this method!
            // Use the overloaded method below!
            assert( 0 );
        }

        void Redraw( const fheroes::SupportedLanguage mainLanguage )
        {
            // Since we are setting languages for each item to be rendered
            // we cannot afford to switch the language back to the current one using fheroes::LanguageSwitcher.
            // This is why we set language switcher here and then initiate real rendering.
            const fheroes::LanguageSwitcher languageSwitcher( mainLanguage );

            // Rendering of items in the base class is strictly in the top-to-bottom order.
            // However, this class cannot afford doing the same due to extensive calculations
            // for the language switching operation.
            // This is why we first check the location of each language,
            // then group them by code page,
            // and only afterward render all languages, minimizing the number of resource switching operations.
            _languagesRenderingInfo.clear();

            Interface::ListBox<fheroes::SupportedLanguage>::Redraw();

            // Now we can do the real rendering.
            fheroes::Display & display = fheroes::Display::instance();

            for ( const auto & [codePage, languages] : _languagesRenderingInfo ) {
                for ( const auto & info : languages ) {
                    Settings::Get().setGameLanguage( fheroes::getLanguageAbbreviation( info.language ) );

                    const fheroes::Text languageName( fheroes::getLanguageName( info.language ),
                                                       info.isSelected ? fheroes::FontType::normalYellow() : fheroes::FontType::normalWhite() );
                    languageName.draw( ( textAreaWidth - languageName.width() ) / 2 + info.offset.x, info.offset.y, display );
                }
            }
        }

    private:
        struct LanguageInfo
        {
            LanguageInfo() = delete;

            LanguageInfo( const fheroes::SupportedLanguage language_, int32_t offsetX, int32_t offsetY, bool isSelected_ )
                : language( language_ )
                , offset( offsetX, offsetY )
                , isSelected( isSelected_ )
            {
                // Do nothing.
            }

            fheroes::SupportedLanguage language{ fheroes::SupportedLanguage::English };

            fheroes::Point offset;

            bool isSelected{ false };
        };

        bool _isDoubleClicked;
        std::unique_ptr<fheroes::ImageRestorer> _listBackground;

        std::map<fheroes::CodePage, std::vector<LanguageInfo>> _languagesRenderingInfo;
    };

    void redrawDialogInfo( const fheroes::Rect & listRoi, const fheroes::SupportedLanguage language, const bool isGameLanguage )
    {
        fheroes::Display & display = fheroes::Display::instance();

        const fheroes::FontType fontType = fheroes::FontType::normalYellow();

        const fheroes::Text title( isGameLanguage ? _( "Select Game Language:" ) : _( "Select Language:" ), fontType );
        title.draw( listRoi.x + ( listRoi.width - title.width() ) / 2, listRoi.y - ( verticalPaddingAreasHight + title.height() + 2 ) / 2, display );

        const fheroes::LanguageSwitcher languageSwitcher( language );

        const fheroes::Text selectedLanguage( fheroes::getLanguageName( language ), fontType );

        selectedLanguage.draw( listRoi.x + ( listRoi.width - selectedLanguage.width() ) / 2, listRoi.y + listRoi.height + 12 + ( 21 - selectedLanguage.height() ) / 2 + 2,
                               display );
    }

    bool getLanguage( const std::vector<fheroes::SupportedLanguage> & languages, fheroes::SupportedLanguage & chosenLanguage, const bool isGameLanguage )
    {
        // setup cursor
        const CursorRestorer cursorRestorer( true, Cursor::POINTER );

        const int32_t listHeightDeduction = 112;
        const int32_t listAreaOffsetY = 3;
        const int32_t listAreaHeightDeduction = 4;

        // If we don't have many languages, we reduce the maximum dialog height,
        // but not less than enough for 11 elements.
        // We also limit the maximum list height to 22 lines.
        const int32_t maxDialogHeight = fheroes::getFontHeight( fheroes::FontSize::NORMAL ) * std::clamp( static_cast<int32_t>( languages.size() ), 11, 22 )
                                        + listAreaOffsetY + listAreaHeightDeduction + listHeightDeduction;

        fheroes::Display & display = fheroes::Display::instance();

        // Dialog height is also capped with the current screen height.
        fheroes::StandardWindow background( paddingLeftSide + textAreaWidth + scrollBarAreaWidth + 3, std::min( display.height() - 100, maxDialogHeight ), true,
                                             display );

        const fheroes::Rect roi( background.activeArea() );
        const fheroes::Rect listRoi( roi.x + paddingLeftSide, roi.y + 37, textAreaWidth, roi.height - listHeightDeduction );

        // We divide the list: language list and selected language.
        const fheroes::Rect selectedLangRoi( listRoi.x, listRoi.y + listRoi.height + 12, listRoi.width, 21 );
        background.applyTextBackgroundShading( selectedLangRoi );
        background.applyTextBackgroundShading( { listRoi.x, listRoi.y, listRoi.width, listRoi.height } );

        fheroes::ImageRestorer titleBackground( fheroes::Display::instance(), roi.x, listRoi.y - verticalPaddingAreasHight, roi.width, verticalPaddingAreasHight );
        fheroes::ImageRestorer selectedLangBackground( fheroes::Display::instance(), selectedLangRoi.x, selectedLangRoi.y, listRoi.width, selectedLangRoi.height );
        fheroes::ImageRestorer buttonsBackground( fheroes::Display::instance(), roi.x, selectedLangRoi.y + selectedLangRoi.height + 10, roi.width,
                                                   verticalPaddingAreasHight );

        LanguageList listBox( roi.getPosition() );

        listBox.initListBackgroundRestorer( listRoi );

        Settings & conf = Settings::Get();

        // Prepare OKAY and CANCEL buttons and render their shadows.
        fheroes::Button buttonOk;
        fheroes::Button buttonCancel;
        background.renderOkayCancelButtons( buttonOk, buttonCancel );

        listBox.SetAreaItems( { listRoi.x, listRoi.y + 3, listRoi.width - 3, listRoi.height - 4 } );

        const bool isEvilInterface = conf.isEvilInterfaceEnabled();

        int32_t scrollbarOffsetX = roi.x + roi.width - 35;
        background.renderScrollbarBackground( { scrollbarOffsetX, listRoi.y, listRoi.width, listRoi.height }, isEvilInterface );

        const int listIcnId = isEvilInterface ? ICN::SCROLLE : ICN::SCROLL;
        const int32_t topPartHeight = 19;
        ++scrollbarOffsetX;

        listBox.SetScrollButtonUp( listIcnId, 0, 1, { scrollbarOffsetX, listRoi.y + 1 } );
        listBox.SetScrollButtonDn( listIcnId, 2, 3, { scrollbarOffsetX, listRoi.y + listRoi.height - 15 } );
        listBox.setScrollBarArea( { scrollbarOffsetX + 2, listRoi.y + topPartHeight, 10, listRoi.height - 2 * topPartHeight } );
        listBox.setScrollBarImage( fheroes::AGG::GetICN( listIcnId, 4 ) );
        listBox.SetAreaMaxItems( ( listRoi.height - 7 ) / fheroes::getFontHeight( fheroes::FontSize::NORMAL ) );
        std::vector<fheroes::SupportedLanguage> temp = languages;
        listBox.SetListContent( temp );
        listBox.updateScrollBarImage();

        for ( size_t i = 0; i < languages.size(); ++i ) {
            if ( languages[i] == chosenLanguage ) {
                listBox.SetCurrent( i );
                break;
            }
        }

        listBox.Redraw( chosenLanguage );

        redrawDialogInfo( listRoi, chosenLanguage, isGameLanguage );

        display.render( background.totalArea() );

        LocalEvent & le = LocalEvent::Get();
        while ( le.HandleEvents() ) {
            if ( buttonOk.isEnabled() ) {
                buttonOk.drawOnState( le.isMouseLeftButtonPressedAndHeldInArea( buttonOk.area() ) );
            }

            buttonCancel.drawOnState( le.isMouseLeftButtonPressedAndHeldInArea( buttonCancel.area() ) );

            if ( le.isMouseRightButtonPressedInArea( listRoi ) ) {
                continue;
            }

            const int listId = listBox.getCurrentId();
            listBox.QueueEventProcessing();
            const bool needRedraw = listId != listBox.getCurrentId();

            if ( ( buttonOk.isEnabled() && le.MouseClickLeft( buttonOk.area() ) ) || Game::HotKeyPressEvent( Game::HotKeyEvent::DEFAULT_OKAY )
                 || listBox.isDoubleClicked() ) {
                return true;
            }

            if ( le.MouseClickLeft( buttonCancel.area() ) || Game::HotKeyPressEvent( Game::HotKeyEvent::DEFAULT_CANCEL ) ) {
                return false;
            }

            if ( le.isMouseRightButtonPressedInArea( buttonCancel.area() ) ) {
                fheroes::showStandardTextMessage( _( "Cancel" ), _( "Exit this menu without doing anything." ), Dialog::ZERO );
            }
            else if ( le.isMouseRightButtonPressedInArea( buttonOk.area() ) ) {
                fheroes::showStandardTextMessage( _( "Okay" ), _( "Click to choose the selected language." ), Dialog::ZERO );
            }

            if ( !listBox.IsNeedRedraw() ) {
                continue;
            }

            if ( needRedraw ) {
                const fheroes::SupportedLanguage newChosenLanguage = listBox.GetCurrent();
                if ( newChosenLanguage != chosenLanguage ) {
                    chosenLanguage = newChosenLanguage;

                    if ( isGameLanguage ) {
                        conf.setGameLanguage( fheroes::getLanguageAbbreviation( chosenLanguage ) );
                    }

                    titleBackground.restore();
                    selectedLangBackground.restore();
                    redrawDialogInfo( listRoi, chosenLanguage, isGameLanguage );
                    buttonsBackground.restore();
                    background.renderOkayCancelButtons( buttonOk, buttonCancel );
                }
            }

            listBox.Redraw( chosenLanguage );
            display.render( roi );
        }

        return false;
    }
}

namespace fheroes
{
    SupportedLanguage selectLanguage( const std::vector<SupportedLanguage> & languages, const SupportedLanguage currentLanguage, const bool isGameLanguage )
    {
        if ( languages.empty() ) {
            // Why do you even call this function having 0 languages?
            assert( 0 );
            Settings::Get().setGameLanguage( fheroes::getLanguageAbbreviation( SupportedLanguage::English ) );
            return SupportedLanguage::English;
        }

        if ( languages.size() == 1 ) {
            Settings::Get().setGameLanguage( fheroes::getLanguageAbbreviation( languages.front() ) );
            return languages.front();
        }

        SupportedLanguage chosenLanguage = languages.front();
        for ( const SupportedLanguage language : languages ) {
            if ( currentLanguage == language ) {
                chosenLanguage = currentLanguage;
                break;
            }
        }

        if ( !getLanguage( languages, chosenLanguage, isGameLanguage ) ) {
            if ( isGameLanguage ) {
                Settings::Get().setGameLanguage( fheroes::getLanguageAbbreviation( currentLanguage ) );
            }

            return currentLanguage;
        }

        return chosenLanguage;
    }
}
