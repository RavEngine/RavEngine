#include <Imath/ImathPlane.h>
#include <cassert>

void
plane3_example()
{
    Imath::V3f a (1.0f, 0.0f, 0.0f);
    Imath::V3f b (0.0f, 1.0f, 0.0f);
    Imath::V3f c (0.0f, 0.0f, 1.0f);

    Imath::Plane3f p (a, b, c);

    Imath::V3f n (1.0f,  1.0f,  1.0f);
    n.normalize();

    assert (p.normal == n);

    Imath::V3f o (0.0f, 0.0f, 0.0f);
    float d = p.distanceTo (o);
    assert (Imath::equalWithAbsError (d, -0.57735f, 1e-6f));
}
