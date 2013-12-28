//
//  BaseLibrary.cpp
//  07_dart_bindings
//
//  Created by Marcus Wendt on 27/12/2013.
//
//

#include "BaseLibrary.h"

#include <iostream>
#include <sstream>

#include "dart_api.h"
#include "Library.h"
#include "utilities.h"

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

static NativeFunctionMap base_native_functions;
    
Library* CreateBaseLibrary()
{
    base_native_functions.insert(std::make_pair("exit", &base_exit));
    base_native_functions.insert(std::make_pair("print", &base_print));
    base_native_functions.insert(std::make_pair("Logger_PrintString", &base_print));
    
    // TODO need a better way to set this path
    const char* source = ReadFileContents("../Resources/fkbase.dart").c_str();
//    LOG_I("source "<< source);
//    std::cout << source << std::endl;
    
    return new Library(FKDART_BASE_LIBRARY,
                       source,
                       LibraryResolver<base_native_functions>,
                       NULL);
}

} }