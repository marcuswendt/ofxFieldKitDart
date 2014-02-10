/*
*      _____  __  _____  __     ____
*     / ___/ / / /____/ / /    /    \   ofxFieldKitDart
*    / ___/ /_/ /____/ / /__  /  /  /   (c) 2013, FIELD. All rights reserved.
*   /_/        /____/ /____/ /_____/    http://www.field.io
*
*   Created by Marcus Wendt on 27/12/2013.
*/


#pragma once

#include "dart_api.h"
#include "ofMain.h"

namespace fieldkit { namespace dart {


	// ff decl.
	class Library;
	class Invoke;

	class Isolate {
	public:
		Isolate(Dart_Isolate isolate, Dart_Handle library)
			: isolate_(isolate),
			library_(library) {}

		~Isolate() {}

		/// Invokes a function from the scripts library, without caring about 
		/// return values. See this as a procedure call.
		void invoke(const char* function, int argc = 0, Dart_Handle* args = 0);

		/// Invokes a constructor, creating a new object.
		Dart_Handle create(const char* type, int argc = 0, Dart_Handle* args = 0);

		/// Accessors
		Dart_Isolate getIsolate() { return isolate_; }
		Dart_Handle getLibrary() { return library_; }

	private:
		Dart_Isolate isolate_;
		Dart_Handle library_;
		friend class Invoke;

	};

	// -----------------------------------------------------------------------------

	class Invoke {
	private:
		// this will cause Dart_EnterScope() to be called 
		// everytime a new instance of this method is created.
		// and Dart_ExitScope() everytime this method gets out of scope.
		Dart_Handle mResultHandle;
		/// no copy
		Invoke(){};
	public:

		/// brief:   calls a dart method and allows access to any result handles.
		/// detail:  opens a dart scope, calls the specified function_ in the script 
		///          specified by target_ and stores a handle to the result of the invocation.
		///          the result handle will be kept alive for as long as this object is in the 
		///          current scope.
		Invoke(Dart_Handle target_, std::string function_, int argc_ = 0, Dart_Handle* args_ = 0);

		/// move constructor, makes sure to transfer ownership of the handle, and 
		/// also makes sure the handle remains unique.
		Invoke(const Invoke&& lhs_)
		: mResultHandle(std::move(lhs_.mResultHandle)){
			// make sure we have invalidated the source object handle.
			const_cast<Dart_Handle>(lhs_.mResultHandle) = nullptr;
		};


		/// closes the current scope and releases any dart result handles.
		~Invoke();

		/// returns: the current handle holding the result of the Invoke 
		/// i.e. a Handle to whatever dart returned.
		const Dart_Handle& getResult() {return mResultHandle; }

	};


} } // namespace fieldkit::dart