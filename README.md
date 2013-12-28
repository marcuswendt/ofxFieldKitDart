```
      _____  __  _____  __     ____
     / ___/ / / /____/ / /    /    \   ofxFieldKitDart
    / ___/ /_/ /____/ / /__  /  /  /   (c) 2013, FIELD. All rights reserved.
   /_/        /____/ /____/ /_____/    http://www.field.io

```

Dart scripting language bindings for OpenFrameworks.

Work in progress!


## Installation instructions

Binaries for OS X / i386 are included via submodule reference (warning: they are big, ~250mb just for mac osx x86 ). To checkout everything in one go, do:

```
cd <openframeworks/addons>
git clone --recursive git://github.com/field/ofxFieldKitDart.git 
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

* built-in libraries as modules in dartvm
* replace LOG_E with Dart_NewApiError etc.