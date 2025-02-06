#include <Imath/ImathMatrix.h>
#include <Imath/ImathMatrixAlgo.h>
#include <cassert>

void
matrix22_example()
{
    Imath::M22f M (Imath::UNINITIALIZED); // uninitialized

    M.makeIdentity();
    assert (M[0][0] == 1.0f);
    assert (M[0][1] == 0.0f);

    Imath::M22f Minv = M.inverse();

    Imath::M22f R;
    assert (R == Imath::identity22f);

    R.rotate (M_PI/4);
    
    M = R * M;

    Imath::V2f v2 (1.0f, 0.0f);
    Imath::V2f r2 = v2 * M;

    assert (r2.equalWithAbsError (Imath::V2f (0.707107f, 0.707107f), 1e-6f));
}
