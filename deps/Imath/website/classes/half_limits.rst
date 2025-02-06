..
  SPDX-License-Identifier: BSD-3-Clause
  Copyright Contributors to the OpenEXR Project.

half Limits
###########

Constants
---------

``HALF_DENORM_MIN``
  Smallest positive denormalized half.

``HALF_NRM_MIN``
  Smallest positive normalized half.

``HALF_MIN``
  Smallest positive normalized half.

``HALF_MAX``
  Largest positive half.

``HALF_EPSILON``
  Smallest positive e for which half(1.0 + e) != half(1.0)

``HALF_MANT_DIG``
  Number of digits in mantissa (significand + hidden leading 1)

``HALF_DIG``
  Number of base 10 digits that can be represented without change:

    floor( (``HALF_MANT_DIG`` - 1) * log10(2) ) => 3.01... -> 3

``HALF_DECIMAL_DIG``
  Number of base-10 digits that are necessary to uniquely represent
  all distinct values:

    ceil(``HALF_MANT_DIG`` * log10(2) + 1) => 4.31... -> 5

``HALF_RADIX``
  Base of the exponent.

``HALF_DENORM_MIN_EXP``
  Minimum negative integer such that ``HALF_RADIX`` raised to the
  power of one less than that integer is a normalized half.

``HALF_MAX_EXP``
  Maximum positive integer such that ``HALF_RADIX`` raised to the
  power of one less than that integer is a normalized half.

``HALF_DENORM_MIN_10_EXP``
  Minimum positive integer such that 10 raised to that power is a
  normalized half.

``HALF_MAX_10_EXP``
  Maximum positive integer such that 10 raised to that power is a
  normalized half.

``std::numeric_limits<half>``
-----------------------------

The ``half`` type provides specializations for
``std::numeric_limits<half>``:

+----------------------------------------------------------------------------------------------------------------------------+----------------------------+
| `std::numeric_limits<half>::min() <https://en.cppreference.com/w/cpp/types/numeric_limits/min>`_                           | ``HALF_MIN``               |
+----------------------------------------------------------------------------------------------------------------------------+----------------------------+
| `std::numeric_limits<half>::max() <https://en.cppreference.com/w/cpp/types/numeric_limits/max>`_                           | ``HALF_MAX``               |
+----------------------------------------------------------------------------------------------------------------------------+----------------------------+
| `std::numeric_limits<half>::lowest() <https://en.cppreference.com/w/cpp/types/numeric_limits/lowest>`_                     | ``-HALF_MAX``              |
+----------------------------------------------------------------------------------------------------------------------------+----------------------------+
| `std::numeric_limits<half>::digits <https://en.cppreference.com/w/cpp/types/numeric_limits/digits>`_                       | ``HALF_MANT_DIG``          |
+----------------------------------------------------------------------------------------------------------------------------+----------------------------+
| `std::numeric_limits<half>::digits10 <https://en.cppreference.com/w/cpp/types/numeric_limits/digits10>`_                   | ``HALF_DIG``               |
+----------------------------------------------------------------------------------------------------------------------------+----------------------------+
| `std::numeric_limits<half>::max_digits10 <https://en.cppreference.com/w/cpp/types/numeric_limits/max_digits10>`_           | ``HALF_DECIMAL_DIG``       |
+----------------------------------------------------------------------------------------------------------------------------+----------------------------+
| `std::numeric_limits<half>::is_signed <https://en.cppreference.com/w/cpp/types/numeric_limits/is_signed>`_                 | ``true``                   |
+----------------------------------------------------------------------------------------------------------------------------+----------------------------+
| `std::numeric_limits<half>::is_integer <https://en.cppreference.com/w/cpp/types/numeric_limits/is_integer>`_               | ``false``                  |
+----------------------------------------------------------------------------------------------------------------------------+----------------------------+
| `std::numeric_limits<half>::is_exact <https://en.cppreference.com/w/cpp/types/numeric_limits/is_exact>`_                   | ``false``                  |
+----------------------------------------------------------------------------------------------------------------------------+----------------------------+
| `std::numeric_limits<half>::radix <https://en.cppreference.com/w/cpp/types/numeric_limits/radix>`_                         | ``HALF_RADIX``             |
+----------------------------------------------------------------------------------------------------------------------------+----------------------------+
| `std::numeric_limits<half>::epsilon() <https://en.cppreference.com/w/cpp/types/numeric_limits/epsilon>`_                   | ``HALF_EPSILON``           |
+----------------------------------------------------------------------------------------------------------------------------+----------------------------+
| `std::numeric_limits<half>::round_error() <https://en.cppreference.com/w/cpp/types/numeric_limits/round_error()>`_         | ``0.5``                    |
+----------------------------------------------------------------------------------------------------------------------------+----------------------------+
| `std::numeric_limits<half>::min_exponent <https://en.cppreference.com/w/cpp/types/numeric_limits/min_exponent>`_           | ``HALF_DENORM_MIN_EXP``    |
+----------------------------------------------------------------------------------------------------------------------------+----------------------------+
| `std::numeric_limits<half>::min_exponent10 <https://en.cppreference.com/w/cpp/types/numeric_limits/min_exponent10>`_       | ``HALF_DENORM_MIN_10_EXP`` |
+----------------------------------------------------------------------------------------------------------------------------+----------------------------+
| `std::numeric_limits<half>::max_exponent <https://en.cppreference.com/w/cpp/types/numeric_limits/max_exponent>`_           | ``HALF_MAX_EXP``           |
+----------------------------------------------------------------------------------------------------------------------------+----------------------------+
| `std::numeric_limits<half>::max_exponent10 <https://en.cppreference.com/w/cpp/types/numeric_limits/max_exponent10>`_       | ``HALF_MAX_10_EXP``        |
+----------------------------------------------------------------------------------------------------------------------------+----------------------------+
| `std::numeric_limits<half>::has_infinity <https://en.cppreference.com/w/cpp/types/numeric_limits/has_infinity>`_           | ``true``                   |
+----------------------------------------------------------------------------------------------------------------------------+----------------------------+
| `std::numeric_limits<half>::has_quiet_NaN <https://en.cppreference.com/w/cpp/types/numeric_limits/has_quiet_NaN>`_         | ``true``                   |
+----------------------------------------------------------------------------------------------------------------------------+----------------------------+
| `std::numeric_limits<half>::has_signaling_NaN <https://en.cppreference.com/w/cpp/types/numeric_limits/has_signaling_NaN>`_ | ``true``                   |
+----------------------------------------------------------------------------------------------------------------------------+----------------------------+
| `std::numeric_limits<half>::has_denorm <https://en.cppreference.com/w/cpp/types/numeric_limits/denorm_style>`_             | ``std::denorm_present``    |
+----------------------------------------------------------------------------------------------------------------------------+----------------------------+
| `std::numeric_limits<half>::has_denorm_loss <https://en.cppreference.com/w/cpp/types/numeric_limits/has_denorm_loss>`_     | ``false``                  |
+----------------------------------------------------------------------------------------------------------------------------+----------------------------+
| `std::numeric_limits<half>::infinity() <https://en.cppreference.com/w/cpp/types/numeric_limits/infinity()>`_               | ``half::posInf()``         |
+----------------------------------------------------------------------------------------------------------------------------+----------------------------+
| `std::numeric_limits<half>::quiet_NaN() <https://en.cppreference.com/w/cpp/types/numeric_limits/quiet_NaN()>`_             | ``half::qNan()``           |
+----------------------------------------------------------------------------------------------------------------------------+----------------------------+
| `std::numeric_limits<half>::signaling_NaN() <https://en.cppreference.com/w/cpp/types/numeric_limits/signaling_NaN()>`_     | ``half::sNan()``           |
+----------------------------------------------------------------------------------------------------------------------------+----------------------------+
| `std::numeric_limits<half>::denorm_min() <https://en.cppreference.com/w/cpp/types/numeric_limits/denorm_min()>`_           | ``HALF_DENORM_MIN``        |
+----------------------------------------------------------------------------------------------------------------------------+----------------------------+

  
