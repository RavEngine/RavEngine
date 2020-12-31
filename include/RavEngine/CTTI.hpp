#pragma once
#include <stddef.h>

namespace RavEngine{

typedef size_t ctti_t;

inline int type_id_seq = 0;
template<typename T> inline const ctti_t CTTI = type_id_seq++;

}
