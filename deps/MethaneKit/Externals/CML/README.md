## Configurable Math Library

For CML version 1, please see https://github.com/demianmnave/CML1.

## License

The Configurable Math Library (CML) is released under the [Boost Software
License, Version 1.0.](http://www.boost.org/LICENSE_1_0.txt).


## Status

[![Build status](https://ci.appveyor.com/api/projects/status/r3l3xnhxe8djjimg/branch/master?svg=true)](https://ci.appveyor.com/project/demianmnave/cml/branch/master)
[![Build Status](https://travis-ci.org/demianmnave/CML.svg?branch=master)](https://travis-ci.org/demianmnave/CML/builds)


## Using the CML

Currently, CML does not have a `make install` option.  As it is header-only, it is simple enough to copy the `cml` header directory into your project, and setup your build to reference it.


## Running Tests

To run the test suite from a command prompt using a Makefile-like generator, start in your build directory and execute:

`cmake . -G<generator name> -DCML_BUILD_TESTING=On -DCML_BUILD_TYPE=RELEASE`

Then, to build the tests (again from your build directory):

`cmake --build . --config Release`

You can run the full test suite from your build directory by executing:

`ctest -C Release`

If you have multiple CPUs (e.g. 4 in this case), you can speed things up a bit using, for example:

`cmake --build . --config Release -- -j4`

`ctest -C Release -j4`

Visual Studio 12 (2013), 14 (2015), and 15 (2017) are also supported, as are XCode 7.3 and 8.3.
