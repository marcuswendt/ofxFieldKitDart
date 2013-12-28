//
//  BaseLibrary.h
//  07_dart_bindings
//
//  Created by Marcus Wendt on 27/12/2013.
//
//


#pragma once

#include "Library.h"

namespace fieldkit { namespace dart {

#define CORE_LIBRARY_NAME "fkdart"
    
    class CoreLibrary : public Library {
    public:
        CoreLibrary() {}
        ~CoreLibrary() {}
        void Init();
    };
    
} }