module mandelbrot
  use iso_fortran_env
  implicit none

  integer, parameter, public :: k_arr_size_ = 4

  type, public :: Coord
    integer(kind=int32) :: x, y
  end type

  type, public :: Bounds
    real :: x, y
  end type

  type, public :: Frame
    type(Bounds) :: lower, upper
  contains
    procedure :: width  => frame_width
    procedure :: height => frame_height
  end type

  type, public :: Image
    private
    integer      :: i, imax
    type(Coord)  :: res, current
    type(Frame)  :: frame
    type(Bounds) :: scaling
    complex      :: c, z
  contains
    procedure             :: clean    => image_clean
    procedure             :: calc     => image_calc
    procedure, pass(this) :: save_pgm => image_save_pgm
  end type

contains

  real elemental function frame_width(this) result(r)
    class(Frame), intent(in) :: this

    r = this%upper%x - this%lower%x
  end function

  real elemental function frame_height(this) result(r)
    class(Frame), intent(in) :: this

    r = this%upper%x - this%lower%x
  end function

  type(Image) elemental function make_image(res, frame_in, imax) result(img)
    type(Coord),         intent(in) :: res
    type(Frame),         intent(in) :: frame_in
    integer(kind=int32), intent(in) :: imax

    img%imax  = imax
    img%res   = res
    img%frame = frame_in
  end function

  elemental subroutine image_clean(this)
    class(Image), intent(inout) :: this
  end subroutine

  elemental subroutine image_calc(this)
    class(Image), intent(inout) :: this
  end subroutine

  elemental subroutine image_save_pgm(filename, this)
    character(len=*), intent(in)    :: filename
    class(Image),     intent(inout) :: this
  end subroutine 

end module

program main
  use iso_fortran_env
  use mandelbrot
  implicit none

end program
