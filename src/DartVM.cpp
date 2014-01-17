/*
 *      _____  __  _____  __     ____
 *     / ___/ / / /____/ / /    /    \   ofxFieldKitDart
 *    / ___/ /_/ /____/ / /__  /  /  /   (c) 2013, FIELD. All rights reserved.
 *   /_/        /____/ /____/ /_____/    http://www.field.io
 *
 *   Created by Marcus Wendt on 27/12/2013.
 */


#include "DartVM.h"

#include <string>
#include "dart_api.h"

#include "Isolate.h"
#include "Library.h"
#include "utilities.h"
#include "utils/ofUtils.h"
#include "utils/ofFileUtils.h"

#include "CoreLibrary.h"

using namespace std;

namespace fieldkit { namespace dart {
    
    
    DartVM::DartVM( std::string snapshotFilePath )
    {
		if ( !ofFile::doesFileExist( snapshotFilePath ) )
		{
			ofLogFatalError( "DartVM cannot find snapshot file!" );
			// exit( -1 );
		}

        LoadSnapshot( snapshotFilePath );
        
        add(new CoreLibrary());
    }


    DartVM::~DartVM()
    {
        assert(libraries_.size() > 0);
        while(!libraries_.empty()) {
            delete libraries_.back();
            libraries_.pop_back();
        }
    }
    
    
    #pragma mark ---- Isolate Creation ----
    Dart_Handle LibraryTagHandler(Dart_LibraryTag tag,
                                  Dart_Handle library,
                                  Dart_Handle url)
    {
//        const char* url_str = NULL;
//        Dart_Handle result = Dart_StringToCString(url, &url_str);
//        if (Dart_IsError(result))
//            return result;
//        assert(false);  // TODO: implement.
        
        if(tag == Dart_kCanonicalizeUrl)
            return url;

        LOG_W("shouldnt happen!");
        
//        std::string urlStr = GetString(url);
//        if(urlStr == FKDART_BASE_LIBRARY) {
//            DartVM *dartVm = static_cast<DartVM *>(Dart_CurrentIsolateData());
//            
//            std::string script = ReadFileContents(dartVm->getLibraryScript());
//            Dart_Handle source = Dart_NewStringFromCString( script.c_str() );
//            CHECK(source);
//            
//            Dart_Handle library = Dart_LoadLibrary(url, source);
//            CHECK(library);
//            
////            CHECK(Dart_SetNativeResolver(library, ResolveName));
//            
//            return library;
//        }

    }

    
    Dart_Handle ResolveScript(const char* script, Dart_Handle core_library) 
	{
		string filePath	= script;
		string fileName	= Poco::Path( filePath ).getFileName();
		string rootDir	= filePath.substr( 0, filePath.size() - fileName.size() );
	
        Dart_Handle args[3] = {
			NewString( rootDir.c_str() ),
			NewString( fileName.c_str() ),
            Dart_True()  // TODO: should this be true or false?
        };
        Dart_Handle ret = Dart_Invoke(core_library, NewString("_resolveScriptUri"), 3, args);
        
        return ret;
    }
    
    Dart_Handle FilePathFromUri(Dart_Handle script, Dart_Handle core_library) {
        Dart_Handle args[2] = {
            script,
            Dart_True()  // TODO: should this be true or false?
        };
        return Dart_Invoke(core_library, NewString("_filePathFromUri"), 2, args);
    }
    
    Dart_Handle ReadSource(Dart_Handle script, Dart_Handle core_library) {
        Dart_Handle script_path = FilePathFromUri(script, core_library);
        if (Dart_IsError(script_path))
            return script_path;
        
        const char* script_path_str;
        Dart_StringToCString(script_path, &script_path_str);
        
        FILE* file = fopen(script_path_str, "r");
        if (file == NULL)
            LOG_W("Unable to read file " << script_path_str);
//            return Dart_NewApiError("Unable to read file '%s'", script_path_str);
        
        fseek(file, 0, SEEK_END);
        long length = ftell(file);
        fseek(file, 0, SEEK_SET);
        
        char* buffer = new char[length + 1];
        size_t read = fread(buffer, 1, length, file);
        fclose(file);
        buffer[read] = '\0';
        
        Dart_Handle source = NewString(buffer);
        delete[] buffer;
        return source;
    }
    
    // Temporary fix until we get the internal libraries to load
    Dart_Handle ReadSourceFixed(Dart_Handle script, Dart_Handle core_library) {
        Dart_Handle scriptPath = script;
        Dart_Handle source = fieldkit::dart::NewString(ReadFileContents(GetString(scriptPath)).c_str());
        return source;
    }

    
    Dart_Handle LoadScript(const char* script,
                           bool resolve,
                           Dart_Handle core_library) {
//        std::cout << __FUNCTION__ << ": " << script << ", " << resolve << std::endl;
        Dart_Handle resolved_script;
        
        if (resolve) {
            resolved_script = ResolveScript(script, core_library);
            if (Dart_IsError(resolved_script))
                return resolved_script;
        } else {
            resolved_script = NewString(script);
        }
        
//        Dart_Handle source = ReadSource(resolved_script, core_library);
        Dart_Handle source = ReadSourceFixed(resolved_script, core_library);
        if (Dart_IsError(source))
            return source;
        
        return Dart_LoadScript(resolved_script, source, 0, 0);
    }
    
    
    Isolate* CreateIsolate(const std::string scriptFile, const char* main, bool resolve,
                           void* data, char** error)
    {
        
        DartVM* dartVm = reinterpret_cast<DartVM*>(data);
        
		uint8_t* snapshotBuffer;
		
		// on windows there is a problem with the snapshot
		// Dart_CreateIsolate abort() because the snapshot is not a kFull
		#ifdef _MSC_VER
			snapshotBuffer = NULL;
		#else
			snapshotBuffer = dartVm->getSnapshot();
		#endif
        
        Dart_Isolate isolate = Dart_CreateIsolate(scriptFile.c_str(),
                                                  main,
                                                  snapshotBuffer,
                                                  dartVm,
                                                  error);
        assert(isolate);
        
        LOG_I("Created isolate");
        Dart_EnterScope();
        
        Dart_Handle result = Dart_SetLibraryTagHandler(LibraryTagHandler);
        
        if (Dart_IsError(result)) {
            *error = strdup(Dart_GetError(result));
            Dart_ExitScope();
            Dart_ShutdownIsolate();
            return NULL;
        }
        
        // Load libraries
        Dart_Handle core_library;        
        for(Library* library : dartVm->getLibraries())
        {
            result = library->Load();
            
            if (Dart_IsError(result)) {
                *error = strdup(Dart_GetError(result));
                Dart_ExitScope();
                Dart_ShutdownIsolate();
                return NULL;
            }
            
            if(library->getName() == CORE_LIBRARY_NAME)
                core_library = result;
        }
        LOG_I("Loaded built-in libraries")
        
        
        LOG_I("About to load " << scriptFile)
        Dart_Handle library = LoadScript(scriptFile.c_str(), false, core_library);
        
        if (Dart_IsError(library)) {
            *error = strdup(Dart_GetError(library));
            Dart_ExitScope();
            Dart_ShutdownIsolate();
            return NULL;
        }
        
        result = Dart_LibraryImportLibrary(library, core_library, Dart_Null());
        
        if (Dart_IsError(result)) {
            *error = strdup(Dart_GetError(result));
            Dart_ExitScope();
            Dart_ShutdownIsolate();
            return NULL;
        }
        
        if (Dart_IsError(result)) {
            *error = strdup(Dart_GetError(result));
            Dart_ExitScope();
            Dart_ShutdownIsolate();
            return NULL;
        }
        
        return new Isolate(isolate, library);
    }
    
    
    #pragma mark ---- Initialisation ----
    Dart_Isolate IsolateCreateCb(const char* script_uri,
                         const char* main,
                         void* callback_data,
                         char** error)
    {
        Isolate* isolate = CreateIsolate(script_uri, main, true, NULL, error);
        if (isolate == NULL) {
            std::cerr << "Failed to create Isolate: " << script_uri << "|" << main
            << ": " << error << std::endl;
        }
        return isolate->getIsolate();
    }
    
    bool InterruptIsolateCb()
    {
        return true;
    }
    
    void UnhandledExceptionCb(Dart_Handle error)
    {
        LOG_E("UnhandledException " << Dart_GetError(error));
    }
    
    void ShutdownIsolateCb(void *callbackData)
    {
    }
    
    // file callbacks have been copied verbatum from included sample... plus verbose logging. don't event know yet if we need them
    void* OpenFileCb(const char* name, bool write)
    {
        //	LOG_V( "name: " << name << ", write mode: " << boolalpha << write << dec );
        return fopen(name, write ? "w" : "r");
    }
    
    
    void ReadFileCb(const uint8_t** data, intptr_t* fileLength, void* stream )
    {
        if (!stream) {
            *data = 0;
            *fileLength = 0;
        } else {
            FILE* file = reinterpret_cast<FILE*>(stream);
            
            // Get the file size.
            fseek(file, 0, SEEK_END);
            *fileLength = ftell(file);
            rewind(file);
            
            // Allocate data buffer.
            *data = new uint8_t[*fileLength];
            *fileLength = fread(const_cast<uint8_t*>(*data), 1, *fileLength, file);
        }
    }
    
    
    static void WriteFileCb(const void* data, intptr_t length, void* file)
    {
        fwrite(data, 1, length, reinterpret_cast<FILE*>(file));
    }
    
    
    static void CloseFileCb(void* file)
    {
        fclose(reinterpret_cast<FILE*>(file));
    }
    
    
    static bool EntropySourceCb(uint8_t* buffer, intptr_t length)
    {
        return true;
    }
    
    void DartVM::Init(const bool checkedMode)
    {
        // setting VM startup options
        std::vector<std::string> vmFlags;
        if(checkedMode)
            vmFlags.push_back("--enable-checked-mode");
        
        const char **vmFlagsC = (const char **)malloc(vmFlags.size() * sizeof( const char * ));
        for(size_t i = 0; i < vmFlags.size(); i++)
            vmFlagsC[i] = vmFlags[i].c_str();
        
        bool success = Dart_SetVMFlags(vmFlags.size(), vmFlagsC);
        assert(success);
        free(vmFlagsC);
        
        success = Dart_Initialize(IsolateCreateCb, InterruptIsolateCb,
                                  UnhandledExceptionCb, ShutdownIsolateCb,
                                  OpenFileCb, ReadFileCb,
                                  WriteFileCb, CloseFileCb, EntropySourceCb);
        assert(success);
        
        // initialise libraries
        for(Library* library : libraries_)
            library->Init();
    }
    
    
    void DartVM::LoadSnapshot(const std::string file)
    {
        LOG_I("Loading Snapshot " << file);
        // TODO should error if snapshot is empty
        snapshotBuffer_ = (uint8_t*) ReadFileContents(file).c_str();
    }
    
    
    #pragma mark ---- LoadScript ----
    Isolate* DartVM::LoadScript(const std::string scriptFile)
    {
        char* error = NULL;
        Isolate* isolate = CreateIsolate(scriptFile, "main", true, this, &error);
        if (!isolate)
            LOG_E("Failed to create Isolate." << std::endl << error);
        return isolate;
    }

    #pragma mark ---- Accessors ----
    void DartVM::add(Library* library)
    {
        libraries_.push_back(library);
    }
    
    std::string DartVM::getVersion() { return Dart_VersionString(); }

} }  // namespace fieldkit::dart