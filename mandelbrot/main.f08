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
    type(Coord)                              :: res, current
    type(Frame)                              :: frame
    type(Bounds)                             :: scaling
    complex                                  :: c = 0, z = 0
    integer(kind=int32)                      :: i = 0, imax
    integer(kind=int32), allocatable, public :: img(:, :)
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

  type(Image) function make_image(res, frame_in, imax) result(img)
    type(Coord),         intent(in) :: res
    type(Frame),         intent(in) :: frame_in
    integer(kind=int32), intent(in) :: imax

    img%imax      = imax
    img%res       = res
    img%frame     = frame_in
    img%scaling%x = img%frame%width() / img%res%x
    img%scaling%y = img%frame%height() / img%res%y

    allocate(img%img(res%x, res%y))
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

  type(Image) :: img

  img = make_image(Coord(x=1024, y=768), &
                   Frame(Bounds(-2.0, -1.2), Bounds(1.0, 1.2)), &
                   4096)

end program
