CMAKE_MINIMUM_REQUIRED(VERSION 3.15)
PROJECT(stl2bmp
        VERSION 1.1.1
        DESCRIPTION "Voxelization of STL files"
        HOMEPAGE_URL https://github.com/tmichi/stl2bmp
        LANGUAGES CXX)

SET(PACK_ENABLED 1)
SET(CMAKE_BUILD_TYPE Release) #Debug,RelWithDebInfo,MinSizeRel
SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_CXX_EXTENSIONS OFF)

IF (MSVC)
    SET(Eigen3_DIR "C:/eigen-3.4.0/cmake")
    SET(glfw3_DIR "C:/glfw-3.3.7/lib/cmake/glfw3")
    SET(GLEW_ROOT "C:/glew-2.1.0")
    SET(GLEW_USE_STATIC_LIBS TRUE)
ENDIF (MSVC)

FIND_PACKAGE(OpenGL REQUIRED)
FIND_PACKAGE(Eigen3 3.4 REQUIRED)
FIND_PACKAGE(glfw3 3.3 REQUIRED)
FIND_PACKAGE(GLEW REQUIRED)

IF (UNIX)
    IF (APPLE)
        SET(CMAKE_MACOSX_RPATH 1)
        SET(CMAKE_CXX_FLAGS "-DGL_SILENCE_DEPRECATION -Wall -Wextra -pedantic -Wcast-align -Wcast-qual -Wconversion -Wdisabled-optimization -Wnon-virtual-dtor -Wfloat-equal -Winit-self -Winline -Wmissing-include-dirs -Woverloaded-virtual -Wpacked -Wpointer-arith -Wredundant-decls -Wshadow -Wsign-promo -Wswitch-default -Wswitch-enum -Wvariadic-macros -Wwrite-strings  -Wnon-virtual-dtor -Wold-style-cast")
        SET(CMAKE_CXX_FLAGS_DEBUG "-g3 -O0 -pg")
        SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG ")
        SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g3 -Og -pg")
        SET(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -DNDEBUG -march=native")
    ELSEIF (CMAKE_COMPILER_IS_GNUCXX) #UNIX
        SET(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic")
        SET(CMAKE_CXX_FLAGS_DEBUG "-g3 -O0 -pg")
        SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native")
        SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g3 -Og -pg")
        SET(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -DNDEBUG -march=native")
    ELSE ()
        MESSAGE(fatal, "Unknown platform")
    ENDIF ()
ELSEIF (MSVC) #Visual Studio
    ADD_COMPILE_DEFINITIONS(/wd4244 /wd4819 /wd4127 /wd26495 /wd26451 /wd26812) #disable warnings
    SET(CMAKE_CXX_FLAGS "/W4 /EHsc")
ELSE ()
    MESSAGE(fatal, "Unknown platform")
ENDIF (UNIX)
CONFIGURE_FILE(stl2bmp_version.hpp.in stl2bmp_version.hpp)
CONFIGURE_FILE(README.txt.in README.txt)
ADD_EXECUTABLE(stl2bmp stl2bmp_main.cpp OffScreenRenderer.hpp)
TARGET_INCLUDE_DIRECTORIES(stl2bmp PRIVATE ${CMAKE_BINARY_DIR} .)
TARGET_LINK_LIBRARIES(stl2bmp PRIVATE Eigen3::Eigen glfw OpenGL::GL GLEW::GLEW)
#make check test programs with different DPIs
ADD_CUSTOM_TARGET(check
        DEPENDS stl2bmp
        COMMAND stl2bmp ${CMAKE_SOURCE_DIR}/data/torus.stl
        COMMAND stl2bmp ${CMAKE_SOURCE_DIR}/data/torus.stl 14400
        COMMAND stl2bmp ${CMAKE_SOURCE_DIR}/data/torus.stl 180
        )

IF (PACK_ENABLED)
    SET(SCRIPT_DIR "${CMAKE_SOURCE_DIR}/scripts")
    IF (MSVC)
        INSTALL(FILES ${SCRIPT_DIR}/stl2bmp720.bat ${SCRIPT_DIR}/bmp2png.bat DESTINATION .)
    ELSE ()
        INSTALL(FILES ${SCRIPT_DIR}/bmp2png.sh DESTINATION .)
    ENDIF ()
    INSTALL(TARGETS stl2bmp RUNTIME DESTINATION .)
    INSTALL(FILES ${CMAKE_BINARY_DIR}/README.txt ${CMAKE_SOURCE_DIR}/data/torus.stl DESTINATION .)
    SET(CPACK_GENERATOR "ZIP")
ELSE ()
    INSTALL(TARGETS stl2bmp RUNTIME DESTINATION bin)
    SET(CPACK_GENERATOR "ZIP")
ENDIF ()
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_IGNORE_FILES "cmake-build-release" ".git" ".idea")
set(CPACK_SOURCE_PACKAGE_FILE_NAME ${CMAKE_PROJECT_NAME}-${CMAKE_PROJECT_VERSION})
INCLUDE(CPack)