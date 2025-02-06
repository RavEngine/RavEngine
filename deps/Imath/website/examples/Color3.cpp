#include <Imath/ImathColor.h>
#include <cassert>

void
color3_example()
{
    Imath::C3c   r (255, 0, 0);
    Imath::C3c   g (0, 255, 0);
    Imath::C3c   b (0, 0, 255);
    
    Imath::C3c   c = r + g + b;

    assert (c.x == 255);
    assert (c.x == 255);
    assert (c.x == 255);
}
