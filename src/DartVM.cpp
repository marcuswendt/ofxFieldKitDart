//
//  DartVM.cpp
//  07_dart_bindings
//
//  Created by Marcus Wendt on 26/12/2013.
//
//

#include "DartVM.h"

#include <iostream>
#include <sstream>


namespace fieldkit { namespace dart {

// -- Utilities ------------------------------------------------------------------------
#pragma mark Utilities

#define FKDART_CORE_LIBRARY "fkdart"
    
    
// Logger
#define LOG(TYPE, MSG) \
    { \
        std::stringstream ss; \
        ss << TYPE << ": " << MSG; \
        ss << "\n"; \
        std::cout << ss.str(); \
    }

#define LOG_I(MSG) LOG("INFO", MSG)
#define LOG_E(MSG) LOG("ERROR", MSG)


// Reads a file into a stl string.
static std::string ReadFileContents(std::string const& path)
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


// -- Dart Utilities ------------------------------------------------------------------------------
#pragma mark Dart Utilities

#define CHECK(result)                               \
{													\
    if(Dart_IsError(result)) {                      \
       LOG_E(Dart_GetError(result));                \
       assert(0);                                   \
    }												\
}

#define CHECK_RETURN(result)                        \
{													\
    if(Dart_IsError(result)) {                      \
       LOG_E(Dart_GetError(result));                \
       return;										\
    }												\
}

    
static Dart_Handle NewString(const char* str)
{
    return Dart_NewStringFromCString(str);
}

//static std::string GetString(Dart_Handle handle)
//{
//    const char *result;
//    CHECK(Dart_StringToCString(handle, &result));
//    return std::string(result);
//}


#pragma mark Constructor
    
DartVM::DartVM()
{
    std::string basePath = "../Resources"; // OS dependent
    coreLibraryScript_ = basePath + "/fkdart.dart";
}
    
    
// -- VM Initialisation ------------------------------------------------------------------------
#pragma mark Initialisation

static Dart_NativeFunction ResolveName(Dart_Handle handle, int argc, bool* auto_setup_scope)
{
    assert(Dart_IsString(handle));
    
    DartScope enterScope;
    std::string name = fieldkit::dart::GetString(handle);
    
    DartVM *dartVm = static_cast<DartVM *>( Dart_CurrentIsolateData() );
    auto& functionMap = dartVm->nativeFunctions_;
    auto functionIt = functionMap.find( name );
    if( functionIt != functionMap.end() )
        return functionIt->second;
    
    return NULL;
}

    
static Dart_Handle LibraryTagHandler(Dart_LibraryTag tag, Dart_Handle library, Dart_Handle urlHandle)
{
    LOG_I("LibraryTagHandler");
    
    if(tag == Dart_kCanonicalizeUrl )
        return urlHandle;
    
    std::string url = GetString(urlHandle);
    if(url == FKDART_CORE_LIBRARY) {
        DartVM *dartVm = static_cast<DartVM *>(Dart_CurrentIsolateData());
        
        std::string script = ReadFileContents(dartVm->getCoreLibraryScript());
        Dart_Handle source = Dart_NewStringFromCString( script.c_str() );
        CHECK(source);
        
        Dart_Handle library = Dart_LoadLibrary( urlHandle, source );
        CHECK(library);
        
        CHECK(Dart_SetNativeResolver(library, ResolveName));
        
        return library;
    }
    
    assert(false && "unreachable");
    return NULL;
}


Dart_Isolate CreateIsolate(const char* script_uri, const char* main, void* data, char** error)
{
    DartVM *dartVm = reinterpret_cast<DartVM *>(data);
    const uint8_t* snapshotData = (const uint8_t*) ReadFileContents(dartVm->snapshotFile_).c_str();
    
	LOG_I("Creating isolate " << script_uri << ", " << main);
	Dart_Isolate isolate = Dart_CreateIsolate(script_uri, main, snapshotData, data, error);
	if(!isolate) {
		LOG_E("Couldn't create isolate: " << *error);
		return NULL;
	}
    
	// Set up the library tag handler for this isolate.
	DartScope scope;

    LOG_I("set library tag handler");
	Dart_Handle result = Dart_SetLibraryTagHandler(LibraryTagHandler);
    CHECK(result);
    
	return isolate;
}

    
Dart_Isolate IsolateCreateCb(const char* script_uri,
                             const char* main,
                             void* callback_data,
                             char** error)
{
    Dart_Isolate isolate = CreateIsolate(script_uri, main, callback_data, error);
//    if (isolate == NULL) {
//        std::cerr << "Failed to create Isolate: " << script_uri << "|" << main
//        << ": " << error << std::endl;
//    }
//    return isolate != NULL;
    return isolate;
}
    
bool InterruptIsolateCb()
{
    LOG_I("InterruptIsolate");
	return true;
}

void UnhandledExceptionCb(Dart_Handle error)
{
    LOG_E("UnhandledException " << Dart_GetError(error));
}

void ShutdownIsolateCb(void *callbackData)
{
    LOG_I("ShutdownIsolate");
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


void DartVM::init(const std::string snapshotFile, const bool checkedMode)
{
    snapshotFile_ = snapshotFile;
    
    // setting VM startup options
    if(checkedMode)
        vmFlags_.push_back("--enable-checked-mode");
    
	const char **vmFlags = (const char **)malloc(vmFlags_.size() * sizeof( const char * ));
	for(size_t i = 0; i < vmFlags_.size(); i++)
		vmFlags[i] = vmFlags_[i].c_str();
    
	bool success = Dart_SetVMFlags(vmFlags_.size(), vmFlags);
	assert(success);
	free(vmFlags);
    
    success = Dart_Initialize(IsolateCreateCb, InterruptIsolateCb,
                              UnhandledExceptionCb, ShutdownIsolateCb,
                              OpenFileCb, ReadFileCb,
                              WriteFileCb, CloseFileCb, EntropySourceCb);
	assert(success);
}


// -- Script Execution ------------------------------------------------------------------------
#pragma mark Script Execution

bool DartVM::loadScript(const std::string scriptFile)
{
    Dart_Isolate currentIsolate = Dart_CurrentIsolate();
	if(currentIsolate && currentIsolate == isolate_) {
		printf("isolate already loaded, shutting down first");
		Dart_ShutdownIsolate();
	}
    
	char *error;
    isolate_ = CreateIsolate(scriptFile.c_str(), "main", this, &error);
	if(!isolate_) {
        LOG_E( "could not create isolate: " << error );
		assert(false);
	}

//    const char* script = scriptFile.c_str();
//    const char* main = "main";
//    const uint8_t* snapshotBuffer = (const uint8_t*) ReadFileContents(snapshotFile_).c_str();
//    
//    char* error = NULL;
//    isolate_ = Dart_CreateIsolate(script, main, snapshotBuffer, NULL, &error);
//    assert(isolate_);

    
	DartScope scope;
    
    // load source
    Dart_Handle url = NewString(scriptFile.c_str());
    
    const char* sourceStr = ReadFileContents(scriptFile.c_str()).c_str();
	Dart_Handle source = Dart_NewStringFromCString(sourceStr);
    
	CHECK(source);
	CHECK_RETURN(Dart_LoadScript(url, source, 0, 0));

    // load libraries
    Dart_Handle rootLib = Dart_RootLibrary();
	assert(!Dart_IsNull(rootLib));

    CHECK(Dart_SetNativeResolver(rootLib, ResolveName));

	return true;
}


void DartVM::invoke(const std::string &functionName, int argc, Dart_Handle* args)
{
	DartScope scope;
    
	Dart_Handle library = Dart_RootLibrary();
	assert(!Dart_IsNull(library));
    
	Dart_Handle nameHandle = Dart_NewStringFromCString(functionName.c_str());
	Dart_Handle result = Dart_Invoke(library, nameHandle, argc, args);
	CHECK_RETURN(result);
    
	// TODO: there was originally a note saying this probably isn't necessary.. try removing
	// Keep handling messages until the last active receive port is closed.
	result = Dart_RunLoop();
	CHECK(result);
    
	return;
}


void DartVM::add(std::string name, Dart_NativeFunction function)
{
    LOG_I("add native function '" << name << "'");
    nativeFunctions_.insert(std::pair<std::string, Dart_NativeFunction>(name, function));
}
    

#pragma mark Accessors
std::string DartVM::getCoreLibraryScript() { return coreLibraryScript_; }
    
std::string DartVM::getVersion() {
    return Dart_VersionString();
}

    
} } // namespace fieldkit::dart
