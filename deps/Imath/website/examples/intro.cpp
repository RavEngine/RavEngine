#include <Imath/ImathMatrix.h>
#include <Imath/ImathVec.h>
#include <iostream>
#include <cassert>

int
main()
{
    const Imath::V3f v (3.0f, 4.0f, 5.0f);
   
    Imath::M44f M;
    const Imath::V3f t(1.0f, 2.0f, 3.0f);
    M.translate (t);

    Imath::V3f p;
    M.multVecMatrix(v, p);

    std::cout << "What's your vector, Victor? " << p << std::endl;

    Imath::V3f vt = v + t;
    assert (p.equalWithAbsError(vt, 1e-6f));

    return 0;
}
