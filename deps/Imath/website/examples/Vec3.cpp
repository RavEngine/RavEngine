#include <Imath/ImathVec.h>
#include <cassert>

void
vec3_example()
{
    Imath::V3f   a (1.0f, 2.0f, 3.0f);
    Imath::V3f   b; // b is uninitialized

    b.x = a[0];
    b.y = a[1];
    b.z = a[2];

    assert (a == b);

    assert (a.length() == sqrt (a ^ a));

    a.normalize();
    assert (Imath::equalWithAbsError (a.length(), 1.0f, 1e-6f));
}
