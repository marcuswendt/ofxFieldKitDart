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


## License

Released under the BSD license. Full details in the included LICENSE.txt file.

Copyright (c) 2013 Marcus Wendt <marcus@field.io>



## TODO

* built-in libraries as modules, especially dart:io
* replace LOG_E with Dart_NewApiError etc.
* More spectacular samples ;-)
* Snapshots don't work on Windows, Dart_CreateIsolate() quit the app because the snapshot is not kFull. To fix this the snapshot is set ```NULL``` on windows. DartVM.cpp\(\#178\)