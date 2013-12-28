/*
 *      _____  __  _____  __     ____
 *     / ___/ / / /____/ / /    /    \   ofxFieldKitDart
 *    / ___/ /_/ /____/ / /__  /  /  /   (c) 2013, FIELD. All rights reserved.
 *   /_/        /____/ /____/ /_____/    http://www.field.io
 *
 *   Created by Marcus Wendt on 27/12/2013.
 */


#include "Library.h"

#include "utilities.h"
#include "dart_api.h"

namespace fieldkit { namespace dart {
    
    Library::Library(const char* name, const char* source,
                     Dart_NativeEntryResolver native_resolver,
                     Dart_LibraryInitializer initializer)
    : name_(name),
      source_(source),
      nativeResolver_(native_resolver),
      initializer_(initializer) {}
    
    Dart_Handle Library::Load()
    {
        Dart_Handle url = NewString(name_);
        Dart_Handle library = Dart_LookupLibrary(url);
        
        if (Dart_IsError(library))
            library = Dart_LoadLibrary(url, NewString(source_));
        
        if (Dart_IsError(library)) {
            LOG_E("Failed to load library (name: " << name_ << " source: " << source_ << ")\n Error: " << Dart_GetError(library))
            return library;
        }
        
        if (nativeResolver_ != NULL)
            Dart_SetNativeResolver(library, nativeResolver_);
        
        if (initializer_ != NULL)
            initializer_(library);
        
        return library;
    }
    
} } // namespace fieldkit::dart