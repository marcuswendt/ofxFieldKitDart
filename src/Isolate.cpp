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


namespace fieldkit { namespace dart {
    

    void Isolate::Invoke(Dart_Handle target, const char* function, int argc, Dart_Handle* args)
    {
        Dart_EnterScope();
        Dart_Handle result = Dart_Invoke(target,
                                         NewString(function),
                                         argc,
                                         args);

        if (Dart_IsError(result)) {
            LOG_E(Dart_GetError(result) << " in "<< function)
        }
        
        Dart_RunLoop();
        Dart_ExitScope();
    }
    
    
    void Isolate::Invoke(const char* function, int argc, Dart_Handle* args)
    {
        Invoke(library_, function, argc, args);
    }
    
    
    Dart_Handle Isolate::New(const char* typeName, int argc, Dart_Handle* args)
    {
        // entering and leaving the scope here causes us to invalidate the instance handle!
//        Dart_EnterScope();
        
        // Get type
        Dart_Handle type = Dart_GetType(library_, NewString(typeName), 0, NULL);
        if(Dart_IsError(type)) {
            LOG_E(Dart_GetError(type));
        }
        
        // Invoke the unnamed constructor.
        Dart_Handle instance = Dart_New(type, Dart_Null(), argc, args);
        
        if (Dart_IsError(instance)) {
            //            Dart_NewApiError
            LOG_E(Dart_GetError(instance) << " while instantiating '"<< type <<"'")
        }

//        Dart_ExitScope();
        return instance;
    }
    
} }
