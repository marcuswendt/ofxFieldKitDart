/*
 *      _____  __  _____  __     ____
 *     / ___/ / / /____/ / /    /    \   ofxFieldKitDart
 *    / ___/ /_/ /____/ / /__  /  /  /   (c) 2013, FIELD. All rights reserved.
 *   /_/        /____/ /____/ /_____/    http://www.field.io
 *
 *   Created by Marcus Wendt on 27/12/2013.
 */

#include "CoreLibrary.h"

#include <iostream>
#include <sstream>

#include "dart_api.h"
#include "Library.h"
#include "utilities.h"

/// tig to make inline code editing easier.
#ifndef DART_LANG_SOURCE
#define DART_LANG_SOURCE(x) #x
#endif 


namespace fieldkit { namespace dart {
    
    void base_exit(Dart_NativeArguments args)
    {
        exit(0);
    }

    void base_print(Dart_NativeArguments arguments)
    {
        std::stringstream ss;
        for(int i=0; i<Dart_GetNativeArgumentCount(arguments); i++) {
            Dart_Handle value = Dart_GetNativeArgument(arguments, i);
            std::string str = GetString(value);
            ss << str << "\n";
        }
        
        std::cout << ss.str();
    }

        
    void CoreLibrary::Init()
    {
        mName = CORE_LIBRARY_NAME;
        
		/// dart code
        mSource = DART_LANG_SOURCE (
            void exit(num code) native "exit";
            void print(String message) native "print";
		);

        add("exit", base_exit);
        add("print", base_print);
    }

} }