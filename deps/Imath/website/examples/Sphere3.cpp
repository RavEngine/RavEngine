#include <Imath/ImathSphere.h>
#include <cassert>

void
sphere3_example()
{
    Imath::V3f center (1.0f, 1.0f, 1.0f); 
    float radius = 2.0f;
    Imath::Sphere3f s (center, radius);

    assert (s.center == center);
    assert (s.radius == radius);
    
    Imath::Line3f line (Imath::V3f (0.0f, 0.0f, 0.0f),
                        Imath::V3f (1.0f, 1.0f, 1.0f));

    Imath::V3f v;
    assert (s.intersect (line, v));

    assert (v.equalWithAbsError (Imath::V3f(2.1547f, 2.1547f, 2.1547f), 1e-6f));    
}
