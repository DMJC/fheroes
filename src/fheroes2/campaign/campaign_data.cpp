/***************************************************************************
 *   fheroes2: https://github.com/ihhub/fheroes2                           *
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

#include "campaign_data.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <utility>

#include "artifact.h"
#include "color.h"
#include "game_video_type.h"
#include "heroes.h"
#include "maps_fileinfo.h"
#include "monster.h"
#include "resource.h"
#include "spell.h"
#include "tools.h"
#include "translations.h"

namespace
{
    const Campaign::VideoSequence emptyPlayback;

    std::vector<Campaign::CampaignAwardData> getRolandCampaignAwardData( const int scenarioID )
    {
        std::vector<Campaign::CampaignAwardData> obtainableAwards;

        switch ( scenarioID ) {
        case 2:
            obtainableAwards.emplace_back( 0, Campaign::CampaignAwardData::TYPE_CREATURE_ALLIANCE, Monster::DWARF, gettext_noop( "Dwarven Alliance" ) );
            break;
        case 5:
            obtainableAwards.emplace_back( 1, Campaign::CampaignAwardData::TYPE_HIREABLE_HERO, Heroes::ELIZA, 0, 0, gettext_noop( "Sorceress Guild" ) );
            break;
        case 6:
            obtainableAwards.emplace_back( 2, Campaign::CampaignAwardData::TYPE_CARRY_OVER_FORCES, 0, 0, 9 );
            break;
        case 7:
            obtainableAwards.emplace_back( 3, Campaign::CampaignAwardData::TYPE_GET_ARTIFACT, Artifact::ULTIMATE_CROWN, 1, 9 );
            break;
        case 8:
            obtainableAwards.emplace_back( 4, Campaign::CampaignAwardData::TYPE_DEFEAT_ENEMY_HERO, Heroes::CORLAGON, 0, 9 );
            break;
        default:
            break;
        }

        return obtainableAwards;
    }

    std::vector<Campaign::CampaignAwardData> getArchibaldCampaignAwardData( const int scenarioID )
    {
        std::vector<Campaign::CampaignAwardData> obtainableAwards;

        switch ( scenarioID ) {
        case 2:
            obtainableAwards.emplace_back( 1, Campaign::CampaignAwardData::TYPE_HIREABLE_HERO, Heroes::BRAX, 0, 0, gettext_noop( "Necromancer Guild" ) );
            break;
        case 3:
            obtainableAwards.emplace_back( 2, Campaign::CampaignAwardData::TYPE_CREATURE_ALLIANCE, Monster::OGRE, gettext_noop( "Ogre Alliance" ) );
            obtainableAwards.emplace_back( 3, Campaign::CampaignAwardData::TYPE_CREATURE_CURSE, Monster::DWARF, gettext_noop( "Dwarfbane" ) );
            break;
        case 6:
            obtainableAwards.emplace_back( 4, Campaign::CampaignAwardData::TYPE_CREATURE_ALLIANCE, Monster::GREEN_DRAGON, gettext_noop( "Dragon Alliance" ) );
            break;
        case 8:
            obtainableAwards.emplace_back( 5, Campaign::CampaignAwardData::TYPE_GET_ARTIFACT, Artifact::ULTIMATE_CROWN );
            break;
        case 9:
            obtainableAwards.emplace_back( 6, Campaign::CampaignAwardData::TYPE_CARRY_OVER_FORCES, 0 );
            break;
        default:
            break;
        }

        return obtainableAwards;
    }


    Campaign::CampaignData getRolandCampaignData()
    {
        const int scenarioCount = 11;

        const std::array<std::string, scenarioCount> scenarioName
            = { gettext_noop( "Force of Arms" ), gettext_noop( "Annexation" ),    gettext_noop( "Save the Dwarves" ),
                gettext_noop( "Carator Mines" ), gettext_noop( "Turning Point" ), gettext_noop( "scenarioName|Defender" ),
                gettext_noop( "The Gauntlet" ),  gettext_noop( "The Crown" ),     gettext_noop( "Corlagon's Defense" ),
                gettext_noop( "Final Justice" ), gettext_noop( "Betrayal" ) };
        const std::array<std::string, scenarioCount> scenarioDescription = {
            gettext_noop(
                "Roland needs you to defeat the lords near his castle to begin his war of rebellion against his brother. They are not allied with each other, so they will spend"
                " most of their time fighting with one another. Victory is yours when you have defeated all of their castles and heroes." ),
            gettext_noop(
                "The local lords refuse to swear allegiance to Roland, and must be subdued. They are wealthy and powerful, so be prepared for a tough fight. Capture all enemy castles to win." ),
            gettext_noop(
                "Your task is to defend the Dwarves against Archibald's forces. Capture all of the enemy towns and castles to win, and be sure not to lose all of the dwarf towns at once, or the enemy will have won." ),
            gettext_noop( "You will face four allied enemies in a straightforward fight for resources and treasure. Capture all of the enemy castles for victory." ),
            gettext_noop(
                "Your enemies are allied against you and start close by, so be ready to come out fighting. You will need to own all four castles in this small valley to win." ),
            gettext_noop(
                "The Sorceress' guild of Noraston has requested Roland's aid against an attack from Archibald's allies. Capture all of the enemy castles to win, and don't lose Noraston, or you'll lose the scenario. (Hint: There is an enemy castle on an island in the ocean.)" ),
            gettext_noop(
                "Gather as large an army as possible and capture the enemy castle within 8 weeks. You are opposed by only one enemy, but must travel a long way to get to the enemy castle. Any troops you have in your army at the end of this scenario will be with you in the final battle." ),
            gettext_noop( "Find the Crown before Archibald's heroes find it. Roland will need the Crown for the final battle against Archibald." ),
            gettext_noop(
                "Three allied enemies stand before you and victory, including Lord Corlagon. Roland is in a castle to the northwest, and you will lose if he falls to the enemy. Remember that capturing Lord Corlagon will ensure that he will not fight against you in the final scenario." ),
            gettext_noop( "This is the final battle. Both you and your enemy are armed to the teeth, and all are allied against you. Capture Archibald to end the war!" ),
            gettext_noop(
                "Switching sides leaves you with three castles against the enemy's one. This battle will be the easiest one you will face for the rest of the war...traitor." ) };
        std::vector<Campaign::ScenarioData> scenarioDatas;
        scenarioDatas.reserve( scenarioCount );

        std::vector<Campaign::ScenarioInfoId> scenarioInfo;
        scenarioInfo.reserve( scenarioCount );
        for ( int i = 0; i < scenarioCount; ++i ) {
            scenarioInfo.emplace_back( Campaign::ROLAND_CAMPAIGN, i );
        }

        scenarioDatas.emplace_back( scenarioInfo[0], std::vector{ scenarioInfo[1] }, "CAMPG01.H2C", scenarioName[0], scenarioDescription[0],
                                    Campaign::VideoSequence{
                                        { { "GOOD01.SMK", Video::VideoControl::PLAY_VIDEO_LOOP }, { "GOOD01V.SMK", Video::VideoControl::PLAY_CUTSCENE, { 348, 420 } } } },
                                    emptyPlayback );
        scenarioDatas
            .emplace_back( scenarioInfo[1], std::vector{ scenarioInfo[2], scenarioInfo[3] }, "CAMPG02.H2C", scenarioName[1], scenarioDescription[1],
                           Campaign::VideoSequence{ { { "GOOD02.SMK", Video::VideoControl::PLAY_VIDEO_LOOP }, { "GOOD02W.SMK", Video::VideoControl::PLAY_AUDIO } } },
                           Campaign::VideoSequence{ { { "GOOD03.SMK", Video::VideoControl::PLAY_VIDEO_LOOP }, { "GOOD03QW.SMK", Video::VideoControl::PLAY_AUDIO } } } );
        scenarioDatas.emplace_back( scenarioInfo[2], std::vector{ scenarioInfo[3] }, "CAMPG03.H2C", scenarioName[2], scenarioDescription[2], emptyPlayback,
                                    Campaign::VideoSequence{
                                        { { "GOOD04.SMK", Video::VideoControl::PLAY_VIDEO_LOOP }, { "GOOD04W.SMK", Video::VideoControl::PLAY_AUDIO } } },
                                    Campaign::ScenarioVictoryCondition::STANDARD, Campaign::ScenarioLossCondition::LOSE_ALL_SORCERESS_VILLAGES );
        scenarioDatas.emplace_back( scenarioInfo[3], std::vector{ scenarioInfo[4], scenarioInfo[10] }, "CAMPG04.H2C", scenarioName[3], scenarioDescription[3],
                                    emptyPlayback,
                                    Campaign::VideoSequence{ { { "GOOD05.SMK", Video::VideoControl::PLAY_VIDEO_LOOP },
                                                               { "GOOD05V.SMK", Video::VideoControl::PLAY_CUTSCENE, { 88, 345 } } } } );
        scenarioDatas.emplace_back( scenarioInfo[4], std::vector{ scenarioInfo[5] }, "CAMPG05.H2C", scenarioName[4], scenarioDescription[4], emptyPlayback,
                                    Campaign::VideoSequence{ { { "GOOD06.SMK", Video::VideoControl::PLAY_VIDEO_LOOP },
                                                               { "GOOD06AV.SMK", Video::VideoControl::PLAY_CUTSCENE, { 280, 186 } } } } );
        scenarioDatas.emplace_back( scenarioInfo[5], std::vector{ scenarioInfo[6], scenarioInfo[7] }, "CAMPG06.H2C", scenarioName[5], scenarioDescription[5],
                                    emptyPlayback,
                                    Campaign::VideoSequence{
                                        { { "GOOD07.SMK", Video::VideoControl::PLAY_VIDEO_LOOP }, { "GOOD07QW.SMK", Video::VideoControl::PLAY_AUDIO } } } );
        scenarioDatas.emplace_back( scenarioInfo[6], std::vector{ scenarioInfo[8] }, "CAMPG07.H2C", scenarioName[6], scenarioDescription[6], emptyPlayback,
                                    emptyPlayback );
        scenarioDatas.emplace_back( scenarioInfo[7], std::vector{ scenarioInfo[8] }, "CAMPG08.H2C", scenarioName[7], scenarioDescription[7], emptyPlayback, emptyPlayback,
                                    Campaign::ScenarioVictoryCondition::OBTAIN_ULTIMATE_CROWN );
        scenarioDatas.emplace_back( scenarioInfo[8], std::vector{ scenarioInfo[9] }, "CAMPG09.H2C", scenarioName[8], scenarioDescription[8],
                                    Campaign::VideoSequence{
                                        { { "GOOD09.SMK", Video::VideoControl::PLAY_VIDEO_LOOP }, { "GOOD09W.SMK", Video::VideoControl::PLAY_AUDIO } } },
                                    emptyPlayback );
        scenarioDatas
            .emplace_back( scenarioInfo[9], std::vector<Campaign::ScenarioInfoId>{}, "CAMPG10.H2C", scenarioName[9], scenarioDescription[9],
                           Campaign::VideoSequence{ { { "GOOD10.SMK", Video::VideoControl::PLAY_VIDEO_LOOP }, { "GOOD10W.SMK", Video::VideoControl::PLAY_AUDIO } } },
                           Campaign::VideoSequence{ { { "LIBRARY.SMK", Video::VideoControl::PLAY_VIDEO_LOOP }, { "LIBRARYW.SMK", Video::VideoControl::PLAY_AUDIO } } } );

        // At the end of the Betrayal scenario we should start an Archibald scenario.
        scenarioDatas.emplace_back( scenarioInfo[10], std::vector{ Campaign::ScenarioInfoId( Campaign::ARCHIBALD_CAMPAIGN, 5 ) }, "CAMPG05B.H2C", scenarioName[10],
                                    scenarioDescription[10], emptyPlayback,
                                    Campaign::VideoSequence{
                                        { { "EVIL06.SMK", Video::VideoControl::PLAY_VIDEO_LOOP }, { "EVIL06BW.SMK", Video::VideoControl::PLAY_AUDIO } } } );

        Campaign::CampaignData campaignData;
        campaignData.setCampaignID( Campaign::ROLAND_CAMPAIGN );
        campaignData.setCampaignScenarios( std::move( scenarioDatas ) );

        return campaignData;
    }

    Campaign::CampaignData getArchibaldCampaignData()
    {
        const int scenarioCount = 12;

        const std::array<std::string, scenarioCount> scenarioName
            = { gettext_noop( "First Blood" ),   gettext_noop( "Barbarian Wars" ), gettext_noop( "Necromancers" ),  gettext_noop( "Slay the Dwarves" ),
                gettext_noop( "Turning Point" ), gettext_noop( "Rebellion" ),      gettext_noop( "Dragon Master" ), gettext_noop( "Country Lords" ),
                gettext_noop( "The Crown" ),     gettext_noop( "Greater Glory" ),  gettext_noop( "Apocalypse" ),    gettext_noop( "Betrayal" ) };
        const std::array<std::string, scenarioCount> scenarioDescription = {
            gettext_noop(
                "King Archibald requires you to defeat the three enemies in this region. They are not allied with one another, so they will spend most of their energy fighting"
                " amongst themselves. You will win when you own all of the enemy castles and there are no more heroes left to fight." ),
            gettext_noop(
                "You must unify the barbarian tribes of the north by conquering them. As in the previous mission, the enemy is not allied against you, but they have more resources at their disposal. You will win when you own all of the enemy castles and there are no more heroes left to fight." ),
            gettext_noop(
                "Do-gooder wizards have taken the Necromancers' castle. You must retake it to achieve victory. Remember that while you start with a powerful army, you have no castle and must take one within 7 days, or lose this battle. (Hint: The nearest castle is to the southeast.)" ),
            gettext_noop(
                "The dwarves need conquering before they can interfere in King Archibald's plans. Roland's forces have more than one hero and many towns to start with, so be ready for attack from multiple directions. You must capture all of the enemy towns and castles to claim victory." ),
            gettext_noop(
                "Your enemies are allied against you and start close by, so be ready to come out fighting. You will need to own all four castles in this small valley to win." ),
            gettext_noop(
                "You must put down a peasant revolt led by Roland's forces. All are allied against you, but you have Lord Corlagon, an experienced hero, to help you. Capture all enemy castles to win." ),
            gettext_noop(
                "There are two enemies allied against you in this mission. Both are well armed and seek to evict you from their island. Avoid them and capture Dragon City to win." ),
            gettext_noop(
                "Your orders are to conquer the country lords that have sworn to serve Roland. All of the enemy castles are unified against you. Since you start without a castle, you must hurry to capture one before the end of the week. Capture all enemy castles for victory." ),
            gettext_noop( "Find the Crown before Roland's heroes find it. Archibald will need the Crown for the final battle against Roland." ),
            gettext_noop(
                "Gather as large an army as possible and capture the enemy castle within 8 weeks. You are opposed by only one enemy, but must travel a long way to get to the enemy castle. Any troops you have in your army at the end of this scenario will be with you in the final battle." ),
            gettext_noop(
                "This is the final battle. Both you and your enemy are armed to the teeth, and all are allied against you. Capture Roland to win the war, and be sure not to lose Archibald in the fight!" ),
            gettext_noop(
                "Switching sides leaves you with three castles against the enemy's one. This battle will be the easiest one you will face for the rest of the war...traitor." ) };

        std::vector<Campaign::ScenarioData> scenarioDatas;
        scenarioDatas.reserve( scenarioCount );

        std::vector<Campaign::ScenarioInfoId> scenarioInfo;
        scenarioInfo.reserve( scenarioCount );
        for ( int i = 0; i < scenarioCount; ++i ) {
            scenarioInfo.emplace_back( Campaign::ARCHIBALD_CAMPAIGN, i );
        }

        scenarioDatas.emplace_back( scenarioInfo[0], std::vector{ scenarioInfo[1] }, "CAMPE01.H2C", scenarioName[0], scenarioDescription[0],
                                    Campaign::VideoSequence{
                                        { { "EVIL01.SMK", Video::VideoControl::PLAY_VIDEO_LOOP }, { "EVIL01V.SMK", Video::VideoControl::PLAY_CUTSCENE, { 366, 403 } } } },
                                    emptyPlayback );
        scenarioDatas
            .emplace_back( scenarioInfo[1], std::vector{ scenarioInfo[2], scenarioInfo[3] }, "CAMPE02.H2C", scenarioName[1], scenarioDescription[1],
                           Campaign::VideoSequence{ { { "EVIL02.SMK", Video::VideoControl::PLAY_VIDEO_LOOP }, { "EVIL02W.SMK", Video::VideoControl::PLAY_AUDIO } } },
                           Campaign::VideoSequence{ { { "EVIL03.SMK", Video::VideoControl::PLAY_VIDEO_LOOP }, { "EVIL03QW.SMK", Video::VideoControl::PLAY_AUDIO } } } );
        scenarioDatas.emplace_back( scenarioInfo[2], std::vector{ scenarioInfo[4], scenarioInfo[11] }, "CAMPE03.H2C", scenarioName[2], scenarioDescription[2],
                                    emptyPlayback,
                                    Campaign::VideoSequence{ { { "EVIL05.SMK", Video::VideoControl::PLAY_VIDEO_LOOP },
                                                               { "EVIL05AV.SMK", Video::VideoControl::PLAY_CUTSCENE, { 87, 345 } } },
                                                             { { "EVIL05.SMK", Video::VideoControl::PLAY_VIDEO_LOOP },
                                                               { "RBETRAYV.SMK", Video::VideoControl::PLAY_CUTSCENE, { 87, 345 } } } } );
        scenarioDatas.emplace_back( scenarioInfo[3], std::vector{ scenarioInfo[4], scenarioInfo[11] }, "CAMPE04.H2C", scenarioName[3], scenarioDescription[3],
                                    emptyPlayback,
                                    Campaign::VideoSequence{ { { "EVIL05.SMK", Video::VideoControl::PLAY_VIDEO_LOOP },
                                                               { "EVIL05BV.SMK", Video::VideoControl::PLAY_CUTSCENE, { 87, 345 } } },
                                                             { { "EVIL05.SMK", Video::VideoControl::PLAY_VIDEO_LOOP },
                                                               { "RBETRAYV.SMK", Video::VideoControl::PLAY_CUTSCENE, { 87, 345 } } } } );
        scenarioDatas.emplace_back( scenarioInfo[4], std::vector{ scenarioInfo[5] }, "CAMPE05.H2C", scenarioName[4], scenarioDescription[4], emptyPlayback,
                                    Campaign::VideoSequence{
                                        { { "EVIL06.SMK", Video::VideoControl::PLAY_VIDEO_LOOP }, { "EVIL06AW.SMK", Video::VideoControl::PLAY_AUDIO } } } );
        scenarioDatas.emplace_back( scenarioInfo[5], std::vector{ scenarioInfo[6], scenarioInfo[7] }, "CAMPE06.H2C", scenarioName[5], scenarioDescription[5],
                                    emptyPlayback,
                                    Campaign::VideoSequence{
                                        { { "EVIL07.SMK", Video::VideoControl::PLAY_VIDEO_LOOP }, { "EVIL07W.SMK", Video::VideoControl::PLAY_AUDIO } } } );
        scenarioDatas.emplace_back( scenarioInfo[6], std::vector{ scenarioInfo[7] }, "CAMPE07.H2C", scenarioName[6], scenarioDescription[6], emptyPlayback,
                                    Campaign::VideoSequence{ { { "EVIL08.SMK", Video::VideoControl::PLAY_CUTSCENE } } },
                                    Campaign::ScenarioVictoryCondition::CAPTURE_DRAGON_CITY );
        scenarioDatas.emplace_back( scenarioInfo[7], std::vector{ scenarioInfo[8], scenarioInfo[9] }, "CAMPE08.H2C", scenarioName[7], scenarioDescription[7],
                                    emptyPlayback,
                                    Campaign::VideoSequence{
                                        { { "EVIL09.SMK", Video::VideoControl::PLAY_VIDEO_LOOP }, { "EVIL09W.SMK", Video::VideoControl::PLAY_AUDIO } } } );
        scenarioDatas.emplace_back( scenarioInfo[8], std::vector{ scenarioInfo[10] }, "CAMPE09.H2C", scenarioName[8], scenarioDescription[8], emptyPlayback,
                                    emptyPlayback, Campaign::ScenarioVictoryCondition::OBTAIN_ULTIMATE_CROWN );
        scenarioDatas.emplace_back( scenarioInfo[9], std::vector{ scenarioInfo[10] }, "CAMPE10.H2C", scenarioName[9], scenarioDescription[9], emptyPlayback,
                                    emptyPlayback );
        scenarioDatas.emplace_back( scenarioInfo[10], std::vector<Campaign::ScenarioInfoId>{}, "CAMPE11.H2C", scenarioName[10], scenarioDescription[10],
                                    Campaign::VideoSequence{
                                        { { "EVIL10.SMK", Video::VideoControl::PLAY_VIDEO_LOOP }, { "EVIL11W.SMK", Video::VideoControl::PLAY_AUDIO } } },
                                    Campaign::VideoSequence{ { { "PRISON.SMK", Video::VideoControl::PLAY_CUTSCENE } } } );

        // At the end of the Betrayal scenario we should start a Roland scenario.
        scenarioDatas.emplace_back( scenarioInfo[11], std::vector{ Campaign::ScenarioInfoId( Campaign::ROLAND_CAMPAIGN, 5 ) }, "CAMPE05B.H2C", scenarioName[11],
                                    scenarioDescription[11], emptyPlayback,
                                    Campaign::VideoSequence{
                                        { { "GOOD06.SMK", Video::VideoControl::PLAY_VIDEO_LOOP }, { "GOOD06BV.SMK", Video::VideoControl::PLAY_CUTSCENE } } } );

        Campaign::CampaignData campaignData;
        campaignData.setCampaignID( Campaign::ARCHIBALD_CAMPAIGN );
        campaignData.setCampaignScenarios( std::move( scenarioDatas ) );

        return campaignData;
    }

}

namespace Campaign
{
    // this is used to get awards that are not directly obtainable via scenario clear, such as assembling artifacts
    std::vector<Campaign::CampaignAwardData> CampaignAwardData::getExtraCampaignAwardData( const int campaignID )
    {
        assert( campaignID >= 0 );

        switch ( campaignID ) {
        case ROLAND_CAMPAIGN:
        case ARCHIBALD_CAMPAIGN:
            break;
        default:
            // Did you add a new campaign? Add the case handling code for it!
            assert( 0 );
            break;
        }

        return {};
    }

    const std::vector<ScenarioInfoId> & CampaignData::getScenariosAfter( const ScenarioInfoId & scenarioInfo )
    {
        const CampaignData & campaignData = getCampaignData( scenarioInfo.campaignId );
        const std::vector<ScenarioData> & scenarios = campaignData.getAllScenarios();
        assert( scenarioInfo.scenarioId >= 0 && static_cast<size_t>( scenarioInfo.scenarioId ) < scenarios.size() );

        return scenarios[scenarioInfo.scenarioId].getNextScenarios();
    }

    std::vector<ScenarioInfoId> CampaignData::getStartingScenarios() const
    {
        std::vector<ScenarioInfoId> startingScenarios;

        for ( size_t i = 0; i < _scenarios.size(); ++i ) {
            const int scenarioID = _scenarios[i].getScenarioID();
            if ( isStartingScenario( { _campaignID, scenarioID } ) )
                startingScenarios.emplace_back( _campaignID, scenarioID );
        }

        return startingScenarios;
    }

    bool CampaignData::isStartingScenario( const ScenarioInfoId & scenarioInfo ) const
    {
        // starting scenario = a scenario that is never included as a nextMap
        for ( size_t i = 0; i < _scenarios.size(); ++i ) {
            const std::vector<ScenarioInfoId> & nextMaps = _scenarios[i].getNextScenarios();

            if ( std::find( nextMaps.begin(), nextMaps.end(), scenarioInfo ) != nextMaps.end() )
                return false;
        }

        return true;
    }

    bool CampaignData::isAllCampaignMapsPresent() const
    {
        for ( size_t i = 0; i < _scenarios.size(); ++i ) {
            if ( !_scenarios[i].isMapFilePresent() )
                return false;
        }

        return true;
    }

    bool CampaignData::isLastScenario( const Campaign::ScenarioInfoId & scenarioInfoId ) const
    {
        assert( !_scenarios.empty() );
        for ( const ScenarioData & scenario : _scenarios ) {
            if ( ( scenario.getScenarioInfoId() == scenarioInfoId ) && scenario.getNextScenarios().empty() ) {
                return true;
            }
        }

        return false;
    }

    void CampaignData::setCampaignScenarios( std::vector<ScenarioData> && scenarios )
    {
        _scenarios = std::move( scenarios );
    }

    const CampaignData & CampaignData::getCampaignData( const int campaignID )
    {
        switch ( campaignID ) {
        case ROLAND_CAMPAIGN: {
            static const Campaign::CampaignData campaign( getRolandCampaignData() );
            return campaign;
        }
        case ARCHIBALD_CAMPAIGN: {
            static const Campaign::CampaignData campaign( getArchibaldCampaignData() );
            return campaign;
        }
        default: {
            // Did you add a new campaign? Add the corresponding case above.
            assert( 0 );
            static const Campaign::CampaignData noCampaign;
            return noCampaign;
        }
        }
    }

    void CampaignData::updateScenarioGameplayConditions( const Campaign::ScenarioInfoId & scenarioInfoId, Maps::FileInfo & mapInfo )
    {
        bool allAIPlayersInAlliance = false;

        switch ( scenarioInfoId.campaignId ) {
        case ROLAND_CAMPAIGN:
            if ( scenarioInfoId.scenarioId == 9 ) {
                allAIPlayersInAlliance = true;
            }
            break;
        case ARCHIBALD_CAMPAIGN:
            if ( scenarioInfoId.scenarioId == 5 || scenarioInfoId.scenarioId == 10 ) {
                allAIPlayersInAlliance = true;
            }
            break;
        default:
            // Did you add a new campaign? Add the corresponding case above.
            assert( 0 );
            return;
        }

        if ( allAIPlayersInAlliance ) {
            const PlayerColorsVector humanColors( mapInfo.colorsAvailableForHumans );
            // Make sure that this is only one human player on the map.
            if ( humanColors.size() != 1 ) {
                // Looks like somebody is modifying the original map.
                assert( 0 );
                return;
            }

            const PlayerColorsSet aiColors = ( mapInfo.kingdomColors & ( ~mapInfo.colorsAvailableForHumans ) );
            if ( aiColors == 0 ) {
                // This is definitely not the map to modify.
                assert( 0 );
                return;
            }

            const PlayerColor humanColor = humanColors.front();

            for ( PlayerColorsSet & allianceColor : mapInfo.unions ) {
                if ( allianceColor != static_cast<PlayerColorsSet>( humanColor ) && ( allianceColor & aiColors ) ) {
                    allianceColor = aiColors;
                }
            }
        }
    }

    // default amount to 1 for initialized campaign award data
    CampaignAwardData::CampaignAwardData( const int32_t id, const int32_t type, const int32_t subType )
        : CampaignAwardData( id, type, subType, 1, 0 )
    {
        // Do nothing.
    }

    CampaignAwardData::CampaignAwardData( const int32_t id, const int32_t type, const int32_t subType, const int32_t amount )
        : CampaignAwardData( id, type, subType, amount, 0 )
    {
        // Do nothing.
    }

    CampaignAwardData::CampaignAwardData( const int32_t id, const int32_t type, const int32_t subType, std::string customName )
        : CampaignAwardData( id, type, subType, 1, 0, std::move( customName ) )
    {
        // Do nothing.
    }

    CampaignAwardData::CampaignAwardData( const int32_t id, const int32_t type, const int32_t subType, const int32_t amount, const int32_t startScenarioID,
                                          std::string customName )
        : _id( id )
        , _type( type )
        , _subType( subType )
        , _amount( amount )
        , _startScenarioID( startScenarioID )
        , _customName( std::move( customName ) )
    {
        // Do nothing.
    }

    std::string CampaignAwardData::getName() const
    {
        if ( !_customName.empty() )
            return _( _customName );

        switch ( _type ) {
        case CampaignAwardData::TYPE_CREATURE_CURSE:
            return Monster( _subType ).GetName() + std::string( _( " bane" ) );
        case CampaignAwardData::TYPE_CREATURE_ALLIANCE:
            return Monster( _subType ).GetName() + std::string( _( " alliance" ) );
        case CampaignAwardData::TYPE_GET_ARTIFACT:
            return Artifact( _subType ).GetName();
        case CampaignAwardData::TYPE_CARRY_OVER_FORCES:
            return _( "Carry-over forces" );
        case CampaignAwardData::TYPE_RESOURCE_BONUS:
            return Resource::String( _subType ) + std::string( _( " bonus" ) );
        case CampaignAwardData::TYPE_GET_SPELL:
            return Spell( _subType ).GetName();
        case CampaignAwardData::TYPE_HIREABLE_HERO:
            return Heroes( _subType, 0 ).GetName();
        case CampaignAwardData::TYPE_DEFEAT_ENEMY_HERO:
            return Heroes( _subType, 0 ).GetName() + std::string( _( " defeated" ) );
        default:
            // Did you add a new award? Add the logic above!
            assert( 0 );
            break;
        }

        return {};
    }

    std::string CampaignAwardData::getDescription() const
    {
        switch ( _type ) {
        case CampaignAwardData::TYPE_CREATURE_CURSE: {
            std::vector<Monster> monsters;
            monsters.emplace_back( _subType );
            std::string description( monsters.back().GetMultiName() );

            while ( monsters.back() != monsters.back().GetUpgrade() ) {
                monsters.emplace_back( monsters.back().GetUpgrade() );
                description += ", ";
                description += monsters.back().GetMultiName();
            }

            description += _( " will always run away from your army." );
            return description;
        }
        case CampaignAwardData::TYPE_CREATURE_ALLIANCE: {
            std::vector<Monster> monsters;
            monsters.emplace_back( _subType );
            std::string description( monsters.back().GetMultiName() );

            while ( monsters.back() != monsters.back().GetUpgrade() ) {
                monsters.emplace_back( monsters.back().GetUpgrade() );
                description += ", ";
                description += monsters.back().GetMultiName();
            }

            description += _( " will be willing to join your army." );
            return description;
        }
        case CampaignAwardData::TYPE_GET_ARTIFACT: {
            std::string description( _( "\"%{artifact}\" artifact will be carried over in the campaign." ) );
            StringReplace( description, "%{artifact}", Artifact( _subType ).GetName() );
            return description;
        }
        case CampaignAwardData::TYPE_CARRY_OVER_FORCES: {
            return _( "The army will be carried over in the campaign." );
        }
        case CampaignAwardData::TYPE_RESOURCE_BONUS: {
            std::string description( _( "The kingdom will have +%{count} %{resource} each day." ) );
            StringReplace( description, "%{count}", std::to_string( _amount ) );
            StringReplace( description, "%{resource}", Resource::String( _subType ) );
            return description;
        }
        case CampaignAwardData::TYPE_GET_SPELL: {
            std::string description( _( "The \"%{spell}\" spell will be carried over in the campaign." ) );
            StringReplace( description, "%{spell}", Spell( _subType ).GetName() );
            return description;
        }
        case CampaignAwardData::TYPE_HIREABLE_HERO: {
            std::string description( _( "%{hero} can be hired during scenarios." ) );
            StringReplace( description, "%{hero}", Heroes( _subType, 0 ).GetName() );
            return description;
        }
        case CampaignAwardData::TYPE_DEFEAT_ENEMY_HERO: {
            std::string description( _( "%{hero} has been defeated and will not appear in subsequent scenarios." ) );
            StringReplace( description, "%{hero}", Heroes( _subType, 0 ).GetName() );
            return description;
        }
        default:
            // Did you add a new award? Add the logic above!
            assert( 0 );
            break;
        }

        return {};
    }

    std::vector<Campaign::CampaignAwardData> CampaignAwardData::getCampaignAwardData( const ScenarioInfoId & scenarioInfo )
    {
        assert( scenarioInfo.campaignId >= 0 && scenarioInfo.scenarioId >= 0 );

        switch ( scenarioInfo.campaignId ) {
        case ROLAND_CAMPAIGN:
            return getRolandCampaignAwardData( scenarioInfo.scenarioId );
        case ARCHIBALD_CAMPAIGN:
            return getArchibaldCampaignAwardData( scenarioInfo.scenarioId );
        default:
            // Did you add a new campaign? Add the corresponding case above.
            assert( 0 );
            break;
        }

        return {};
    }

    const char * CampaignAwardData::getAllianceJoiningMessage( const int monsterId )
    {
        switch ( monsterId ) {
        case Monster::DWARF:
            return _( "The dwarves recognize their allies and gladly join your forces." );
        case Monster::OGRE:
            return _( "The ogres recognize you as the Dwarfbane and lumber over to join you." );
        case Monster::GREEN_DRAGON:
            return _( "The dragons, snarling and growling, agree to join forces with you, their 'Ally'." );
        case Monster::ELF:
            return _(
                "As you approach the group of elves, their leader calls them all to attention. He shouts to them, \"Who of you is brave enough to join this fearless ally of ours?\" The group explodes with cheers as they run to join your ranks." );
        default:
            break;
        }

        assert( 0 ); // Did you forget to add a new alliance type?
        return nullptr;
    }

    const char * CampaignAwardData::getAllianceFleeingMessage( const int monsterId )
    {
        switch ( monsterId ) {
        case Monster::DWARF:
            return _( "The dwarves hail you, \"Any friend of Roland is a friend of ours. You may pass.\"" );
        case Monster::OGRE:
            return _( "The ogres give you a grunt of recognition, \"Archibald's allies may pass.\"" );
        case Monster::GREEN_DRAGON:
            return _(
                "The dragons see you and call out. \"Our alliance with Archibald compels us to join you. Unfortunately you have no room. A pity!\" They quickly scatter." );
        case Monster::ELF:
            return _(
                "The elves stand at attention as you approach. Their leader calls to you and says, \"Let us not impede your progress, ally! Move on, and may victory be yours.\"" );
        default:
            break;
        }

        assert( 0 ); // Did you forget to add a new alliance type?
        return nullptr;
    }

    const char * CampaignAwardData::getBaneFleeingMessage( const int monsterId )
    {
        switch ( monsterId ) {
        case Monster::DWARF:
            return _( "\"The Dwarfbane!!!!, run for your lives.\"" );
        default:
            break;
        }

        assert( 0 ); // Did you forget to add a new bane type?
        return nullptr;
    }
}
