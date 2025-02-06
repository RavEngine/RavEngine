#include <Imath/ImathVec.h>
#include <cassert>

void
vec2_example()
{
    Imath::V2f   a (1.0f, 2.0f);
    Imath::V2f   b; // b is uninitialized

    b.x = a[0];
    b.y = a[1];

    assert (a == b);

    assert (a.length() == sqrt (a ^ a));

    a.normalize();
    assert (Imath::equalWithAbsError (a.length(), 1.0f, 1e-6f));
}
