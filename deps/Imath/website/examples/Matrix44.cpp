#include <Imath/ImathMatrix.h>
#include <Imath/ImathMatrixAlgo.h>
#include <cassert>

void
matrix44_example()
{
    Imath::M44f M (Imath::UNINITIALIZED); // uninitialized

    M.makeIdentity();
    assert (M[0][0] == 1.0f);
    assert (M[0][1] == 0.0f);

    Imath::M44f Minv = M.inverse();

    Imath::M44f R;
    assert (R == Imath::identity44f);

    R.rotate (Imath::V3f (0.02f, M_PI/4, 0.0f));
    
    M = R * M;

    Imath::V3f v3 (1.0f, 0.0f, 0.0f);
    Imath::V4f v4 (1.0f, 0.0f, 0.0f, 1.0f);

    Imath::V3f r3 = v3 * M;
    assert (r3.equalWithAbsError (Imath::V3f (0.707107f, 0.0f, -0.7071070f), 1e-6f));

    Imath::V4f r4 = v4 * M;
    assert (r4.equalWithAbsError (Imath::V4f (0.707107f, 0.0f, -0.7071070f, 1.0f), 1e-6f));
}
