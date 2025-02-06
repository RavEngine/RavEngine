#include <Imath/ImathFrustum.h>
#include <cassert>

void
frustum_example()
{
    float near = 1.7f;
    float far = 567.0f;
    float left = -3.5f;
    float right = 2.0f;
    float top = 0.9f;
    float bottom = -1.3f;

    Imath::Frustumf frustum (near, far, left, right, top, bottom, false);

    Imath::M44f m = frustum.projectionMatrix();

    Imath::V3f p (1.0f, 1.0f, 1.0f);
    Imath::V2f s = frustum.projectPointToScreen (p);

    assert (s.equalWithAbsError (Imath::V2f (-0.345455f, -1.36364f), 0.0001f));
}
