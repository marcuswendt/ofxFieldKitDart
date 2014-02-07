```
      _____  __  _____  __     ____
     / ___/ / / /____/ / /    /    \   ofxFieldKitDart
    / ___/ /_/ /____/ / /__  /  /  /   (c) 2013, FIELD. All rights reserved.
   /_/        /____/ /____/ /_____/    http://www.field.io

```

Dart Virtual Machine embedding toolkit to easily add scripting + live-coding functionality to native C++ applications.

Although my use case is within an OpenFrameworks OpenGL app, there should be no problem (and dependency) to use this library in other contexts e.g. together with the Cinder creative coding library.

**Version 0.1**

The basic bits are in place but I haven't used this in a production scenario yet. So beware!

The Dart API offers a lot more functionality than I'm exposing at the moment.

## Installation instructions

Binaries for OS X / i386 are included via submodule reference (warning: they are big, ~250mb just for mac osx x86 ). To checkout everything in one go, do:

```Shell
cd <openframeworks/addons>
git clone --recursive git://github.com/field/ofxFieldKitDart.git 
```

## Example


```dart
class Sketch {
	Sketch() {
		print("created Sketch");
	}

	update() {
		print("update");
	}
}

main() 
{
	print("Hello, Darter!");
	print("Bingo!");
}
```

```C++
dartVM.Init(true);
script = dartVM.LoadScript("../Resources/main.dart");
script->Invoke("main");
sketch = script->New("Sketch");
```


## References

Dart Runtime components
https://github.com/field/dart-runtime

Richard Eakins cinder-dart block
https://github.com/richardeakin/Cinder-Dart

FieldKit.cpp/ ScriptKit an earlier solution of mine integrating v8 into Cinder/ C++ apps.
https://github.com/field/FieldKit.cpp


## (tig) Building dart for openFrameworks / VS2012

Note that we're using dart 1.0 for now, until the API stabilises.

* install depot-tools

* then, in the depot-tools folder, using cmd.exe:

	mkdir dart-repo
	
	cd dart-repo

	gclient config https://dart.googlecode.com/svn/branches/1.0/deps/standalone.deps

	git svn clone https://dart.googlecode.com/svn/branches/1.0/dart dart

	gclient sync -n && gclient runhooks

* there should be a vs solution in runtime now
* then copy cygwin from depot-tools to runtime/../third-party/cygwin
* open dart.sln with visual studio 
* switch all relevant targets to c/c++ > Code Generation > Runtime Library > Multi-threaded Debug DLL (/MDd) [debug] , Multi-threaded DLL (/MD) [release]
* compile vs2012

## License

Released under the BSD license. Full details in the included LICENSE.txt file.

Copyright (c) 2013 Marcus Wendt <marcus@field.io>

With additions & fixes 

Copyright (c) 2014 tgfrerer <tim@poniesandlight.co.uk>

