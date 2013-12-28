/*
 *      _____  __  _____  __     ____
 *     / ___/ / / /____/ / /    /    \   ofxFieldKitDart
 *    / ___/ /_/ /____/ / /__  /  /  /   (c) 2013, FIELD. All rights reserved.
 *   /_/        /____/ /____/ /_____/    http://www.field.io
 *
 *   Created by Marcus Wendt on 27/12/2013.
 */


#pragma once

#include <string>
#include <vector>

namespace fieldkit { namespace dart {
    
    class Isolate;
    class Library;
    
    class DartVM {
    public:
        DartVM();
        ~DartVM();
        
        //! prepares the dart virtual machine
        void Init(const bool checkedMode = true);
        
        void LoadSnapshot(const std::string file);
        
        //! load, parse and compile a script, returns a script isolate
        Isolate* LoadScript(const std::string scriptFile);
        
        void add(Library* library);
        
        // Accessors
        std::string getVersion();
        uint8_t* getSnapshot() { return snapshotBuffer_; }
        std::vector<Library*> getLibraries() { return libraries_; }
        
//        std::string getLibraryScript() { return libraryScript_; }
        
    private:
        uint8_t* snapshotBuffer_;
        std::vector<Library*> libraries_;
//        std::string libraryScript_;
    };
    
} } // namespace fieldkit::dart