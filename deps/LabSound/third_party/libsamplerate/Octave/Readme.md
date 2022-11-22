# Gneerating Filter Coefficients

The scripts below are used in [GNU Octave][GNU Octave] which should be available
on most platforms including Linux, Mac and Windows.

On Debian and other Debian derived Linux distributions, Octave can be installed
using: `sudo apt install octave octave-signal`.

Filter coefficients can be generated as follows by running the `octave` command
in the same directory as the scripts and this Readme file:

```
octave:1> pkg load signal
octave:2> f = make_filter (8, 128, 100.3) ;
# f = make_fip_filter (8, 128, 100.3) ;
#   Coeff. count     : 4922
#   Fudge factor     : 1.2018769
#   Pass band width  : 0.0039062499 (should be 0.0039062500)
#   Stop band atten. : 100.71 dB
#   -3dB band Width  : 0.490
#   half length      : 2464
#   increment        : 128
```

The parameters above generate the coefficient set used in `src/fastest_coeffs.h`.

[GNU Octave]: https://www.gnu.org/software/octave/
