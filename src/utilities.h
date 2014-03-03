/*
*      _____  __  _____  __     ____
*     / ___/ / / /____/ / /    /    \   ofxFieldKitDart
*    / ___/ /_/ /____/ / /__  /  /  /   (c) 2013, FIELD. All rights reserved.
*   /_/        /____/ /____/ /_____/    http://www.field.io
*
*   Created by Marcus Wendt on 27/12/2013.
*/

#pragma once

#include <iostream>
#include <sstream>
#include <map>

#include "dart_api.h"

namespace fieldkit { namespace dart {

	// Logging
#define LOG(TYPE, MSG) \
	{ \
	std::stringstream ss; \
	ss << TYPE << ": " << MSG; \
	ss << std::endl; \
	std::cout << ss.str(); \
}

#define LOG_I(MSG) LOG("INFO", MSG)
#define LOG_E(MSG) LOG("ERROR", MSG)
#define LOG_W(MSG) LOG("WARNING", MSG)


	// Dart API Utilities    
	//#define EXPECT_VALID(handle)                                                         \
	//    do {                                                                             \
	//        Dart_Handle tmp_handle = (handle);                                           \
	//        if (Dart_IsError(tmp_handle)) {                                              \
	//            dart::Expect(__FILE__, __LINE__).Fail(                                   \
	//                "expected '%s' to be a valid handle but found an error handle:\n"    \
	//                "    '%s'\n",                                                        \
	//                #handle, Dart_GetError(tmp_handle));                                 \
	//        }                                                                            \
	//    } while (0)


	struct DartScope {
		DartScope() { Dart_EnterScope(); }
		~DartScope() { Dart_ExitScope(); }
	};

	static Dart_Handle handleError(Dart_Handle handle) {
		if (Dart_IsError(handle)) {
			Dart_PropagateError(handle);
		}
		return handle;
	}

	static Dart_Handle newString(const char* str)
	{
		return Dart_NewStringFromCString(str);
	}

	static std::string getString(Dart_Handle handle)
	{
		const char *result;
		Dart_StringToCString(handle, &result);
		return std::string(result);
	}

	/// (tig) macros to make inline code editing + dart glue easier.
#ifndef DART_LANG_SOURCE
#define DART_LANG_SOURCE(x) #x
#endif 

	/// prepends native_ to your function name and sets the correct function arguments for 
	/// it to be a dart native function
#define NATIVE_FUNCTION(FUNCTION) void native_##FUNCTION ( Dart_NativeArguments args )

	/// prepends native_ to your function name and sets the correct function arguments for 
	/// it to be a dart native function
#define NATIVE_STATIC_FUNCTION(FUNCTION) static void native_##FUNCTION ( Dart_NativeArguments args )

	// we need to tell dart which sendport to use to reach our native receive port.		
	// and so we create a sendport from our new native port (which i interpret as a 	
	// native receive port. Anyhting delivered in dart to this sendport will arrive at our 	 
	// native receive port.																	
#define NATIVE_PORT(NAME) void native_port_##NAME ( Dart_NativeArguments args )  \
	{																			 \
	Dart_EnterScope();															 \
	Dart_SetReturnValue(args,Dart_Null());									     \
	Dart_Port receivePort = Dart_NewNativePort(#NAME "Service",  	             \
	native_responder_##NAME,true);										         \
	if (receivePort != ILLEGAL_PORT) {										     \
	Dart_Handle sendPort = handleError(Dart_NewSendPort(receivePort));	         \
	Dart_SetReturnValue(args, sendPort);								         \
	}																		     \
	Dart_ExitScope();															 \
	}																			 \
	/// use this macro to expose the port you created with the macro above, names in 
	// DART_EXPOSE_NATIVE_PORT, NATIVE_PORT and NATIVE_RESPONDER need to match.
#define DART_EXPOSE_NATIVE_PORT(NAME) add(#NAME, native_port_##NAME);

	/// defines a native responder which will answer to calls to our native port.
#define NATIVE_RESPONDER(FUNCTION) void native_responder_##FUNCTION ( Dart_Port dest_port_id, Dart_CObject* message )

	/// calls .add(dartname, nativename) on the current library, 
	/// mapping the dart method name to the native method native_name
	/// use this macro in yout library.init() method, after youre done settin up 
	/// mSource. 
//#define DART_EXPOSE_NATIVE(FUNCTION) add(#FUNCTION, native_##FUNCTION);          \
//	mSource += DART_LANG_SOURCE (         										 \
//	void FUNCTION##() native #FUNCTION ;   							    	     \
//	); 																			 \

#define DART_EXPOSE_NATIVE(FUNCTION, ...) add(#FUNCTION, native_##FUNCTION);  \
	mSource += DART_LANG_SOURCE (         									  \
	void FUNCTION##( __VA_ARGS__ ) native #FUNCTION ;   					  \
	); 																		  \



} } // namespace fieldkit::dart