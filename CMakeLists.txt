CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
PROJECT(stl2bmp VERSION 0.1.0 LANGUAGES CXX)
SET(CMAKE_BUILD_TYPE Release) #Debug,RelWithDebInfo,MinSizeRel
# C++ options
SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_CXX_EXTENSIONS OFF)

IF(MSVC)
        SET(Eigen3_DIR "")
        SET(glfw3_DIR "C:/glfw-3.3.7/lib/cmake/glfw3")
ENDIF(MSVC)
FIND_PACKAGE(Eigen3 3.4 REQUIRED)
FIND_PACKAGE(glfw3 3.3 REQUIRED)
FIND_PACKAGE(OpenGL)

#FIND_PACKAGE(Boost 1.36.0 REQUIRED COMPONENTS filesystem system)


IF(UNIX)
        IF (APPLE)
                SET(CMAKE_MACOSX_RPATH 1)
                SET(CMAKE_CXX_FLAGS "-DGL_SILENCE_DEPRECATION -Wall -Wextra -pedantic -Wcast-align -Wcast-qual -Wconversion -Wdisabled-optimization -Wnon-virtual-dtor -Wfloat-equal -Winit-self -Winline -Wmissing-include-dirs -Woverloaded-virtual -Wpacked -Wpointer-arith -Wredundant-decls -Wshadow -Wsign-promo -Wswitch-default -Wswitch-enum -Wvariadic-macros -Wwrite-strings  -Wnon-virtual-dtor -Wold-style-cast")
                SET(CMAKE_CXX_FLAGS_DEBUG "-g3 -O0 -pg")
                SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG ")
                SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g3 -Og -pg")
                SET(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -DNDEBUG -march=native")
        ELSEIF(CMAKE_COMPILER_IS_GNUCXX) #UNIX
                SET(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic")
                SET(CMAKE_CXX_FLAGS_DEBUG "-g3 -O0 -pg")
                SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native")
                SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g3 -Og -pg")
                SET(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -DNDEBUG -march=native")
        ELSE()
                MESSAGE(fatal, "Unknown platform")
        ENDIF ()
ELSEIF(MSVC) #Visual Studio
        ADD_COMPILE_DEFINITIONS(/wd4244 /wd4819 /wd4127 /wd26495 /wd26451 /wd26812) #disable warnings
        SET(CMAKE_CXX_FLAGS "/W4 /EHsc")
ELSE()
        MESSAGE(fatal, "Unknown platform")
ENDIF(UNIX)
CONFIGURE_FILE(stl2bmp_version.hpp.in stl2bmp_version.hpp)
ADD_EXECUTABLE(stl2bmp stl2bmp_main.cpp)
TARGET_INCLUDE_DIRECTORIES(stl2bmp PRIVATE ${CMAKE_BINARY_DIR})
TARGET_LINK_LIBRARIES(stl2bmp PRIVATE Eigen3::Eigen glfw OpenGL::GL)
#TARGET_LINK_LIBRARIES(stl2bmp PRIVATE Eigen3::Eigen glfw OpenGL::GL Boost::filesystem )