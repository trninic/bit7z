// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2021  Riccardo Ostani - All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * Bit7z is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bit7z; if not, see https://www.gnu.org/licenses/.
 */

#include "bititemsvector.hpp"

#include "bitexception.hpp"
#include "internal/bufferitem.hpp"
#include "internal/fsindexer.hpp"
#include "internal/stdinputitem.hpp"

using namespace bit7z;
using filesystem::FSItem;
using filesystem::FSIndexer;

void BitItemsVector::indexDirectory( const fs::path& in_dir, const tstring& filter, bool recursive ) {
    FSItem dir_item{ in_dir }; //Note: if in_dir is an invalid path, FSItem constructor throws a BitException!
    if ( filter.empty() && !dir_item.inArchivePath().empty() ) {
        mItems.emplace_back( std::make_unique< FSItem >( dir_item ) );
    }
    FSIndexer indexer{ dir_item, filter };
    indexer.listDirectoryItems( mItems, recursive );
}

void BitItemsVector::indexPaths( const vector< tstring >& in_paths, bool ignore_dirs ) {
    for ( const auto& file_path : in_paths ) {
        FSItem item{ file_path };
        indexItem( item, ignore_dirs );
    }
}

void BitItemsVector::indexPathsMap( const map< tstring, tstring >& in_paths, bool ignore_dirs ) {
    for ( const auto& file_pair : in_paths ) {
        FSItem item{ fs::path( file_pair.first ), fs::path( file_pair.second ) };
        indexItem( item, ignore_dirs );
    }
}

void BitItemsVector::indexItem( const FSItem& item, bool ignore_dirs ) {
    if ( !item.isDir() ) {
        mItems.emplace_back( std::make_unique< FSItem >( item ) );
    } else if ( !ignore_dirs ) { //item is a directory
        if ( !item.inArchivePath().empty() ) {
            mItems.emplace_back( std::make_unique< FSItem >( item ) );
        }
        FSIndexer indexer{ item };
        indexer.listDirectoryItems( mItems, true );
    }
}

void BitItemsVector::indexFile( const tstring& in_file, const tstring& name ) {
    if ( fs::is_directory( in_file ) ) {
        throw BitException( "Input path points to a directory, not a file",
                            std::make_error_code( std::errc::invalid_argument ), in_file );
    }
    mItems.emplace_back( std::make_unique< FSItem >( in_file, name ) );
}

void BitItemsVector::indexBuffer( const vector< byte_t >& in_buffer, const tstring& name ) {
    mItems.emplace_back( std::make_unique< BufferItem >( in_buffer, name ) );
}

void BitItemsVector::indexStream( std::istream& in_stream, const tstring& name ) {
    mItems.emplace_back( std::make_unique< StdInputItem >( in_stream, name ) );
}

size_t BitItemsVector::size() const {
    return mItems.size();
}

const GenericInputItem& BitItemsVector::operator[]( GenericInputItemVector::size_type index ) const {
    // Note: here index is expected to be a correct one!
    return *mItems[ index ];
}

GenericInputItemVector::const_iterator BitItemsVector::begin() const noexcept {
    return mItems.cbegin();
}

GenericInputItemVector::const_iterator BitItemsVector::end() const noexcept {
    return mItems.cend();
}

GenericInputItemVector::const_iterator BitItemsVector::cbegin() const noexcept {
    return mItems.cbegin();
}

GenericInputItemVector::const_iterator BitItemsVector::cend() const noexcept {
    return mItems.cend();
}

BitItemsVector::~BitItemsVector() = default;
