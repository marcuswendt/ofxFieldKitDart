/*
 *      _____  __  _____  __     ____
 *     / ___/ / / /____/ / /    /    \   ofxFieldKitDart
 *    / ___/ /_/ /____/ / /__  /  /  /   (c) 2013, FIELD. All rights reserved.
 *   /_/        /____/ /____/ /_____/    http://www.field.io
 *
 *   Created by Marcus Wendt on 27/12/2013.
 */


#include "Library.h"

#include "dart_api.h"

#include "DartVM.h"
#include "utilities.h"


namespace fieldkit { namespace dart {
    
    
    Dart_NativeFunction LibraryResolver(Dart_Handle name, int argc, bool* auto_setup_scope = false)
    {
        const char* native_function_name = 0;
        Dart_StringToCString(name, &native_function_name);
     
        DartVM* dartVM = static_cast<DartVM*>(Dart_CurrentIsolateData());
        
        for(Library* library : dartVM->getLibraries())
        {
            auto functionMap = library->getFunctions();
            auto functionIt = functionMap.find(native_function_name);
            
            if( functionIt != functionMap.end() )
                return functionIt->second;
    
        }
        
        LOG_W(native_function_name << " is unresolved.");
        return NULL;
    }
    
    
    Library::Library()
    : name_(NULL), source_(NULL), initializer_(NULL)
    {}
    
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
        
        Dart_SetNativeResolver(library, LibraryResolver);
        
        if (initializer_ != NULL)
            initializer_(library);
        
        return library;
    }
    
    
    void Library::add(const char* name, Dart_NativeFunction function)
    {
        functions_.insert(std::make_pair(name, function));
    }
    
} } // namespace fieldkit::dart