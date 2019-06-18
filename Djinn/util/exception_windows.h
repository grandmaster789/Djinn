#pragma once

#include "preprocessor.h"
#include <cstdint>
#include <exception>
#include <string>

#if DJINN_PLATFORM == DJINN_PLATFORM_WINDOWS

namespace djinn::util {
    /*
        Utilities for converting a windows system level error (the kind you'd need to use GetLastError() for) into a
       readable exception
    */
    class WinException: public std::exception {
    public:
        WinException();  // uses GetLastErrorCode() to fetch the last system level error
        WinException(DWORD errorCode);

        virtual const char* what() const;
        DWORD               getErrorCode() const;

    private:
        std::string mDescription;
        DWORD       mErrorCode;
    };

    // MSDN HRESULT reference:
    // https://msdn.microsoft.com/en-us/library/cc231198.aspx
    class HresultException: public std::exception {
    public:
        HresultException(HRESULT result);

        virtual const char* what() const;
        HRESULT             getResult() const;

    private:
        HRESULT mResult;
    };

    void throwIfFailed(HRESULT result);
}  // namespace djinn::util

#endif
