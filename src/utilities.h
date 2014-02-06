/*
 *      _____  __  _____  __     ____
 *     / ___/ / / /____/ / /    /    \   ofxFieldKitDart
 *    / ___/ /_/ /____/ / /__  /  /  /   (c) 2013, FIELD. All rights reserved.
 *   /_/        /____/ /____/ /_____/    http://www.field.io
 *
 *   Created by Marcus Wendt on 27/12/2013.
 */

#pragma once

#include <iostream>
#include <sstream>
#include <map>

#include "dart_api.h"

namespace fieldkit { namespace dart {

    // Logging
    #define LOG(TYPE, MSG) \
    { \
        std::stringstream ss; \
        ss << TYPE << ": " << MSG; \
        ss << std::endl; \
        std::cout << ss.str(); \
    }

    #define LOG_I(MSG) LOG("INFO", MSG)
    #define LOG_E(MSG) LOG("ERROR", MSG)
    #define LOG_W(MSG) LOG("WARNING", MSG)

    
    // Dart API Utilities    
//#define EXPECT_VALID(handle)                                                         \
//    do {                                                                             \
//        Dart_Handle tmp_handle = (handle);                                           \
//        if (Dart_IsError(tmp_handle)) {                                              \
//            dart::Expect(__FILE__, __LINE__).Fail(                                   \
//                "expected '%s' to be a valid handle but found an error handle:\n"    \
//                "    '%s'\n",                                                        \
//                #handle, Dart_GetError(tmp_handle));                                 \
//        }                                                                            \
//    } while (0)
    
    
#define CHECK_RETURN(result)                        \
{													\
    if(Dart_IsError(result)) {                      \
        LOG_E(Dart_GetError(result));               \
        return;										\
    }												\
}
    
    struct DartScope {
        DartScope() { Dart_EnterScope(); }
        ~DartScope() { Dart_ExitScope(); }
    };
    
    static Dart_Handle NewString(const char* str)
    {
        return Dart_NewStringFromCString(str);
    }
    
    static std::string GetString(Dart_Handle handle)
    {
        const char *result;
        Dart_StringToCString(handle, &result);
        return std::string(result);
    }

} } // namespace fieldkit::dart