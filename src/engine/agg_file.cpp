/***************************************************************************
 *   fheroes2: https://github.com/ihhub/fheroes2                           *
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

namespace fheroes2
{
    // HoMM2 AGG FAT entry: hash(u32) + offset(u32) + size(u32)        — names stored at end of file
    // HoMM1 AGG FAT entry: name(13 bytes) + offset(u32) + size(u32) + size2(u32) — names inline
    static constexpr size_t _homm1FilenameSize = 13; // 8.3 ASCIIZ, no extra padding
    static constexpr size_t _homm1FileRecordSize = _homm1FilenameSize + sizeof( uint32_t ) * 3;
    static constexpr size_t _homm2FileRecordSize = sizeof( uint32_t ) * 3;

    bool AGGFile::_openHoMM2( const size_t count, const size_t size )
    {
        ROStreamBuf fileEntries = _stream.getStreamBuf( count * _homm2FileRecordSize );
        const size_t nameEntriesSize = _maxFilenameSize * count;
        _stream.seek( size - nameEntriesSize );
        ROStreamBuf nameEntries = _stream.getStreamBuf( nameEntriesSize );

        for ( size_t i = 0; i < count; ++i ) {
            std::string name = nameEntries.getString( _maxFilenameSize );

            // Check 32-bit filename hash.
            if ( fileEntries.getLE32() != calculateAggFilenameHash( name ) ) {
                _files.clear();
                return false;
            }

            const uint32_t fileOffset = fileEntries.getLE32();
            const uint32_t fileSize = fileEntries.getLE32();
            _files.try_emplace( std::move( name ), std::make_pair( fileSize, fileOffset ) );
        }

        if ( _files.size() != count ) {
            _files.clear();
            return false;
        }

        return !_stream.fail();
    }

    bool AGGFile::_openHoMM1( const size_t count, const size_t size )
    {
        if ( count * _homm1FileRecordSize >= size ) {
            return false;
        }

        ROStreamBuf fileEntries = _stream.getStreamBuf( count * _homm1FileRecordSize );

        for ( size_t i = 0; i < count; ++i ) {
            std::string name = fileEntries.getString( _homm1FilenameSize );
            const uint32_t fileOffset = fileEntries.getLE32();
            const uint32_t fileSize = fileEntries.getLE32();
            fileEntries.getLE32(); // size2 — always equals fileSize, discard

            if ( !name.empty() ) {
                _files.try_emplace( std::move( name ), std::make_pair( fileSize, fileOffset ) );
            }
        }

        if ( _files.size() != count ) {
            _files.clear();
            return false;
        }

        return !_stream.fail();
    }

    bool AGGFile::open( const std::string & fileName )
    {
        if ( !_stream.open( fileName, "rb" ) ) {
            return false;
        }

        const size_t size = _stream.size();
        const size_t count = _stream.getLE16();

        // Try HoMM2 format first (FAT has hash+offset+size, names at end of file).
        if ( count * ( _homm2FileRecordSize + _maxFilenameSize ) < size ) {
            const size_t fatStart = _stream.tell();
            if ( _openHoMM2( count, size ) ) {
                return true;
            }
            // Hash validation failed — not HoMM2. Fall through to HoMM1.
            _stream.seek( fatStart );
        }

        // Try HoMM1 format (FAT has inline name+offset+size+size2).
        return _openHoMM1( count, size );
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
        uint32_t hash = 0;
        uint32_t sum = 0;

        for ( auto iter = str.rbegin(); iter != str.rend(); ++iter ) {
            const unsigned char c = static_cast<unsigned char>( std::toupper( static_cast<unsigned char>( *iter ) ) );

            hash = ( hash << 5 ) + ( hash >> 25 );

            sum += c;
            hash += sum + c;
        }

        return hash;
    }
}

IStreamBase & operator>>( IStreamBase & stream, fheroes2::ICNHeader & icn )
{
    icn.offsetX = static_cast<int16_t>( stream.getLE16() );
    icn.offsetY = static_cast<int16_t>( stream.getLE16() );
    icn.width = stream.getLE16();
    icn.height = stream.getLE16();
    icn.animationFrames = stream.get();
    icn.offsetData = stream.getLE32();

    return stream;
}
