/***************************************************************************
 *   fheroes: https://github.com/ihhub/fheroes                           *
 *   Copyright (C) 2019 - 2024                                             *
 *                                                                         *
 *   Free Heroes Engine: http://sourceforge.net/projects/fheroes         *
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

#include <list>
#include <stdexcept>
#include <utility>

#include "agg.h"
#include "agg_file.h"
#include "dir.h"
#include "settings.h"
#include "tools.h"

namespace
{
    fheroes::AGGFile heroes_agg;
    fheroes::AGGFile heroesx_agg;
}

std::vector<uint8_t> AGG::getDataFromAggFile( const std::string & key, const bool ignoreExpansion )
{
    if ( !ignoreExpansion && heroesx_agg.isGood() ) {
        // Make sure that the below container is not const and not a reference
        // so returning it from the function will invoke a move constructor instead of copy constructor.
        std::vector<uint8_t> buf = heroesx_agg.read( key );
        if ( !buf.empty() )
            return buf;
    }

    return heroes_agg.read( key );
}

AGG::AGGInitializer::AGGInitializer()
{
    if ( init() ) {
        return;
    }

    throw std::logic_error( "No AGG data files found." );
}

bool AGG::AGGInitializer::init()
{
    const ListFiles aggFileNames = Settings::FindFiles( "data", ".agg", false );
    if ( aggFileNames.empty() ) {
        return false;
    }

    const std::string heroesAggFileName( "heroes.agg" );
    std::string heroesAggFilePath;
    std::string aggLowerCaseFilePath;

    for ( const std::string & path : aggFileNames ) {
        if ( path.size() < heroesAggFileName.size() ) {
            // Obviously this is not a correct file.
            continue;
        }

        std::string tempPath = StringLower( path );

        if ( tempPath.compare( tempPath.size() - heroesAggFileName.size(), heroesAggFileName.size(), heroesAggFileName ) == 0 ) {
            heroesAggFilePath = path;
            aggLowerCaseFilePath = std::move( tempPath );
            break;
        }
    }

    if ( heroesAggFilePath.empty() ) {
        // The main game resource file was not found.
        return false;
    }

    if ( !heroes_agg.open( heroesAggFilePath ) ) {
        return false;
    }

    _originalAGGFilePath = std::move( heroesAggFilePath );

    // Find "heroesx.agg" file.
    std::string heroesXAggFilePath;
    fheroes::replaceStringEnding( aggLowerCaseFilePath, ".agg", "x.agg" );

    for ( const std::string & path : aggFileNames ) {
        const std::string tempPath = StringLower( path );
        if ( tempPath == aggLowerCaseFilePath ) {
            heroesXAggFilePath = path;
            break;
        }
    }

    if ( !heroesXAggFilePath.empty() && heroesx_agg.open( heroesXAggFilePath ) ) {
        _expansionAGGFilePath = std::move( heroesXAggFilePath );
    }

    Settings::Get().EnablePriceOfLoyaltySupport( heroesx_agg.isGood() );

    return true;
}
