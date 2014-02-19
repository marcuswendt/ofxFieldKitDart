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

	Dart_Handle ReadSource(Dart_Handle script) {
		Dart_Handle scriptPath = script;

		string filePath = dart::getString(scriptPath);

		if (ofFile::doesFileExist(filePath)){
			Dart_Handle source = dart::newString(ofBufferFromFile(filePath,true).getBinaryBuffer());
			return source;
		} 

		// ---------| invariant: file not found.

		ofLogError() << "file not found: " << filePath;
		return Dart_NewStringFromCString("");


	}


	bool DartVM::isInitialized = false;

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
			loadSnapshot( snapshotFilePath );
		} else {
			ofLogError("DartVM cannot find snapshot file!");
		}
		add(new CoreLibrary());
	}

	// -----------------------------------------------------------------------------

	DartVM::~DartVM()
	{

		Dart_ShutdownIsolate();

		while(!libraries_.empty()) {
			delete libraries_.back();
			libraries_.pop_back();
		}

		//bool result = Dart_Cleanup();
		//ofLog() << "Dart Cleanup" << (result ? " successful" : " error");
	}

	// -----------------------------------------------------------------------------

#pragma mark ---- Isolate Creation ----
	Dart_Handle LibraryTagHandler(Dart_LibraryTag tag, Dart_Handle library, Dart_Handle url)
	{
		// this will resolve any dependencies from within .dart source.
		switch (tag) {
		case Dart_kCanonicalizeUrl: 
			{
				// this is where we resolve the include paths, relative to the library including
				// any other files.

				string filePath = dart::getString(url);

				if (filePath.find("fkdart:") == 0){
					// we have one of our libraries (which are probably already loaded), 
					// so we need to chomp off the prefix.
					filePath = filePath.substr(7,string::npos);
					Dart_Handle shortUrl = Dart_NewStringFromCString(filePath.c_str());
					return shortUrl;
				} 

				if (filePath == "fkdart" || filePath.find("dart:") == 0 ){
					// no need to resolve core libraries!
					return url;
				} else {

					Dart_Handle libraryUrlHandle = Dart_LibraryUrl(library);
					string libraryPath = dart::getString(libraryUrlHandle);
					//ofLogNotice() << "library path: " << libraryPath;
					string base_dir = ofFilePath::getEnclosingDirectory(libraryPath,true);

					string resolvedPath = "";

					unsigned pos = filePath.find("package:");

					if (pos != string::npos) {
						// "package:" was found, we store what comes after to resolve.
						string packageName = filePath.substr(pos + 8,string::npos);
						resolvedPath = "dart/packages/" + packageName;

					} else {
						resolvedPath = base_dir + "/" + filePath;
					}

					// careful: this is a potential leak, since we don't know what dart is going to 
					// do with the other url.
					Dart_Handle resolvedUrl = Dart_NewStringFromCString(resolvedPath.c_str());
					return resolvedUrl;

				}

				break;
			}
		case Dart_kImportTag: 
			{
				// this is invoked if our .dart file contains an import tag,
				// and will attempt to read the specified .dart source file

				string filePath = dart::getString(url);
				string absoluteDataPath = ofFilePath::getAbsolutePath("",true);

				unsigned pos = filePath.find(absoluteDataPath);
				if (pos!=string::npos){
					string relativePath = filePath.substr(absoluteDataPath.length(),string::npos);
					ofLogNotice() << "import: " << relativePath;
				} else {
					ofLogNotice() << "import: " << filePath;
				}

				Dart_Handle source = ReadSource(url);

				if (Dart_IsError(source)) 
				{
					return source;
				}
				Dart_Handle lib = Dart_LoadLibrary(url, source);
				if (Dart_IsError(lib)) 
				{
					return lib;
				}

				return lib;
			}
			break;
		case Dart_kSourceTag: 
			{
				string filePath = dart::getString(url);
				string absoluteDataPath = ofFilePath::getAbsolutePath("",true);

				unsigned pos = filePath.find(absoluteDataPath);
				if (pos!=string::npos){
					string relativePath = filePath.substr(absoluteDataPath.length(),string::npos);
					ofLogNotice() << "source import: " << relativePath;
				} else {
					ofLogNotice() << "source import: " << filePath;
				}


				Dart_Handle source = ReadSource(url);
				if (Dart_IsError(source)) {
					return source;
				}
				return Dart_LoadSource(library, url, source);
			}
			break;
		default:

			break;
		}

		return Dart_Null();
	}

	// -----------------------------------------------------------------------------

	Dart_Handle ResolveScript(const char* script, Dart_Handle core_library) 
	{
		string filePath	= script;
		string fileName	= Poco::Path( filePath ).getFileName();
		string rootDir	= filePath.substr( 0, filePath.size() - fileName.size() );

		Dart_Handle args[3] = {
			dart::newString( rootDir.c_str() ),
			dart::newString( fileName.c_str() ),
			Dart_True()  // TODO: should this be true or false?
		};
		Dart_Handle ret = Dart_Invoke(core_library, dart::newString("_resolveScriptUri"), 3, args);

		return ret;
	}

	// -----------------------------------------------------------------------------

	Dart_Handle FilePathFromUri(Dart_Handle script, Dart_Handle core_library) {
		Dart_Handle args[2] = {
			script,
			Dart_True()  // TODO: should this be true or false?
		};
		return Dart_Invoke(core_library, dart::newString("_filePathFromUri"), 2, args);
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
				resolved_script = dart::newString(script);
			}
			Dart_Handle source = ReadSource(resolved_script);
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
			return nullptr;
		} else {
			ofLogNotice() << "Successfully created Isolate";
		}

		Dart_EnterScope();

		Dart_Handle result = Dart_SetLibraryTagHandler(LibraryTagHandler);

		if (Dart_IsError(result)) {
			*error = strdup(Dart_GetError(result));
			Dart_ExitScope();
			Dart_ShutdownIsolate();
			return nullptr;
		}

		// Load libraries
		Dart_Handle core_library;        
		for(Library* library : dartVm->getLibraries())
		{
			result = library->load();

			if (Dart_IsError(result)) {
				*error = strdup(Dart_GetError(result));
				Dart_ExitScope();
				Dart_ShutdownIsolate();
				return nullptr;
			}

			if(library->getName() == CORE_LIBRARY_NAME)
				core_library = result;
		}

		ofLogNotice() << "Loading: " << scriptFile;

		Dart_Handle library = LoadScript(scriptFile.c_str(), false, core_library);

		if (Dart_IsError(library)) {
			*error = strdup(Dart_GetError(library));
			Dart_ExitScope();
			Dart_ShutdownIsolate();
			return nullptr;
		}

		result = Dart_LibraryImportLibrary(library, core_library, Dart_Null());

		if (Dart_IsError(result)) {
			*error = strdup(Dart_GetError(result));
			Dart_ExitScope();
			Dart_ShutdownIsolate();
			return nullptr;
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
		auto isolate = CreateIsolate(script_uri, main, true, NULL, error);
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

	void DartVM::init(const bool checkedMode)
	{
		if (!isInitialized){
			// setting VM startup options
			std::vector<std::string> vmFlags;
			if(checkedMode)
				vmFlags.push_back("--enable-checked-mode");

			const char **vmFlagsC = (const char **)malloc(vmFlags.size() * sizeof( const char * ));
			for(size_t i = 0; i < vmFlags.size(); i++)
				vmFlagsC[i] = vmFlags[i].c_str();

			bool success = Dart_SetVMFlags(vmFlags.size(), vmFlagsC);
			free(vmFlagsC);

			success = Dart_Initialize(IsolateCreateCb, InterruptIsolateCb,
				UnhandledExceptionCb, ShutdownIsolateCb,
				OpenFileCb, ReadFileCb,
				WriteFileCb, CloseFileCb, EntropySourceCb);
			assert(success);
			isInitialized = true;
		} 

		// initialise libraries
		for(Library* library : libraries_)
			library->init();
	}

	// -----------------------------------------------------------------------------

	void DartVM::loadSnapshot(const std::string file)
	{
		ofLogNotice(__FUNCTION__) << " : Loading Snapshot " << file;

		mSnapshotBuffer = ofBufferFromFile(file,true);
	}

	// -----------------------------------------------------------------------------

#pragma mark ---- LoadScript ----
	Isolate* DartVM::loadScript(const std::string scriptFile)
	{
		char* error = NULL;
		Isolate* isolate = CreateIsolate(scriptFile, "main", true, this, &error);
		if (!isolate)
			LOG_E("Failed to create Isolate." << std::endl << error);
		// make sure we haven't already loaded this Isolate
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