#include <Imath/ImathQuat.h>
#include <cassert>

void
quat_example()
{
    Imath::Quatf q (2.0f, 3.0f, 4.0f, 5.0f);
    assert (q.r == 2.0f && q.v == Imath::V3f (3.0f, 4.0f, 5.0f));

    Imath::Quatf r (1.0f, 0.0f, 0.0f, 1.0f);
    assert (r.inverse() == Imath::Quatf (0.5f, 0.0f, 0.0f, -0.5f));
}
