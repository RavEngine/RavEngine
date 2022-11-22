function f = generate_filter (cycles, fudge_factor, increment, atten)

if nargin != 4,
	error ("Need four args.") ;
	endif

# Calclate N and make sure it is even.
N = fix (4 * cycles * fudge_factor * increment) ;

if rem (N, 2) != 0,
	N = N - 1 ;
	endif

# Generate the Sinc function.

m = -((N-1)/2):((N-1)/2) ;
f = sinc (m / fudge_factor / increment) ;

# Genertate the window function and apply it.

w = kaiser (N, (atten + 0.5) / 10) ;
w = w' ;

f = f .* w ;

f = f / sum (f) ;

endfunction

# Do not edit or modify anything in this comment block.
# The arch-tag line is a file identity tag for the GNU Arch 
# revision control system.
#
# arch-tag: 7e57a3cb-3f5c-4346-bfcd-4da1e758e2a7

