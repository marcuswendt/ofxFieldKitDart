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


	// -----------------------------------------------------------------------------

	DartVM::DartVM( std::string snapshotFilePath_ )
	{
		string snapshotFilePath = snapshotFilePath_;
		// relative paths are believed relative to current data path,
		// absolute paths are left untouched.
		if (!ofFilePath::isAbsolute(snapshotFilePath)){
			snapshotFilePath = ofToDataPath(snapshotFilePath);
		}

		if ( ofFile::doesFileExist( snapshotFilePath ) )
		{
			LoadSnapshot( snapshotFilePath );
		} else {
			ofLogError("DartVM cannot find snapshot file!");
		}
		add(new CoreLibrary());
	}

	// -----------------------------------------------------------------------------

	DartVM::~DartVM()
	{
		while(!libraries_.empty()) {
			delete libraries_.back();
			libraries_.pop_back();
		}
	}

	// -----------------------------------------------------------------------------

#pragma mark ---- Isolate Creation ----
	Dart_Handle LibraryTagHandler(Dart_LibraryTag tag,
		Dart_Handle library,
		Dart_Handle url)
	{
		if(tag == Dart_kCanonicalizeUrl)
			return url;
	}

	// -----------------------------------------------------------------------------

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

	// -----------------------------------------------------------------------------

	Dart_Handle FilePathFromUri(Dart_Handle script, Dart_Handle core_library) {
		Dart_Handle args[2] = {
			script,
			Dart_True()  // TODO: should this be true or false?
		};
		return Dart_Invoke(core_library, NewString("_filePathFromUri"), 2, args);
	}

	// -----------------------------------------------------------------------------

	Dart_Handle ReadSource(Dart_Handle script, Dart_Handle core_library) {
		Dart_Handle scriptPath = script;
		Dart_Handle source = fieldkit::dart::NewString(ofBufferFromFile(GetString(scriptPath),true).getBinaryBuffer());
		return source;
	}

	// -----------------------------------------------------------------------------

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
			Dart_Handle source = ReadSource(resolved_script, core_library);
			if (Dart_IsError(source))
				return source;

			return Dart_LoadScript(resolved_script, source, 0, 0);
	}

	// -----------------------------------------------------------------------------

	Isolate* CreateIsolate(const std::string scriptFile, const char* main, bool resolve,
		DartVM* dartVm, char** error)
	{

		uint8_t* snapshotBuffer = nullptr;

		snapshotBuffer = dartVm->getSnapshot();

		Dart_Isolate isolate = Dart_CreateIsolate(scriptFile.c_str(),
			main,
			snapshotBuffer,
			dartVm,
			error);

		if (isolate==NULL) {
			ofLogError() << "Dart error : " << *error;
			return NULL;
		} else {
			ofLogNotice() << "Successfully created Isolate";
		}

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

	// -----------------------------------------------------------------------------

#pragma mark ---- Initialisation ----
	Dart_Isolate IsolateCreateCb(const char* script_uri,
		const char* main,
		void* callback_data,
		char** error)
	{
		Isolate* isolate = CreateIsolate(script_uri, main, true, NULL, error);
		if (isolate == NULL) {
			ofLogError(__FUNCTION__)  << "Failed to create Isolate: " << script_uri << "|" << main
				<< ": " << error;
		}
		return isolate->getIsolate();
	}

	// -----------------------------------------------------------------------------

	bool InterruptIsolateCb()
	{
		return true;
	}

	// -----------------------------------------------------------------------------

	void UnhandledExceptionCb(Dart_Handle error)
	{
		ofLogError(__FUNCTION__) << "UnhandledException " << Dart_GetError(error);
	}

	// -----------------------------------------------------------------------------

	void ShutdownIsolateCb(void *callbackData)
	{
	}

	// -----------------------------------------------------------------------------

	// file callbacks have been copied verbatim from included sample... 
	// plus verbose logging. don't event know yet if we need them
	void* OpenFileCb(const char* name, bool write)
	{
		//	LOG_V( "name: " << name << ", write mode: " << boolalpha << write << dec );
		return fopen(name, write ? "w" : "r");
	}

	// -----------------------------------------------------------------------------

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

	// -----------------------------------------------------------------------------

	static void WriteFileCb(const void* data, intptr_t length, void* file)
	{
		fwrite(data, 1, length, reinterpret_cast<FILE*>(file));
	}


	// -----------------------------------------------------------------------------

	static void CloseFileCb(void* file)
	{
		fclose(reinterpret_cast<FILE*>(file));
	}

	// -----------------------------------------------------------------------------

	static bool EntropySourceCb(uint8_t* buffer, intptr_t length)
	{
		return true;
	}

	// -----------------------------------------------------------------------------

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

	// -----------------------------------------------------------------------------

	void DartVM::LoadSnapshot(const std::string file)
	{
		ofLogNotice(__FUNCTION__) << " : Loading Snapshot " << file;

		mSnapshotBuffer = ofBufferFromFile(file,true);
	}

	// -----------------------------------------------------------------------------

#pragma mark ---- LoadScript ----
	Isolate* DartVM::LoadScript(const std::string scriptFile)
	{
		char* error = NULL;
		Isolate* isolate = CreateIsolate(scriptFile, "main", true, this, &error);
		if (!isolate)
			LOG_E("Failed to create Isolate." << std::endl << error);
		return isolate;
	}

	// -----------------------------------------------------------------------------

#pragma mark ---- Accessors ----
	void DartVM::add(Library* library)
	{
		libraries_.push_back(library);
	}

	// -----------------------------------------------------------------------------

	std::string DartVM::getVersion()
	{
		return Dart_VersionString();
	}

	// -----------------------------------------------------------------------------

	uint8_t* DartVM::getSnapshot() const { 
		if (mSnapshotBuffer.size() ==0 ){
			ofLogError() << __FUNCTION__ << " : requested contents of empty snapshot.";
			return NULL;
		} else {
			return (uint8_t*)mSnapshotBuffer.getBinaryBuffer(); 
		}
	}

} }  // namespace fieldkit::dart