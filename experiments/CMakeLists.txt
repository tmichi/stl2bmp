ADD_EXECUTABLE(glfwtest glfwtest.cpp)
TARGET_LINK_LIBRARIES(glfwtest PRIVATE glfw OpenGL::GL)
ADD_EXECUTABLE(osr osr.cpp)
TARGET_LINK_LIBRARIES(osr PRIVATE glfw OpenGL::GL GLEW::GLEW)
ADD_CUSTOM_TARGET(osrtest
        DEPENDS osr
        COMMAND osr
        )
