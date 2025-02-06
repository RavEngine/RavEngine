#include <Imath/ImathLine.h>
#include <cassert>

void
line3_example()
{
    Imath::V3f   a (0.0f, 0.0f, 0.0f);
    Imath::V3f   b (1.0f, 1.0f, 1.0f);

    Imath::Line3f line (a, b);
  
    assert (line.pos == a);
    assert (line.dir == (b-a).normalized());
    
    Imath::V3f   c (0.5f, 0.5f, 0.5f);

    float f = line.distanceTo (c);
    assert (Imath::equalWithAbsError (f, 0.0f, 0.0001f));

    Imath::V3f p = line (0.5f); // midpoint, i.e. 0.5 units from a along (b-a)

    assert (p.equalWithAbsError (Imath::V3f (0.288675f, 0.288675f, 0.288675f), 0.0001f));
}
