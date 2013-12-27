//
//  DartVM.h
//  07_dart_bindings
//
//  Created by Marcus Wendt on 26/12/2013.
//
//

#pragma once

#include <string>
#include <vector>
#include <map>

#include "dart_api.h"


namespace fieldkit { namespace dart {
    
    typedef std::map<std::string, Dart_NativeFunction> NativeFunctionMap;
    
    
    class DartVM {
    public:
        DartVM();
        ~DartVM() {}
        
        void init(const std::string snapshotFile, const bool checkedMode = true);

        //! load, parse and compile a script
        bool loadScript(const std::string scriptFile = "");

        //! call the given dart function incl. arguments
        void invoke(const std::string &functionName, int argc = 0, Dart_Handle *args = NULL);
        
        //! registers a new native function
        void add(std::string name, Dart_NativeFunction function);
        
        std::string getVersion();
        std::string getCoreLibraryScript();
        
        // internal
        std::string snapshotFile_;
        NativeFunctionMap nativeFunctions_;
    
    private:
        Dart_Isolate isolate_;
        std::vector<std::string> vmFlags_;
        std::string coreLibraryScript_;
    };
    
    
    // Utilities
    struct DartScope {
        DartScope() { Dart_EnterScope(); }
        ~DartScope() { Dart_ExitScope(); }
    };

    static Dart_Handle NewString(const char* str);
    
    static std::string GetString(Dart_Handle handle)
    {
        const char *result;
        Dart_StringToCString(handle, &result);
        return std::string(result);
    }
    
    
} } // namespace fieldkit::dart