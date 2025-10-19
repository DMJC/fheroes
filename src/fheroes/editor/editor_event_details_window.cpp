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

#include "editor_event_details_window.h"

#include <array>
#include <cassert>
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "agg_image.h"
#include "artifact.h"
#include "cursor.h"
#include "dialog.h"
#include "dialog_selectitems.h"
#include "editor_ui_helper.h"
#include "game_hotkeys.h"
#include "heroes.h"
#include "icn.h"
#include "image.h"
#include "localevent.h"
#include "map_format_info.h"
#include "math_base.h"
#include "mp2.h"
#include "resource.h"
#include "screen.h"
#include "settings.h"
#include "skill.h"
#include "spell.h"
#include "tools.h"
#include "translations.h"
#include "ui_button.h"
#include "ui_dialog.h"
#include "ui_text.h"
#include "ui_tool.h"
#include "ui_window.h"

namespace
{
    const std::array<int, 7> resourceTypes = { Resource::WOOD, Resource::SULFUR, Resource::CRYSTAL, Resource::MERCURY, Resource::ORE, Resource::GEMS, Resource::GOLD };

    const int32_t elementOffset{ 9 };
    const int32_t sectionWidth{ ( fheroes::Display::DEFAULT_WIDTH - elementOffset * 3 ) / 2 };
    const fheroes::Size messageArea{ sectionWidth, 207 };
}

namespace Editor
{
    bool eventDetailsDialog( Maps::Map_Format::AdventureMapEventMetadata & eventMetadata, const PlayerColorsSet humanPlayerColors,
                             const PlayerColorsSet computerPlayerColors, const fheroes::SupportedLanguage language )
    {
        // First, make sure that the event has proper player colors according to the map specification.
        eventMetadata.humanPlayerColors = eventMetadata.humanPlayerColors & humanPlayerColors;
        eventMetadata.computerPlayerColors = eventMetadata.computerPlayerColors & computerPlayerColors;

        const CursorRestorer cursorRestorer( true, Cursor::POINTER );

        fheroes::Display & display = fheroes::Display::instance();

        const bool isDefaultScreenSize = display.isDefaultSize();

        fheroes::StandardWindow background( sectionWidth * 2 + elementOffset * 3, fheroes::Display::DEFAULT_HEIGHT, !isDefaultScreenSize, display );
        const fheroes::Rect dialogRoi = background.activeArea();

        const bool isEvilInterface = Settings::Get().isEvilInterfaceEnabled();

        if ( isDefaultScreenSize ) {
            const fheroes::Sprite & backgroundImage = fheroes::AGG::GetICN( isEvilInterface ? ICN::STONEBAK_EVIL : ICN::STONEBAK, 0 );
            fheroes::Copy( backgroundImage, 0, 0, display, dialogRoi );
        }

        int32_t offsetY = dialogRoi.y + elementOffset;

        const fheroes::Text title( MP2::StringObject( MP2::OBJ_EVENT ), fheroes::FontType::normalYellow() );
        title.draw( dialogRoi.x + ( dialogRoi.width - title.width() ) / 2, offsetY, display );

        offsetY += title.height() + elementOffset;
        const int32_t startOffsetY = offsetY;

        fheroes::Text text{ _( "Message Text:" ), fheroes::FontType::normalWhite() };

        const fheroes::Rect messageRoi{ dialogRoi.x + elementOffset, offsetY + text.height(), messageArea.width, messageArea.height };
        background.applyTextBackgroundShading( messageRoi );

        fheroes::ImageRestorer messageRoiRestorer( display, messageRoi.x, messageRoi.y, messageRoi.width, messageRoi.height );

        text.draw( messageRoi.x + ( messageRoi.width - text.width() ) / 2, offsetY, display );

        text.set( eventMetadata.message, fheroes::FontType::normalWhite(), language );
        text.drawInRoi( messageRoi.x + 5, messageRoi.y + 5, messageRoi.width - 10, display, messageRoi );

        offsetY = startOffsetY;
        const int32_t secondColumnOffsetX = dialogRoi.x + sectionWidth + 2 * elementOffset;

        text.set( _( "Effects:" ), fheroes::FontType::normalWhite() );
        text.draw( secondColumnOffsetX + ( sectionWidth - text.width() ) / 2, offsetY, display );

        offsetY += text.height( sectionWidth );

        // Resources
        const fheroes::Rect resourceRoi{ secondColumnOffsetX, offsetY, sectionWidth, 99 };
        background.applyTextBackgroundShading( resourceRoi );

        fheroes::ImageRestorer resourceRoiRestorer( display, resourceRoi.x, resourceRoi.y, resourceRoi.width, resourceRoi.height );

        std::array<fheroes::Rect, 7> individualResourceRoi;
        renderResources( eventMetadata.resources, resourceRoi, display, individualResourceRoi );

        // Artifact
        offsetY += resourceRoi.height + elementOffset;

        auto artifactUI = std::make_unique<fheroes::ArtifactDialogElement>( eventMetadata.artifact );
        const fheroes::Rect artifactRoi{ secondColumnOffsetX, offsetY, artifactUI->area().width, artifactUI->area().height };

        artifactUI->draw( display, artifactRoi.getPosition() );

        const int minibuttonIcnId = isEvilInterface ? ICN::CELLWIN_EVIL : ICN::CELLWIN;

        const fheroes::Sprite & buttonImage = fheroes::AGG::GetICN( minibuttonIcnId, 17 );
        const int32_t buttonWidth = buttonImage.width();

        fheroes::Button buttonDeleteArtifact( artifactRoi.x + ( artifactRoi.width - buttonWidth ) / 2, artifactRoi.y + artifactRoi.height + 5, minibuttonIcnId, 17, 18 );
        buttonDeleteArtifact.draw();

        // Experience
        auto experienceUI = std::make_unique<fheroes::ExperienceDialogElement>( 0 );
        const fheroes::Rect experienceRoi{ secondColumnOffsetX + sectionWidth - experienceUI->area().width,
                                            offsetY + ( artifactRoi.height - experienceUI->area().height ) / 2, experienceUI->area().width, experienceUI->area().height };

        const fheroes::Rect experienceValueRoi{ experienceRoi.x, buttonDeleteArtifact.area().y, experienceRoi.width, buttonDeleteArtifact.area().height };
        background.applyTextBackgroundShading( experienceValueRoi );

        fheroes::ImageRestorer experienceRoiRestorer( display, experienceRoi.x, experienceRoi.y, experienceRoi.width,
                                                       experienceValueRoi.y + experienceValueRoi.height - experienceRoi.y );

        experienceUI->draw( display, experienceRoi.getPosition() );
        text.set( std::to_string( eventMetadata.experience ), fheroes::FontType::smallWhite() );
        text.draw( experienceValueRoi.x + ( experienceValueRoi.width - text.width() ) / 2, experienceValueRoi.y + 5, display );

        // Secondary Skill
        const Heroes fakeHero;
        auto secondarySkillUI
            = std::make_unique<fheroes::SecondarySkillDialogElement>( Skill::Secondary{ eventMetadata.secondarySkill, eventMetadata.secondarySkillLevel }, fakeHero );

        const fheroes::Rect secondarySkillRoi{ ( artifactRoi.x + artifactRoi.width + experienceRoi.x - secondarySkillUI->area().width ) / 2,
                                                offsetY + ( artifactRoi.height - secondarySkillUI->area().height ) / 2, secondarySkillUI->area().width,
                                                secondarySkillUI->area().height };

        secondarySkillUI->draw( display, secondarySkillRoi.getPosition() );

        fheroes::Button buttonDeleteSecondarySkill( secondarySkillRoi.x + ( secondarySkillRoi.width - buttonWidth ) / 2, artifactRoi.y + artifactRoi.height + 5,
                                                     minibuttonIcnId, 17, 18 );
        buttonDeleteSecondarySkill.draw();

        // Conditions
        offsetY = messageRoi.y + messageRoi.height + 2 * elementOffset;

        text.set( _( "Conditions:" ), fheroes::FontType::normalWhite() );
        text.draw( dialogRoi.x + ( dialogRoi.width - text.width() ) / 2, offsetY, display );

        offsetY += text.height() + elementOffset;
        const int32_t conditionSectionOffsetY = offsetY;

        const fheroes::Point recurringEventPos{ messageRoi.x + elementOffset, offsetY };

        fheroes::MovableSprite recurringEventCheckbox;
        const fheroes::Rect recurringEventArea = drawCheckboxWithText( recurringEventCheckbox, _( "Cancel event after first visit" ), display, recurringEventPos.x,
                                                                        recurringEventPos.y, isEvilInterface, dialogRoi.width - 2 * elementOffset );
        if ( eventMetadata.isRecurringEvent ) {
            recurringEventCheckbox.hide();
        }
        else {
            recurringEventCheckbox.show();
        }

        offsetY = conditionSectionOffsetY;

        text.set( _( "Player colors allowed to get event:" ), fheroes::FontType::normalWhite() );

        int32_t textWidth = sectionWidth;
        // If the text fits on one line, make it span two lines.
        while ( text.rows( textWidth ) < 2 ) {
            textWidth = textWidth * 2 / 3;
        }

        text.draw( secondColumnOffsetX + ( sectionWidth - textWidth ) / 2, offsetY, textWidth, display );

        const int32_t availablePlayersCount = Color::Count( humanPlayerColors | computerPlayerColors );
        const int32_t checkOffX = ( sectionWidth - availablePlayersCount * 32 ) / 2;

        offsetY += 3 + text.height( textWidth );
        std::vector<std::unique_ptr<Checkbox>> humanCheckboxes;
        createColorCheckboxes( humanCheckboxes, humanPlayerColors, eventMetadata.humanPlayerColors, secondColumnOffsetX + checkOffX, offsetY, display );

        assert( humanCheckboxes.size() == static_cast<size_t>( Color::Count( humanPlayerColors ) ) );

        offsetY += 35;

        text.set( _( "Computer colors allowed to get the event:" ), fheroes::FontType::normalWhite() );

        textWidth = sectionWidth;

        // If the text fits on one line, make it span two lines.
        while ( text.rows( textWidth ) < 2 ) {
            textWidth = textWidth * 2 / 3;
        }

        text.draw( secondColumnOffsetX + ( sectionWidth - textWidth ) / 2, offsetY, textWidth, display );

        offsetY += 3 + text.height( textWidth );
        std::vector<std::unique_ptr<Checkbox>> computerCheckboxes;
        createColorCheckboxes( computerCheckboxes, computerPlayerColors, eventMetadata.computerPlayerColors, secondColumnOffsetX + checkOffX, offsetY, display );

        assert( computerCheckboxes.size() == static_cast<size_t>( Color::Count( computerPlayerColors ) ) );

        // Window buttons
        fheroes::Button buttonOk;
        fheroes::Button buttonCancel;

        background.renderOkayCancelButtons( buttonOk, buttonCancel );

        display.render( background.totalArea() );

        bool isRedrawNeeded = false;

        LocalEvent & le = LocalEvent::Get();
        while ( le.HandleEvents() ) {
            buttonOk.drawOnState( le.isMouseLeftButtonPressedAndHeldInArea( buttonOk.area() ) );
            buttonCancel.drawOnState( le.isMouseLeftButtonPressedAndHeldInArea( buttonCancel.area() ) );
            buttonDeleteArtifact.drawOnState( le.isMouseLeftButtonPressedAndHeldInArea( buttonDeleteArtifact.area() ) );
            buttonDeleteSecondarySkill.drawOnState( le.isMouseLeftButtonPressedAndHeldInArea( buttonDeleteSecondarySkill.area() ) );

            if ( le.MouseClickLeft( buttonCancel.area() ) || Game::HotKeyPressEvent( Game::HotKeyEvent::DEFAULT_CANCEL ) ) {
                return false;
            }

            if ( buttonOk.isEnabled() && ( le.MouseClickLeft( buttonOk.area() ) || Game::HotKeyPressEvent( Game::HotKeyEvent::DEFAULT_OKAY ) ) ) {
                break;
            }

            for ( const auto & humanCheckbox : humanCheckboxes ) {
                const fheroes::Rect & checkboxRect = humanCheckbox->getRect();

                if ( le.MouseClickLeft( checkboxRect ) ) {
                    const PlayerColor color = humanCheckbox->getColor();
                    if ( humanCheckbox->toggle() ) {
                        eventMetadata.humanPlayerColors |= color;
                    }
                    else {
                        eventMetadata.humanPlayerColors ^= color;
                    }

                    break;
                }

                if ( le.isMouseRightButtonPressedInArea( checkboxRect ) ) {
                    std::string header = _( "Allow %{color} human player to get event" );
                    std::string messageText = _( "If this checkbox is checked, this event will trigger for the %{color} player if they are controlled by a human." );

                    const std::string colorString = Color::String( humanCheckbox->getColor() );
                    StringReplace( header, "%{color}", colorString );
                    StringReplace( messageText, "%{color}", colorString );

                    fheroes::showStandardTextMessage( std::move( header ), std::move( messageText ), Dialog::ZERO );

                    break;
                }
            }

            for ( const auto & computerCheckbox : computerCheckboxes ) {
                const fheroes::Rect & checkboxRect = computerCheckbox->getRect();

                if ( le.MouseClickLeft( checkboxRect ) ) {
                    const PlayerColor color = computerCheckbox->getColor();
                    if ( computerCheckbox->toggle() ) {
                        eventMetadata.computerPlayerColors |= color;
                    }
                    else {
                        eventMetadata.computerPlayerColors ^= color;
                    }

                    break;
                }

                if ( le.isMouseRightButtonPressedInArea( checkboxRect ) ) {
                    std::string header = _( "Allow %{color} computer player to get event" );
                    std::string messageText = _( "If this checkbox is checked, this event will trigger for the %{color} player if they are controlled by a computer." );

                    const std::string colorString = Color::String( computerCheckbox->getColor() );
                    StringReplace( header, "%{color}", colorString );
                    StringReplace( messageText, "%{color}", colorString );

                    fheroes::showStandardTextMessage( std::move( header ), std::move( messageText ), Dialog::ZERO );

                    break;
                }
            }

            for ( size_t i = 0; i < individualResourceRoi.size(); ++i ) {
                if ( le.MouseClickLeft( individualResourceRoi[i] ) ) {
                    const int resourceType = resourceTypes[i];
                    int32_t * resourcePtr = eventMetadata.resources.GetPtr( resourceType );
                    assert( resourcePtr != nullptr );

                    int32_t temp = *resourcePtr;

                    const fheroes::ResourceDialogElement resourceUI( resourceType, {} );

                    std::string message = _( "Set %{resource-type} Count" );
                    StringReplace( message, "%{resource-type}", Resource::String( resourceType ) );

                    if ( Dialog::SelectCount( std::move( message ), -99999, 999999, temp, 1, &resourceUI ) ) {
                        *resourcePtr = temp;
                    }

                    resourceRoiRestorer.restore();

                    renderResources( eventMetadata.resources, resourceRoi, display, individualResourceRoi );
                    display.render( resourceRoi );
                    break;
                }
            }

            if ( le.MouseClickLeft( messageRoi ) ) {
                std::string temp = eventMetadata.message;

                const fheroes::Text body{ _( "Message:" ), fheroes::FontType::normalWhite() };
                if ( Dialog::inputString( fheroes::Text{}, body, temp, 200, true, language ) ) {
                    eventMetadata.message = std::move( temp );

                    messageRoiRestorer.restore();
                    text.set( eventMetadata.message, fheroes::FontType::normalWhite(), language );
                    text.drawInRoi( messageRoi.x + 5, messageRoi.y + 5, messageRoi.width - 10, display, messageRoi );
                    isRedrawNeeded = true;
                }
            }
            else if ( le.MouseClickLeft( recurringEventArea ) ) {
                eventMetadata.isRecurringEvent = !eventMetadata.isRecurringEvent;
                eventMetadata.isRecurringEvent ? recurringEventCheckbox.hide() : recurringEventCheckbox.show();
                display.render( recurringEventCheckbox.getArea() );
            }
            else if ( le.MouseClickLeft( artifactRoi ) ) {
                const Artifact artifact = Dialog::selectArtifact( eventMetadata.artifact, false );
                if ( artifact.isValid() ) {
                    int32_t artifactMetadata = eventMetadata.artifactMetadata;

                    if ( artifact.GetID() == Artifact::SPELL_SCROLL ) {
                        artifactMetadata = Dialog::selectSpell( artifactMetadata, true ).GetID();

                        if ( artifactMetadata == Spell::NONE ) {
                            // No spell for the Spell Scroll artifact was selected - cancel the artifact selection.
                            continue;
                        }
                    }
                    else {
                        artifactMetadata = 0;
                    }

                    eventMetadata.artifact = artifact.GetID();
                    eventMetadata.artifactMetadata = artifactMetadata;

                    artifactUI = std::make_unique<fheroes::ArtifactDialogElement>( eventMetadata.artifact );
                    artifactUI->draw( display, artifactRoi.getPosition() );
                }

                // The opened selectArtifact() dialog might be bigger than this dialog so we render the whole screen.
                display.render();
            }
            else if ( le.MouseClickLeft( buttonDeleteArtifact.area() ) ) {
                eventMetadata.artifact = 0;
                eventMetadata.artifactMetadata = 0;

                const fheroes::Sprite & artifactImage = fheroes::AGG::GetICN( ICN::ARTIFACT, Artifact( eventMetadata.artifact ).IndexSprite64() );
                fheroes::Copy( artifactImage, 0, 0, display, artifactRoi.x + 6, artifactRoi.y + 6, artifactImage.width(), artifactImage.height() );

                display.render( artifactRoi );
            }
            else if ( le.MouseClickLeft( secondarySkillRoi ) ) {
                const Skill::Secondary skill = Dialog::selectSecondarySkill( fakeHero, eventMetadata.secondarySkill * 3 + eventMetadata.secondarySkillLevel );
                if ( skill.isValid() ) {
                    eventMetadata.secondarySkill = static_cast<uint8_t>( skill.Skill() );
                    eventMetadata.secondarySkillLevel = static_cast<uint8_t>( skill.Level() );

                    secondarySkillUI = std::make_unique<fheroes::SecondarySkillDialogElement>( skill, fakeHero );

                    secondarySkillUI->draw( display, secondarySkillRoi.getPosition() );
                }

                // The opened selectSecondarySkill() dialog might be bigger than this dialog so we render the whole screen.
                display.render();
            }
            else if ( le.MouseClickLeft( buttonDeleteSecondarySkill.area() ) ) {
                eventMetadata.secondarySkill = 0;
                eventMetadata.secondarySkillLevel = 0;

                secondarySkillUI = std::make_unique<fheroes::SecondarySkillDialogElement>( Skill::Secondary{}, fakeHero );
                secondarySkillUI->draw( display, secondarySkillRoi.getPosition() );

                display.render( secondarySkillRoi );
            }
            else if ( le.MouseClickLeft( experienceRoiRestorer.rect() ) ) {
                const fheroes::ExperienceDialogElement tempExperienceUI{ 0 };
                int32_t tempValue{ eventMetadata.experience };

                if ( Dialog::SelectCount( _( "Set Experience value" ), 0, static_cast<int32_t>( Heroes::getExperienceMaxValue() ), tempValue, 1, &tempExperienceUI ) ) {
                    eventMetadata.experience = tempValue;

                    experienceRoiRestorer.restore();
                    experienceUI->draw( display, experienceRoi.getPosition() );
                    text.set( std::to_string( eventMetadata.experience ), fheroes::FontType::smallWhite() );
                    text.draw( experienceValueRoi.x + ( experienceValueRoi.width - text.width() ) / 2, experienceValueRoi.y + 5, display );
                }

                // The opened SelectCount() dialog might be bigger than this dialog so we render the whole screen.
                display.render();
            }
            else if ( le.isMouseRightButtonPressedInArea( buttonCancel.area() ) ) {
                fheroes::showStandardTextMessage( _( "Cancel" ), _( "Exit this menu without doing anything." ), Dialog::ZERO );
            }
            else if ( le.isMouseRightButtonPressedInArea( buttonOk.area() ) ) {
                fheroes::showStandardTextMessage( _( "Okay" ), _( "Click to save the Event properties." ), Dialog::ZERO );
            }
            else if ( le.isMouseRightButtonPressedInArea( artifactRoi ) ) {
                // Since Artifact class does not allow to set a random spell (for obvious reasons),
                // we have to use special UI code to render the popup window with all needed information.
                const Artifact artifact( eventMetadata.artifact );

                if ( artifact.isValid() ) {
                    artifactUI->showPopup( Dialog::ZERO );
                }
                else {
                    fheroes::showStandardTextMessage( _( "Artifact" ), _( "No artifact will be given by the event." ), Dialog::ZERO );
                }
            }
            else if ( le.isMouseRightButtonPressedInArea( buttonDeleteArtifact.area() ) ) {
                fheroes::showStandardTextMessage( _( "Delete Artifact" ), _( "Delete an artifact from the event." ), Dialog::ZERO );
            }
            else if ( le.isMouseRightButtonPressedInArea( secondarySkillRoi ) ) {
                const Skill::Secondary skill( eventMetadata.secondarySkill, eventMetadata.secondarySkillLevel );

                if ( skill.isValid() ) {
                    secondarySkillUI->showPopup( Dialog::ZERO );
                }
                else {
                    fheroes::showStandardTextMessage( _( "Secondary Skill" ), _( "No Secondary Skill will be given by the event." ), Dialog::ZERO );
                }
            }
            else if ( le.isMouseRightButtonPressedInArea( buttonDeleteSecondarySkill.area() ) ) {
                fheroes::showStandardTextMessage( _( "Delete Secondary Skill" ), _( "Delete the Secondary Skill from the event." ), Dialog::ZERO );
            }
            else if ( le.isMouseRightButtonPressedInArea( experienceRoiRestorer.rect() ) ) {
                experienceUI->showPopup( Dialog::ZERO );
            }
            else if ( le.isMouseRightButtonPressedInArea( resourceRoi ) ) {
                if ( eventMetadata.resources.GetValidItemsCount() == 0 ) {
                    fheroes::showStandardTextMessage( _( "Resources" ), _( "No resources will be part of this event." ), Dialog::ZERO );
                }
                else {
                    fheroes::showResourceMessage( fheroes::Text( _( "Resources" ), fheroes::FontType::normalYellow() ),
                                                   fheroes::Text{ _( "These resources will be a part of the event." ), fheroes::FontType::normalWhite() }, Dialog::ZERO,
                                                   eventMetadata.resources );
                }
            }
            else if ( le.isMouseRightButtonPressedInArea( recurringEventArea ) ) {
                fheroes::showStandardTextMessage(
                    _( "Cancel event after first visit" ),
                    _( "If this checkbox is checked, the event will trigger only once. If not checked, the event will trigger every time one of the specified players crosses the event tile." ),
                    Dialog::ZERO );
            }
            else if ( le.isMouseRightButtonPressedInArea( messageRoi ) ) {
                fheroes::showStandardTextMessage( _( "Event Message Text" ), _( "Click here to change the event message." ), Dialog::ZERO );
            }

            if ( isRedrawNeeded ) {
                isRedrawNeeded = false;

                display.render( dialogRoi );
            }
        }

        return true;
    }
}
