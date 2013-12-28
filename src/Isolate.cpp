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
