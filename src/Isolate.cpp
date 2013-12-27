//
//  Isolate.cpp
//  07_dart_bindings
//
//  Created by Marcus Wendt on 27/12/2013.
//
//

#include "Isolate.h"

#include <map>

#include "Library.h"
#include "utilities.h"

#define BUILTIN_FUNCTION_NAME(name) Builtin_##name
#define DECLARE_BUILTIN_FUNCTION(name) \
extern void BUILTIN_FUNCTION_NAME(name)(Dart_NativeArguments args)

//DECLARE_BUILTIN_FUNCTION(Exit);
//DECLARE_BUILTIN_FUNCTION(Logger_PrintString);


namespace fieldkit { namespace dart {
    
    // Static
    std::vector<Library*> Isolate::builtInLibraries;
    
    Library* Isolate::coreLibrary = NULL;
    Library* Isolate::uriLibrary = NULL;
    Library* Isolate::ioLibrary = NULL;

    
    #pragma mark ---- BuiltIn Libraries ----
    typedef std::map<std::string, Dart_NativeFunction> NativeFunctionMap;
    
    NativeFunctionMap coreNativeFunctions;
    NativeFunctionMap ioNativeFunctions;

    template<const NativeFunctionMap& function_map>
    Dart_NativeFunction LibraryResolver(Dart_Handle name, int argc, bool* auto_setup_scope = false)
    {
        const char* native_function_name = 0;
        
        Dart_StringToCString(name, &native_function_name);
        
        NativeFunctionMap::const_iterator func_it =
        function_map.find(native_function_name);
        
        if (func_it != function_map.end())
            return func_it->second;
        
        LOG_W(native_function_name << " is unresolved.");
        return NULL;
    }

    void coreExit(Dart_NativeArguments args)
    {
        exit(0);
    }
    
    void corePrint(Dart_NativeArguments arguments)
    {
        std::stringstream ss;
        for(int i=0; i<Dart_GetNativeArgumentCount(arguments); i++) {
            Dart_Handle value = Dart_GetNativeArgument(arguments, i);
            std::string str = GetString(value);
            ss << str << "\n";
        }

        std::cout << ss.str();
    }

    Library* CreateCoreLibrary()
    {
        coreNativeFunctions.insert(std::make_pair("exit", &coreExit));
        coreNativeFunctions.insert(std::make_pair("print", &corePrint));
//        coreNativeFunctions.insert(std::make_pair("Exit", BUILTIN_FUNCTION_NAME(Exit)));
//        coreNativeFunctions.insert(std::make_pair("Logger_PrintString", BUILTIN_FUNCTION_NAME(Logger_PrintString)));
        
        return new Library("dart:builtin",
                           NULL,
                           LibraryResolver<coreNativeFunctions>,
                           NULL);
    }
    
    void IOLibraryInitializer(Dart_Handle library)
    {
        Dart_Handle timer_closure =
        Dart_Invoke(library, NewString("_getTimerFactoryClosure"), 0, 0);
        Dart_Handle isolate_library =
        Dart_LookupLibrary(NewString("dart:isolate"));
        
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
        
        coreLibrary = CreateCoreLibrary();
        uriLibrary = CreateUriLibrary();
        ioLibrary = CreateIOLibrary();
        
        builtInLibraries.push_back(coreLibrary);
        builtInLibraries.push_back(ioLibrary);
        builtInLibraries.push_back(uriLibrary);
    }

    void Isolate::ShutdownBuiltinLibraries()
    {
        assert(builtInLibraries.size() > 0);
        while(!builtInLibraries.empty()) delete builtInLibraries.back(), builtInLibraries.pop_back();
    }
    
} }
