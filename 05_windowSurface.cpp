#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <optional>
#include <set>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif


//******************************************************************************************
// 
//  Name:           CreateDebugUtilsMessengerEXT
//  Arguments:      kInstance instance
//                  const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo
//					const VkAllocationCallbacks* pAllocator
//					VkDebugUtilsMessengerEXT* pDebugMessenger
//
//
//******************************************************************************************

VkResult CreateDebugUtilsMessengerEXT(	  VkInstance instance
                                        , const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo
                                        , const VkAllocationCallbacks* pAllocator
                                        , VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(  instance
									   , "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
	return func(  instance
		    , pCreateInfo
		    , pAllocator
		    , pDebugMessenger);
    } else {
	return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}


//******************************************************************************************
// 
//  Name:           DestroyDebugUtilsMessengerEXT
//  Arguments:      VkInstance instance
//				   , VkDebugUtilsMessengerEXT debugMessenger
//				   , const VkAllocationCallbacks* pAllocator
//  Description:
// 
//******************************************************************************************

void DestroyDebugUtilsMessengerEXT(  VkInstance instance
				   , VkDebugUtilsMessengerEXT debugMessenger
				   , const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(  instance
									    , "vkDestroyDebugUtilsMessengerEXT");
    if(func != nullptr) {
		func( instance
			, debugMessenger
			, pAllocator);
    }
}


//******************************************************************************************
// 
//  Name:           QueueFamilyIndices
//  Arguments:      graphicsFamily
//                  presentFamily
//  Description:
// 
//******************************************************************************************

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
	
	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};


//******************************************************************************************
// 
//  Name:           HelloTriangleApplication
//  Arguments:
//  Description:
// 
//******************************************************************************************

class HelloTriangleApplication
{
    public:


//******************************************************************************************
// 
//  Name:           run
//  Arguments:      N/A
//  Description:    Provides the next level down for the control flow of the application.
// 
//******************************************************************************************
    
        void run()
        {
            initWindow();
            initVulkan();
            mainLoop();
            cleanup();
        }

    private:
        GLFWwindow *window = nullptr;

	    VkInstance instance;
	    VkDebugUtilsMessengerEXT debugMessenger;
        VkSurfaceKHR surface;

		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		VkDevice device;

		VkQueue graphicsQueue;
        VkQueue presentQueue;


//******************************************************************************************
// 
//  Name:           initWindow
//  Arguments:      N/A
//  Description:    Calls glfw functions to initialize a window to be displayed 
//                  on the screen.
// 
//******************************************************************************************

        void initWindow()
        {
            glfwInit();

            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

            window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
        }


//******************************************************************************************
// 
//  Name:           initVulkan
//  Arguments:      N/A
//  Description:    Control structure for initializing the Vulkan framework.
// 
//******************************************************************************************

        void initVulkan()
        {
			createInstance();
			setupDebugMessenger();
            createSurface();
			pickPhysicalDevice();
			createLogicalDevice();
        }

//******************************************************************************************
// 
//  Name:           mainLoop
//  Arguments:      N/A
//  Description:    Checks events acted on the window (for now...).  Checks to see if 
//                  window is closed.
// 
//******************************************************************************************

        void mainLoop()
        {
            while (!glfwWindowShouldClose(window))
            {
                glfwPollEvents();
            }
        }


//******************************************************************************************
// 
//  Name:           cleanup
//  Arguments:      N/A
//  Description:    Garbage collection for destroying instances of Vulkan objects and
//                  GLFW objects. 
// 
//******************************************************************************************

        void cleanup()
        {
			vkDestroyDevice(  device
							, nullptr);

			if (enableValidationLayers) {
				DestroyDebugUtilsMessengerEXT(  instance
											  , debugMessenger
											  , nullptr);
			}		
			
            vkDestroySurfaceKHR(  instance
                                , surface
                                , nullptr);
			vkDestroyInstance(  instance
							  , nullptr);

            glfwDestroyWindow(window);

            glfwTerminate();
        }


//******************************************************************************************
// 
//  Name:           createInstance
//  Arguments:      N/A
//  Description:    Creates an instance of a Vulkan object. Provides initialization 
//                  parameters for the Vulkan object, especially appInfo and createInfo.
// 
//******************************************************************************************

        void createInstance() {
			if (enableValidationLayers && !checkValidationLayerSupport()) {
				throw std::runtime_error("Validation layers requested, but not available!");
			}

            VkApplicationInfo appInfo{};
            appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.pApplicationName = "Hello Triangle";
            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.pEngineName = "No Engine";
            appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.apiVersion = VK_API_VERSION_1_0;

            VkInstanceCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            createInfo.pApplicationInfo = &appInfo;
	    
			auto extensions = getRequiredExtensions();
			createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
			createInfo.ppEnabledExtensionNames = extensions.data();
			
			VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
			if (enableValidationLayers) {
				createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
				createInfo.ppEnabledLayerNames = validationLayers.data();
				
				populateDebugMessengerCreateInfo(debugCreateInfo);
				createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
			} else {
				createInfo.enabledLayerCount = 0;
				
				createInfo.pNext = nullptr;
			}
			
			if (vkCreateInstance( &createInfo
                                , nullptr
                                , &instance) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create instance!");
			}
		}


//******************************************************************************************
// 
//  Name:           populateDebugMessengerCreateInfo
//  Arguments:      VkDebugUtilsMessengerCreateInfoEXT& createInfo
//  Description:    Initializes debug messenger used in the validation layers.
// 
//******************************************************************************************
	
        void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
            createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            createInfo.messageSeverity =  VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT 
                                        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT 
                                        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            createInfo.messageType =  VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT 
                                    | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT 
                                    | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            createInfo.pfnUserCallback = debugCallback;
        }


//******************************************************************************************
// 
//  Name:           setupDebugMessenger
//  Arguments:      N/A
//  Description:    If validation layers are enabled, declares createInfo so that the
//                  debug messenger can be initialized for debugging. 
// 
//******************************************************************************************
        
        void setupDebugMessenger() {
            if (!enableValidationLayers) return;
            
            VkDebugUtilsMessengerCreateInfoEXT createInfo;
            populateDebugMessengerCreateInfo(createInfo);
            
            if (CreateDebugUtilsMessengerEXT(  instance
                                             , &createInfo
                                             , nullptr
                                             , &debugMessenger) != VK_SUCCESS) {
                throw std::runtime_error("Failed to set up debug messenger!");
            }
        }


//******************************************************************************************
// 
//  Name:           createSurface
//  Arguments:      N/A
//  Description:    Must be an OpenGL thing. An OpenGL window needs a surface to draw on.
//                  Or some such.
// 
//******************************************************************************************

        void createSurface() {
            if (glfwCreateWindowSurface(  instance
                                        , window
                                        , nullptr
                                        , &surface) != VK_SUCCESS) {
                                            throw std::runtime_error("Failed to create window surface!");
                                        }
        }


//******************************************************************************************
// 
//  Name:           pickPhysicalDevice
//  Arguments:      N/A
//  Description:    Checks for physical devices (graphics cards) which have Vulkan support.
// 
//******************************************************************************************

        void pickPhysicalDevice() {
            uint32_t deviceCount = 0;
            vkEnumeratePhysicalDevices(   instance
                                        , &deviceCount
                                        , nullptr);

            if (deviceCount == 0) {
                throw std::runtime_error("Failed to find GPUs with Vulkan support!");
            }

            std::vector<VkPhysicalDevice> devices(deviceCount);
            vkEnumeratePhysicalDevices(   instance
                                        , &deviceCount
                                        , devices.data());

            for (const auto& device : devices) {
                if (isDeviceSuitable(device)) {
                    physicalDevice = device;
                    break;
                }
            }

            if (physicalDevice == VK_NULL_HANDLE) {
                throw std::runtime_error("Failed to find a suitable GPU!");
            }
        }


//******************************************************************************************
// 
//  Name:           createLogicalDevice
//  Arguments:      N/A
//  Description:    
// 
//******************************************************************************************

        void createLogicalDevice() {
            QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

            std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
            std::set<uint32_t> uniqueQueueFamilies = {
                indices.graphicsFamily.value()
                , indices.presentFamily.value()
            };
            
            float queuePriority = 1.0f;
            for (uint32_t queueFamily : uniqueQueueFamilies) {
                VkDeviceQueueCreateInfo queueCreateInfo{};
                queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queueCreateInfo.queueFamilyIndex = queueFamily;
                queueCreateInfo.queueCount = 1;
                queueCreateInfo.pQueuePriorities = &queuePriority;
                queueCreateInfos.push_back(queueCreateInfo);
            }
            
            VkPhysicalDeviceFeatures deviceFeatures{};

            VkDeviceCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

            createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
            createInfo.pQueueCreateInfos = queueCreateInfos.data();

            createInfo.pEnabledFeatures = &deviceFeatures;

            createInfo.enabledExtensionCount = 0;

            if (enableValidationLayers) {
                createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
                createInfo.ppEnabledLayerNames = validationLayers.data();
            } else {
                createInfo.enabledLayerCount = 0;
            }

            if (vkCreateDevice(   physicalDevice
                                , &createInfo
                                , nullptr
                                , &device) != VK_SUCCESS) {
                                    throw std::runtime_error("Failed to create logical device!");
                                }
            
            vkGetDeviceQueue(  device
                            , indices.graphicsFamily.value()
                            , 0
                            , &graphicsQueue);
            
            vkGetDeviceQueue(  device
                            , indices.presentFamily.value()
                            , 0
                            , &presentQueue);
        }


//******************************************************************************************
// 
//  Name:
//  Arguments:
//  Description:
// 
//******************************************************************************************

        bool isDeviceSuitable(VkPhysicalDevice device) {
            QueueFamilyIndices indices = findQueueFamilies(device);

            return indices.isComplete();
        }

        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
            QueueFamilyIndices indices;

            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(  device
                                                    , &queueFamilyCount
                                                    , nullptr);

            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(  device
                                                     , &queueFamilyCount
                                                     , queueFamilies.data());

            int i = 0;
            for (const auto& queueFamily : queueFamilies) {
                if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    indices.graphicsFamily = i;
                }

                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(  device
                                                     , i
                                                     , surface
                                                     , &presentSupport);

                if (presentSupport) {
                    indices.presentFamily = i;
                }

                if (indices.isComplete()) {
                    break;
                }

                i++;
            }

            return indices;
        }


//******************************************************************************************
// 
//  Name:
//  Arguments:
//  Description:
// 
//******************************************************************************************
        
        std::vector<const char*> getRequiredExtensions() {
            uint32_t glfwExtensionCount = 0;
            const char** glfwExtensions;
                glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
            std::vector<const char*> extensions(  glfwExtensions
                                                , glfwExtensions + glfwExtensionCount);
                            
            if (enableValidationLayers) {
                extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            }
            
            return extensions;
        }


//******************************************************************************************
// 
//  Name:
//  Arguments:
//  Description:
// 
//******************************************************************************************
        
        bool checkValidationLayerSupport() {
            uint32_t layerCount;
            vkEnumerateInstanceLayerProperties(   &layerCount
                                                , nullptr);
                            
            std::vector<VkLayerProperties> availableLayers(layerCount);
            vkEnumerateInstanceLayerProperties(   &layerCount
                                                , availableLayers.data());
            
            for (const char* layerName : validationLayers) {
                bool layerFound = false;
                
                for (const auto& layerProperties : availableLayers) {
                    if (strcmp(   layerName
                                , layerProperties.layerName) == 0) {
                            layerFound = true;
                            break;
                        }
                }

                if (!layerFound) {
                    return false;
                }
            }
            return true;
        }


//******************************************************************************************
// 
//  Name:
//  Arguments:
//  Description:
// 
//******************************************************************************************
        
        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(  VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity
                                                            , VkDebugUtilsMessageTypeFlagsEXT messageType
                                                            , const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData
                                                            , void* pUserData) {
            std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;
            
            return VK_FALSE;
        }
};


//******************************************************************************************
// 
//  Name:           main
//  Arguments:      N/A
//  Description:    Main control function to call app with simple error checking.
// 
//******************************************************************************************

int main()
{
    HelloTriangleApplication app;

    try
    {
        app.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
