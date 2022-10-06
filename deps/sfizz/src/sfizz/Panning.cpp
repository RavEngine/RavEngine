#include "Panning.h"
#include "MathHelpers.h"
#include <array>
#include <cmath>

#if SFIZZ_HAVE_NEON
#include <arm_neon.h>
#include "simd/Common.h"
using Type = float;
constexpr unsigned TypeAlignment = 4;
constexpr unsigned ByteAlignment = TypeAlignment * sizeof(Type);
#endif


namespace sfz
{

constexpr int panSize { 4095 };

// Table of pan values for the left channel, extra element for safety
static const auto panData = []()
{
    std::array<float, panSize + 1> pan;
    int i = 0;

    for (; i < panSize; ++i)
        pan[i] = std::cos(i * (piTwo<double>() / (panSize - 1)));

    for (; i < static_cast<int>(pan.size()); ++i)
        pan[i] = pan[panSize - 1];

    return pan;
}();

float panLookup(float pan)
{
    // reduce range, round to nearest
    const long index = lroundPositive(pan * (panSize - 1));
    return panData[index];
}

inline void tickPan(const float* pan, float* leftBuffer, float* rightBuffer)
{
    auto p = (*pan + 1.0f) * 0.5f;
    p = clamp(p, 0.0f, 1.0f);
    *leftBuffer *= panLookup(p);
    *rightBuffer *= panLookup(1 - p);
}

void pan(const float* panEnvelope, float* leftBuffer, float* rightBuffer, unsigned size) noexcept
{
    const auto* sentinel = panEnvelope + size;

#if SFIZZ_HAVE_NEON
    const auto* firstAligned = prevAligned<ByteAlignment>(panEnvelope + TypeAlignment - 1);

    if (willAlign<ByteAlignment>(panEnvelope, leftBuffer, rightBuffer) && (firstAligned < sentinel)) {
        while (panEnvelope < firstAligned) {
            tickPan(panEnvelope, leftBuffer, rightBuffer);
            incrementAll(panEnvelope, leftBuffer, rightBuffer);
        }

        uint32_t indices[TypeAlignment];
        float leftPan[TypeAlignment];
        float rightPan[TypeAlignment];
        const auto* lastAligned = prevAligned<ByteAlignment>(sentinel);
        while (panEnvelope < lastAligned) {
            float32x4_t mmPan = vld1q_f32(panEnvelope);
            mmPan = vaddq_f32(mmPan, vdupq_n_f32(1.0f));
            mmPan = vmulq_n_f32(mmPan, 0.5f * panSize);
            mmPan = vaddq_f32(mmPan, vdupq_n_f32(0.5f));
            uint32x4_t mmIdx = vcvtq_u32_f32(mmPan);
            mmIdx = vminq_u32(mmIdx, vdupq_n_u32(panSize - 1));
            mmIdx = vmaxq_u32(mmIdx, vdupq_n_u32(0));
            vst1q_u32(indices, mmIdx);

            leftPan[0] = panData[indices[0]];
            rightPan[0] = panData[panSize - indices[0] - 1];
            leftPan[1] = panData[indices[1]];
            rightPan[1] = panData[panSize - indices[1] - 1];
            leftPan[2] = panData[indices[2]];
            rightPan[2] = panData[panSize - indices[2] - 1];
            leftPan[3] = panData[indices[3]];
            rightPan[3] = panData[panSize - indices[3] - 1];

            vst1q_f32(leftBuffer, vmulq_f32(vld1q_f32(leftBuffer), vld1q_f32(leftPan)));
            vst1q_f32(rightBuffer, vmulq_f32(vld1q_f32(rightBuffer), vld1q_f32(rightPan)));

            incrementAll<TypeAlignment>(panEnvelope, leftBuffer, rightBuffer);
        }
    }
#endif

    while (panEnvelope < sentinel) {
        tickPan(panEnvelope, leftBuffer, rightBuffer);
        incrementAll(panEnvelope, leftBuffer, rightBuffer);
    }

}

inline void tickWidth(const float* width, float* leftBuffer, float* rightBuffer)
{
    float w = (*width + 1.0f) * 0.5f;
    w = clamp(w, 0.0f, 1.0f);
    const auto coeff1 = panLookup(w);
    const auto coeff2 = panLookup(1 - w);
    const auto l = *leftBuffer;
    const auto r = *rightBuffer;
    *leftBuffer = l * coeff2 + r * coeff1;
    *rightBuffer = l * coeff1 + r * coeff2;
}

void width(const float* widthEnvelope, float* leftBuffer, float* rightBuffer, unsigned size) noexcept
{
    const auto* sentinel = widthEnvelope + size;

#if SFIZZ_HAVE_NEON
    const auto* firstAligned = prevAligned<ByteAlignment>(widthEnvelope + TypeAlignment  - 1);

    if (willAlign<ByteAlignment>(widthEnvelope, leftBuffer, rightBuffer) && firstAligned < sentinel) {
        while (widthEnvelope < firstAligned) {
            tickWidth(widthEnvelope, leftBuffer, rightBuffer);
            incrementAll(widthEnvelope, leftBuffer, rightBuffer);
        }

        uint32_t indices[TypeAlignment];
        float coeff1[TypeAlignment];
        float coeff2[TypeAlignment];
        const auto* lastAligned = prevAligned<ByteAlignment>(sentinel);
        while (widthEnvelope < lastAligned) {
            float32x4_t mmWidth = vld1q_f32(widthEnvelope);
            mmWidth = vaddq_f32(mmWidth, vdupq_n_f32(1.0f));
            mmWidth = vmulq_n_f32(mmWidth, 0.5f * panSize);
            mmWidth = vaddq_f32(mmWidth, vdupq_n_f32(0.5f));
            uint32x4_t mmIdx = vcvtq_u32_f32(mmWidth);
            mmIdx = vminq_u32(mmIdx, vdupq_n_u32(panSize - 1));
            mmIdx = vmaxq_u32(mmIdx, vdupq_n_u32(0));
            vst1q_u32(indices, mmIdx);

            coeff1[0] = panData[indices[0]];
            coeff2[0] = panData[panSize - indices[0] - 1];
            coeff1[1] = panData[indices[1]];
            coeff2[1] = panData[panSize - indices[1] - 1];
            coeff1[2] = panData[indices[2]];
            coeff2[2] = panData[panSize - indices[2] - 1];
            coeff1[3] = panData[indices[3]];
            coeff2[3] = panData[panSize - indices[3] - 1];

            float32x4_t mmCoeff1 = vld1q_f32(coeff1);
            float32x4_t mmCoeff2 = vld1q_f32(coeff2);
            float32x4_t mmLeft = vld1q_f32(leftBuffer);
            float32x4_t mmRight = vld1q_f32(rightBuffer);

            vst1q_f32(leftBuffer, vaddq_f32(vmulq_f32(mmCoeff2, mmLeft), vmulq_f32(mmCoeff1, mmRight)));
            vst1q_f32(rightBuffer, vaddq_f32(vmulq_f32(mmCoeff1, mmLeft), vmulq_f32(mmCoeff2, mmRight)));

            incrementAll<TypeAlignment>(widthEnvelope, leftBuffer, rightBuffer);
        }
    }
#endif // SFIZZ_HAVE_NEON

    while (widthEnvelope < sentinel) {
        tickWidth(widthEnvelope, leftBuffer, rightBuffer);
        incrementAll(widthEnvelope, leftBuffer, rightBuffer);
    }
}

}
