project( PeopleCounter )

file( GLOB sources *.cpp )

add_executable( PeopleCounter ${sources} )

target_link_libraries( PeopleCounter ${OpenCV_LIBS} )
