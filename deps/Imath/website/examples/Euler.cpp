#include <Imath/ImathEuler.h>
#include <Imath/ImathMatrixAlgo.h>
#include <cassert>

void
euler_example()
{
    int i, j, k;
    
    Imath::Eulerf xyz (Imath::Eulerf::XYZ);
    xyz.angleOrder (i, j, k);
    assert (i == 0 && j == 1 && k == 2);

    Imath::Eulerf xzy (Imath::Eulerf::XZY);
    xzy.angleOrder (i, j, k);
    assert (i == 0 && j == 2 && k == 1);

    Imath::Eulerf e1 (0.0f, 0.0f, 0.1f + 2 * M_PI);
    Imath::Eulerf e2 (0.0f, 0.0f, 0.1f);

    e1.makeNear (e2);
    Imath::V3f v = e2.toXYZVector();
    assert (v.equalWithAbsError (Imath::V3f (0.0f, 0.0f, 0.1f), 0.00001f));
}
  
