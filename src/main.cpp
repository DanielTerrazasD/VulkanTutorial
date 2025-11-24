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

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

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
    VkQueue mPresentationQueue;

    VkSurfaceKHR mSurface;

    VkSwapchainKHR mSwapChain;

    // The images are created by the implementation for the swap chain and they will be automatically
    // cleaned up once the swap chain has been destroyed.
    std::vector<VkImage> mSwapChainImages;

    VkFormat mSwapChainImageFormat;
    VkExtent2D mSwapChainExtent;

    std::vector<VkImageView> mSwapChainImageViews;

    /**
     * @brief GLFW Window Initialization.
     * 
     */
    void InitWindow();

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
     * @brief Create and initialize some VkImageView objects (according to the number of VkImages in the
     * swap chain).
     * 
     */
    void CreateImageViews();

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
    // Disabling resizing for now.
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    mWindow = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
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
    vkGetDeviceQueue(mDevice, indices.mPresentFamily.value(), 0, &mPresentationQueue);
}

VKAPI_ATTR VkBool32 VKAPI_CALL HelloTriangleApp::DebugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                                VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                                const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                                void* pUserData)
{
    std::cerr << "Validation Layer: " << pCallbackData->pMessage << std::endl;

    // The callback returns a boolean that indicates if the Vulkan call that triggered the validation
    // layer message should be aborted.If the callback returns true, then the call is aborted with the
    // VK_ERROR_VALIDATION_FAILED_EXT error. This is normally only used to test the validation layers
    // themselves, so you should always return VK_FALSE.
    return VK_FALSE;
}

void HelloTriangleApp::MainLoop()
{
    while (!glfwWindowShouldClose(mWindow))
    {
        glfwPollEvents();
    }
}

void HelloTriangleApp::Cleanup()
{
    for (auto imageView : mSwapChainImageViews)
    {
        vkDestroyImageView(mDevice, imageView, nullptr);
    }

    vkDestroySwapchainKHR(mDevice, mSwapChain, nullptr);
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