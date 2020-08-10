///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// HlslIntrinsicOp.h                                                         //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Enumeration for HLSL intrinsics operations.                               //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once
namespace hlsl
{

enum class IntrinsicOp
{
/* <py>
import hctdb_instrhelp
</py> */

/* <py::lines('HLSL-INTRINSICS')>hctdb_instrhelp.enum_hlsl_intrinsics()</py>*/
// HLSL-INTRINSICS:BEGIN
  IOP_AcceptHitAndEndSearch,
  IOP_AddUint64,
  IOP_AllMemoryBarrier,
  IOP_AllMemoryBarrierWithGroupSync,
  IOP_AllocateRayQuery,
  IOP_CallShader,
  IOP_CheckAccessFullyMapped,
  IOP_CreateResourceFromHeap,
  IOP_D3DCOLORtoUBYTE4,
  IOP_DeviceMemoryBarrier,
  IOP_DeviceMemoryBarrierWithGroupSync,
  IOP_DispatchMesh,
  IOP_DispatchRaysDimensions,
  IOP_DispatchRaysIndex,
  IOP_EvaluateAttributeAtSample,
  IOP_EvaluateAttributeCentroid,
  IOP_EvaluateAttributeSnapped,
  IOP_GeometryIndex,
  IOP_GetAttributeAtVertex,
  IOP_GetRenderTargetSampleCount,
  IOP_GetRenderTargetSamplePosition,
  IOP_GroupMemoryBarrier,
  IOP_GroupMemoryBarrierWithGroupSync,
  IOP_HitKind,
  IOP_IgnoreHit,
  IOP_InstanceID,
  IOP_InstanceIndex,
  IOP_InterlockedAdd,
  IOP_InterlockedAnd,
  IOP_InterlockedCompareExchange,
  IOP_InterlockedCompareStore,
  IOP_InterlockedExchange,
  IOP_InterlockedMax,
  IOP_InterlockedMin,
  IOP_InterlockedOr,
  IOP_InterlockedXor,
  IOP_NonUniformResourceIndex,
  IOP_ObjectRayDirection,
  IOP_ObjectRayOrigin,
  IOP_ObjectToWorld,
  IOP_ObjectToWorld3x4,
  IOP_ObjectToWorld4x3,
  IOP_PrimitiveIndex,
  IOP_Process2DQuadTessFactorsAvg,
  IOP_Process2DQuadTessFactorsMax,
  IOP_Process2DQuadTessFactorsMin,
  IOP_ProcessIsolineTessFactors,
  IOP_ProcessQuadTessFactorsAvg,
  IOP_ProcessQuadTessFactorsMax,
  IOP_ProcessQuadTessFactorsMin,
  IOP_ProcessTriTessFactorsAvg,
  IOP_ProcessTriTessFactorsMax,
  IOP_ProcessTriTessFactorsMin,
  IOP_QuadReadAcrossDiagonal,
  IOP_QuadReadAcrossX,
  IOP_QuadReadAcrossY,
  IOP_QuadReadLaneAt,
  IOP_RayFlags,
  IOP_RayTCurrent,
  IOP_RayTMin,
  IOP_ReportHit,
  IOP_SetMeshOutputCounts,
  IOP_TraceRay,
  IOP_WaveActiveAllEqual,
  IOP_WaveActiveAllTrue,
  IOP_WaveActiveAnyTrue,
  IOP_WaveActiveBallot,
  IOP_WaveActiveBitAnd,
  IOP_WaveActiveBitOr,
  IOP_WaveActiveBitXor,
  IOP_WaveActiveCountBits,
  IOP_WaveActiveMax,
  IOP_WaveActiveMin,
  IOP_WaveActiveProduct,
  IOP_WaveActiveSum,
  IOP_WaveGetLaneCount,
  IOP_WaveGetLaneIndex,
  IOP_WaveIsFirstLane,
  IOP_WaveMatch,
  IOP_WaveMultiPrefixBitAnd,
  IOP_WaveMultiPrefixBitOr,
  IOP_WaveMultiPrefixBitXor,
  IOP_WaveMultiPrefixCountBits,
  IOP_WaveMultiPrefixProduct,
  IOP_WaveMultiPrefixSum,
  IOP_WavePrefixCountBits,
  IOP_WavePrefixProduct,
  IOP_WavePrefixSum,
  IOP_WaveReadLaneAt,
  IOP_WaveReadLaneFirst,
  IOP_WorldRayDirection,
  IOP_WorldRayOrigin,
  IOP_WorldToObject,
  IOP_WorldToObject3x4,
  IOP_WorldToObject4x3,
  IOP_abort,
  IOP_abs,
  IOP_acos,
  IOP_all,
  IOP_any,
  IOP_asdouble,
  IOP_asfloat,
  IOP_asfloat16,
  IOP_asin,
  IOP_asint,
  IOP_asint16,
  IOP_asuint,
  IOP_asuint16,
  IOP_atan,
  IOP_atan2,
  IOP_ceil,
  IOP_clamp,
  IOP_clip,
  IOP_cos,
  IOP_cosh,
  IOP_countbits,
  IOP_cross,
  IOP_ddx,
  IOP_ddx_coarse,
  IOP_ddx_fine,
  IOP_ddy,
  IOP_ddy_coarse,
  IOP_ddy_fine,
  IOP_degrees,
  IOP_determinant,
  IOP_distance,
  IOP_dot,
  IOP_dot2add,
  IOP_dot4add_i8packed,
  IOP_dot4add_u8packed,
  IOP_dst,
  IOP_exp,
  IOP_exp2,
  IOP_f16tof32,
  IOP_f32tof16,
  IOP_faceforward,
  IOP_firstbithigh,
  IOP_firstbitlow,
  IOP_floor,
  IOP_fma,
  IOP_fmod,
  IOP_frac,
  IOP_frexp,
  IOP_fwidth,
  IOP_isfinite,
  IOP_isinf,
  IOP_isnan,
  IOP_ldexp,
  IOP_length,
  IOP_lerp,
  IOP_lit,
  IOP_log,
  IOP_log10,
  IOP_log2,
  IOP_mad,
  IOP_max,
  IOP_min,
  IOP_modf,
  IOP_msad4,
  IOP_mul,
  IOP_normalize,
  IOP_pow,
  IOP_radians,
  IOP_rcp,
  IOP_reflect,
  IOP_refract,
  IOP_reversebits,
  IOP_round,
  IOP_rsqrt,
  IOP_saturate,
  IOP_sign,
  IOP_sin,
  IOP_sincos,
  IOP_sinh,
  IOP_smoothstep,
  IOP_source_mark,
  IOP_sqrt,
  IOP_step,
  IOP_tan,
  IOP_tanh,
  IOP_tex1D,
  IOP_tex1Dbias,
  IOP_tex1Dgrad,
  IOP_tex1Dlod,
  IOP_tex1Dproj,
  IOP_tex2D,
  IOP_tex2Dbias,
  IOP_tex2Dgrad,
  IOP_tex2Dlod,
  IOP_tex2Dproj,
  IOP_tex3D,
  IOP_tex3Dbias,
  IOP_tex3Dgrad,
  IOP_tex3Dlod,
  IOP_tex3Dproj,
  IOP_texCUBE,
  IOP_texCUBEbias,
  IOP_texCUBEgrad,
  IOP_texCUBElod,
  IOP_texCUBEproj,
  IOP_transpose,
  IOP_trunc,
  MOP_Append,
  MOP_RestartStrip,
  MOP_CalculateLevelOfDetail,
  MOP_CalculateLevelOfDetailUnclamped,
  MOP_GetDimensions,
  MOP_Load,
  MOP_Sample,
  MOP_SampleBias,
  MOP_SampleCmp,
  MOP_SampleCmpLevelZero,
  MOP_SampleGrad,
  MOP_SampleLevel,
  MOP_Gather,
  MOP_GatherAlpha,
  MOP_GatherBlue,
  MOP_GatherCmp,
  MOP_GatherCmpAlpha,
  MOP_GatherCmpBlue,
  MOP_GatherCmpGreen,
  MOP_GatherCmpRed,
  MOP_GatherGreen,
  MOP_GatherRed,
  MOP_GetSamplePosition,
  MOP_Load2,
  MOP_Load3,
  MOP_Load4,
  MOP_InterlockedAdd,
  MOP_InterlockedAnd,
  MOP_InterlockedCompareExchange,
  MOP_InterlockedCompareStore,
  MOP_InterlockedExchange,
  MOP_InterlockedMax,
  MOP_InterlockedMin,
  MOP_InterlockedOr,
  MOP_InterlockedXor,
  MOP_Store,
  MOP_Store2,
  MOP_Store3,
  MOP_Store4,
  MOP_DecrementCounter,
  MOP_IncrementCounter,
  MOP_Consume,
  MOP_WriteSamplerFeedback,
  MOP_WriteSamplerFeedbackBias,
  MOP_WriteSamplerFeedbackGrad,
  MOP_WriteSamplerFeedbackLevel,
  MOP_Abort,
  MOP_CandidateGeometryIndex,
  MOP_CandidateInstanceContributionToHitGroupIndex,
  MOP_CandidateInstanceID,
  MOP_CandidateInstanceIndex,
  MOP_CandidateObjectRayDirection,
  MOP_CandidateObjectRayOrigin,
  MOP_CandidateObjectToWorld3x4,
  MOP_CandidateObjectToWorld4x3,
  MOP_CandidatePrimitiveIndex,
  MOP_CandidateProceduralPrimitiveNonOpaque,
  MOP_CandidateTriangleBarycentrics,
  MOP_CandidateTriangleFrontFace,
  MOP_CandidateTriangleRayT,
  MOP_CandidateType,
  MOP_CandidateWorldToObject3x4,
  MOP_CandidateWorldToObject4x3,
  MOP_CommitNonOpaqueTriangleHit,
  MOP_CommitProceduralPrimitiveHit,
  MOP_CommittedGeometryIndex,
  MOP_CommittedInstanceContributionToHitGroupIndex,
  MOP_CommittedInstanceID,
  MOP_CommittedInstanceIndex,
  MOP_CommittedObjectRayDirection,
  MOP_CommittedObjectRayOrigin,
  MOP_CommittedObjectToWorld3x4,
  MOP_CommittedObjectToWorld4x3,
  MOP_CommittedPrimitiveIndex,
  MOP_CommittedRayT,
  MOP_CommittedStatus,
  MOP_CommittedTriangleBarycentrics,
  MOP_CommittedTriangleFrontFace,
  MOP_CommittedWorldToObject3x4,
  MOP_CommittedWorldToObject4x3,
  MOP_Proceed,
  MOP_RayFlags,
  MOP_RayTMin,
  MOP_TraceRayInline,
  MOP_WorldRayDirection,
  MOP_WorldRayOrigin,
#ifdef ENABLE_SPIRV_CODEGEN
  MOP_SubpassLoad,
#endif // ENABLE_SPIRV_CODEGEN
  // unsigned
  IOP_InterlockedUMax,
  IOP_InterlockedUMin,
  IOP_WaveActiveUMax,
  IOP_WaveActiveUMin,
  IOP_WaveActiveUProduct,
  IOP_WaveActiveUSum,
  IOP_WaveMultiPrefixUProduct,
  IOP_WaveMultiPrefixUSum,
  IOP_WavePrefixUProduct,
  IOP_WavePrefixUSum,
  IOP_uabs,
  IOP_uclamp,
  IOP_ufirstbithigh,
  IOP_umad,
  IOP_umax,
  IOP_umin,
  IOP_umul,
  IOP_usign,
  MOP_InterlockedUMax,
  MOP_InterlockedUMin,
  Num_Intrinsics,
// HLSL-INTRINSICS:END
};

inline bool HasUnsignedIntrinsicOpcode(IntrinsicOp opcode) {
  switch (opcode) {
/* <py>
import hctdb_instrhelp
</py> */

/* <py::lines('HLSL-HAS-UNSIGNED-INTRINSICS')>hctdb_instrhelp.has_unsigned_hlsl_intrinsics()</py>*/
// HLSL-HAS-UNSIGNED-INTRINSICS:BEGIN
  case IntrinsicOp::IOP_InterlockedMax:
  case IntrinsicOp::IOP_InterlockedMin:
  case IntrinsicOp::IOP_WaveActiveMax:
  case IntrinsicOp::IOP_WaveActiveMin:
  case IntrinsicOp::IOP_WaveActiveProduct:
  case IntrinsicOp::IOP_WaveActiveSum:
  case IntrinsicOp::IOP_WaveMultiPrefixProduct:
  case IntrinsicOp::IOP_WaveMultiPrefixSum:
  case IntrinsicOp::IOP_WavePrefixProduct:
  case IntrinsicOp::IOP_WavePrefixSum:
  case IntrinsicOp::IOP_abs:
  case IntrinsicOp::IOP_clamp:
  case IntrinsicOp::IOP_firstbithigh:
  case IntrinsicOp::IOP_mad:
  case IntrinsicOp::IOP_max:
  case IntrinsicOp::IOP_min:
  case IntrinsicOp::IOP_mul:
  case IntrinsicOp::IOP_sign:
  case IntrinsicOp::MOP_InterlockedMax:
  case IntrinsicOp::MOP_InterlockedMin:
// HLSL-HAS-UNSIGNED-INTRINSICS:END
    return true;
  default:
    return false;
  }
}

inline unsigned GetUnsignedIntrinsicOpcode(IntrinsicOp opcode) {
  switch (opcode) {
/* <py>
import hctdb_instrhelp
</py> */

/* <py::lines('HLSL-GET-UNSIGNED-INTRINSICS')>hctdb_instrhelp.get_unsigned_hlsl_intrinsics()</py>*/
// HLSL-GET-UNSIGNED-INTRINSICS:BEGIN
  case IntrinsicOp::IOP_InterlockedMax:
    return static_cast<unsigned>(IntrinsicOp::IOP_InterlockedUMax);
  case IntrinsicOp::IOP_InterlockedMin:
    return static_cast<unsigned>(IntrinsicOp::IOP_InterlockedUMin);
  case IntrinsicOp::IOP_WaveActiveMax:
    return static_cast<unsigned>(IntrinsicOp::IOP_WaveActiveUMax);
  case IntrinsicOp::IOP_WaveActiveMin:
    return static_cast<unsigned>(IntrinsicOp::IOP_WaveActiveUMin);
  case IntrinsicOp::IOP_WaveActiveProduct:
    return static_cast<unsigned>(IntrinsicOp::IOP_WaveActiveUProduct);
  case IntrinsicOp::IOP_WaveActiveSum:
    return static_cast<unsigned>(IntrinsicOp::IOP_WaveActiveUSum);
  case IntrinsicOp::IOP_WaveMultiPrefixProduct:
    return static_cast<unsigned>(IntrinsicOp::IOP_WaveMultiPrefixUProduct);
  case IntrinsicOp::IOP_WaveMultiPrefixSum:
    return static_cast<unsigned>(IntrinsicOp::IOP_WaveMultiPrefixUSum);
  case IntrinsicOp::IOP_WavePrefixProduct:
    return static_cast<unsigned>(IntrinsicOp::IOP_WavePrefixUProduct);
  case IntrinsicOp::IOP_WavePrefixSum:
    return static_cast<unsigned>(IntrinsicOp::IOP_WavePrefixUSum);
  case IntrinsicOp::IOP_abs:
    return static_cast<unsigned>(IntrinsicOp::IOP_uabs);
  case IntrinsicOp::IOP_clamp:
    return static_cast<unsigned>(IntrinsicOp::IOP_uclamp);
  case IntrinsicOp::IOP_firstbithigh:
    return static_cast<unsigned>(IntrinsicOp::IOP_ufirstbithigh);
  case IntrinsicOp::IOP_mad:
    return static_cast<unsigned>(IntrinsicOp::IOP_umad);
  case IntrinsicOp::IOP_max:
    return static_cast<unsigned>(IntrinsicOp::IOP_umax);
  case IntrinsicOp::IOP_min:
    return static_cast<unsigned>(IntrinsicOp::IOP_umin);
  case IntrinsicOp::IOP_mul:
    return static_cast<unsigned>(IntrinsicOp::IOP_umul);
  case IntrinsicOp::IOP_sign:
    return static_cast<unsigned>(IntrinsicOp::IOP_usign);
  case IntrinsicOp::MOP_InterlockedMax:
    return static_cast<unsigned>(IntrinsicOp::MOP_InterlockedUMax);
  case IntrinsicOp::MOP_InterlockedMin:
    return static_cast<unsigned>(IntrinsicOp::MOP_InterlockedUMin);
// HLSL-GET-UNSIGNED-INTRINSICS:END
  default:
    return static_cast<unsigned>(opcode);
  }
}

}
