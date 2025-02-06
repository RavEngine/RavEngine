#include <Imath/half.h>
#include <math.h>

void
half_example()
{
    half a (3.5);
    float b (a + sqrt (a));
    a += b;
    b += a;
    b = a + 7;
}

