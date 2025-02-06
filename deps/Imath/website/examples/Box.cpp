#include <Imath/ImathBox.h>
        
void
box_example()
{
    Imath::V3f   a (0, 0, 0);
    Imath::V3f   b (1, 1, 1);
    Imath::V3f   c (2, 9, 2);

    Imath::Box3f box (a);

    assert (box.isEmpty());
    assert (!box.isInfinite());
    assert (!box.hasVolume());
    
    box.extendBy (c);

    assert (box.size() == (c-a));
    assert (box.intersects (b));
    assert (box.max[0] > box.min[0]);
    assert (box.max[1] > box.min[1]);
    assert (box.max[2] > box.min[2]);
    assert (box.hasVolume());
    assert (box.majorAxis() == 1);
}
