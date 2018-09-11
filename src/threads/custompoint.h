/* Assume that x and y are fixed-point numbers, and n is an integer. */
/* Fixed point numbers are in signed p : q format, where p + q = 31, and f is 1 << q. */
/* We assume p = 17, q = 14 here. */

#define f (1 << 14)
#define INT_MAX ((1 << 31) - 1)
#define INT_MIN (-(1 << 31))

/* Ameya's code below. */

int 
to_float (int n)
{
  return n * f;
}

int 
round_to_zero (int x)
{
  return x / f;
}

int 
round_to_nearest (int x)
{
  if (x >= 0)
    return (x + f / 2) / f;
  else	
    return (x - f / 2) / f;
}

int 
add_float_float (int x, int y)
{
  return x + y;
}

int 
subtract_from_float_float (int x, int y)
{
  return x - y;
}

int 
add_float_int (int x, int n)
{
  return x + n * f;	
}

int 
subtract_from_float_int (int x, int n)
{
  return x - n * f; 
}

int 
multiply_float_float (int x, int y)
{
  return ((int64_t)x) * y / f;
}

int 
multiply_float_int (int x, int y)
{
  return x * y;
}

int 
divide_float_by_float (int x, int y)
{
  return ((int64_t)x) * f / y;	
}

int 
divide_float_by_int (int x, int y)
{
  return x / y;
}
