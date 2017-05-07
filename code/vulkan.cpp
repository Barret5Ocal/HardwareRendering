/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Barret Gaylor $
   $Notice: (C) Copyright 2017 by Barret Gaylor, Inc. All Rights Reserved. $
   ======================================================================== */
#define VK_USE_PLATFORM_WIN32_KHR
#define VK_NO_PROTOTYPES
#include "vulkan.h"

struct vulkan_context
{
    uint32 Width;
    uint32 Height;

    uint32_t PresentQueueIdx;
    VkQueue PresentQueue;
    
    VkInstance Instance;

    VkDebugReportCallbackEXT Callback;
    
    VkSurfaceKHR Surface;
    VkSwapchainKHR SwapChain;

    VkPhysicalDevice PhysicalDevice;
    VkPhysicalDeviceProperties PhysicalDeviceProperties;
    VkPhysicalDeviceMemoryProperties MemoryProperties;
};

vulkan_context Context; 

PFN_vkCreateInstance vkCreateInstance = NULL;
PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties = NULL;
PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = NULL; 
PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties = NULL;
PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices = NULL; 
PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties = NULL; 
PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties = NULL; 
PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR = NULL;

void Win32LoadVulkan()
{
    HMODULE VulkanModule = LoadLibrary("vulkan-1.dll");
    Assert(VulkanModule);

    vkCreateInstance = (PFN_vkCreateInstance) GetProcAddress(VulkanModule, "vkCreateInstance");
    Assert(vkCreateInstance);

    vkEnumerateInstanceLayerProperties = (PFN_vkEnumerateInstanceLayerProperties) GetProcAddress(VulkanModule, "vkEnumerateInstanceLayerProperties");
    Assert(vkEnumerateInstanceLayerProperties);

    vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr) GetProcAddress(VulkanModule, "vkGetInstanceProcAddr");
    Assert(vkGetInstanceProcAddr);

    vkEnumerateInstanceExtensionProperties = (PFN_vkEnumerateInstanceExtensionProperties) GetProcAddress(VulkanModule, "vkEnumerateInstanceExtensionProperties");
    Assert(vkEnumerateInstanceExtensionProperties);

    vkEnumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices) GetProcAddress(VulkanModule, "vkEnumeratePhysicalDevices");
    Assert(vkEnumeratePhysicalDevices);

    vkGetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties) GetProcAddress(VulkanModule, "vkGetPhysicalDeviceProperties");
    Assert(vkGetPhysicalDeviceProperties);

    vkGetPhysicalDeviceQueueFamilyProperties = (PFN_vkGetPhysicalDeviceQueueFamilyProperties)GetProcAddress(VulkanModule, "vkGetPhysicalDeviceQueueFamilyProperties");
    Assert(vkGetPhysicalDeviceQueueFamilyProperties);

    vkGetPhysicalDeviceSurfaceSupportKHR = (PFN_vkGetPhysicalDeviceSurfaceSupportKHR)GetProcAddress(VulkanModule, "vkGetPhysicalDeviceQueueFamilyProperties");
    Assert(vkGetPhysicalDeviceQueueFamilyProperties);

    
}

PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT = NULL;
PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT = NULL;
PFN_vkDebugReportMessageEXT vkDebugReportMessageEXT = NULL;
PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR = NULL; 

void win32_LoadVulkanExtensions( vulkan_context &context ) {

    *(void **)&vkCreateDebugReportCallbackEXT = vkGetInstanceProcAddr( context.Instance, 
                                                "vkCreateDebugReportCallbackEXT" );
    *(void **)&vkDestroyDebugReportCallbackEXT = vkGetInstanceProcAddr( context.Instance, 
                                                "vkDestroyDebugReportCallbackEXT" );
    *(void **)&vkDebugReportMessageEXT = vkGetInstanceProcAddr( context.Instance, 
                                                "vkDebugReportMessageEXT" );
    *(void **)&vkCreateWin32SurfaceKHR = vkGetInstanceProcAddr( context.Instance, 
                                                "vkCreateWin32SurfaceKHR" );
}

VKAPI_ATTR VkBool32 VKAPI_CALL
MyDebugReportCallback( VkDebugReportFlagsEXT flags, 
                       VkDebugReportObjectTypeEXT objectType,
                       uint64_t object, size_t location, 
                       int32_t messageCode, const char* pLayerPrefix,
                       const char* pMessage, void* pUserData )
{
    OutputDebugStringA(pLayerPrefix);
    OutputDebugStringA(" ");
    OutputDebugStringA(pMessage);
    OutputDebugStringA("\n");
    return VK_FALSE;
}

void Win32InitVulkan(HINSTANCE Instance, HWND Window)
{
    Win32LoadVulkan();

    VkBool32 Result;
    
    uint32 LayerCount = 0;
    vkEnumerateInstanceLayerProperties(&LayerCount, 0);
    VkLayerProperties *LayersAvailable = new VkLayerProperties[LayerCount];
    vkEnumerateInstanceLayerProperties(&LayerCount, LayersAvailable);

    bool FoundValidation = false;
    for(int32 Index = 0;
        Index < LayerCount;
        ++Index)
    {
        if(strcmp(LayersAvailable[Index].layerName, "VK_LAYER_LUNARG_standard_validation") == 0)
        {
            FoundValidation = true; 
        }
    }
    Assert(FoundValidation);
    char *Layers[] = {"VK_LAYER_LUNARG_standard_validation"};

    //NOTE(barret): Extensions
    uint32 ExtensionCount = 0;
    vkEnumerateInstanceExtensionProperties(0, &ExtensionCount, 0);
    VkExtensionProperties *ExtensionsAvailable = new VkExtensionProperties[ExtensionCount];
    vkEnumerateInstanceExtensionProperties(0, &ExtensionCount, ExtensionsAvailable);

    const char *Extensions[] = { "VK_KHR_surface", "VK_KHR_win32_surface", "VK_EXT_debug_report" };
    uint32_t NumberRequiredExtensions = sizeof(Extensions) / sizeof(char*);
    uint32_t FoundExtensions = 0;
    for( uint32_t i = 0; i < ExtensionCount; ++i ) {
        for( int j = 0; j < NumberRequiredExtensions; ++j ) {
            if( strcmp( ExtensionsAvailable[i].extensionName, Extensions[j] ) == 0 ) {
                FoundExtensions++;
            }
        }
    }
    Assert( FoundExtensions == NumberRequiredExtensions);
    
    VkApplicationInfo ApplicationInfo = {}; 
    ApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    ApplicationInfo.pApplicationName = "First Test";
    ApplicationInfo.engineVersion = 1;
    ApplicationInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);

    VkInstanceCreateInfo InstanceInfo = {};
    InstanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    InstanceInfo.pApplicationInfo = &ApplicationInfo;
    InstanceInfo.enabledLayerCount = 1;
    InstanceInfo.ppEnabledLayerNames = Layers;
    InstanceInfo.enabledExtensionCount = 3;
    InstanceInfo.ppEnabledExtensionNames = Extensions;

    vkCreateInstance(&InstanceInfo, 0, &Context.Instance);

    win32_LoadVulkanExtensions(Context);

    VkDebugReportCallbackCreateInfoEXT CallbackCreateInfo = {};
    CallbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
    CallbackCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT |
        VK_DEBUG_REPORT_WARNING_BIT_EXT |
        VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
    CallbackCreateInfo.pfnCallback = &MyDebugReportCallback;
    CallbackCreateInfo.pUserData = 0;

    Result = vkCreateDebugReportCallbackEXT(Context.Instance, &CallbackCreateInfo,
                                            NULL, &Context.Callback);

    VkWin32SurfaceCreateInfoKHR SurfaceCreateInfo = {};   
    SurfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    SurfaceCreateInfo.hinstance = Instance;
    SurfaceCreateInfo.hwnd = Window;

    Result = vkCreateWin32SurfaceKHR(Context.Instance, &SurfaceCreateInfo, 0, &Context.Surface);
    Assert(Result);
    
    uint32 PhysicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(Context.Instance, &PhysicalDeviceCount, 0);
    VkPhysicalDevice *PhysicalDevices = new VkPhysicalDevice[PhysicalDeviceCount];
    vkEnumeratePhysicalDevices(Context.Instance, &PhysicalDeviceCount, PhysicalDevices);

    for(uint32 Index = 0;
        Index < PhysicalDeviceCount;
        ++Index)
    {
        VkPhysicalDeviceProperties DeviceProperties = {};
        vkGetPhysicalDeviceProperties(PhysicalDevices[Index], &DeviceProperties);

        uint32 QueueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevices[Index], &QueueFamilyCount, 0);
        VkQueueFamilyProperties *QueueFamilyProperties = new VkQueueFamilyProperties[QueueFamilyCount];
        vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevices[Index],
                                                 &QueueFamilyCount,
                                                 QueueFamilyProperties);

        for(uint32 Index2 = 0;
            Index2 < QueueFamilyCount;
            ++Index2)
        {
            VkBool32 SupportsPresent;
            vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDevices[Index], Index2, Context.Surface,
                                                 &SupportsPresent);

            if(SupportsPresent && (QueueFamilyProperties[Index2].queueFlags & VK_QUEUE_GRAPHICS_BIT))
            {
                Context.PhysicalDevice = PhysicalDevices[Index];
                Context.PhysicalDeviceProperties = DeviceProperties;
                Context.PresentQueueIdx = Index2;
                break; 
            }
        }
        delete[] QueueFamilyProperties;

        if(Context.PhysicalDevice)
        {
            break; 
        }
    }
    delete[] PhysicalDevices;

    Assert(Context.PhysicalDevice);
}

void DisplayFrame(HDC DeviceContext, int WindowWidth, int WindowHeight,
                  render_fuctions *Fuctions, render_data *Data)
{
    
}



