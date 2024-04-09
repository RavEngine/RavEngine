
#include "pf_conv.h"

#include <string.h>
#include <assert.h>

#include <algorithm>

#if 0
#include <stdio.h>

#define DPRINT(...) fprintf(stderr, __VA_ARGS__)

#else
#define DPRINT(...) do { } while (0)
#endif


#ifdef HAVE_MIPP
#include <mipp.h>
#endif


#ifndef CONV_ARCH_POST
#error CONV_ARCH_POST not defined
#endif

#define PP_STRINGIFY(X) #X
#define PP_TOSTRING(X)  PP_STRINGIFY(X)
#define PP_CONCAT_IMPL(x, y) x##y
#define PP_CONCAT(x, y) PP_CONCAT_IMPL( x, y )

#define ARCHFUNCNAME(X) PP_CONCAT(X##_,CONV_ARCH_POST)


const char * ARCHFUNCNAME(id)()
{
    return PP_TOSTRING(CONV_ARCH_POST);
}


int ARCHFUNCNAME(conv_float_simd_size)()
{
#if defined(MIPP_NO_INTRINSICS) || !defined(HAVE_MIPP)
    // have a completely MIPP independent implementation
    return 1;
#else
    return mipp::N<float>();
#endif
}


void ARCHFUNCNAME(conv_float_move_rest)(float * RESTRICT s, conv_buffer_state * RESTRICT state)
{
    int R = state->size - state->offset;    // this many samples from prev conv_float were not processed
    if (R > 0)
    {
        // memmove(s, &s[state->offset], R * sizeof(s[0]));   // move them to the begin
        std::copy(&s[state->offset], &s[state->size], s);
    }
    else
        R = 0;
    state->offset = 0;      // data - to be processed - is at begin
    state->size = R;        // this many unprocessed samples
}


void ARCHFUNCNAME(conv_cplx_move_rest)(complexf * RESTRICT s, conv_buffer_state * RESTRICT state)
{
    int R = state->size - state->offset;    // this many samples from prev conv_float were not processed
    if (R > 0)
    {
        // memmove(s, &s[state->offset], R * sizeof(s[0]));   // move them to the begin
        std::copy(&s[state->offset], &s[state->size], s);
    }
    else
        R = 0;
    state->offset = 0;      // data - to be processed - is at begin
    state->size = R;        // this many unprocessed samples
}


#if defined(MIPP_NO_INTRINSICS)
// have a completely MIPP independent implementation
// #error missing HAVE_MIPP: there is no MIPP-independent implementation

int ARCHFUNCNAME(conv_float_inplace)(
        float * RESTRICT s, conv_buffer_state * RESTRICT state,
        const float * RESTRICT filter, const int sz_filter
        )
{
    const int off0 = state->offset;
    const int sz_s = state->size;
    int offset;

    for ( offset = off0; offset + sz_filter <= sz_s; ++offset)
    {
        float accu = 0.0F;
        for (int k = 0; k < sz_filter; ++k)
            accu += s[offset+k] * filter[k];
        s[offset] = accu;
    }

    state->offset = offset;
    return offset - off0;
}


int ARCHFUNCNAME(conv_float_oop)(
        const float * RESTRICT s, conv_buffer_state * RESTRICT state,
        const float * RESTRICT filter, const int sz_filter,
        float * RESTRICT y
        )
{
    const int off0 = state->offset;
    const int sz_s = state->size;
    int offset;

    for ( offset = off0; offset + sz_filter <= sz_s; ++offset)
    {
        float accu = 0.0F;
        for (int k = 0; k < sz_filter; ++k)
            accu += s[offset+k] * filter[k];
        y[offset] = accu;
    }

    state->offset = offset;
    return offset - off0;
}


int ARCHFUNCNAME(conv_cplx_float_oop)(
        const complexf * RESTRICT s_cplx, conv_buffer_state * RESTRICT state,
        const float * RESTRICT filter, const int sz_filter,
        complexf * RESTRICT y_cplx
        )
{
    const int off0 = state->offset;
    const int sz_s = state->size;
    const int sz_f = sz_filter;
    int offset;

    for ( offset = off0; offset + sz_f <= sz_s; ++offset)
    {
        float accu_re = 0.0F;
        float accu_im = 0.0F;
        for (int k = 0; k < sz_filter; ++k)
        {
            accu_re = s_cplx[offset+k].i * filter[k];   // accu += rS * rH;
            accu_im = s_cplx[offset+k].q * filter[k];   // accu += rS * rH;
        }
        y_cplx[offset].i = accu_re;  // == hadd() == sum of real parts
        y_cplx[offset].q = accu_im;  // == hadd() == sum of imag parts
    }

    state->offset = offset;
    return offset - off0;
}


#elif defined(HAVE_MIPP)


int ARCHFUNCNAME(conv_float_inplace)(
        float * RESTRICT s, conv_buffer_state * RESTRICT state,
        const float * RESTRICT filter, const int sz_filter
        )
{
    assert( (sz_filter % mipp::N<float>()) == 0 );  // size of filter must be divisible by conv_float_simd_size()

    mipp::Reg<float> accu, rS, rH;
    const int off0 = state->offset;
    const int sz_s = state->size;
    int offset;

    for ( offset = off0; offset + sz_filter <= sz_s; ++offset)
    {
        accu.set0();
        for (int k = 0; k < sz_filter; k += mipp::N<float>())
        {
            rS.load(&s[offset+k]);
            rH.load(&filter[k]);
            accu = mipp::fmadd(rS, rH, accu);   // accu += rS * rH;
        }
        s[offset] = accu.sum();    // == hadd()
    }

    state->offset = offset;
    return offset - off0;
}


int ARCHFUNCNAME(conv_float_oop)(
        const float * RESTRICT s, conv_buffer_state * RESTRICT state,
        const float * RESTRICT filter, const int sz_filter,
        float * RESTRICT y
        )
{
    assert( (sz_filter % mipp::N<float>()) == 0 );  // size of filter must be divisible by conv_float_simd_size()

    mipp::Reg<float> accu, rS, rH;
    const int off0 = state->offset;
    const int sz_s = state->size;
    int offset;

    for ( offset = off0; offset + sz_filter <= sz_s; ++offset)
    {
        accu.set0();
        for (int k = 0; k < sz_filter; k += mipp::N<float>())
        {
            rS.loadu(&s[offset+k]);
            rH.load(&filter[k]);
            accu = mipp::fmadd(rS, rH, accu);   // accu += rS * rH;
        }
        y[offset] = accu.sum();    // == hadd()
    }

    state->offset = offset;
    return offset - off0;
}


int ARCHFUNCNAME(conv_cplx_float_oop)(
        const complexf * RESTRICT s_cplx, conv_buffer_state * RESTRICT state,
        const float * RESTRICT filter, const int sz_filter,
        complexf * RESTRICT y_cplx
        )
{
    assert( (sz_filter % mipp::N<float>()) == 0 );  // size of filter must be divisible by conv_float_simd_size()
    const float * RESTRICT s = &(s_cplx[0].i);
    float * RESTRICT y = &(y_cplx[0].i);

    mipp::Regx2<float> accu_x2, rS_x2, H_x2;
    const int off0 = 2 * state->offset;
    const int sz_s = 2 * state->size;
    const int sz_f2 = 2 * sz_filter;
    int offset;

    for ( offset = off0; offset + sz_f2 <= sz_s; offset += 2)
    {
        accu_x2.val[0].set0();
        accu_x2.val[1].set0();
        for (int k = 0; k < sz_filter; k += mipp::N<float>())
        {
            mipp::Reg<float> rH;
            rS_x2.loadu(&s[offset+2*k]);
            rH.load(&filter[k]);
            H_x2 = mipp::interleave<float>(rH, rH);
            accu_x2.val[0] = mipp::fmadd(rS_x2.val[0], H_x2.val[0], accu_x2.val[0]);   // accu += rS * rH;
            accu_x2.val[1] = mipp::fmadd(rS_x2.val[1], H_x2.val[1], accu_x2.val[1]);   // accu += rS * rH;
        }
        H_x2 = mipp::deinterleave(accu_x2);
        y[offset]   = H_x2.val[0].sum();  // == hadd() == sum of real parts
        y[offset+1] = H_x2.val[1].sum();  // == hadd() == sum of imag parts
    }

    state->offset = offset /2;
    return (offset - off0) / 2;
}

#endif


static const conv_f_ptrs conv_ptrs =
{
    PP_TOSTRING(CONV_ARCH_POST),
#ifndef MIPP_NO_INTRINSICS
    1,
#else
    0,
#endif

    ARCHFUNCNAME(id),
    ARCHFUNCNAME(conv_float_simd_size),

#if defined(MIPP_NO_INTRINSICS) || defined(HAVE_MIPP)
    ARCHFUNCNAME(conv_float_move_rest),
    ARCHFUNCNAME(conv_float_inplace),
    ARCHFUNCNAME(conv_float_oop),

    ARCHFUNCNAME(conv_cplx_move_rest),
    ARCHFUNCNAME(conv_cplx_float_oop)
#else
    nullptr,
    nullptr,
    nullptr,

    nullptr,
    nullptr
#endif
};


const conv_f_ptrs* ARCHFUNCNAME(conv_ptrs)()
{
    DPRINT("arch pointer for '%s':\n", conv_ptrs.id);
    if (!strcmp(conv_ptrs.id, "none"))
        return &conv_ptrs;

#if defined(MIPP_NO_INTRINSICS)
    DPRINT("arch pointer for '%s' - BUT defined(MIPP_NO_INTRINSICS)\n", conv_ptrs.id);
    return &conv_ptrs;
#elif defined(HAVE_MIPP)
    DPRINT("arch pointer for '%s' - defined(HAVE_MIPP)\n", conv_ptrs.id);
    DPRINT("'%s': conv_ptrs.using_mipp %d\n", conv_ptrs.id, conv_ptrs.using_mipp);
    DPRINT("'%s': simd_size() %d\n", conv_ptrs.id, conv_ptrs.fp_conv_float_simd_size());
    if (conv_ptrs.using_mipp && conv_ptrs.fp_conv_float_simd_size() > 1)
        return &conv_ptrs;
    else
        DPRINT("arch pointer for '%s': HAVE_MIPP BUT using_mipp %d, float_simd_size %d\n", conv_ptrs.id, conv_ptrs.using_mipp, conv_ptrs.fp_conv_float_simd_size());
#else
    DPRINT("arch pointer for '%s': neither MIPP_NO_INTRINSICS nor HAVE_MIPP\n", conv_ptrs.id);
#endif
    DPRINT("arch pointer for '%s' => nullptr\n", conv_ptrs.id);
    return nullptr;
}

#if defined(__cplusplus) && (__cplusplus >= 201703L)
[[maybe_unused]]
#endif
static f_conv_ptrs test_f_ptrs = ARCHFUNCNAME(conv_ptrs);

