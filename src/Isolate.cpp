/*
*      _____  __  _____  __     ____
*     / ___/ / / /____/ / /    /    \   ofxFieldKitDart
*    / ___/ /_/ /____/ / /__  /  /  /   (c) 2013, FIELD. All rights reserved.
*   /_/        /____/ /____/ /_____/    http://www.field.io
*
*   Created by Marcus Wendt on 27/12/2013.
*/


#include "Isolate.h"

#include <map>

#include "Library.h"
#include "utilities.h"


namespace fieldkit { namespace dart {

	// ----------------------------------------------------------------------

	void Isolate::invoke(const char* function, int argc, Dart_Handle* args)
	{
		// we create an invoke object, which will automatically enter the scope 
		// on creation and close the scope when falling out of c++ scope, i.e. 
		// when this method returns.
		Invoke(library_, function, argc, args);
	}

	// ----------------------------------------------------------------------

	Dart_Handle Isolate::create(const char* typeName, int argc, Dart_Handle* args)
	{

		// Get type
		Dart_Handle type = Dart_GetType(library_, newString(typeName), 0, NULL);
		if(Dart_IsError(type)) {
			LOG_E(Dart_GetError(type));
		}

		// Invoke the unnamed constructor.
		Dart_Handle instance = Dart_New(type, Dart_Null(), argc, args);

		if (Dart_IsError(instance)) {
			//            Dart_NewApiError
			LOG_E(Dart_GetError(instance) << " while instancing '"<< type <<"'")
		}
		return instance;
	}

	// ----------------------------------------------------------------------

	Invoke::Invoke(Dart_Handle target_, std::string function_, int argc_, Dart_Handle* args_ )
		: mResultHandle(nullptr)
	{
		Dart_EnterScope();
		Dart_Handle strH = Dart_NewStringFromCString(function_.c_str());
		// invokes the actual method, captures return values in an opaque handle
		mResultHandle = Dart_Invoke(target_, strH, argc_, args_);
		if (Dart_IsError(mResultHandle)) {
			ofLogError(__FUNCTION__) << Dart_GetError(mResultHandle) << " in "<< function_;
		}
		Dart_RunLoop();
	};

	// ----------------------------------------------------------------------

	Invoke::~Invoke(){
		if (mResultHandle != nullptr) Dart_ExitScope(); 
	}

} }
