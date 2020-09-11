project( gtestpeoplecounter )

find_package(Threads REQUIRED)

file( GLOB sources *.cpp )

add_executable( gtestpeoplecounter ${sources} )

target_link_libraries( gtestpeoplecounter ${OpenCV_LIBS} gtest gmock Threads::Threads)

add_test(NAME gtestpeoplecounter
         COMMAND gtestpeoplecounter)

add_custom_target(gtest)
add_custom_command( TARGET gtest
	POST_BUILD COMMAND ./gtestpeoplecounter)
