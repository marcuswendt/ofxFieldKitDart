/*
 *      _____  __  _____  __     ____
 *     / ___/ / / /____/ / /    /    \   ofxFieldKitDart
 *    / ___/ /_/ /____/ / /__  /  /  /   (c) 2013, FIELD. All rights reserved.
 *   /_/        /____/ /____/ /_____/    http://www.field.io
 *
 *   Created by Marcus Wendt on 27/12/2013.
 */


#pragma once

#include "dart_api.h"

namespace fieldkit { namespace dart {
    
    typedef void (*Dart_LibraryInitializer)(Dart_Handle library);
    
    class Library {
    public:
        Library(const char* name, const char* source,
                Dart_NativeEntryResolver nativeResolver,
                Dart_LibraryInitializer initializer);
        
        Dart_Handle Load();
        
    private:
        const char* name_;
        const char* source_;
        Dart_NativeEntryResolver nativeResolver_;
        Dart_LibraryInitializer initializer_;
    };
    
} }