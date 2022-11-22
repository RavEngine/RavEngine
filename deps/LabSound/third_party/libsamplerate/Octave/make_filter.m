function f = make_filter (cycles, increment, atten, filename)

# This works :
# 
# f = make_filter (67, 128, 100.3) ;
# f = make_filter (13, 128, 100.5) ;
# f = make_filter (185, 4, 157.0) ;


#=======================================================================

if nargin < 3,
	error ('Try make_filter (12, 32, 88, "output.txt")') ;
	endif

if nargin < 4,
	filename = 0 ;
elseif (isstr (filename) == 0),
	error ("Fourth parameter must be a file name.") ;
	endif

fudge_factor1 = 1.0 ;
f1 = generate_filter (cycles, fudge_factor1, increment, atten) ;
[stop_atten stop_band_start1 minus_3db] = measure_filter (f1, atten) ;
printf ("    fudge_factor : %15.13f   stop_band_start : %15.13f   1\n", fudge_factor1, stop_band_start1) ;

fudge_factor2 = 1.25 ;
f2 = generate_filter (cycles, fudge_factor2, increment, atten) ;
[stop_atten stop_band_start2 minus_3db] = measure_filter (f2, atten) ;
printf ("    fudge_factor : %15.13f   stop_band_start : %15.13f   2\n", fudge_factor2, stop_band_start2) ;

f = f1 ;
fudge_factor = fudge_factor1 ;
stop_band_start = stop_band_start1 ;

while ((stop_band_start1 - stop_band_start2) > 0.0000000001)
	if (stop_band_start1 < stop_band_start2)
		printf ("stop_band_start1 < stop_band_start2\n") ;
		break ;
		endif

	fudge_factor = fudge_factor1 + (fudge_factor2 - fudge_factor1) / 2 ;
	f = generate_filter (cycles, fudge_factor, increment, atten) ;
	[stop_atten stop_band_start minus_3db] = measure_filter (f, atten) ;
	
	if (stop_band_start > 1.0)
		printf ("A %10.8f   %10.8f   %10.8f\n", fudge_factor1, fudge_factor, fudge_factor2) ;
		continue ;
		endif

	if (stop_band_start < 0.5 / increment)
		f2 = f ;
		stop_band_start2 = stop_band_start ;
		fudge_factor2 = fudge_factor ;
		choice = 2 ;
	else
		f1 = f ;
		stop_band_start1 = stop_band_start ;
		fudge_factor1 = fudge_factor ;
		choice = 1 ;
		endif

	printf ("    fudge_factor : %15.13f   stop_band_start : %15.13f   %d\n", fudge_factor, stop_band_start, choice) ;
	endwhile

printf ("\n") ;

#-------------------------------------------------------------------------------
# Grab only half the coefficients.

N = length (f) ;

f = increment * f' ;

if rem (length (f), 2) == 0,
	index = find (f == max (f)) ;
	index = min (index) - 1 ;
	half_f = f (index:length (f)) ;
else
	error ("Length should be even.") ;
	endif

trailing_zeros = 4 - rem (length (half_f), 4) ;

#-------------------------------------------------------------------------------
# Print analysis.

printf ("# f = make_filter (%d, %d, %4.1f) ;\n", cycles, increment, atten) ;
printf ("#   Coeff. count     : %d\n", N) ;
printf ("#   Fudge factor     : %9.7f\n", fudge_factor) ;
printf ("#   Pass band width  : %12.10f (should be %12.10f)\n", stop_band_start, 0.5 / increment) ;
printf ("#   Stop band atten. : %5.2f dB\n", abs (stop_atten)) ;
printf ("#   -3dB band Width  : %5.3f\n", 0.5 / increment / minus_3db) ;
printf ("#   half length      : %d\n", length (half_f) + trailing_zeros) ;
printf ("#   increment        : %d\n", increment) ;

if filename,
	file = fopen (filename, "w") ;
	if file == 0,
		str = sprintf ("Error, not able to open '%s'", filename)
		error (str) ;
		endif

	fprintf (file, "/*\n") ;
	fprintf (file, "**  f = make_filter (%d, %d, %4.1f) ;\n", cycles, increment, atten) ;
	fprintf (file, "**  Pass band width  : %9.7f (should be %9.7f)\n", stop_band_start, 0.5 / increment) ;
	fprintf (file, "**  Stop band atten. : %5.2f dB\n", abs (stop_atten)) ;
	fprintf (file, "**  -3dB band width  : %5.3f\n", 0.5 / increment / minus_3db) ;
	fprintf (file, "**  half length      : %d\n", length (half_f)) ;
	fprintf (file, "**  increment        : %d\n", increment) ;
	fprintf (file, "*/\n\n") ;

	for val = half_f,
		fprintf (file, "% 24.20e,\n", val) ;
		endfor

	if trailing_zeros > 0,
		for val = 2:trailing_zeros,
			fprintf (file, " 0,\n") ;
			endfor
		fprintf (file, " 0\n") ;
		endif

	fclose (file) ;
	endif

endfunction

# Do not edit or modify anything in this comment block.
# The arch-tag line is a file identity tag for the GNU Arch 
# revision control system.
#
# arch-tag: 2f1ff4fa-ea6a-4e54-a5f8-dad55def9834

