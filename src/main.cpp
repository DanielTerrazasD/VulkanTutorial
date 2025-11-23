#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <optional>
#include <set>

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
    std::optional<uint32_t> mPresentationFamily;

    bool IsComplete()
    {
        return  mGraphicsFamily.has_value() &&
                mPresentationFamily.has_value();
    }
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
     * @brief Create and initialize a Logical Device object
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

void HelloTriangleApp::PickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr);

    if (deviceCount == 0)
    {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
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
    return indices.IsComplete();
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
            indices.mPresentationFamily = i;
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
        indices.mPresentationFamily.value()
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

    // Previous implementations of Vulkan made a distinction between instance and device specific
    // validation layers, but this is no longer the case. That means that the enabledLayerCount and
    // ppEnabledLayerNames fields of VkDeviceCreateInfo are ignored by up-to-date implementations.
    // However, it is still a good idea to set them anyway to be compatible with older implementations.
    // We won't be needing any device specific extensions for now.
    createInfo.enabledExtensionCount = 0;
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
    vkGetDeviceQueue(mDevice, indices.mPresentationFamily.value(), 0, &mPresentationQueue);
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