SOURCE: http://staffwww.itn.liu.se/~stegu/aqsis/aqsis-newnoise/
GIT-HUB C-VERSION: https://github.com/stegu/perlin-noise

This directory contains my completely rewritten noise
implementation for Aqsis. It is not 100% compatible
with the old implementation, but almost: you only need
to remove the offset and scaling from "shadeops.cpp".
(I rescaled the noise to [0,1] inside CqNoise instead of
requiring that to be done externally, it seemed logical.)
The archive contains an edited version of "shadeops.cpp"
as well, where I think I also wired the 4D noise and all
the pnoise variants in correctly, but you'd better check
that, I haven't even test compiled that file. As far as
I can see, "shadeops.cpp" was the only place where the
CqNoise class was used in the 1.1 source tree.

I restructured the code quite a bit. There's actually
nothing left of the old code.

"noise1234.h" and "noise1234.cpp" contains the
actual noise functions, in a class "Noise1234" which is
deliberately decoupled from the Aqsis data types and other
external dependencies. This is a highly reusable class
that does nothing else than define eight very basic
noise functions. I liked that approach, but you might
want to reinsert "TqFloat" for "float", e.g. if you want
to make the noise configurable to use "double" instead.
(I don't know enough C++ yet to make a templatized version,
although I realise that would be a good solution here.)

"noise.h" and "noise.cpp" contain the previously defined
class "CqNoise", now merely a wrapper class to "Noise1234".
The new functions have the same argument lists as the old
ones, but as I said, the range of return values is now
[0,1] to better match their use, not [-1, 1] as before.
Aprt from that small change, these two classes together
should be a drop-in replacement for the old CqNoise class.

Note that all the methods of the CqNoise class are
now statically callable, and no initialisation is
required for the class. You don't really have to
instance "m_noise" any longer, although I much prefer
"m_noise.something()" to "CqNoise::something()" in
the code for readability (I'm used to Java), so I
left it as it is.

I did not have the right tools (or skills) to compile
the entire project, but I have tested all the noise
functions separately using header stubs to define the
relevant Aqsis data types and macros, and the tests ran
fine (after some debugging). All functions appear to
give correct visual results with good performance.
I might have missed some obscure bug, but as far as
I can see, my code is correct.

I also validated it against BMRT and PRMan.
1D, 2D and 3D noise make a very close visual match with
PRMan and BMRT in terms of both appearance and statistics.
The 4D noise is very close to that of BMRT, but not quite
as uniform as PRMan's if viewed in planar 2D slices
(two of x,y,z,t held constant).
4D noise should really not be applied that way, but I
suspect Pixar is doing something to prevent some users'
ignorant abuse of 4D noise in planar slices to look bad.
I'm not sure exactly what, but they probably apply some
simple local distorsion to the (x,y,z,t) domain before
evaluating noise4(). I won't do that for this version,
it only complicates things, and the 4D noise looks good
if it is only applied as it should, to curved surfaces
or in a rotated space. If people complain, I can look
into it, but this is a clean and straightforward
implementation of Perlin noise, as defined by Ken Perlin.

This noise actually looks better in at least one respect
(no ugly axis-aligned artefacts) than the noise in
PRMan 12.5.2, Pixar's current renderer. I was really
happy to see that. :)


  Stefan Gustavson 2005-11-09