cmake_minimum_required(VERSION 3.5.1)

project(SymphonyOfEmpires)
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/src)
set(PROJECT_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)

#set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Isrc -Isrc/client -pthread")
IF(WIN32)
# Use x86_64-w64-mingw-* toolchain for windows builds on linux
	set(CMAKE_C_COMPILER "i686-w64-mingw32-gcc-posix")
	set(CMAKE_CXX_COMPILER "i686-w64-mingw32-g++-posix")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Dwindows")
	link_directories(${CMAKE_SOURCE_DIR}/windows_libs/lib)
	include_directories(${CMAKE_SOURCE_DIR}/windows_libs/include)
	include_directories(${CMAKE_SOURCE_DIR}/windows_libs/include/SDL2)
ELSE()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Dunix")
# Workaround for clang (see https://bugs.launchpad.net/ubuntu/+source/libsdl2-ttf/+bug/1872023)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/usr/include/SDL2")
	link_directories(/usr/local/lib)
ENDIF()

# Helps github workflows to not die
IF(UNIT_TEST)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUNIT_TEST")
ENDIF()

include_directories("${PROJECT_SOURCE_DIR}")

file(GLOB MAIN_SOURCES "${PROJECT_SOURCE_DIR}/*.cpp")
file(GLOB AI_SOURCES "${PROJECT_SOURCE_DIR}/simple_ai/*.cpp")
file(GLOB CLIENT_SOURCES "${PROJECT_SOURCE_DIR}/client/*.cpp")
file(GLOB RENDER_SOURCES "${PROJECT_SOURCE_DIR}/client/render/*.cpp")
file(GLOB INTERFACE_SOURCES "${PROJECT_SOURCE_DIR}/client/interface/*.cpp")
add_executable(SymphonyOfEmpiresClient ${MAIN_SOURCES} ${CLIENT_SOURCES} ${AI_SOURCES} ${RENDER_SOURCES} ${INTERFACE_SOURCES})

IF(UNIT_TEST)
	target_link_libraries(SymphonyOfEmpiresClient PUBLIC tbb)
ENDIF()

IF(WIN32)
	target_link_libraries(SymphonyOfEmpiresClient PUBLIC opengl32 glu32 ws2_32 winpthread intl lua)
ELSE()
	target_link_libraries(SymphonyOfEmpiresClient PUBLIC GL GLEW GLU SDL2main stdc++)
ENDIF()
target_link_libraries(SymphonyOfEmpiresClient PUBLIC SDL2 SDL2_ttf m z tbb)

file(GLOB SERVER_SOURCES "${PROJECT_SOURCE_DIR}/server/*.cpp")
add_executable(SymphonyOfEmpiresServer ${MAIN_SOURCES} ${SERVER_SOURCES})
target_link_libraries(SymphonyOfEmpiresServer PUBLIC stdc++ m z)
IF(lua54)
	target_link_libraries(SymphonyOfEmpiresServer PUBLIC lua5.4)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLUA54")
	message("Making with Lua 5.4")
ELSE()
	target_link_libraries(SymphonyOfEmpiresServer PUBLIC lua5.3)
	message("Making with Lua 5.3")
ENDIF()
