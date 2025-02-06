#include <Imath/ImathMatrix.h>
#include <Imath/ImathMatrixAlgo.h>
#include <cassert>

void
matrix33_example()
{
    Imath::M33f M (Imath::UNINITIALIZED); // uninitialized

    M.makeIdentity();
    assert (M[0][0] == 1.0f);
    assert (M[0][1] == 0.0f);

    Imath::M33f Minv = M.inverse();

    Imath::M33f R;
    assert (R == Imath::identity33f);

    R.rotate (M_PI/4);
    
    M = R * M;

    Imath::V3f v3 (1.0f, 0.0f, 0.0f);
    Imath::V3f r3 = v3 * M;

    assert (r3.equalWithAbsError (Imath::V3f (0.707107f, 0.7071070f, 0.0f), 1e-6f));
}
