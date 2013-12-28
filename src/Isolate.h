/*
 *      _____  __  _____  __     ____
 *     / ___/ / / /____/ / /    /    \   ofxFieldKitDart
 *    / ___/ /_/ /____/ / /__  /  /  /   (c) 2013, FIELD. All rights reserved.
 *   /_/        /____/ /____/ /_____/    http://www.field.io
 *
 *   Created by Marcus Wendt on 27/12/2013.
 */


#pragma once

#include <vector>

#include "dart_api.h"

namespace fieldkit { namespace dart {
    
    class Library;
    
    class Isolate {
    public:
        Isolate(Dart_Isolate isolate, Dart_Handle library)
        : isolate_(isolate),
          library_(library) {}
        
        ~Isolate() {}
        
        static void InitializeBuiltinLibraries();
        static void ShutdownBuiltinLibraries();
        
        static std::vector<Library*> builtInLibraries;

        // dart core libraries
        static Library* core_library;
        static Library* uri_library;
        static Library* io_library;
        
        // fieldkit-dart core library
        static Library* base_library;

        //! calls a function by name with the given arguments
        void Invoke(const char* function, int argc = 0, Dart_Handle* args = NULL);
        
        Dart_Isolate getIsolate() { return isolate_; }
        Dart_Handle getLibrary() { return library_; }
        
    private:
        Dart_Isolate isolate_;
        Dart_Handle library_; // ?
    };
    
} } // namespace fieldkit::dart