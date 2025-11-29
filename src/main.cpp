#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <optional>
#include <set>
#include <cstdint>
#include <limits>
#include <algorithm>
#include <fstream>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;
const int MAX_FRAMES_IN_FLIGHT = 2;

#ifdef NDEBUG
const bool kEnableValidationLayers = false;
#else
const bool kEnableValidationLayers = true;
#endif

const std::vector<const char*> kValidationLayers =
{
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> kDeviceExtensions =
{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

/**
 * @brief Create a Debug Utils Messenger EXT object
 * 
 * @param instance Vulkan Instance
 * @param pCreateInfo Debug Messenger Create Info struct
 * @param pAllocator 
 * @param pDebugMessenger Debug Messenger object.
 * @return VkResult
 */
VkResult CreateDebugUtilsMessengerEXT(  VkInstance instance,
                                        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator,
                                        VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    // Look up for vkCreateDebugUtilsMessengerEXT address using vkGetInstanceProcAddr.
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

/**
 * @brief Destroy the Debug Utils Messenger EXT object
 * 
 * @param instance Vulkan Instance
 * @param debugMessenger Debug Messenger object.
 * @param pAllocator 
 */
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
    // Look up for vkDestroyDebugUtilsMessengerEXT address using vkGetInstanceProcAddr.
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}

struct QueueFamilyIndices
{
    std::optional<uint32_t> mGraphicsFamily;
    std::optional<uint32_t> mPresentFamily;

    bool IsComplete()
    {
        return  mGraphicsFamily.has_value() &&
                mPresentFamily.has_value();
    }
};

struct SwapChainSupportDetails
{
    // Basic Surface Capabilities (min/max number of images in swap chain, min/max width and height of images).
    VkSurfaceCapabilitiesKHR mCapabilities;
    // Surface Formats (pixel format, color space)
    std::vector<VkSurfaceFormatKHR> mFormats;
    // Available Presentation Modes
    std::vector<VkPresentModeKHR> mPresentModes;
};

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
    GLFWwindow* mWindow;

    VkInstance mInstance;
    VkDebugUtilsMessengerEXT mDebugMessenger;

    // The physical device is implicitly destroyed when the VkInstance is destroyed.
    VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;

    VkDevice mDevice; // Logical Device
    // Queues are automatically created along with the logical device, but we need a handle to interface with them.
    // Also, device queues are implicitly cleaned up when the device is destroyed.
    VkQueue mGraphicsQueue;
    VkQueue mPresentQueue;

    VkSurfaceKHR mSurface;

    VkSwapchainKHR mSwapChain;
    // The images are created by the implementation for the swap chain and they will be automatically
    // cleaned up once the swap chain has been destroyed.
    std::vector<VkImage> mSwapChainImages;
    VkFormat mSwapChainImageFormat;
    VkExtent2D mSwapChainExtent;
    std::vector<VkImageView> mSwapChainImageViews;
    std::vector<VkFramebuffer> mSwapChainFramebuffers;

    VkRenderPass mRenderPass;
    VkPipelineLayout mPipelineLayout;
    VkPipeline mGraphicsPipeline;

    VkCommandPool mCommandPool;
    std::vector<VkCommandBuffer> mCommandBuffers;

    // Semaphores to signal that an image has been acquired from the swapchain and is ready for rendering.
    std::vector<VkSemaphore> mImageAvailableSemaphores;
    // Semaphores to signal that rendering has finished and presentation can happen.
    std::vector<VkSemaphore> mRenderFinishedSemaphores;
    // Fences to ensure that only one frame is rendering at a time.
    std::vector<VkFence> mInFlightFences;
    uint32_t mCurrentFrame = 0;
    bool mFramebufferResized = false;

    /**
     * @brief GLFW Window Initialization.
     * 
     */
    void InitWindow();

    /**
     * @brief GLFW Callback function that detects framebuffer resizes.
     * 
     * @param window GLFW Window handle.
     * @param width New Framebuffer's width.
     * @param height New Framebuffer's height.
     */
    static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);

    /**
     * @brief Vulkan Initialization.
     * 
     */
    void InitVulkan();

    /**
     * @brief Create and initialize a Vulkan Instance object
     * 
     */
    void CreateInstance();

    /**
     * @brief Setup the Debug Messenger VkDebugUtilsMessengerEXT object.
     * 
     */
    void SetupDebugMessenger();

    /**
     * @brief Create and initialize a VkSurfaceKHR object.
     * 
     */
    void CreateSurface();

    /**
     * @brief Fill the given VkDebugUtilsMessengerCreateInfoEXT struct with the appropiate configuration.
     * 
     * @param createInfo 
     */
    void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    /**
     * @brief Checks if the list of kValidationLayers is supported by the driver.
     * 
     * @return True if validation layer is supported.
     * @return False, otherwise.
     */
    bool CheckValidationLayerSupport();

    /**
     * @brief Get the required extensions for GLFW + Debug.
     * 
     * @return A list of extension names.
     */
    std::vector<const char*> GetRequiredExtensions();

    /**
     * @brief Check if the list of kDeviceExtensions is supported by the Physical Device.
     * 
     * @param device 
     * @return true 
     * @return false 
     */
    bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

    /**
     * @brief Given a Vulkan Physical Device returns a struct containing the Swap Chain Support
     * Details (capabilities, formats and present modes).
     * 
     * @param device Vulkan Physical Device
     * @return SwapChainSupportDetails 
     */
    SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

    /**
     * @brief Choose the Swap Chain format (ideally format = VK_FORMAT_B8G8R8A8_SRGB,
     * colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR).
     * 
     * @param availableFormats List of available formats supported by the Physical Device.
     * @return VkSurfaceFormatKHR object supporting the desired format
     */
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

    /**
     * @brief Choose the Swap Chain present mode (only the VK_PRESENT_MODE_FIFO_KHR mode is guaranteed
     * to be available, if energy is not a concern, ideally presentMode = VK_PRESENT_MODE_MAILBOX_KHR)
     * 
     * @param availablePresentModes 
     * @return VkPresentModeKHR 
     */
    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

    /**
     * @brief Choose the Swap Chain extent, which is the resolution of the Swap Chain images and
     * it's almost exactly equal to the resolution of the window that the app is drawing to in pixels.
     * 
     * @param capabilities Surface Capabilities, containing the range of possible resolutions.
     * @return VkExtent2D 
     */
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    /**
     * @brief Create and initialize a VkSwapchainKHR object
     * 
     */
    void CreateSwapChain();

    /**
     * @brief Recreate the SwapChain when the window surface changes.
     * 
     */
    void RecreateSwapChain();

    /**
     * @brief Cleanup the Vulkan objects related to the swapchain.
     * 
     */
    void CleanupSwapChain();

    /**
     * @brief Create and initialize a list of VkImageView objects (according to the number of VkImages in the
     * swap chain).
     * 
     */
    void CreateImageViews();

    /**
     * @brief Create and initialize a VkRenderPass object 
     * 
     */
    void CreateRenderPass();

    /**
     * @brief Create and initialize a VkPipeline object.
     * 
     */
    void CreateGraphicsPipeline();

    /**
     * @brief Create and initialize a list of VkFramebuffer objects (according to the number of VkImageView in the
     * swap chain).
     * 
     */
    void CreateFramebuffers();

    /**
     * @brief Create a VkCommandPool object.
     * 
     */
    void CreateCommandPool();

    /**
     * @brief Create VkCommandBuffer objects according to the number of frames in flight.
     * 
     */
    void CreateCommandBuffers();

    /**
     * @brief Function that writes the commands into a command buffer.
     * 
     * @param commandBuffer 
     * @param imageIndex 
     */
    void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    /**
     * @brief Create and initialize Sync Objects object (semaphores and fences).
     * 
     */
    void CreateSyncObjects();

    /**
     * @brief Present a frame onto the screen.
     * 
     */
    void DrawFrame();

    /**
     * @brief Create and initialize a VkShaderModule object from a bytecode buffer.
     * 
     * @param code Bytecode buffer.
     * @return VkShaderModule 
     */
    VkShaderModule CreateShaderModule(const std::vector<char>& code);

    /**
     * @brief Find and initialize a Vulkan Physical Device object.
     * 
     */
    void PickPhysicalDevice();

    /**
     * @brief Evaluate if the given device complies with the features of our application.
     * 
     * @param device Vulkan Physical Device.
     * @return True if physical device is suitable for our application.
     * @return False, otherwise.
     */
    bool IsDeviceSuitable(VkPhysicalDevice device);

    /**
     * @brief Find and initialize the family indices for the queues our application needs.
     * 
     * @param device Vulkan Physical Device.
     * @return QueueFamilyIndices.
     */
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

    /**
     * @brief Create and initialize a (logical) VkDevice object
     * 
     */
    void CreateLogicalDevice();

    /**
     * @brief Debug callback triggered by the validation layer.
     * 
     * VKAPI_ATTR and VKAPI_CALL ensure that the function has the right signature for Vulkan to call it.
     * 
     * @param messageSeverity See enum: VkDebugUtilsMessageSeverityFlagBitsEXT.
     * @param messageType See enum: VkDebugUtilsMessageTypeFlagBitsEXT.
     * @param pCallbackData pointer to VkDebugUtilsMessengerCallbackDataEXT.
     * @param pUserData pointer to user data.
     * @return True to abort the call that triggered the validation layer message.
     * @return False, to not abort.
     */
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                        void* pUserData);

    /**
     * @brief Utility function to read a SPIR-V bytecode file.
     * 
     * @param filename Path to file name.
     * @return std::vector<char> Bytecode.
     */
    std::vector<char> ReadFile(const std::string& filename);

    /**
     * @brief Application running loop.
     * 
     */
    void MainLoop();

    /**
     * @brief Cleanup procedure.
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

    mWindow = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    glfwSetWindowUserPointer(mWindow, this);
    glfwSetFramebufferSizeCallback(mWindow, FramebufferResizeCallback);
}

void HelloTriangleApp::FramebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    auto app = reinterpret_cast<HelloTriangleApp*>(glfwGetWindowUserPointer(window));
    app->mFramebufferResized = true;
}

void HelloTriangleApp::InitVulkan()
{
    CreateInstance();
    SetupDebugMessenger();
    CreateSurface();
    PickPhysicalDevice();
    CreateLogicalDevice();
    CreateSwapChain();
    CreateImageViews();
    CreateRenderPass();
    CreateGraphicsPipeline();
    CreateFramebuffers();
    CreateCommandPool();
    CreateCommandBuffers();
    CreateSyncObjects();
}

void HelloTriangleApp::CreateInstance()
{
    // Check if Validation Layer is supported before creating the Instance.
    if (kEnableValidationLayers && !CheckValidationLayerSupport())
    {
        throw std::runtime_error("Validation layers requested, but not available!");
    }

    // VkApplicationInfo struct:
    // Technically optional but provide useful information to the driver.
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    // VkInstanceCreateInfo struct:
    // Required by the driver to specify which global extensions and validation layers we want to use.
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    // Vulkan is a platform agnostic API, which means we need an extension to interface with the window system.
    // GLFW has a built-in function that returns the extension(s) it needs to do that.
    auto extensions = GetRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    // Since the vkCreateDebugUtilsMessengerEXT call requires a valid instance to have been created and
    // vkDestroyDebugUtilsMessengerEXT must be called before the instance is destroyed.
    // This currently leaves us unable to debug any issues in the vkCreateInstance and vkDestroyInstance calls.
    // By creating an additional debug messenger this way it will automatically be used
    // during vkCreateInstance and vkDestroyInstance and cleaned up after that.
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

    // The last two members of the struct determine the global validation layers to enable.
    if (kEnableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(kValidationLayers.size());
        createInfo.ppEnabledLayerNames = kValidationLayers.data();

        PopulateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    }
    else
    {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    // To retrieve a list of supported extensions before creating an instance use vkEnumerateInstanceExtensionProperties.
    uint32_t instanceExtensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, nullptr);
    std::vector<VkExtensionProperties> instanceExtensions(instanceExtensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, instanceExtensions.data());
    std::cout << "Available Extensions:\n";
    for (const auto& ext : instanceExtensions)
        std::cout << "    " << ext.extensionName << '\n';
    std::cout << '\n';

    // Issue the Vulkan create instance call and check for errors:
    if (vkCreateInstance(&createInfo, nullptr, &mInstance) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create instance!");
    }
}

void HelloTriangleApp::SetupDebugMessenger()
{
    if (!kEnableValidationLayers) return;

    // VkDebugUtilsMessengerCreateInfoEXT struct:
    // Required by the debug messenger extension to specify the details about the messenger and its callback.
    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    PopulateDebugMessengerCreateInfo(createInfo);

    // Look for the vkCreateDebugUtilsMessengerEXT function address and try to create the Debug Messenger object.
    if (CreateDebugUtilsMessengerEXT(mInstance, &createInfo, nullptr, &mDebugMessenger) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to set up debug messenger!");
    }
}

void HelloTriangleApp::CreateSurface()
{
    // Although the VkSurfaceKHR object and its usage is platform agnostic, its creation isn’t
    // because it depends on window system details.
    // Because a window surface is a Vulkan object, it comes with a VkWin32SurfaceCreateInfoKHR struct
    // that needs to be filled in.
    // After that the surface can be created with vkCreateWin32SurfaceKHR.
    // Technically this is a WSI (Window System Integration) extension function, but it is so commonly
    // used that the standard Vulkan loader includes it, so unlike other extensions you don’t need to
    // explicitly load it.
    // The glfwCreateWindowSurface function performs exactly this operation with a different implementation
    // for each platform. It takes simple parameters instead of a struct which makes the implementation
    // of the function very straightforward:
    if (glfwCreateWindowSurface(mInstance, mWindow, nullptr, &mSurface) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create window surface!");
    }
}

void HelloTriangleApp::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =    VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType =    VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = DebugCallback;
    createInfo.pUserData = nullptr; // Optional
}

bool HelloTriangleApp::CheckValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    std::cout << "Available Instance Layers:\n";
    for (const auto& layerProperties : availableLayers)
        std::cout << "    " << layerProperties.layerName << '\n';
    std::cout << '\n';

    bool layerFound = false;
    for (const char* layerName : kValidationLayers)
    {
        for (const auto& layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }
    }

    return layerFound;
}

std::vector<const char*> HelloTriangleApp::GetRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (kEnableValidationLayers)
    {
        // Adding the "VK_EXT_debug_utils" extension required by the debug messenger
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

bool HelloTriangleApp::CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(kDeviceExtensions.begin(), kDeviceExtensions.end());
    for (const auto& extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

SwapChainSupportDetails HelloTriangleApp::QuerySwapChainSupport(VkPhysicalDevice device)
{
        SwapChainSupportDetails details;

        // Fill the Surface Capabilities
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, mSurface, &details.mCapabilities);

        // Fill the Surface Formats
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, mSurface, &formatCount, nullptr);
        if (formatCount != 0)
        {
            details.mFormats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, mSurface, &formatCount, details.mFormats.data());
        }

        // Fill the Surface Presentation modes
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, mSurface, &presentModeCount, nullptr);
        if (presentModeCount != 0)
        {
            details.mPresentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, mSurface, &presentModeCount, details.mPresentModes.data());
        }

        return details;
}

VkSurfaceFormatKHR HelloTriangleApp::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR HelloTriangleApp::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D HelloTriangleApp::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize(mWindow, &width, &height); // Get window size in pixels instead of screen coordinates.

        VkExtent2D actualExtent =
        {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        // Clamp the Extent to the max/min values supported by the Surface Capabilities.
        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

void HelloTriangleApp::CreateSwapChain()
{
    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(mPhysicalDevice);

    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.mFormats);
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.mPresentModes);
    VkExtent2D extent = ChooseSwapExtent(swapChainSupport.mCapabilities);

    // Specify how many images are required in the swap chain.
    // The implementation specifies the minimum number that it requires to function:
    // However, simply sticking to this minimum means that our app may sometimes have to wait on the
    // driver to complete internal operations before we can acquire another image to render to.
    // Therefore it is recommended to request at least one more image than the minimum:
    uint32_t imageCount = swapChainSupport.mCapabilities.minImageCount + 1;
    // Also make sure to not exceed the maximum number of images while doing this
    // where 0 is a special value that means that there is no maximum:
    if (swapChainSupport.mCapabilities.maxImageCount > 0 && imageCount > swapChainSupport.mCapabilities.maxImageCount)
    {
        imageCount = swapChainSupport.mCapabilities.maxImageCount;
    }

    // VkSwapchainCreateInfoKHR struct:
    // Information required by the driver to create the Swap Chain.
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = mSurface;
    // After specifying which surface the swap chain should be tied to, the details of the swap chain
    // images are specified:
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;

    // The imageArrayLayers specifies the amount of layers each image consists of.
    // This is always 1 unless developing a stereoscopic 3D application.
    createInfo.imageArrayLayers = 1;

    // The imageUsage bit field specifies what kind of operations we’ll use the images in the swap chain for.
    // It is also possible that you’ll render images to a separate image first to perform operations
    // like post-processing. In that case you may use a value like VK_IMAGE_USAGE_TRANSFER_DST_BIT
    // instead and use a memory operation to transfer the rendered image to a swap chain image.
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // Specify how to handle swap chain images that will be used across multiple queue families.
    // That will be the case in our application if the graphics queue family is different from the presentation queue.
    // There are two ways to handle images that are accessed from multiple queues:
    // VK_SHARING_MODE_EXCLUSIVE:
    // VK_SHARING_MODE_CONCURRENT:
    // Concurrent mode requires you to specify in advance between which queue families ownership will
    // be shared using the queueFamilyIndexCount and pQueueFamilyIndices parameters.
    // If the graphics queue family and presentation queue family are the same, which will be the case
    // on most hardware, then we should stick to exclusive mode, because concurrent mode requires you
    // to specify at least two distinct queue families.
    QueueFamilyIndices indices = FindQueueFamilies(mPhysicalDevice);
    uint32_t queueFamilyIndices[] = {indices.mGraphicsFamily.value(), indices.mPresentFamily.value()};

    if (indices.mGraphicsFamily != indices.mPresentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    // Specify that a certain transform should be applied to images in the swap chain (if supported)
    // like a 90 degree clockwise rotation or horizontal flip.
    createInfo.preTransform = swapChainSupport.mCapabilities.currentTransform;
    // The compositeAlpha field specifies if the alpha channel should be used for blending with other
    // windows in the window system. You’ll almost always want to simply ignore the alpha channel,
    // hence compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR.
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    // Set presentation mode
    createInfo.presentMode = presentMode;
    // If the clipped member is set to VK_TRUE then that means that we don’t care about the color of
    // pixels that are obscured, for example because another window is in front of them.
    createInfo.clipped = VK_TRUE;
    // In Vulkan it’s possible that your swap chain becomes invalid or unoptimized while your application
    //  is running, for example because the window was resized. In that case the swap chain actually
    // needs to be recreated from scratch and a reference to the old one must be specified in this field.
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    // Issue the Vulkan create swap chain call and check for errors:
    if (vkCreateSwapchainKHR(mDevice, &createInfo, nullptr, &mSwapChain) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create swap chain!");
    }

    // Now that the Swap Chain has been created, retrieve the handles of the images created by the implementation.
    vkGetSwapchainImagesKHR(mDevice, mSwapChain, &imageCount, nullptr);
    mSwapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(mDevice, mSwapChain, &imageCount, mSwapChainImages.data());

    // Store the format and extent of the swap chain images for later use.
    mSwapChainImageFormat = surfaceFormat.format;
    mSwapChainExtent = extent;
}

void HelloTriangleApp::RecreateSwapChain()
{
    // If the framebuffer's width and height is 0 (due to minimization) then sleep until an event occurs.
    int width = 0, height = 0;
    glfwGetFramebufferSize(mWindow, &width, &height);
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(mWindow, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(mDevice);

    CleanupSwapChain();
    CreateSwapChain();
    CreateImageViews();
    CreateFramebuffers();
}

void HelloTriangleApp::CleanupSwapChain()
{
    for (auto framebuffer : mSwapChainFramebuffers)
    {
        vkDestroyFramebuffer(mDevice, framebuffer, nullptr);
    }

    for (auto imageView : mSwapChainImageViews)
    {
        vkDestroyImageView(mDevice, imageView, nullptr);
    }

    vkDestroySwapchainKHR(mDevice, mSwapChain, nullptr);
}

void HelloTriangleApp::CreateImageViews()
{
    mSwapChainImageViews.resize(mSwapChainImages.size());

    for (size_t i = 0; i < mSwapChainImages.size(); i++)
    {
        // VkImageViewCreateInfo struct:
        // Information required by the driver to create an image view.
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = mSwapChainImages[i];
        // The viewType and format fields specify how the image data should be interpreted.
        // The viewType parameter allows you to treat images as 1D, 2D, 3D textures and cube maps.
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = mSwapChainImageFormat;
        // The components field allows you to swizzle the color channels around.
        // For example, you can map all of the channels to the red channel for a monochrome texture.
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        // The subresourceRange field describes what the image's purpose is and which part of the image should be accessed.
        // Our images will be used as color targets without any mipmapping levels or multiple layers.
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;
        // Issue the Vulkan create image view call and check for errors:
        if (vkCreateImageView(mDevice, &createInfo, nullptr, &mSwapChainImageViews[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create image views!");
        }
    }
}

void HelloTriangleApp::CreateRenderPass()
{
    // This function specifies how many color and depth buffers will be used, how many samples to use
    // for each of them and how their contents should be handled throughout the rendering operations.
    // All of this information is wrapped in a render pass object.
    
    // *** Attachment Description ***
    // In this case we have just a single color buffer attachment represented by one of the images
    // from the swap chain.
    // The format of the color attachment should match the format of the swap chain images and
    // we're not doing anything with multisampling yet, so we'll stick to 1 sample.
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = mSwapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

    // The loadOp and storeOp determine what to do with the data in the attachment before rendering
    // and after rendering.
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    // The loadOp and storeOp apply to color and depth data, and stencilLoadOp / stencilStoreOp apply to
    // stencil data. Our application won’t do anything with the stencil buffer, so the results of loading
    // and storing are irrelevant.
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    // Textures and framebuffers in Vulkan are represented by VkImage objects with a certain pixel format,
    // however the layout of the pixels in memory can change based on what you’re trying to do with an image.
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // *** Subpasses and Attachment References ***
    // A single render pass can consist of multiple subpasses. Subpasses are subsequent rendering operations
    // that depend on the contents of framebuffers in previous passes, for example a sequence of
    // post-processing effects that are applied one after another. If these rendering operations are
    // grouped into one render pass, then Vulkan is able to reorder the operations for possible better performance.

    // Every subpass references one or more of the attachments described.
    VkAttachmentReference colorAttachmentRef{};
    // The attachment parameter specifies which attachment to reference by its index in the attachment
    // descriptions array. Our array consists of a single VkAttachmentDescription, so its index is 0.
    colorAttachmentRef.attachment = 0;
    // The layout specifies which layout we would like the attachment to have during a subpass that uses this reference.
    // Vulkan will automatically transition the attachment to this layout when the subpass is started.
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // The subpass is described using a VkSubpassDescription structure:
    VkSubpassDescription subpass{};
    // Vulkan may also support compute subpasses in the future, so we have to be explicit about this
    // being a graphics subpass.
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    // Next, specify the reference to the color attachment:
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    // The index of the attachment in this array is directly referenced from the fragment shader with the
    // layout(location = 0) out vec4 outColor directive!

    // Subpass dependencies are specified in VkSubpassDependency structs:
    VkSubpassDependency dependency{};
    // VK_SUBPASS_EXTERNAL refers to the implicit subpass before or after the render pass depending
    // on whether it is specified in srcSubpass or dstSubpass.
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    // Index 0 refers to our subpass, which is the first and only one.
    dependency.dstSubpass = 0;
    // The next two fields specify the operations to wait on and the stages in which these operations occur.
    // We need to wait for the swap chain to finish reading from the image before we can access it.
    // This can be accomplished by waiting on the color attachment output stage itself.
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    // The operations that should wait on this are in the color attachment stage and involve the writing of the color attachment.
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    // *** Render Pass ***
    // Now that the attachment and a basic subpass referencing it have been described, the render pass can be created.
    // VkRenderPassCreateInfo struct:
    // The render pass struct contains the information of the reference attachments and subpasses.
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    // Issue the Vulkan create render pass call and check for errors:
    if (vkCreateRenderPass(mDevice, &renderPassInfo, nullptr, &mRenderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create render pass!");
    }
}

void HelloTriangleApp::CreateGraphicsPipeline()
{
    auto vertShaderCode = ReadFile("shaders/vert.spv");
    auto fragShaderCode = ReadFile("shaders/frag.spv");

    VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);

    // To actually use the shaders , those need to be assigned to a specific pipeline stage through
    // VkPipelineShaderStageCreateInfo structures as part of the actual pipeline creation process.
    
    // VkPipelineShaderStageCreateInfo struct:
    // Information required by the driver to create a pipeline (shader) stage.
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main"; // Entry Point

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    // There is another (optional) member, pSpecializationInfo, which is not used here, but is worth
    // discussing. It allows you to specify values for shader constants. You can use a single shader
    // module where its behavior can be configured at pipeline creation by specifying different values
    // for the constants used in it. 
    // This is more efficient than configuring the shader using variables at render time, because the
    // compiler can do optimizations like eliminating if statements that depend on these values.
    // If you don’t have any constants like that, then you can set the member to nullptr, which our
    // struct initialization does automatically.

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    // Fixed function operations:
    // *** Dynamic State ***
    // While most of the pipeline state needs to be baked into the pipeline state, a limited amount 
    // of the state can actually be changed without recreating the pipeline at draw time.
    // Examples are the size of the viewport, line width and blend constants.
    // If desired, keep these properties out, then provide a VkPipelineDynamicStateCreateInfo structure.
    // However, this is not used in this application.

    // *** Vertex Input ***
    // VkPipelineVertexInputStateCreateInfo struct:
    // Describe the format of the vertex data that will be passed to the vertex shader. It describes this in roughly two ways:
    // 1. Bindings: Spacing between data and whether the data is per-vertex or per-instance.
    // 2. Attributes: Type of the attributes passed to the vertex shader, which binding to load them
    // from and at which offset.
    // (Hardcoding these for now.)
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional
    // The pVertexBindingDescriptions and pVertexAttributeDescriptions members point to an array
    // of structs that describe the aforementioned details for loading vertex data.

    // *** Input Assembly ***
    // VkPipelineInputAssemblyStateCreateInfo struct:
    // Describe two things: what kind of geometry will be drawn from the vertices and if primitive restart should be enabled.
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // *** Viewports & Scissors ***
    // A viewport basically describes the region of the framebuffer that the output will be rendered to.
    // This will almost always be (0, 0) to (width, height).

    // While viewports define the transformation from the image to the framebuffer, scissor rectangles define in
    // which regions pixels will actually be stored, if we wanted to draw to the entire framebuffer,
    // we would specify a scissor rectangle that covers it entirely.

    // Viewport(s) and scissor rectangle(s) can either be specified as a static part of the pipeline or as a dynamic
    // state set in the command buffer.

    // When opting for dynamic viewport(s) and scissor rectangle(s) you need to enable the respective dynamic
    // states for the pipeline.
    // Without dynamic state, the viewport and scissor rectangle need to be set in the pipeline using the
    // VkPipelineViewportStateCreateInfo struct. This makes the viewport and scissor rectangle for this
    // pipeline immutable. Any changes required to these values would require a new pipeline to be created
    // with the new values.
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    // *** Rasterizer ***
    // The rasterizer takes the geometry that is shaped by the vertices from the vertex shader and turns it into
    // fragments to be colored by the fragment shader. It also performs depth testing, face culling and the scissor
    // test, and it can be configured to output fragments that fill entire polygons or just the edges
    // All this is configured using the VkPipelineRasterizationStateCreateInfo structure.
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        // depthClampEnable == VK_TRUE means:
        // The fragments that are beyond the near and far planes are clamped to them as opposed to discarding them.
        rasterizer.depthClampEnable = VK_FALSE;
        // rasterizerDiscardEnable == VK_TRUE means:
        // Geometry never passes through the rasterizer stage. This basically disables any output to the framebuffer.
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        // polygonMode determines how fragments are generated for geometry.
        // Using any mode other than fill requires enabling a GPU feature.
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        // The lineWidth member describes the thickness of lines in terms of number of fragments.
        rasterizer.lineWidth = 1.0f;
        // cullMode member determines the type of face culling to use.
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        // frontFace member specifies the vertex order for faces to be considered front-facing
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        // The rasterizer can alter the depth values by adding a constant value or biasing them based
        // on a fragment’s slope. This is sometimes used for shadow mapping.
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f; // Optional
        rasterizer.depthBiasClamp = 0.0f; // Optional
        rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

    // *** Multisampling ***
    // The VkPipelineMultisampleStateCreateInfo struct configures multisampling, which is one of the
    // ways to perform anti-aliasing. It works by combining the fragment shader results of multiple
    // polygons that rasterize to the same pixel.
    // (Disabled for now.)
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    // *** Depth & Stencil Tests ***
    // The VkPipelineDepthStencilStateCreateInfo struct configures the depth and stencil tests.
    // Viewport(s) and scissor rectangle(s) can either be specified as a static part of the pipeline or as a dynamic
    // state set in the command buffer. While the former is more in line with the other states it’s often convenient
    // to make viewport and scissor state dynamic as it gives you a lot more flexibility.

    // Without dynamic state, the viewport and scissor rectangle need to be set in the pipeline using the
    // VkPipelineViewportStateCreateInfo struct.
    // When opting for dynamic viewport(s) and scissor rectangle(s) you need to enable the respective dynamic
    // states for the pipeline:
    std::vector<VkDynamicState> dynamicStates =
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    // *** Color Blending ***
    // After a fragment shader has returned a color, it needs to be combined with the color that is
    // already in the framebuffer. This transformation is known as color blending and there are two ways to do it:
    // 1. Mix the old and new value to produce a final color.
    // 2. Combine the old and new value using a bitwise operation.
    // There are two types of structs to configure color blending:
    // 1. VkPipelineColorBlendAttachmentState contains the configuration per attached framebuffer
    // 2. VkPipelineColorBlendStateCreateInfo contains the global color blending settings.
    // In this case, there's only one framebuffer:
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

    // The second structure references the array of structures for all of the framebuffers and allows
    // you to set blend constants that you can use as blend factors.
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    // *** Pipeline Layout ***
    // You can use uniform values in shaders, which are globals similar to dynamic state variables that
    // can be changed at drawing time to alter the behavior of your shaders without having to recreate them.
    // These uniform values need to be specified during pipeline creation by creating a VkPipelineLayout object.
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pushConstantRangeCount = 0;

    // Issue the Vulkan create pipeline layout call and check for errors:
    if (vkCreatePipelineLayout(mDevice, &pipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create pipeline layout!");
    }

    // VkGraphicsPipelineCreateInfo struct:
    // Contains:
    // 1. Shader Stages.
    // 2. Fixed-Function State.
    // 3. Pipeline Layout.
    // 4. Render Pass.
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    // Start by referencing the array VkPipelineShaderStageCreateInfo
    pipelineInfo.pStages = shaderStages;
    // Then reference all of the structures describing the fixed-function stage.
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    // Then comes the pipeline layout, which is a Vulkan handle rather than a struct pointer.
    pipelineInfo.layout = mPipelineLayout;
    // Finally the render pass and the index of the sub pass where this graphics pipeline will be used.
    pipelineInfo.renderPass = mRenderPass;
    pipelineInfo.subpass = 0;
    // Actually two more parameters basePipelineHandle and basePipelineIndex.
    // Vulkan allows you to create a new graphics pipeline by deriving from an existing pipeline.
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional

    // Issue the Vulkan create graphics pipelines call and check for errors:
    if (vkCreateGraphicsPipelines(mDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mGraphicsPipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(mDevice, fragShaderModule, nullptr);
    vkDestroyShaderModule(mDevice, vertShaderModule, nullptr);
}

void HelloTriangleApp::CreateFramebuffers()
{
    // Allocate Framebuffer objects equal to the number of VkImageViews from the swap chain.
    mSwapChainFramebuffers.resize(mSwapChainImageViews.size());

    // Iterate through the image views and create framebuffers from them:
    for (size_t i = 0; i < mSwapChainImageViews.size(); i++)
    {
        VkImageView attachments[] = { mSwapChainImageViews[i] };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        // Specify with which mRenderPass the framebuffer needs to be compatible.
        // Basically means that they use the same number and type of attachments.
        framebufferInfo.renderPass = mRenderPass;
        // The attachmentCount and pAttachments parameters specify the VkImageView objects that should be bound
        // to the respective attachment descriptions in the render pass pAttachment array.
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = mSwapChainExtent.width;
        framebufferInfo.height = mSwapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(mDevice, &framebufferInfo, nullptr, &mSwapChainFramebuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void HelloTriangleApp::CreateCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(mPhysicalDevice);

    // Command pool creation only takes two parameters:
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    // There are two possible flags for command pools:
    // 1. VK_COMMAND_POOL_CREATE_TRANSIENT_BIT: Hint that command buffers are rerecorded with new commands
    // very often (may change memory allocation behavior).
    // 2. VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT: Allow command buffers to be rerecorded individually,
    // without this flag they all have to be reset together.
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.mGraphicsFamily.value();

    // Each command pool can only allocate command buffers that are submitted on a single type of queue.
    // We’re going to record commands for drawing, which is why we’ve chosen the graphics queue family.

    // Issue the Vulkan create command pool call and check for errors:
    if (vkCreateCommandPool(mDevice, &poolInfo, nullptr, &mCommandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create command pool!");
    }
}

void HelloTriangleApp::CreateCommandBuffers()
{
    mCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    // VkCommandBufferAllocateInfo struct:
    // Specifies the command pool and number of buffers to allocate.
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = mCommandPool;
    // The level parameter specifies if the allocated command buffers are primary or secondary command buffers.
    // 1. VK_COMMAND_BUFFER_LEVEL_PRIMARY: Can be submitted to a queue for execution, but cannot be called
    // from other command buffers.
    // 2. VK_COMMAND_BUFFER_LEVEL_SECONDARY: Cannot be submitted directly, but can be called from primary
    // command buffers.
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(mCommandBuffers.size());

    // Issue the Vulkan allocate command buffers call and check for errors:
    if (vkAllocateCommandBuffers(mDevice, &allocInfo, mCommandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate command buffers!");
    }
}

void HelloTriangleApp::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    // Always begin recording a command buffer by calling vkBeginCommandBuffer with a small
    // VkCommandBufferBeginInfo structure as argument that specifies some details about the usage of
    // this specific command buffer.
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    // The flags parameter specifies how we’re going to use the command buffer.
    beginInfo.flags = 0; // Optional
    // The pInheritanceInfo parameter is only relevant for secondary command buffers.
    // It specifies which state to inherit from the calling primary command buffers.
    beginInfo.pInheritanceInfo = nullptr; // Optional

    // If the command buffer was already recorded once, then a call to vkBeginCommandBuffer will
    // implicitly reset it. It’s not possible to append commands to a buffer at a later time.
    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }

    // *** Starting a Render Pass ***
    // Drawing starts by beginning the render pass with vkCmdBeginRenderPass.
    // The render pass is configured using some parameters in a VkRenderPassBeginInfo struct:
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = mRenderPass;
    // We created a framebuffer for each swap chain image where it is specified as a color attachment.
    // Thus we need to bind the framebuffer for the swapchain image we want to draw to, using the imageIndex.
    renderPassInfo.framebuffer = mSwapChainFramebuffers[imageIndex];
    // The next two parameters define the size of the render area. 
    // The pixels outside this region will have undefined values.
    // It should match the size of the attachments for best performance.
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = mSwapChainExtent;
    // The last two parameters define the clear values to use for VK_ATTACHMENT_LOAD_OP_CLEAR, which
    // we used as load operation for the color attachment.
    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}}; // Black with 100% opacity.
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    // The render pass can now begin. 
    // All of the functions that record commands can be recognized by their vkCmd prefix.
    // They all return void, so there will be no error handling until we’ve finished recording.
    // The first parameter for every command is always the command buffer to record the command to.
    // The second parameter specifies the details of the render pass we’ve just provided.
    // The final parameter controls how the drawing commands within the render pass will be provided.
    // It can have one of two values:
    // 1. VK_SUBPASS_CONTENTS_INLINE: The render pass commands will be embedded in the primary command
    // buffer itself and no secondary command buffers will be executed.
    // 2. VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS: The render pass commands will be executed
    // from secondary command buffers.
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        // Bind the graphics pipeline:
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline);
        // As noted in the fixed functions, we did specify viewport and scissor state for this pipeline
        // to be dynamic. So we need to set them in the command buffer before issuing our draw command:
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)mSwapChainExtent.width;
        viewport.height = (float)mSwapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = mSwapChainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
        // Now issue the draw command for the triangle:
        vkCmdDraw(commandBuffer,
                    3,  // vertexCount: 3 vertices to draw.
                    1,  // instanceCount: Used for instanced rendering, use 1 if not using instance rendering.
                    0,  // firstVertex: Used as an offset into the vertex buffer, defines the lowest value of gl_VertexIndex.
                    0); // firstInstance: Used as an offset for instanced rendering, defines the lowest value of gl_InstanceIndex.

    // The render pass can now end.
    vkCmdEndRenderPass(commandBuffer);

    // Finish recording the command and check for errors:
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to record command buffer!");
    }
}

void HelloTriangleApp::CreateSyncObjects()
{
    mImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    mRenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    mInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    // Creating semaphores requires filling in the VkSemaphoreCreateInfo, but in the current version
    // of the API it doesn't actually have any required fields besides sType:
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    // Creating a fence requires filling in the VkFenceCreateInfo:
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Start the fence in a signaled state.

    for (size_t i = 0; i <MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &mImageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &mRenderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(mDevice, &fenceInfo, nullptr, &mInFlightFences[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create synchronization objects for a frame!");
        }
    }
}

void HelloTriangleApp::DrawFrame()
{
    // Rendering a frame in Vulkan consists of common set of steps:
    // 1. Wait for the previous frame to finish.
    // 2. Acquire an image from the swap chain.
    // 3. Record a command buffer which draws the scene onto that image.
    // 4. Submit the recorded command buffer.
    // 5. Present the swap chain image.

    // There are a number of events that need explicit order, because they happen on the GPU, such as:
    // - Acquire an image from the swap chain.
    // - Execute commands that draw onto the acquired image.
    // - Present that image to the screen for presentation, returning it to the swapchain.

    // Each of these events is set in motion using a single function call, but are all executed asynchronously.
    // The function calls will return before the operations are actually finished and the order of execution
    // is also undefined. That is unfortunate, because each of the operations depends on the previous
    // one finishing. Thus synchronization primitives are needed to achieve the desired ordering.

    // *** Semaphores ***
    // Semaphore are used to both order work inside the same queue and between different queues.
    // There are 2 types of semaphores in Vulkan, binary and timeline. But timeline semaphores are not used in this application.
    // A semaphore is either unsignaled or signaled. It begins life as unsignaled.

    // *** Fences ***
    // A fence has a similar purpose, in that it is used to synchronize execution, but it is for ordering
    // the execution on the CPU, otherwise known as the host. Simply put, if the host needs to know
    // when the GPU has finished something, we use a fence.
    // Whenever we submit work to execute, we can attach a fence to that work. When the work is finished,
    // the fence will be signaled. Then we can make the host wait for the fence to be signaled,
    // guaranteeing that the work has finished before the host continues.

    // In summary, semaphores are used to specify the execution order of operations on the GPU
    // while fences are used to keep the CPU and GPU in sync with each-other.

    // *** Waiting for the previous frame ***
    // At the start of the frame, we wait until the previous frame has finished, so that the command
    // buffer and semaphores are available to use.
    vkWaitForFences(mDevice,                            // Logical device.
                    1,                                  // Number of fences in the array.
                    &mInFlightFences[mCurrentFrame],    // Array of fences.
                    VK_TRUE,                            // VK_TRUE means wait for all the fences.
                    UINT64_MAX);                        // UINT64_MAX effectively disables the timeout.

    // *** Acquiring an image from the swap chain ***
    // The next thing we need to do is acquire an image from the swap chain.
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(mDevice,                                    // Logical device.
                                            mSwapChain,                                 // Swapchain.
                                            UINT64_MAX,                                 // Timeout in nanoseconds, (UINT64_MAX = disable the timeout).
                                            mImageAvailableSemaphores[mCurrentFrame],   // Semaphore to be signaled when the presentation engine is finished using the image.
                                            VK_NULL_HANDLE,                             // Fence to be signaled when the presentation engine is finished using the image.
                                            &imageIndex);                               // Index of the available VkImage in mSwapChainImages.

    // Recreate the Swap Chain if it is no longer adequate for presentation.
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        RecreateSwapChain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("Failed to acquire swap chain image!");
    }

    // (Only reset the fence if we are submitting work)
    // After waiting, we need to manually reset the fence to the unsignaled state with the vkResetFences call:
    vkResetFences(mDevice, 1, &mInFlightFences[mCurrentFrame]);

    // *** Recording the command buffer ***
    // Use imageIndex to record the command buffer.
    // Call vkResetCommandBuffer to make sure it is able to be recorded.
    vkResetCommandBuffer(mCommandBuffers[mCurrentFrame], 0);
    // Now record the commands.
    RecordCommandBuffer(mCommandBuffers[mCurrentFrame], imageIndex);
    /// *** Submitting the command buffer ***
    // Queue submission and synchronization is configured through the VkSubmitInfo structure.
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    VkSemaphore waitSemaphores[] = {mImageAvailableSemaphores[mCurrentFrame]};
    // Wait with writing colors to the image until it's available
    // Each entry in the waitStages array corresponds to the semaphore with the same index in pWaitSemaphores.
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    // The next two parameters specify which command buffers submit for execution.
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &mCommandBuffers[mCurrentFrame];
    // The signalSemaphoreCount and pSignalSemaphores parameters specify which semaphores to signal
    // once the command buffer(s) have finished execution.
    VkSemaphore signalSemaphores[] = {mRenderFinishedSemaphores[mCurrentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;
    // Now submit the command buffer to the graphics queue using vkQueueSubmit
    // The last parameter references an optional fence that will be signaled when the command buffers
    // finish execution.
    if (vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, mInFlightFences[mCurrentFrame]) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit draw command buffer!");
    }

    // *** Subpass dependencies ***
    // Subpass dependencies specify memory and execution dependencies between subpasses.
    // We have a single subpass right now, but the operations before and right after this subpass
    // also count as implicit "subpasses"
    // There are two built-in dependencies that take care of the transition at the start of the render pass
    // and at the end, but the former does not occur at the right time. It assumes that the transition
    // occurs at the start of the pipeline, but we haven't acquired the image yet at that point.
    // To ensure that the render passes don't begin until the image is available we can make the render
    // pass wait for the VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT stage.

    // *** Presentation ***
    // The last step of drawing a frame is submitting the result back to the swap chain to have it
    // eventually show up on the screen.
    // Presentation is configured through a VkPresentInfoKHR structure:
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    // The first two parameters specify which semaphores to wait on before presentation can happen.
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    VkSwapchainKHR swapChains[] = {mSwapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    result = vkQueuePresentKHR(mPresentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || mFramebufferResized)
    {
        mFramebufferResized = false;
        RecreateSwapChain();
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to present swap chain image!");
    }

    // Wrap-around counter
    mCurrentFrame = (mCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

VkShaderModule HelloTriangleApp::CreateShaderModule(const std::vector<char>& code)
{
    // VkShaderModuleCreateInfo struct:
    // Information required by the driver to create a shader module.
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    // The bytecode pointer is a uint32_t pointer rather than a char pointer.
    // Therefore we will need to cast the pointer with reinterpret_cast as shown below.
    // When you perform a cast like this, you also need to ensure that the data satisfies the alignment
    // requirements of uint32_t.
    // Lucky, the data is stored in an std::vector where the default allocator already ensures
    // that the data satisfies the worst case alignment requirements.
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(mDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create shader module!");
    }

    return shaderModule;
}

void HelloTriangleApp::PickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr);

    if (deviceCount == 0)
    {
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(mInstance, &deviceCount, devices.data());

    for (const auto& device : devices)
    {
        if (IsDeviceSuitable(device))
        {
            mPhysicalDevice = device;
            break;
        }
    }

    if (mPhysicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Failed to find a suitable GPU!");
    }
}

bool HelloTriangleApp::IsDeviceSuitable(VkPhysicalDevice device)
{
    QueueFamilyIndices indices = FindQueueFamilies(device);
    bool extensionsSupported = CheckDeviceExtensionSupport(device);

    bool isSwapChainAdequate = false;
    if (extensionsSupported)
    {
        SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
        isSwapChainAdequate = !swapChainSupport.mFormats.empty() && !swapChainSupport.mPresentModes.empty();
    }
    return  indices.IsComplete() &&
            extensionsSupported &&
            isSwapChainAdequate;
}

QueueFamilyIndices HelloTriangleApp::FindQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies)
    {
        // Check if queue family has Graphics capabilities.
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.mGraphicsFamily = i;
        }
        // Check if queue family that has the capability of presenting to our window surface.
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, mSurface, &presentSupport);
        if (presentSupport)
        {
            indices.mPresentFamily = i;
        }
        if (indices.IsComplete())
        {
            break;
        }
        i++;
    }

    return indices;
}

void HelloTriangleApp::CreateLogicalDevice()
{
    QueueFamilyIndices indices = FindQueueFamilies(mPhysicalDevice);
    std::set<uint32_t> uniqueQueueFamilies =
    {
        indices.mGraphicsFamily.value(),
        indices.mPresentFamily.value()
    };

    // VkDeviceQueueCreateInfo struct:
    // Specify the number of queues the application needs for a single queue family (usually only one).
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    // Assign priority to queue to influence the scheduling of command buffer execution using floating
    // point numbers between 0.0 and 1.0 (the higher value, the higher priority).
    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = indices.mGraphicsFamily.value();
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    // Specify the feature we need from the physical device.
    // Leaving as default for now.
    VkPhysicalDeviceFeatures deviceFeatures{};

    // VkDeviceCreateInfo struct:
    // Information required by the driver to create the logical device.
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());;
    createInfo.pEnabledFeatures = &deviceFeatures;

    // Enabling device extensions (swap_chain, etc.)
    createInfo.enabledExtensionCount = static_cast<uint32_t>(kDeviceExtensions.size());
    createInfo.ppEnabledExtensionNames = kDeviceExtensions.data();

    // Previous implementations of Vulkan made a distinction between instance and device specific
    // validation layers, but this is no longer the case. That means that the enabledLayerCount and
    // ppEnabledLayerNames fields of VkDeviceCreateInfo are ignored by up-to-date implementations.
    // However, it is still a good idea to set them anyway to be compatible with older implementations.
    if (kEnableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(kValidationLayers.size());
        createInfo.ppEnabledLayerNames = kValidationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    // Issue the Vulkan create logical device call and check for errors:
    if (vkCreateDevice(mPhysicalDevice, &createInfo, nullptr, &mDevice) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create logical device!");
    }

    // Retrieve queue handles for each queue family.
    vkGetDeviceQueue(mDevice, indices.mGraphicsFamily.value(), 0, &mGraphicsQueue);
    vkGetDeviceQueue(mDevice, indices.mPresentFamily.value(), 0, &mPresentQueue);
}

VKAPI_ATTR VkBool32 VKAPI_CALL HelloTriangleApp::DebugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                                VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                                const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                                void* pUserData)
{
    std::cerr << "Validation Layer: " << pCallbackData->pMessage << "\n" << std::endl;

    // The callback returns a boolean that indicates if the Vulkan call that triggered the validation
    // layer message should be aborted.If the callback returns true, then the call is aborted with the
    // VK_ERROR_VALIDATION_FAILED_EXT error. This is normally only used to test the validation layers
    // themselves, so you should always return VK_FALSE.
    return VK_FALSE;
}

std::vector<char> HelloTriangleApp::ReadFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file!");
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

void HelloTriangleApp::MainLoop()
{
    while (!glfwWindowShouldClose(mWindow))
    {
        glfwPollEvents();
        DrawFrame();
    }

    vkDeviceWaitIdle(mDevice);
}

void HelloTriangleApp::Cleanup()
{
    CleanupSwapChain();

    vkDestroyPipeline(mDevice, mGraphicsPipeline, nullptr);
    vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);
    vkDestroyRenderPass(mDevice, mRenderPass, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(mDevice, mRenderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(mDevice, mImageAvailableSemaphores[i], nullptr);
        vkDestroyFence(mDevice, mInFlightFences[i], nullptr);
    }

    vkDestroyCommandPool(mDevice, mCommandPool, nullptr);
    vkDestroyDevice(mDevice, nullptr);

    if (kEnableValidationLayers)
    {
        DestroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);
    }

    // Destroy Surface before Instance.
    vkDestroySurfaceKHR(mInstance, mSurface, nullptr);

    // VkInstance should be only destroyed right before the program exits.
    vkDestroyInstance(mInstance, nullptr);

    glfwDestroyWindow(mWindow);

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