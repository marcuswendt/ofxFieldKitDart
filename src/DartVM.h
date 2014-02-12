/*
*      _____  __  _____  __     ____
*     / ___/ / / /____/ / /    /    \   ofxFieldKitDart
*    / ___/ /_/ /____/ / /__  /  /  /   (c) 2013, FIELD. All rights reserved.
*   /_/        /____/ /____/ /_____/    http://www.field.io
*
*   Created by Marcus Wendt on 27/12/2013.
*/

#pragma once

#include "ofMain.h"

namespace fieldkit { namespace dart {

	class Isolate;
	class Library;

	class DartVM {

	private:

		/// disables direct instantiation
		DartVM(){};
		static bool isInitialized;

	public:

		/// factory function
		static shared_ptr<DartVM> create( std::string snapshotFilePath ) {
			return shared_ptr<DartVM> (new DartVM(snapshotFilePath));
		};
		/// constructor
		explicit DartVM( std::string snapshotFilePath_ );

		~DartVM();

		//! prepares the dart virtual machine
		void init(const bool checkedMode = true);

		void loadSnapshot(const std::string file);

		//! load, parse and compile a script, returns a script isolate
		/// the dart owns the isolate. don't destroy manually.
		Isolate* loadScript(const std::string scriptFile);

		/// brief: adds a library to the current virtual machine
		/// note: the vm takes ownership of the library, and 
		/// takes care of deleting these when the VM destructs.
		void add(Library* library);

		// Accessors
		std::string getVersion();
		// 
		uint8_t* getSnapshot() const;

		std::vector<Library*> getLibraries() { return libraries_; }

		//        std::string getLibraryScript() { return libraryScript_; }

	private:
		ofBuffer mSnapshotBuffer;
		std::vector<Library*> libraries_;
		//        std::string libraryScript_;
	};

} } // namespace fieldkit::dart