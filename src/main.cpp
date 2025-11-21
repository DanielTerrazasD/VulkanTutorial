#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

class HelloTriangleApp
{
public:
    void Run()
    {
        InitWindow();
        InitVulkan();
        MainLoop();
        Cleanup();
    }

private:
    GLFWwindow* window;

    /**
     * @brief Initialize GLFW Window
     * 
     */
    void InitWindow();

    /**
     * @brief Vulkan Initialization
     * 
     */
    void InitVulkan();

    /**
     * @brief 
     * 
     */
    void MainLoop();

    /**
     * @brief 
     * 
     */
    void Cleanup();
};

void HelloTriangleApp::InitWindow()
 {
    glfwInit();

    // Since GLFW was designed to create an OpenGL context, we need to specify to
    // not create an OpenGL context (GLFW_NO_API) with the following call:
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
}

void HelloTriangleApp::InitVulkan()
{

}

void HelloTriangleApp::MainLoop()
{
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }
}

void HelloTriangleApp::Cleanup()
{
    glfwDestroyWindow(window);

    glfwTerminate();
}

int main()
{
    HelloTriangleApp app;

    try
    {
        app.Run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}