function [stop_atten stop_band_start minus_3db] = measure_filter (f, atten)


spec_len = 400000 ;

# Calculate the spectrum.

spec = 20 * log10 (abs (fft ([f zeros(1, spec_len - length (f))]))) ;

spec = spec (1:spec_len/2) ;

#-------------------------------------------------------------------------------
# Find the first null which starts off the stop band.

first_null = 0 ;
for k = 2:length (spec) - 1,
	if spec (k) < -0.8 * atten && spec (k-1) > spec (k) && spec (k) < spec (k + 1),
		first_null = k ;
		break
		endif
	endfor

#-------------------------------------------------------------------------------
# Find the stop band minimum attenuation.

stop_atten = max (spec (first_null:length (spec))) ;

#-------------------------------------------------------------------------------
# Find the x position on the transition band which has the same attenuation.

atten_start = 0 ;
for k = 1:first_null,
	if spec (k) > stop_atten && spec (k + 1) < stop_atten,
		atten_start = k ;
		break ;
		endif
	endfor
	
atten_start = atten_start - 1 ;		# Arrays are 1 based so subtract 1.
	
stop_band_start = atten_start + (stop_atten - spec (atten_start)) / (spec (atten_start+1) - spec (atten_start)) ;


stop_band_start = stop_band_start / spec_len ;

#-------------------------------------------------------------------------------
# Find -3db point.

minus_3db = 0 ;
for k = 1:first_null,
	if spec (k) > -3.0 && spec (k + 1) < -3.0,
		minus_3db = k ;
		break ;
		endif
	endfor
	
minus_3db = minus_3db - 1 ; 		# Arrays are 1 based so subtract 1.
	
minus_3db = minus_3db + (stop_atten - spec (minus_3db)) / (spec (minus_3db+1) - spec (minus_3db)) ;

minus_3db = minus_3db / spec_len ;

endfunction

# Do not edit or modify anything in this comment block.
# The arch-tag line is a file identity tag for the GNU Arch 
# revision control system.
#
# arch-tag: cc2bc9a2-d387-4fed-aa0a-570e91f17c99

