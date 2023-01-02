// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "bitexception.hpp"

#include "internal/internalcategory.hpp"
#include "internal/hresultcategory.hpp"
#include "internal/windows.hpp"
#if !defined(BIT7Z_USE_NATIVE_STRING) && defined(_WIN32)
#include "internal/util.hpp"
#endif

std::error_code bit7z::make_hresult_code( HRESULT res ) noexcept {
    return std::error_code{ static_cast< int >( res ), bit7z::hresult_category() };
}

std::error_code bit7z::last_error_code() noexcept {
    return std::error_code{ static_cast< int >( GetLastError() ), std::system_category() };
}

using bit7z::BitException;
using bit7z::FailedFiles;
using bit7z::tstring;

BitException::BitException( const char* const message, std::error_code code, FailedFiles&& files )
    : system_error( code, message ), mFailedFiles( std::move( files ) ) { files.clear(); }

BitException::BitException( const char* const message, std::error_code code, const tstring& file )
    : system_error( code, message ) {
    mFailedFiles.emplace_back( file, code );
}

#if !defined(BIT7Z_USE_NATIVE_STRING) && defined(_WIN32)

BitException::BitException( const char* message, std::error_code code, const std::wstring& file )
    : BitException( message, code, bit7z::narrow( file.c_str(), file.size() ) ) {}

#endif

BitException::BitException( const std::string& message, std::error_code code )
    : system_error( code, message.c_str() ) {}

const FailedFiles& BitException::failedFiles() const noexcept {
    return mFailedFiles;
}

BitException::native_code_type BitException::nativeCode() const noexcept {
#ifdef _WIN32 // On Windows, the native code must be a HRESULT value.
    return hresultCode();
#else // On Unix, the native code is a POSIX error code.
    return posixCode();
#endif
}

HRESULT BitException::hresultCode() const noexcept {
    const std::error_code& error = code();
    if ( error.category() == bit7z::hresult_category() ) { // Already a HRESULT value
        return error.value();
    }
#ifdef _MSC_VER
    // Note: MinGW considers POSIX error codes in std::system_category, so this code is valid only for MSVC
    if ( error.category() == std::system_category() ) { // Win32 error code
        return HRESULT_FROM_WIN32( static_cast< DWORD >( error.value() ) );
    }
#endif
    // POSIX error code (generic_category) or BitError code (internal_category)
    if ( error == std::errc::bad_file_descriptor ) {
        return HRESULT_FROM_WIN32( ERROR_INVALID_HANDLE );
    }
    if ( error == std::errc::invalid_argument ) {
        return E_INVALIDARG;
    }
    if ( error == std::errc::not_a_directory ) {
        return HRESULT_FROM_WIN32( ERROR_DIRECTORY );
    }
    if ( error == std::errc::function_not_supported ) {
        return E_NOTIMPL;
    }
    if ( error == std::errc::no_space_on_device ) {
        return HRESULT_FROM_WIN32( ERROR_DISK_FULL );
    }
    if ( error == std::errc::no_such_file_or_directory ) {
        return HRESULT_FROM_WIN32( ERROR_PATH_NOT_FOUND );
    }
    if ( error == std::errc::not_enough_memory ) {
        return E_OUTOFMEMORY;
    }
    if ( error == std::errc::not_supported ) {
        return E_NOINTERFACE;
    }
    if ( error == std::errc::file_exists ) {
        return HRESULT_FROM_WIN32( ERROR_FILE_EXISTS );
    }
    if ( error == std::errc::operation_canceled ) {
        return E_ABORT;
    }
#ifdef _WIN32
    if ( error == std::errc::permission_denied ) {
        return E_ACCESSDENIED;
    }
#endif
    return E_FAIL;
}

int BitException::posixCode() const noexcept {
    const auto& error = code();
#ifdef _MSC_VER
    if ( error.category() == bit7z::hresult_category() ||
         error.category() == bit7z::internal_category() ||
         error.category() == std::system_category() ) {
#else
    if ( error.category() == bit7z::hresult_category() || error.category() == bit7z::internal_category() ) {
#endif
        return error.default_error_condition().value();
    }
    return error.value(); // On POSIX systems, std::system_category == std::generic_category
}
