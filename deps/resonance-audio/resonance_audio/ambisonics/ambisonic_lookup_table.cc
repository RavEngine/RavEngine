/*
Copyright 2018 Google Inc. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS-IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "ambisonics/ambisonic_lookup_table.h"

#include "ambisonics/ambisonic_spread_coefficients.h"
#include "ambisonics/associated_legendre_polynomials_generator.h"
#include "ambisonics/utils.h"
#include "base/constants_and_types.h"
#include "base/misc_math.h"


namespace vraudio {

namespace {

// Number of azimuth angles to store in the pre-computed encoder lookup table
// (for 0 - 90 degrees using 1 degree increments).
const size_t kNumAzimuths = 91;

// Number of elevation angles to store in the pre-computed encoder lookup table
// (for 0 - 90 degrees using 1 degree increments).
const size_t kNumElevations = 91;

// Number of Cartesian axes for which to compute spherical harmonic symmetries.
const size_t kNumAxes = 3;

// Minimum angular source spreads at different orders for which we start to
// apply spread gain correction coefficients. Array index corresponds to
// ambisonic order. For more information about sound source spread control,
// please refer to the Matlab code and the corresponding paper.
const int kMinSpreads[kMaxSupportedAmbisonicOrder + 1] = {361, 54, 40, 31};

// Spread coefficients are stored sequentially in a 1-d table. Therefore, to
// access coefficients for the required ambisonic order we need to apply offsets
// which are equal to the total number of coefficients for previous orders. For
// example, the total number of coefficient in each ambisonic order 'n' is
// equal to the number of unique orders multiplied by number of unique spreads,
// i.e.: (n + 1) * (360 - kMinSpreads[n] + 1).
const int kSpreadCoeffOffsets[kMaxSupportedAmbisonicOrder + 1] = {0, 1, 615,
                                                                  1578};

size_t GetEncoderTableIndex(size_t i, size_t j, size_t k, size_t width,
                            size_t depth) {
  return i * width * depth + j * depth + k;
}

size_t GetSymmetriesTableIndex(size_t i, size_t j, size_t width) {
  return i * width + j;
}

int GetSpreadTableIndex(int ambisonic_order, float source_spread_deg) {
  // Number of unique ambisonic orders in the sound field. For example, in a
  // first order sound field we have components of order 0 and 1.
  const int num_unique_orders = ambisonic_order + 1;
  return kSpreadCoeffOffsets[ambisonic_order] +
         num_unique_orders * (static_cast<int>(source_spread_deg) -
                              kMinSpreads[ambisonic_order]);
}

}  // namespace

AmbisonicLookupTable::AmbisonicLookupTable(int max_ambisonic_order)
    : max_ambisonic_order_(max_ambisonic_order),
      max_num_coeffs_in_table_(
          GetNumPeriphonicComponents(max_ambisonic_order_) - 1),
      encoder_table_(kNumAzimuths * kNumElevations * max_num_coeffs_in_table_),
      symmetries_table_(kNumAxes * max_num_coeffs_in_table_) {
  DCHECK_GE(max_ambisonic_order_, 0);
  DCHECK_LE(max_ambisonic_order_, kMaxSupportedAmbisonicOrder);
  ComputeEncoderTable();
  ComputeSymmetriesTable();
}

void AmbisonicLookupTable::GetEncodingCoeffs(
    int ambisonic_order, const SphericalAngle& source_direction,
    float source_spread_deg, std::vector<float>* encoding_coeffs) const {

  DCHECK_GE(ambisonic_order, 0);
  DCHECK_LE(ambisonic_order, kMaxSupportedAmbisonicOrder);
  DCHECK_GE(source_spread_deg, 0.0f);
  DCHECK_LE(source_spread_deg, 360.0f);
  DCHECK(encoding_coeffs);
  // Raw coefficients are stored only for ambisonic orders 1 and up, since 0th
  // order raw coefficient is always 1.
  const size_t num_raw_coeffs = GetNumPeriphonicComponents(ambisonic_order) - 1;
  // The actual number of returned Ambisonic coefficients is therefore
  // |num_raw_coeffs + 1|.
  DCHECK_EQ(encoding_coeffs->size(), num_raw_coeffs + 1);
  DCHECK_GE(source_direction.azimuth(), -kPi);
  DCHECK_LE(source_direction.azimuth(), kTwoPi);
  DCHECK_GE(source_direction.elevation(), -kHalfPi);
  DCHECK_LE(source_direction.elevation(), kHalfPi);
  const int azimuth_deg =
      source_direction.azimuth() < kPi
          ? static_cast<int>(source_direction.azimuth() * kDegreesFromRadians)
          : static_cast<int>(source_direction.azimuth() * kDegreesFromRadians) -
                360;
  const int elevation_deg =
      static_cast<int>(source_direction.elevation() * kDegreesFromRadians);
  const size_t abs_azimuth_deg = static_cast<size_t>(std::abs(azimuth_deg));
  const size_t azimuth_idx =
      abs_azimuth_deg > 90 ? 180 - abs_azimuth_deg : abs_azimuth_deg;
  const size_t elevation_idx = static_cast<size_t>(std::abs(elevation_deg));
  (*encoding_coeffs)[0] = 1.0f;
  for (size_t raw_coeff_idx = 0; raw_coeff_idx < num_raw_coeffs;
       ++raw_coeff_idx) {
    // Variable to hold information about spherical harmonic phase flip. 1 means
    // no flip; -1 means 180 degrees flip.
    float flip = 1.0f;
    // The following three 'if' statements implement the logic to correct the
    // phase of the current spherical harmonic, depending on which quadrant the
    // sound source is located in. For more information, please see the Matlab
    // code and the corresponding paper.
    if (azimuth_deg < 0) {
      flip = symmetries_table_[GetSymmetriesTableIndex(
          0, raw_coeff_idx, max_num_coeffs_in_table_)];
    }
    if (elevation_deg < 0) {
      flip *= symmetries_table_[GetSymmetriesTableIndex(
          1, raw_coeff_idx, max_num_coeffs_in_table_)];
    }
    if (abs_azimuth_deg > 90) {
      flip *= symmetries_table_[GetSymmetriesTableIndex(
          2, raw_coeff_idx, max_num_coeffs_in_table_)];
    }
    const size_t encoder_table_idx =
        GetEncoderTableIndex(azimuth_idx, elevation_idx, raw_coeff_idx,
                             kNumElevations, max_num_coeffs_in_table_);
    (*encoding_coeffs)[raw_coeff_idx + 1] =
        encoder_table_[encoder_table_idx] * flip;
  }

  // If the spread is more than min. theoretical spread for the given
  // |ambisonic_order|, multiply the encoding coefficients by the required
  // spread control gains from the |kSpreadCoeffs| lookup table.
  if (source_spread_deg >= kMinSpreads[ambisonic_order]) {
    const int spread_table_idx =
        GetSpreadTableIndex(ambisonic_order, source_spread_deg);
    (*encoding_coeffs)[0] *= kSpreadCoeffs[spread_table_idx];
    for (size_t coeff = 1; coeff < encoding_coeffs->size(); ++coeff) {
      const int current_coefficient_degree =
          GetPeriphonicAmbisonicOrderForChannel(coeff);
      (*encoding_coeffs)[coeff] *=
          kSpreadCoeffs[spread_table_idx + current_coefficient_degree];
    }
  }
}

void AmbisonicLookupTable::ComputeEncoderTable() {

  // Associated Legendre polynomial generator.
  AssociatedLegendrePolynomialsGenerator alp_generator(
      max_ambisonic_order_, /*condon_shortley_phase=*/false,
      /*compute_negative_order=*/false);
  // Temporary storage for associated Legendre polynomials generated.
  std::vector<float> temp_associated_legendre_polynomials;
  for (size_t azimuth_idx = 0; azimuth_idx < kNumAzimuths; ++azimuth_idx) {
    for (size_t elevation_idx = 0; elevation_idx < kNumElevations;
         ++elevation_idx) {
      const SphericalAngle angle(
          static_cast<float>(azimuth_idx) * kRadiansFromDegrees,
          static_cast<float>(elevation_idx) * kRadiansFromDegrees);
      temp_associated_legendre_polynomials =
          alp_generator.Generate(std::sin(angle.elevation()));
      // First spherical harmonic is always equal 1 for all angles so we do not
      // need to compute and store it.
      for (int degree = 1; degree <= max_ambisonic_order_; ++degree) {
        for (int order = -degree; order <= degree; ++order) {
          const size_t alp_index =
              alp_generator.GetIndex(degree, std::abs(order));
          const float alp_value =
              temp_associated_legendre_polynomials[alp_index];
          const size_t raw_coeff_idx = AcnSequence(degree, order) - 1;
          const size_t encoder_table_idx =
              GetEncoderTableIndex(azimuth_idx, elevation_idx, raw_coeff_idx,
                                   kNumElevations, max_num_coeffs_in_table_);
          encoder_table_[encoder_table_idx] =
              Sn3dNormalization(degree, order) *
              UnnormalizedSphericalHarmonic(alp_value, order, angle.azimuth());
        }
      }
    }
  }
}

void AmbisonicLookupTable::ComputeSymmetriesTable() {

  for (int degree = 1; degree <= max_ambisonic_order_; ++degree) {
    for (int order = -degree; order <= degree; ++order) {
      const size_t raw_coeff_idx = AcnSequence(degree, order) - 1;
      // Symmetry wrt the left-right axis (Y).
      symmetries_table_[GetSymmetriesTableIndex(0, raw_coeff_idx,
                                                max_num_coeffs_in_table_)] =
          order < 0 ? -1.0f : 1.0f;
      // Symmetry wrt the up-down axis (Z).
      symmetries_table_[GetSymmetriesTableIndex(1, raw_coeff_idx,
                                                max_num_coeffs_in_table_)] =
          static_cast<float>(IntegerPow(-1, degree + order));
      // Symmetry wrt the front-back axis (X).
      symmetries_table_[GetSymmetriesTableIndex(2, raw_coeff_idx,
                                                max_num_coeffs_in_table_)] =
          order < 0 ? static_cast<float>(-IntegerPow(-1, std::abs(order)))
                    : static_cast<float>(IntegerPow(-1, order));
    }
  }
}

float AmbisonicLookupTable::UnnormalizedSphericalHarmonic(float alp_value,
                                                          int order,
                                                          float azimuth_rad) {
  const float horizontal_term =
      (order >= 0) ? std::cos(static_cast<float>(order) * azimuth_rad)
                   : std::sin(static_cast<float>(-order) * azimuth_rad);
  return alp_value * horizontal_term;
}

}  // namespace vraudio
