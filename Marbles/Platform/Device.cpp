// This source file is part of marbles library.
//
// Copyright (c) 2017 Dan Cobban
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// --------------------------------------------------------------------------------------------------------------------

#include <platform/device.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// --------------------------------------------------------------------------------------------------------------------
namespace marbles {

// --------------------------------------------------------------------------------------------------------------------
struct device::manager::instance
{
    instance();
    void init(const char* applicationName, uint32_t version, bool enableValidationLayers);

private:
    bool checkValidationLayerSupport() const;
    void createInstance(const char* applicationName, uint32_t version);
    void setupDebugCallback();
    VkResult createDebugReportCallback(const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, VkDebugReportCallbackEXT* pCallback);
    void destroyDebugReportCallback(VkDebugReportCallbackEXT callback);
    const VkAllocationCallbacks* allocator() const;

    VkAllocationCallbacks _allocCallback;
    shared_ptr<VkInstance_T> _instance;
    shared_ptr<VkDebugReportCallbackEXT_T> _debugCallback;
    bool _enableValidationLayers;

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData);
    static void getRequiredExtensions(vector<const char*>& extensions);
    static const char* const validationLayers[];
};

// --------------------------------------------------------------------------------------------------------------------
const char* const device::manager::instance::validationLayers[] = {
    "VK_LAYER_LUNARG_standard_validation"
};

// --------------------------------------------------------------------------------------------------------------------
device::manager::instance::instance()
{
    memset(&_allocCallback, 0, sizeof(_allocCallback));
    _enableValidationLayers = false;
}

// --------------------------------------------------------------------------------------------------------------------
void device::manager::instance::init(const char* applicationName, uint32_t version, bool enableValidationLayers)
{
    _enableValidationLayers = enableValidationLayers;
    createInstance(applicationName, version);
    setupDebugCallback();
}

// --------------------------------------------------------------------------------------------------------------------
void device::manager::instance::createInstance(const char* applicationName, uint32_t version)
{
    if (_enableValidationLayers && !checkValidationLayerSupport())
        return;

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = applicationName;
    appInfo.applicationVersion = version;
    appInfo.pEngineName = "marbles";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    vector<const char*> extensions;
    getRequiredExtensions(extensions);
    if (_enableValidationLayers)
    {
        extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    createInfo.enabledLayerCount = 0;
    if (_enableValidationLayers)
    {
        createInfo.enabledLayerCount = sizeof(validationLayers) / sizeof(validationLayers[0]);
        createInfo.ppEnabledLayerNames = validationLayers;
    }

    VkInstance local = nullptr;
    if (VK_SUCCESS == vkCreateInstance(&createInfo, allocator(), &local))
    {
        _instance.reset(local, [this](VkInstance obj)
        {
            vkDestroyInstance(obj, this->allocator());
        });
    }
}

// --------------------------------------------------------------------------------------------------------------------
void device::manager::instance::setupDebugCallback()
{
    if (_enableValidationLayers)
    {
        VkDebugReportCallbackCreateInfoEXT debugInfo = {};
        debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        debugInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
        debugInfo.pfnCallback = debugCallback;

        VkDebugReportCallbackEXT reportCallback;
        if (VK_SUCCESS == createDebugReportCallback(&debugInfo, &reportCallback))
        {
            _debugCallback.reset(reportCallback, [this](VkDebugReportCallbackEXT callback)
            {
                this->destroyDebugReportCallback(callback);
            });
        }
    }
}

// --------------------------------------------------------------------------------------------------------------------
VKAPI_ATTR VkBool32 VKAPI_CALL device::manager::instance::debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData)
{
    (void)userData;
    (void)msg;
    (void)layerPrefix; 
    (void)code;
    (void)location;
    (void)obj;
    (void)flags;
    (void)objType;
    // std::cerr << "validation layer: " << msg << std::endl;

    return VK_FALSE;
}

// --------------------------------------------------------------------------------------------------------------------
bool device::manager::instance::checkValidationLayerSupport() const
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	bool layersFound = true;
    for (const char* layerName : validationLayers)
    {
		bool layerFound = false;
        for (const auto& layerProperties : availableLayers)
        {
			layerFound = 0 == strcmp(layerName, layerProperties.layerName);
            
			if (layerFound)
			{
	            break;
            }
        }

		layersFound |= layerFound;
        if (!layersFound)
        {
			break;
        }
    }

    return layersFound;
}

// --------------------------------------------------------------------------------------------------------------------
const VkAllocationCallbacks* device::manager::instance::allocator() const
{
    return nullptr == _allocCallback.pfnAllocation ? nullptr : &_allocCallback;
}

// --------------------------------------------------------------------------------------------------------------------
void device::manager::instance::getRequiredExtensions(vector<const char*>& extensions)
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    extensions.reserve(extensions.size() + glfwExtensionCount);
    for (unsigned int i = 0; i < glfwExtensionCount; i++)
    {
        extensions.push_back(glfwExtensions[i]);
    }
}

// --------------------------------------------------------------------------------------------------------------------
VkResult device::manager::instance::createDebugReportCallback(const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, VkDebugReportCallbackEXT* pCallback)
{
    auto func = vkGetInstanceProcAddr(_instance.get(), "vkCreateDebugReportCallbackEXT");
    if (func != nullptr)
    {
        return reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(func)(_instance.get(), pCreateInfo, allocator(), pCallback);
    }
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

// --------------------------------------------------------------------------------------------------------------------
void device::manager::instance::destroyDebugReportCallback(VkDebugReportCallbackEXT callback)
{
    auto func = vkGetInstanceProcAddr(_instance.get(), "vkDestroyDebugReportCallbackEXT");
    if (func != nullptr)
    {
        reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(func)(_instance.get(), callback, allocator());
    }
}

// --------------------------------------------------------------------------------------------------------------------
device::manager::manager(const char* applicationName, uint32_t version, bool enableValidationLayers)
{
    _self = make_shared<instance>();
    if (_self)
    {
        _self->init(applicationName, version, enableValidationLayers);
    }
}

// --------------------------------------------------------------------------------------------------------------------
device::iterator device::manager::begin()
{
    return device::iterator();
}

// --------------------------------------------------------------------------------------------------------------------
device::iterator device::manager::end()
{
    return device::iterator();
}

// --------------------------------------------------------------------------------------------------------------------
device* device::iterator::get()
{
    shared_ptr<device> it = _device.lock();
    if (it = _device.lock())
    {
        return it.get();
    }
    return nullptr;
}


// --------------------------------------------------------------------------------------------------------------------
const device* device::iterator::get() const
{
    return const_cast<device::iterator*>(this)->get();
}

// --------------------------------------------------------------------------------------------------------------------
} // namespace marbles 

// End of file --------------------------------------------------------------------------------------------------------
