/***************************************************************************
 *   fheroes: https://github.com/ihhub/fheroes                           *
 *   Copyright (C) 2019 - 2024                                             *
 *                                                                         *
 *   Free Heroes2 Engine: http://sourceforge.net/projects/fheroes         *
 *   Copyright (C) 2010 by Andrey Afletdinov <fheroes@gmail.com>          *
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

#include <cassert>
#include <vector>

#include "artifact_info.h"
#include "castle.h"
#include "profit.h"
#include "race.h"
#include "resource.h"

Funds ProfitConditions::FromBuilding( uint32_t building, int race )
{
    switch ( building ) {
    case BUILD_CASTLE:
        return Funds( Cost{ 1000, 0, 0, 0, 0, 0, 0 } );
    case BUILD_TENT:
    case BUILD_STATUE:
        return Funds( Cost{ 250, 0, 0, 0, 0, 0, 0 } );
    case BUILD_SPEC:
        if ( race == Race::WRLK )
            return Funds( Cost{ 500, 0, 0, 0, 0, 0, 0 } );
        break;
    default:
        break;
    }

    return {};
}

Funds ProfitConditions::FromArtifact( int artifact )
{
    const fheroes::ArtifactData & data = fheroes::getArtifactData( artifact );
    Funds cost;

    for ( const fheroes::ArtifactBonus & bonus : data.bonuses ) {
        switch ( bonus.type ) {
        case fheroes::ArtifactBonusType::GOLD_INCOME:
            cost.gold += bonus.value;
            break;
        case fheroes::ArtifactBonusType::WOOD_INCOME:
            cost.wood += bonus.value;
            break;
        case fheroes::ArtifactBonusType::ORE_INCOME:
            cost.ore += bonus.value;
            break;
        case fheroes::ArtifactBonusType::MERCURY_INCOME:
            cost.mercury += bonus.value;
            break;
        case fheroes::ArtifactBonusType::GEMS_INCOME:
            cost.gems += bonus.value;
            break;
        case fheroes::ArtifactBonusType::SULFUR_INCOME:
            cost.sulfur += bonus.value;
            break;
        case fheroes::ArtifactBonusType::CRYSTAL_INCOME:
            cost.crystal += bonus.value;
            break;
        default:
            break;
        }
    }

    for ( const fheroes::ArtifactCurse & curse : data.curses ) {
        switch ( curse.type ) {
        case fheroes::ArtifactCurseType::GOLD_PENALTY:
            cost.gold -= curse.value;
            break;
        default:
            break;
        }
    }

    return cost;
}

Funds ProfitConditions::FromMine( int type )
{
    switch ( type ) {
    case Resource::ORE:
        return Funds( Cost{ 0, 0, 0, 2, 0, 0, 0 } );
    case Resource::WOOD:
        return Funds( Cost{ 0, 2, 0, 0, 0, 0, 0 } );
    case Resource::MERCURY:
        return Funds( Cost{ 0, 0, 1, 0, 0, 0, 0 } );
    case Resource::SULFUR:
        return Funds( Cost{ 0, 0, 0, 0, 1, 0, 0 } );
    case Resource::CRYSTAL:
        return Funds( Cost{ 0, 0, 0, 0, 0, 1, 0 } );
    case Resource::GEMS:
        return Funds( Cost{ 0, 0, 0, 0, 0, 0, 1 } );
    case Resource::GOLD:
        return Funds( Cost{ 1000, 0, 0, 0, 0, 0, 0 } );
    default:
        // Did you add a new type of resources? Add the logic here!
        assert( 0 );
        break;
    }

    return {};
}
