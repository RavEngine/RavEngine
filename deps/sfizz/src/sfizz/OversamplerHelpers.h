#pragma once
#include "SIMDConfig.h"
#include "Config.h"
#include "utility/Debug.h"
#include <type_traits>
#include <cstring>

namespace sfz {
class Upsampler;
class Downsampler;
} // namespace sfz

#include <hiir/Upsampler2xFpu.h>
#include <hiir/Downsampler2xFpu.h>

// Note: according to HIIR documentation, FPU versions are
//       more efficient than SIMD below 12 coefficients.

#if SFIZZ_HAVE_SSE
#include <hiir/Upsampler2xSse.h>
#include <hiir/Downsampler2xSse.h>

namespace hiir {
template <int NC> using Upsampler2x = typename std::conditional<NC >= 12,
    hiir::Upsampler2xSse<NC>, hiir::Upsampler2xFpu<NC>>::type;
template <int NC> using Downsampler2x = typename std::conditional<NC >= 12,
    hiir::Downsampler2xSse<NC>, hiir::Downsampler2xFpu<NC>>::type;
} // namespace hiir

#elif SFIZZ_HAVE_NEON
#include <hiir/Upsampler2xNeon.h>
#include <hiir/Downsampler2xNeon.h>

namespace hiir {
template <int NC> using Upsampler2x = typename std::conditional<NC >= 12,
    hiir::Upsampler2xNeon<NC>, hiir::Upsampler2xFpu<NC>>::type;
template <int NC> using Downsampler2x = typename std::conditional<NC >= 12,
    hiir::Downsampler2xNeon<NC>, hiir::Downsampler2xFpu<NC>>::type;
} // namespace hiir

#else

namespace hiir {
template <int NC> using Upsampler2x = hiir::Upsampler2xFpu<NC>;
template <int NC> using Downsampler2x = hiir::Downsampler2xFpu<NC>;
} // namespace hiir

#endif

#include "OversamplerHelpers.hxx"
