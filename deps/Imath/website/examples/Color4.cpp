#include <Imath/ImathColor.h>
#include <cassert>

void
color4_example()
{
    Imath::C4f   r (1.0f, 0.0f, 0.0f, 1.0f);
    Imath::C4f   g (0.0f, 1.0f, 0.0f, 1.0f);
    Imath::C4f   b (0.0f, 0.0f, 1.0f, 1.0f);
    
    Imath::C4f   w = r + g + b;

    assert (w.r == 1.0f);
    assert (w.g == 1.0f);
    assert (w.b == 1.0f);
    assert (w.a == 3.0f);
}
