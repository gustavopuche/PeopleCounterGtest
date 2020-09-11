cmake_minimum_required(VERSION 3.1)
# Enable C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
find_package( OpenCV REQUIRED )

include(GNUInstallDirs)

enable_testing()

include_directories(include src)
add_subdirectory(src)
add_subdirectory(test)

add_custom_target(run)
add_custom_command( TARGET run
	POST_BUILD COMMAND ./src/PeopleCounter -i input-airport-480x360.mp4)
add_custom_target(run-hog)
add_custom_command( TARGET run-hog
	POST_BUILD COMMAND ./src/PeopleCounter -i input-airport-fhd.mp4 -t hog)
