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
#include "utilities.h"

namespace fieldkit { namespace dart {
    
    typedef void (*Dart_LibraryInitializer)(Dart_Handle library);
    
    typedef std::map<std::string, Dart_NativeFunction> NativeFunctionMap;
    
    
    template<const NativeFunctionMap& function_map>
    Dart_NativeFunction LibraryResolver(Dart_Handle name, int argc, bool* auto_setup_scope = false)
    {
        const char* native_function_name = 0;
        Dart_StringToCString(name, &native_function_name);
        NativeFunctionMap::const_iterator func_it = function_map.find(native_function_name);
        
//        auto_setup_scope = true; // ?
        
        if (func_it != function_map.end())
            return func_it->second;
        
        LOG_W(native_function_name << " is unresolved.");
        return NULL;
    }
    
    
    class Library {
    public:
        Library(const char* name, const char* source,
                Dart_NativeEntryResolver nativeResolver,
                Dart_LibraryInitializer initializer);
        
        Dart_Handle Load();
        

        const char* name_;
        const char* source_;
        
    private:
        Dart_NativeEntryResolver nativeResolver_;
        Dart_LibraryInitializer initializer_;
    };
    
} }