/***************************************************************************
 *   fheroes: https://github.com/ihhub/fheroes                           *
 *   Copyright (C) 2020 - 2024                                             *
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

#include "agg_file.h"

#include <cctype>
#include <cstdint>
#include <iterator>
#include <string>
#include <vector>

namespace fheroes
{
    bool AGGFile::open( const std::string & fileName )
    {
        if ( !_stream.open( fileName, "rb" ) ) {
            return false;
        }

        const size_t size = _stream.size();
        if ( size < sizeof( uint16_t ) ) {
            return false;
        }

        const size_t count = _stream.getLE16();

        if ( count == 0 ) {
            return true;
        }

        const size_t nameEntriesSize = _maxFilenameSize * count;
        if ( sizeof( uint16_t ) + nameEntriesSize > size ) {
            return false;
        }

        const size_t entriesSize = size - sizeof( uint16_t ) - nameEntriesSize;
        if ( entriesSize % count != 0 ) {
            return false;
        }

        const size_t recordSize = entriesSize / count;
        enum class AGGRecordType
        {
            Hash32,
            Hash16,
            NoHash
        };

        AGGRecordType recordType;

        switch ( recordSize ) {
        case sizeof( uint32_t ) * 3:
            recordType = AGGRecordType::Hash32;
            break;
        case sizeof( uint32_t ) * 2 + sizeof( uint16_t ):
            recordType = AGGRecordType::Hash16;
            break;
        case sizeof( uint32_t ) * 2:
            recordType = AGGRecordType::NoHash;
            break;
        default:
            return false;
        }

        ROStreamBuf fileEntries = _stream.getStreamBuf( entriesSize );
        const size_t dataSectionEnd = size - nameEntriesSize;
        _stream.seek( dataSectionEnd );
        ROStreamBuf nameEntries = _stream.getStreamBuf( nameEntriesSize );

        std::vector<std::string> fileNames;
        fileNames.reserve( count );

        for ( size_t i = 0; i < count; ++i ) {
            fileNames.emplace_back( nameEntries.getString( _maxFilenameSize ) );
        }

        for ( size_t i = 0; i < count; ++i ) {
            const std::string & name = fileNames[i];

            uint32_t fileOffset = 0;
            uint32_t fileSize = 0;

            switch ( recordType ) {
            case AGGRecordType::Hash32: {
                const uint32_t storedHash = fileEntries.getLE32();
                if ( storedHash != calculateAggFilenameHash( name ) ) {
                    _files.clear();
                    return false;
                }

                fileOffset = fileEntries.getLE32();
                fileSize = fileEntries.getLE32();
                break;
            }
            case AGGRecordType::Hash16: {
                const uint16_t storedHash = fileEntries.getLE16();
                if ( storedHash != static_cast<uint16_t>( calculateAggFilenameHash( name ) ) ) {
                    _files.clear();
                    return false;
                }

                fileOffset = fileEntries.getLE32();
                fileSize = fileEntries.getLE32();
                break;
            }
            case AGGRecordType::NoHash:
                fileOffset = fileEntries.getLE32();
                fileSize = fileEntries.getLE32();
                break;
            }

            if ( fileOffset + fileSize > dataSectionEnd ) {
                _files.clear();
                return false;
            }

            _files.try_emplace( name, std::make_pair( fileSize, fileOffset ) );
        }

        if ( _files.size() != count ) {
            _files.clear();
            return false;
        }

        return !_stream.fail();
    }

    std::vector<uint8_t> AGGFile::read( const std::string & fileName )
    {
        auto it = _files.find( fileName );
        if ( it == _files.end() ) {
            return {};
        }

        const auto [fileSize, fileOffset] = it->second;
        if ( fileSize > 0 ) {
            _stream.seek( fileOffset );
            return _stream.getRaw( fileSize );
        }

        return {};
    }

    uint32_t calculateAggFilenameHash( const std::string_view str )
    {
        uint16_t hash = 0;
        uint16_t sum = 0;

        for ( auto iter = str.rbegin(); iter != str.rend(); ++iter ) {
            const unsigned char c = static_cast<unsigned char>( std::toupper( static_cast<unsigned char>( *iter ) ) );

            hash = ( hash << 5 ) + ( hash >> 25 );

            sum += c;
            hash += sum + c;
        }

        return hash;
    }
}

IStreamBase & operator>>( IStreamBase & stream, fheroes::ICNHeader & icn )
{
    icn.offsetX = static_cast<int16_t>( stream.getLE16() );
    icn.offsetY = static_cast<int16_t>( stream.getLE16() );
    icn.width = stream.getLE16();
    icn.height = stream.getLE16();
    icn.animationFrames = stream.get();
    icn.offsetData = stream.getLE32();

    return stream;
}
