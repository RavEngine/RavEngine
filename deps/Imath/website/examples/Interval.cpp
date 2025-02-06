#include <Imath/ImathInterval.h>
#include <cassert>

void
interval_example()
{
    Imath::Intervalf v;

    assert (v.isEmpty());
    assert (!v.hasVolume());
    assert (!v.isInfinite());

    v.extendBy (1.0f);
    assert (!v.isEmpty());
    
    v.extendBy (2.0f);
    assert (v.hasVolume());
    assert (v.intersects (1.5f));
}
