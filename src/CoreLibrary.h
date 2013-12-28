/*
 *      _____  __  _____  __     ____
 *     / ___/ / / /____/ / /    /    \   ofxFieldKitDart
 *    / ___/ /_/ /____/ / /__  /  /  /   (c) 2013, FIELD. All rights reserved.
 *   /_/        /____/ /____/ /_____/    http://www.field.io
 *
 *   Created by Marcus Wendt on 27/12/2013.
 */

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