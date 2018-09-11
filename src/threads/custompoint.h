/* Assume that x and y are fixed-point numbers, and n is an integer. */
/* Fixed point numbers are in signed p : q format, where p + q = 31, and f is 1 << q. */
/* As in B.6 Fixed-Point Real Arithmetic, we assume p = 17, q = 14 here. */

#define f (1 << 14)
#define INT_MAX ((1 << 31) - 1)
#define INT_MIN (-(1 << 31))

/* Ameya's code below because he can't stand functions such as add_x_and_y */
/* Most of the underlying code is the same though. Lol. */
/* Remember floats are always the first argument! */

int 
to_float(int n)
{
	return n * f;
}

int 
round_to_zero (int fl)
{
	return fl / f;
}

int
round_to_nearest (int fl)
{
	 if (fl >= 0)
    return (fl + f / 2) / f;
  else	
    return (fl - f / 2) / f;
}

int
add_float_int(int fl, int n)
{
	 	return fl + n * f;
}

int
subtract_from_float_int(int fl, int n)
{
	 	return fl - n * f;
}

int
multiply_float_int(int fl, int n)
{
	 	return fl * n;
}

int
divide_float_by_int(int fl, int n)
{
	 	return fl / n;
}

int
multiply_float_float(int fl1, int fl2)
{
	 	return (((int64_t) fl1) * fl2) / f;
}

int
divide_float_by_float(int fl1, int fl2)
{
	 	return (((int64_t) fl1) * f) / fl2;
}

