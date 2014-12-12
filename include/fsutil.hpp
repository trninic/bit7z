#ifndef BITFILESYSTEM_HPP
#define BITFILESYSTEM_HPP

#include <iostream>

#include "Common/MyString.h"

using namespace std;

namespace Bit7z {
    namespace FileSystem {
        namespace FSUtil {
            bool is_directory( const wstring& path );
            bool path_exists( const wstring& path );
            bool has_ending( const wstring& str, const wstring& ending );

            void normalize_path( wstring& path );
        }
    }
}

#endif // BITFILESYSTEM_HPP