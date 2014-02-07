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

	public:

		/// factory function
		static shared_ptr<DartVM> create( std::string snapshotFilePath ) {
			shared_ptr<DartVM> dartVM;
			dartVM = shared_ptr<DartVM> (new DartVM(snapshotFilePath));
			return dartVM;
		};
		/// constructor
		explicit DartVM( std::string snapshotFilePath_ );

		~DartVM();

		//! prepares the dart virtual machine
		void Init(const bool checkedMode = true);

		void LoadSnapshot(const std::string file);

		//! load, parse and compile a script, returns a script isolate
		/// TODO: who owns the Isolate?
		Isolate* LoadScript(const std::string scriptFile);

		void add(Library* library);

		// Accessors
		std::string getVersion();
		uint8_t* getSnapshot() const { 
			if (mSnapshotBuffer.size() ==0 ){
				ofLogError() << __FUNCTION__ << " : requested contents of empty snapshot.";
				return NULL;
			} else {
				return (uint8_t*)mSnapshotBuffer.getBinaryBuffer(); 
			}
		}
		std::vector<Library*> getLibraries() { return libraries_; }

		//        std::string getLibraryScript() { return libraryScript_; }

	private:
		ofBuffer mSnapshotBuffer;
		std::vector<Library*> libraries_;
		//        std::string libraryScript_;
	};

} } // namespace fieldkit::dart