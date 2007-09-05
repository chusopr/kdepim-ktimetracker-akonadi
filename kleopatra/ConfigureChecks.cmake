# assuan configure checks
include(CheckFunctionExists)

find_package(Assuan)

set( USABLE_ASSUAN_FOUND false )

if ( ASSUAN_FOUND )

  set( CMAKE_REQUIRED_INCLUDES ${ASSUAN_INCLUDES} )

  if ( WIN32 AND ASSUAN_VANILLA_FOUND )
    set( CMAKE_REQUIRED_LIBRARIES ${ASSUAN_VANILLA_LIBRARIES} )
    set( USABLE_ASSUAN_FOUND true )
  elseif( NOT WIN32 AND ASSUAN_PTHREAD_FOUND )
    set( CMAKE_REQUIRED_LIBRARIES ${ASSUAN_PTHREAD_LIBRARIES} )
    set( USABLE_ASSUAN_FOUND true )
  endif( WIN32 AND ASSUAN_VANILLA_FOUND )

endif( ASSUAN_FOUND )

if ( USABLE_ASSUAN_FOUND )

  # check whether assuan and gpgme may be linked to simultaneously
  check_function_exists( "assuan_get_pointer" USABLE_ASSUAN_FOUND )

endif( USABLE_ASSUAN_FOUND )

if ( USABLE_ASSUAN_FOUND )
  # check if assuan has assuan_fd_t
  check_cxx_source_compiles("
        #include <assuan.h>
        int main() {
            assuan_fd_t fd = ASSUAN_INVALID_FD;
            return fd ? 1 : 0 ;
        }
        "
    HAVE_ASSUAN_FD_T )

  if ( WIN32 AND NOT HAVE_ASSUAN_FD_T )
    set( USABLE_ASSUAN_FOUND false )
  endif ( WIN32 AND NOT HAVE_ASSUAN_FD_T )

  # check if assuan has assuan_inquire_ext, old style
  check_function_exists( "assuan_inquire_ext" HAVE_ASSUAN_INQUIRE_EXT )

  if ( NOT HAVE_ASSUAN_INQUIRE_EXT )
    set( USABLE_ASSUAN_FOUND false )
  endif( NOT HAVE_ASSUAN_INQUIRE_EXT )

  # check if assuan has new-style assuan_inquire_ext:
  check_cxx_source_compiles("
        #include <assuan.h>
        static int handler( void *, int, unsigned char*, size_t ) { return 0; }
        int main() {
            assuan_context_t ctx = 0;
            const size_t maxSize = 0U;
            assuan_error_t err = assuan_inquire_ext( ctx, \"FOO\", maxSize, handler, (void*)0 );
            return err ? 1 : 0 ;
        }
        "
    HAVE_NEW_STYLE_ASSUAN_INQUIRE_EXT )

endif ( USABLE_ASSUAN_FOUND )

macro_bool_to_01( USABLE_ASSUAN_FOUND  HAVE_USABLE_ASSUAN )

set(CMAKE_REQUIRED_INCLUDES)
set(CMAKE_REQUIRED_LIBRARIES)
