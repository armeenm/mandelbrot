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
    integer(kind=int32)                      :: imax
    integer(kind=int32), allocatable, public :: img(:, :)
  contains
    procedure, pass(this) :: px_scale => image_px_scale
    procedure, pass(this) :: calc     => image_calc
    procedure             :: calc_all => image_calc_all
    procedure, pass(this) :: save_pgm => image_save_pgm
    final                 :: image_dtor
  end type

contains

  real elemental function frame_width(this) result(r)
    class(Frame), intent(in) :: this

    r = this%upper%x - this%lower%x
  end function

  real elemental function frame_height(this) result(r)
    class(Frame), intent(in) :: this

    r = this%upper%y - this%lower%y
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

  complex elemental function image_px_scale(this, coord_in) result(c)
    type(Coord), intent(in)  :: coord_in
    class(Image), intent(in) :: this

    c = cmplx(this%scaling%x * coord_in%x + this%frame%lower%x, &
              this%scaling%y * coord_in%y + this%frame%lower%y) 
  end function


  integer(kind=int32) function image_calc(this, coord_in) result(i)
    type(Coord),  intent(in) :: coord_in
    class(Image), intent(inout) :: this
    real                     :: tmp
    complex                  :: c, z

    c = this%px_scale(coord_in)
    !print *, 'c: ', real(c), aimag(c)
    z = cmplx(0.0, 0.0)

    do i = 0, this%imax - 2
      tmp = real(z) * real(z) - aimag(z) * aimag(z) + real(c)
      z = cmplx(tmp, 2.0 * real(z) * aimag(z) + aimag(c))

      if (real(z * conjg(z)) > 4.0) then
        exit
      end if
    end do

    this%img(coord_in%x, coord_in%y) = i
  end function

  subroutine image_calc_all(this)
    class(Image), intent(inout) :: this
    integer(kind=int32)         :: i, j

    do i = 1, this%res%x
      do j = 1, this%res%y
        this%img(i, j) = this%calc(Coord(i, j))
      end do
    end do
  end subroutine

  subroutine image_save_pgm(this, filename)
    character(len=*), intent(in) :: filename
    class(Image),     intent(in) :: this
    integer(kind=int32)          :: i, j

    open(10, file=filename, action='write')

    write(10, '(a, i0, a, i0, a, i0, a)') 'P2'//new_line('a'), &
                                          this%res%x, ' ', this%res%y, &
                                          new_line('a'), this%imax, new_line('a')

    do i = 1, this%res%x
      do j = 1, this%res%y
        write(10, '(i0, a)', advance='no') this%img(i, j), ' '
      end do

      write(10, *) ! Newline
    end do
    
    close(10)

  end subroutine 

  subroutine image_dtor(this)
    type(image), intent(inout) :: this

    deallocate(this%img)
  end subroutine

end module

program main
  use iso_fortran_env
  use mandelbrot
  implicit none

  type(Image) :: img
  integer(kind=int32) :: i
  complex :: a, b

  img = make_image(Coord(x=1024, y=768), &
                   Frame(Bounds(-2.0, -1.2), Bounds(1.0, 1.2)), 4096)

  i = img%calc(Coord(x=1, y=384))

  print *, i

  a = cmplx(3, 5)
  print *, real(a * conjg(a))

  !call img%calc_all()
  call img%save_pgm('temp.pgm')

end program
