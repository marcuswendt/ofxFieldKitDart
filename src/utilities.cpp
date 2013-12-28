/*
 *      _____  __  _____  __     ____
 *     / ___/ / / /____/ / /    /    \   ofxFieldKitDart
 *    / ___/ /_/ /____/ / /__  /  /  /   (c) 2013, FIELD. All rights reserved.
 *   /_/        /____/ /____/ /_____/    http://www.field.io
 *
 *   Created by Marcus Wendt on 27/12/2013.
 */


#include "utilities.h"

namespace fieldkit { namespace dart {
    
    // Reads a file into a stl string.
    std::string ReadFileContents(std::string const& path)
    {
        FILE* file = fopen(path.c_str(), "rb");
        if (file == NULL) return "";
        
        fseek(file, 0, SEEK_END);
        int size = ftell(file);
        rewind(file);
        
        char* chars = new char[size + 1];
        chars[size] = '\0';
        for (int i = 0; i < size;) {
            int read = fread(&chars[i], 1, size - i, file);
            i += read;
        }
        fclose(file);
        
        std::string contents = std::string(chars, size);
        delete chars;
        
        return contents;
    }

} }