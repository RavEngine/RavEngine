..
  SPDX-License-Identifier: BSD-3-Clause
  Copyright Contributors to the OpenEXR Project.

.. _About Imath:

About Imath
===========

.. toctree::
   :caption: About
             
Imath is maintained by the OpenEXR project, a part of the `Academy
Software Foundation <https://www.aswf.io>`_.  The library were
originally developed at Industrial Light & Magic and first released as
open source in 2003.

Read the origin story of OpenEXR and Imath, and the ``half`` 16-bit
float type, in particular, on the `ASWF Blog
<https://www.aswf.io/news/aswf-deep-dive-openexr-origin-story-part-1>`_.

Imath is Version 3 because it was previously distributed as a
component of OpenEXR v1 and v2.

OpenEXR and Imath is included in the `VFX Reference Platform <https://vfxplatform.com>`_.

New Features in 3.1
-------------------

The 3.1 release of Imath introduces optimized half-to-float and
float-to-half conversion using the F16C SSE instruction set extension,
if available. These single-instruction conversions offer a 5-10x
speedup for float-to-half and 2x speedup for half-to-float over
Imath/half's traditional table-based conversion (timings depend on the
data).

In the absence of the F16C instruction set, the lookup-table-based
conversion from half to float is still the default, but Imath 3.1 also
introduces an optimized bit-shift conversion algorithm as a
compile-time option that does not require lookup tables, for
architectures where memory is limited. The float-to-half conversion
also no longer requires an exponent lookup table, further reducing
memory requirements.

These new conversions generate the same values as the traditional
methods, which ensures backwards compatibility.  See :doc:`install`
for more installation and configuration options.

Also, ``half.h`` can now be included in pure C code for a definition
of the type and for conversions between half and float.

OpenEXR/Imath 2.x to 3.x Porting Guide
--------------------------------------

See the :doc:`PortingGuide` for help in restructing old code to work
with recent releases of OpenEXR and Imath.

Credits
-------

The ILM Imath library and the ``half`` data type were originally
designed and implemented at Industrial Light & Magic by Florian Kainz,
Wojciech Jarosz, Rod Bogart, and others. Drew Hess packaged and
adapted ILM's internal source code for public release.

For a complete list of contributors see `CONTRIBUTORS.md
<https://github.com/AcademySoftwareFoundation/Imath/blob/main/CONTRIBUTORS.md>`_.




