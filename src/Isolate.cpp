/*
 *      _____  __  _____  __     ____
 *     / ___/ / / /____/ / /    /    \   ofxFieldKitDart
 *    / ___/ /_/ /____/ / /__  /  /  /   (c) 2013, FIELD. All rights reserved.
 *   /_/        /____/ /____/ /_____/    http://www.field.io
 *
 *   Created by Marcus Wendt on 27/12/2013.
 */


#include "Isolate.h"

#include <map>

#include "Library.h"
#include "utilities.h"
#include "BaseLibrary.h"


//#define BUILTIN_FUNCTION_NAME(name) Builtin_##name
//#define DECLARE_BUILTIN_FUNCTION(name) \
//    extern void BUILTIN_FUNCTION_NAME(name)(Dart_NativeArguments args)
//DECLARE_BUILTIN_FUNCTION(Exit);
//DECLARE_BUILTIN_FUNCTION(Logger_PrintString);


namespace fieldkit { namespace dart {
    
    // Static
    std::vector<Library*> Isolate::builtInLibraries;
    
    Library* Isolate::core_library = NULL;
    Library* Isolate::uri_library = NULL;
    Library* Isolate::io_library = NULL;
    Library* Isolate::base_library = NULL;

    
    #pragma mark ---- Dart Core Libraries ----
    NativeFunctionMap coreNativeFunctions;
    NativeFunctionMap ioNativeFunctions;

    Library* CreateCoreLibrary()
    {
//        coreNativeFunctions.insert(std::make_pair("Exit", BUILTIN_FUNCTION_NAME(Exit)));
//        coreNativeFunctions.insert(std::make_pair("Logger_PrintString", BUILTIN_FUNCTION_NAME(Logger_PrintString)));
        return new Library("dart:builtin",
                           NULL,
                           LibraryResolver<coreNativeFunctions>,
                           NULL);
    }
    
    void IOLibraryInitializer(Dart_Handle library)
    {
        Dart_Handle timer_closure = Dart_Invoke(library, NewString("_getTimerFactoryClosure"), 0, 0);
        Dart_Handle isolate_library = Dart_LookupLibrary(NewString("dart:isolate"));
        
        Dart_Handle args[1];
        args[0] = timer_closure;
        Dart_Handle result = Dart_Invoke(isolate_library,
                                         NewString("_setTimerFactoryClosure"),
                                         1,
                                         args);
        assert(!Dart_IsError(result));
    }
    
    Library* CreateIOLibrary()
    {
        // TODO: register any native methods.
        return new Library("dart:io", 0, LibraryResolver<ioNativeFunctions>, IOLibraryInitializer);
    }
    
    Library* CreateUriLibrary()
    {
        return new Library("dart:uri", NULL, NULL, NULL);
    }

    void Isolate::InitializeBuiltinLibraries()
    {
        assert(builtInLibraries.size() == 0);
        
        core_library = CreateCoreLibrary();
        io_library = CreateIOLibrary();
        uri_library = CreateUriLibrary();
        base_library = CreateBaseLibrary();
        
        builtInLibraries.push_back(core_library);
        builtInLibraries.push_back(io_library);
        builtInLibraries.push_back(uri_library);
        builtInLibraries.push_back(base_library);
    }

    void Isolate::ShutdownBuiltinLibraries()
    {
        assert(builtInLibraries.size() > 0);
        while(!builtInLibraries.empty()) delete builtInLibraries.back(), builtInLibraries.pop_back();
    }

    
    #pragma mark ---- Execution ----
    void Isolate::Invoke(const char* function, int argc, Dart_Handle* args)
    {
        Dart_EnterScope();
        Dart_Handle result = Dart_Invoke(library_,
                                         NewString(function),
                                         0,
                                         NULL);

        if (Dart_IsError(result)) {
            LOG_E(Dart_GetError(result) << " in "<< function)
        }
        
        Dart_RunLoop();
        Dart_ExitScope();
    }
    
} }
