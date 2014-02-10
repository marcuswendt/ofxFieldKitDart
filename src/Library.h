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
#include <map>

namespace fieldkit { namespace dart {
    
    typedef void (*Dart_LibraryInitializer)(Dart_Handle library);
    
    typedef std::map<std::string, Dart_NativeFunction> NativeFunctionMap;
    
    
    class Library {
    public:
        Library()
        : mName(nullptr), mSource(nullptr), mInitializer(nullptr) {}
        
        virtual ~Library() {}
        
        //! called once the VM is initialised
        virtual void Init() = 0;
        
        //! called for each new Isolate
        Dart_Handle Load();
        
        const char* getName() { return mName; }
        const NativeFunctionMap& getFunctions() { return mFunctions; }
        
    protected:
        const char* mName;
        const char* mSource;

        NativeFunctionMap mFunctions;
        Dart_LibraryInitializer mInitializer;

        //! Adds a native function
        void add(const char* name, Dart_NativeFunction function);
    };
    
} }