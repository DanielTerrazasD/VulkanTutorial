# Set the minimum version of cmake that can be used
cmake_minimum_required(VERSION 3.10)

# Set the project name
set(PROJECT_NAME VulkanTutorial)
project (${PROJECT_NAME})

# Set the target name
set(TARGET Tutorial)

# Set the C++ standard version
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
# Enable export compile commands to generate a compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# GLFW build configuration
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# Add subdirectories
add_subdirectory(external/glfw)

# Required packages
find_package(Vulkan REQUIRED)

# Set source files
set(SOURCES
    src/main.cpp
)

# Set header files
set(HEADERS
)

# Add executable to be build
add_executable(${TARGET} ${SOURCES} ${HEADERS})

# Set the directories that should be included in the build command for this target
target_include_directories(${TARGET}
    PRIVATE 
        ${Vulkan_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/external/glfw/include
        ${CMAKE_SOURCE_DIR}/external/glm
)

# Link required libraries for this target
target_link_libraries(${TARGET}
        PRIVATE
            Vulkan::Vulkan
            glfw
)

# Creat post build command for VS
# add_custom_command(TARGET ${TARGET} POST_BUILD
#     COMMAND xcopy /E /I /Y 
#     "\"${CMAKE_CURRENT_SOURCE_DIR}/shaders\""  # Source directory
#     "\"$<TARGET_FILE_DIR:${TARGET}>/shaders\"" # Destination directory
# )

# Group together all glfw_targets to the "external" folder
get_property(glfw_targets DIRECTORY external/glfw/src PROPERTY BUILDSYSTEM_TARGETS)
foreach(t ${glfw_targets})
    set_property(TARGET ${t} PROPERTY FOLDER "external")
endforeach()

# Set VS Debugging Working Directory equal to $(TargetPath)
set_target_properties(${TARGET} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${TARGET}>")