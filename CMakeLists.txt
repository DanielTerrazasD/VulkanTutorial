# Set the minimum version of cmake that can be used
cmake_minimum_required(VERSION 3.12)

# Set the project name
set(PROJECT_NAME VulkanTutorial)
project(${PROJECT_NAME})

# Set the target name
set(TARGET Tutorial)

# Set the C++ standard version
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# GLFW build configuration
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)


# Required packages
find_package(Vulkan REQUIRED)

if(TARGET Vulkan::glslc)
    message(STATUS "Using glslc from Vulkan package: Vulkan::glslc")
    set(GLSLC_COMMAND "$<TARGET_FILE:Vulkan::glslc>")
else()
    find_program(GLSLC_EXECUTABLE
        NAMES glslc
        HINTS "$ENV{VULKAN_SDK}/Bin" "$ENV{VULKAN_SDK}/Bin32"
        REQUIRED
    )
    message(STATUS "Using glslc from find_program: ${GLSLC_EXECUTABLE}")
    set(GLSLC_COMMAND "${GLSLC_EXECUTABLE}")
endif()

# Required directories
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/external/glfw/CMakeLists.txt")
    message(FATAL_ERROR "external/glfw missing — run: git submodule update --init --recursive")
endif()
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/external/stb/stb_image.h")
    message(FATAL_ERROR "external/stb missing or incomplete — run: git submodule update --init --recursive")
endif()
add_subdirectory(external/glfw)

# Source files
set(SOURCES src/main.cpp)
# Set header files
set(HEADERS)

add_executable(${TARGET} ${SOURCES} ${HEADERS})

# Required directories
target_include_directories(${TARGET} PRIVATE
    ${CMAKE_SOURCE_DIR}/external/glm
    ${CMAKE_SOURCE_DIR}/external/stb
)

# Required libraries
target_link_libraries(${TARGET} PRIVATE
    Vulkan::Vulkan
    glfw
)

# Recursively find all shader files
file(GLOB SHADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/*.vert"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/*.tesc"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/*.tese"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/*.geom"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/*.frag"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/*.comp"
)

# Compile shaders to SPIR-V using glslc
set(SPIRV_FILES)
foreach(SHADER ${SHADERS})
    get_filename_component(SHADER_NAME ${SHADER} NAME)
    set(SPIRV ${CMAKE_CURRENT_BINARY_DIR}/shaders/${SHADER_NAME}.spv)

    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/shaders
        COMMAND ${GLSLC_COMMAND} ${SHADER} -o ${SPIRV}
        DEPENDS ${SHADER}
        COMMENT "Compiling ${SHADER_NAME}"
    )
    list(APPEND SPIRV_FILES ${SPIRV})
endforeach()

add_custom_target(Shaders DEPENDS ${SPIRV_FILES})
add_dependencies(${TARGET} Shaders)

# Copy compiled shaders (SPIR-V) next to the executable
add_custom_command(
    TARGET ${TARGET}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_BINARY_DIR}/shaders
            $<TARGET_FILE_DIR:${TARGET}>/assets/shaders
)

# Copy models folder next to the executable (if exists)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets/models")
    add_custom_command(
        TARGET ${TARGET}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_CURRENT_SOURCE_DIR}/assets/models
                $<TARGET_FILE_DIR:${TARGET}>/assets/models
    )
endif()

# Copy textures folder next to the executable (if exists)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets/textures")
    add_custom_command(
        TARGET ${TARGET}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_CURRENT_SOURCE_DIR}/assets/textures
                $<TARGET_FILE_DIR:${TARGET}>/assets/textures
    )
endif()

get_property(glfw_targets DIRECTORY external/glfw/src PROPERTY BUILDSYSTEM_TARGETS)
foreach(t ${glfw_targets})
    set_property(TARGET ${t} PROPERTY FOLDER "external")
endforeach()
set_property(TARGET Shaders PROPERTY FOLDER "shaders")

set_target_properties(${TARGET} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${TARGET}>")