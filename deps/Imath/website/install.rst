..
  SPDX-License-Identifier: BSD-3-Clause
  Copyright Contributors to the OpenEXR Project.

.. _Install:

Install
=======

.. toctree::
   :caption: Install
             
The Imath library is available for download and installation in
binary form via package managers on many Linux distributions.

Refer to the current version of Imath on various major Linux distros at
`repology.org <https://repology.org/project/imath/versions>`_:

.. image:: https://repology.org/badge/vertical-allrepos/imath.svg?exclude_unsupported=1&columns=4&header=Imath%20Packaging%20Status&minversion=3.0
   :target: https://repology.org/project/imath/versions

Older versions of Imath were distributed as a component of OpenEXR
called ``ilmbase``. We do not recommend using these outdated
versions.

To install via ``yum`` on RHEL/CentOS:

.. code-block::

    % sudo yum makecache
    % sudo yum install imath

To install via ``apt-get`` on Ubuntu:

.. code-block::

    % sudo apt-get update
    % sudo apt-get install imath

macOS
-----

On macOS, install via `Homebrew <https://formulae.brew.sh/formula/imath>`_:

.. code-block::

   % brew install imath

Alternatively, you can install on macOS via `MacPorts
<https://ports.macports.org/port/imath/>`_:

.. code-block::

   % port install imath

Windows
-------

Install via `vcpkg <https://vcpkg.io/en/packages>`_:

.. code-block::

   % .\vcpkg install imath

Python
------

Please note that ``pip install imath`` installs the `imath
<https://pypi.org/project/imath/>`_ module, which is not
affiliated with the OpenEXR project or the ASWF. Please direct
questions there.

Build from Source
-----------------

Imath builds on Linux, macOS, Microsoft Windows via CMake, and is
cross-compilable on other systems.

Download the source from the `GitHub releases page
<https://github.com/AcademySoftwareFoundation/Imath/releases>`_
page, or clone the `repo <https://github.com/AcademySoftwareFoundation/Imath>`_.

The ``release`` branch of the repo always points to the most advanced
release.


Prerequisites
~~~~~~~~~~~~~

Make sure these are installed on your system before building Imath:

* Imath requires CMake version 3.12 or newer
* C++ compiler that supports C++11

The instructions that follow describe building Imath with CMake.

Linux/macOS
~~~~~~~~~~~

To build via CMake, you need to first identify three directories:

1. The source directory, i.e. the top-level directory of the
   downloaded source archive or cloned repo, referred to below as ``$srcdir``
2. A temporary directory to hold the build artifacts, referred to below as
   ``$builddir``
3. A destination directory into which to install the
   libraries and headers, referred to below as ``$installdir``.  

To build:

.. code-block::

    % cd $builddir
    % cmake $srcdir --install-prefix $installdir
    % cmake --build $builddir --target install --config Release

Note that the CMake configuration prefers to apply an out-of-tree
build process, since there may be multiple build configurations
(i.e. debug and release), one per folder, all pointing at once source
tree, hence the ``$builddir`` noted above, referred to in CMake
parlance as the *build directory*. You can place this directory
wherever you like.

See the CMake Configuration Options section below for the most common
configuration options especially the install directory. Note that with
no arguments, as above, ``make install`` installs the header files in
``/usr/local/include``, the object libraries in ``/usr/local/lib``, and the
executable programs in ``/usr/local/bin``.

Windows
~~~~~~~

Under Windows, if you are using a command line-based setup, such as
cygwin, you can of course follow the above. For Visual Studio, cmake
generators are "multiple configuration", so you don't even have to set
the build type, although you will most likely need to specify the
install location.  Install Directory By default, ``make install``
installs the headers, libraries, and programs into ``/usr/local``, but you
can specify a local install directory to cmake via the
``CMAKE_INSTALL_PREFIX`` variable:

.. code-block::

    % cmake .. -DCMAKE_INSTALL_PREFIX=$Imath_install_directory

Library Names
-------------

By default the installed libraries follow a pattern for how they are
named. This is done to enable multiple versions of the library to be
installed and targeted by different builds depending on the needs of
the project. A simple example of this would be to have different
versions of the library installed to allow for applications targeting
different VFX Platform years to co-exist.

If you are building dynamic libraries, once you have configured, built,
and installed the libraries, you should see the following pattern of
symlinks and files in the install lib folder:

.. code-block::

    libImath.so -> libImath.so.31
    libImath.so.$SOVERSION -> libImath.so.$SOVERSION.$RELEASE
    libImath.so.$SOVERSION.$RELEASE (the shared object file)

The ``SOVERSION`` number identifies the ABI version. Each Imath
release that changes the ABI in backwards-incompatible ways increases
this number. By policy, this changes only for major and minor
releases, never for patch releases. ``RELEASE`` is the
``MAJOR.MINOR.PATCH`` release name. For example, the resulting shared
library filename is ``libImath.so.31.3.1.10`` for Imath release
v3.1.10. This naming scheme reinforces the correspondence between the
real filename of the ``.so`` and the release it corresponds to.

Library Suffix
~~~~~~~~~~~~~~

The ``IMATH_LIB_SUFFIX`` CMake option designates a suffix for the
library and appears between the library base name and the
``.so``. This defaults to encode the major and minor version, as in
``-3_1``:

.. code-block::

    libImath.so -> libImath-3_1.so
    libImath-3_1.so -> libImath-3_1.so.30
    libImath-3_1.so.30 -> libImath-3_1.so.30.3.1.10
    libImath-3_1.so.30.3.1.10 (the shared object file)
    
Porting Applications from OpenEXR v2 to v3
------------------------------------------

See the :doc:`PortingGuide` for details about differences from previous
releases and how to address them. Also refer to the porting guide for
details about changes to Imath.

Building the Website
--------------------

The Imath technical documentation at `https://imath.readthedocs.io
<https://imath.readthedocs.io>`_ is generated via `Sphinx
<https://www.sphinx-doc.org>`_ with the `Breathe
<https://breathe.readthedocs.io>`_ extension using information
extracted from header comments by `Doxygen <https://www.doxygen.nl>`_,
using the `sphinx-press-theme
<https://pypi.org/project/sphinx-press-theme>`_, and is hosted by
`Read the Docs <https://readthedocs.org/projects/imath/>`_.
The website source is in `restructured text
<https://www.sphinx-doc.org/en/master/usage/restructuredtext/basics.html>`_
in the ``website`` directory.

To build the website locally from the source headers and
``.rst`` files, set the CMake option ``BUILD_WEBSITE=ON``. This adds
``website`` CMake target. Generation is off by default.

Building the website requires that ``sphinx``, ``breathe``, and
``doxygen`` are installed. It further requires the `sphinx-press-theme
<https://pypi.org/project/sphinx-press-theme>`_. Complete dependencies are
described in the `requirements.txt
<https://github.com/AcademySoftwareFoundation/imath/blob/main/website/requirements.txt>`_
file.

On Debian/Ubuntu Linux:

.. code-block::

    % apt-get install doxygen python3-sphinx
    % pip3 install breathe
    % pip3 install sphinx_press_theme
   
    % mkdir _build
    % cd _build
    % cmake .. -DBUILD_WEBSITE=ON
    % cmake --build . --target website 

CMake Build-time Configuration Options
--------------------------------------

The default CMake configuration options are stored in
``cmake/ImathSetup.cmake``. To see a complete set of option
variables, run:

.. code-block::

    % cmake -LAH $imath_source_directory

You can customize these options three ways:

1. Modify the ``.cmake`` files in place.
2. Use the UI ``cmake-gui`` or ``ccmake``.
3. Specify them as command-line arguments when you invoke cmake.

Library Naming Options
~~~~~~~~~~~~~~~~~~~~~~

* ``IMATH_LIB_SUFFIX``

  Append the given string to the end of all the Imath
  libraries. Default is ``-<major>_<minor>`` version string. Please
  see the section on library names

Imath Dependency
~~~~~~~~~~~~~~~~

* ``CMAKE_PREFIX_PATH``

  The standard CMake path in which to
  search for dependencies, Imath in particular.  A comma-separated
  path. Add the root directory where Imath is installed.

Namespace Options
~~~~~~~~~~~~~~~~~

* ``IMATH_NAMESPACE``

  Public namespace alias for Imath. Default is ``Imath``.

* ``IMATH_INTERNAL_NAMESPACE``

  Real namespace for Imath that will end up in compiled
  symbols. Default is ``Imath_<major>_<minor>``.

* ``IMATH_NAMESPACE_CUSTOM``

  Whether the namespace has been customized (so external users know)

Component Options
~~~~~~~~~~~~~~~~~

* ``BUILD_TESTING``

  Build the testing tree. Default is ``ON``.  Note that
  this causes the test suite to be compiled, but it is not
  executed. To execute the suite, run "make test".

Additional CMake Options
~~~~~~~~~~~~~~~~~~~~~~~~

See the CMake documentation for more information (https://cmake.org/cmake/help/v3.12/).

* ``CMAKE_BUILD_TYPE``

  For builds when not using a multi-configuration generator. Available
  values: ``Debug``, ``Release``, ``RelWithDebInfo``, ``MinSizeRel``

* ``BUILD_SHARED_LIBS``

  This is the primary control whether to build static libraries or
  shared libraries / dlls (side note: technically a convention, hence
  not an official ``CMAKE_`` variable, it is defined within cmake and
  used everywhere to control this static / shared behavior)

* ``IMATH_CXX_STANDARD``

  C++ standard to compile against. This obeys the global
  ``CMAKE_CXX_STANDARD`` but doesn’t force the global setting to
  enable sub-project inclusion. Default is ``14``.

* ``CMAKE_CXX_COMPILER``

  The C++ compiler.        

* ``CMAKE_C_COMPILER``

  The C compiler.
  
* ``CMAKE_INSTALL_RPATH``

  For non-standard install locations where you don’t want to have to
  set ``LD_LIBRARY_PATH`` to use them

* ``CMAKE_EXPORT_COMPILE_COMMANDS``

  Enable/Disable output of compile commands during generation. Default
  is ``OFF``.

* ``CMAKE_VERBOSE_MAKEFILE``

  Echo all compile commands during make. Default is ``OFF``.

Cross Compiling / Specifying Specific Compilers
-----------------------------------------------

When trying to either cross-compile for a different platform, or for
tasks such as specifying a compiler set to match the `VFX reference
platform <https://vfxplatform.com>`_, cmake provides the idea of a
toolchain which may be useful instead of having to remember a chain of
configuration options. It also means that platform-specific compiler
names and options are out of the main cmake file, providing better
isolation.

A toolchain file is simply just a cmake script that sets all the
compiler and related flags and is run very early in the configuration
step to be able to set all the compiler options and such for the
discovery that cmake performs automatically. These options can be set
on the command line still if that is clearer, but a theoretical
toolchain file for compiling for VFX Platform 2015 is provided in the
source tree at ``cmake/Toolchain-Linux-VFX_Platform15.cmake`` which
will hopefully provide a guide how this might work.

For cross-compiling for additional platforms, there is also an
included sample script in ``cmake/Toolchain-mingw.cmake`` which shows
how cross compiling from Linux for Windows may work. The compiler
names and paths may need to be changed for your environment.

More documentation:

* Toolchains: https://cmake.org/cmake/help/v3.12/manual/cmake-toolchains.7.html
* Cross compiling: https://gitlab.kitware.com/cmake/community/-/wikis/doc/cmake/CrossCompiling

Ninja
-----

If you have `Ninja <https://ninja-build.org>`_ installed, it is faster
than make. You can generate ninja files using cmake when doing the
initial generation:

.. code-block::

    % cmake -G “Ninja” ..

