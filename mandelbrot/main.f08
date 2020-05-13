program test
  use iso_fortran_env
  implicit none

  integer, parameter :: dflint = int32

  integer(kind=dflint) :: c
  real :: a, b, result

  a = 12.0
  b = 15.0
  result = a + b
  print *, 'Total: ', result

end program test
