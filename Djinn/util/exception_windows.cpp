#include "exception_windows.h"

namespace djinn::util {
    namespace {
         // DWORD ~> unsigned long
    	std::string winErrorToString(DWORD errorcode) {
             char* buffer = nullptr;
    
             FormatMessageA(
                 FORMAT_MESSAGE_ALLOCATE_BUFFER |
                 FORMAT_MESSAGE_IGNORE_INSERTS |
                 FORMAT_MESSAGE_FROM_SYSTEM,
                 NULL, // source mssage definition, NULL for system resource
                 errorcode,
                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                 (LPSTR)&buffer,
                 0,      // min
                 nullptr // va_args
             );
    
             // switch from localalloc to standard library
             std::string result(buffer);
             LocalFree(buffer);
    
             return result;
    	}
         
         // HRESULT ~> long
    	std::string winHresultToString(HRESULT hr) {
             char* buffer = nullptr;
    
             FormatMessageA(
                 FORMAT_MESSAGE_ALLOCATE_BUFFER |
                 FORMAT_MESSAGE_IGNORE_INSERTS |
                 FORMAT_MESSAGE_FROM_SYSTEM,
                 NULL, // source mssage definition, NULL for system resource
                 hr,
                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                 (LPSTR)&buffer,
                 0,      // min
                 nullptr // va_args
             );
    
             // switch from localalloc to standard library
             std::string result(buffer);
             LocalFree(buffer);
    
             return result;
    	}
    }
    
    WinException::WinException() :
    	mErrorCode(::GetLastError())
    {
    	mDescription = winErrorToString(mErrorCode);
    }
    
    WinException::WinException(DWORD errorCode) :
    	mErrorCode(errorCode)
    {
    	mDescription = winErrorToString(mErrorCode);
    }
    
    const char* WinException::what() const {
    	return mDescription.c_str();
    }
    
    DWORD WinException::getErrorCode() const {
    	return mErrorCode;
    }
    
    HresultException::HresultException(HRESULT result) :
    	mResult(result)
    {
    }
    
    const char* HresultException::what() const {
    	return winHresultToString(mResult).c_str();
    }
    
    HRESULT HresultException::getResult() const {
    	return mResult;
    }
    
    void throwIfFailed(HRESULT result) {
    	if (result != S_OK)
    		throw HresultException(result);
    }
}