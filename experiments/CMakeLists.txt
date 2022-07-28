ADD_EXECUTABLE(osr osr.cpp)
TARGET_INCLUDE_DIRECTORIES(osr PRIVATE ${CMAKE_BINARY_DIR})
TARGET_LINK_LIBRARIES(osr PRIVATE Eigen3::Eigen glfw OpenGL::GL)
ADD_CUSTOM_TARGET(osrtest
        DEPENDS osr
        COMMAND osr
        )
#TARGET_LINK_LIBRARIES(stl2bmp PRIVATE Eigen3::Eigen glfw OpenGL::GL Boost::filesystem )