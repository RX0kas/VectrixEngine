#pragma once
#include "vcpch.h"
#include "Vectrix/Window.h"

namespace Vectrix {
    struct DescriptorPoolConfig {
        uint32_t uboCount;
        uint32_t ssboCount;
        uint32_t samplerCount;
        uint32_t maxSets;
    };


    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    struct QueueFamilyIndices {
        uint32_t graphicsFamily;
        uint32_t presentFamily;
        bool graphicsFamilyHasValue = false;
        bool presentFamilyHasValue = false;
        bool isComplete() { return graphicsFamilyHasValue && presentFamilyHasValue; }
    };

	class Device
	{
    public:
        #ifdef VC_DEBUG
            constexpr static bool enableValidationLayers = true;
        #else
            constexpr static bool enableValidationLayers = false;
        #endif

        Device(Window& window,DescriptorPoolConfig cfg);
        ~Device();



        VkCommandPool getCommandPool() { return m_commandPool; }
        VkDevice device() { return m_device; }
        VkPhysicalDevice physicalDevice() { return m_physicalDevice; }
        VkInstance instance() { return m_instance; }
        VkSurfaceKHR surface() { return m_surface; }
        VkQueue graphicsQueue() { return m_graphicsQueue; }
        VkQueue presentQueue() { return m_presentQueue; }
        VkFormat imageFormat() { return m_imageFormat; }
	    VkDescriptorPool descriptorPool() { return m_descriptorPool; }
	    VkDescriptorSetLayout descriptorSetLayout() {return m_descriptorSetLayout;}

        SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport(m_physicalDevice); }
        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
        QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(m_physicalDevice); }
        VkFormat findSupportedFormat(
            const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

        // Buffer Helper Functions
        void createBuffer(
            VkDeviceSize size,
            VkBufferUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkBuffer& buffer,
            VkDeviceMemory& bufferMemory);
        VkCommandBuffer beginSingleTimeCommands();
        void endSingleTimeCommands(VkCommandBuffer commandBuffer);
        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
        void copyBufferToImage(
            VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);

        void createImageWithInfo(
            const VkImageCreateInfo& imageInfo,
            VkMemoryPropertyFlags properties,
            VkImage& image,
            VkDeviceMemory& imageMemory);

        VkPhysicalDeviceProperties properties;

        void setImageFormat(VkFormat format) {
            m_imageFormat = format;
        }

	    VkDescriptorSetLayout createFrameSSBOLayout();
    private:
        void createInstance();
        void setupDebugMessenger();
        void createSurface();
        void pickPhysicalDevice();
        void createLogicalDevice();
        void createCommandPool();
	    void createDescriptorPool(const DescriptorPoolConfig& cfg);

        // helper functions
        bool isDeviceSuitable(VkPhysicalDevice device);
        std::vector<const char*> getRequiredExtensions();
        bool checkValidationLayerSupport();
        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
        void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
        void hasGflwRequiredInstanceExtensions();
        bool checkDeviceExtensionSupport(VkPhysicalDevice device);
        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);


        VkDebugUtilsMessengerEXT m_debugMessenger;
        Window& m_window;
        VkCommandPool m_commandPool;

        VkDevice m_device;
        VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
        VkInstance m_instance;
        VkSurfaceKHR m_surface;
        VkQueue m_graphicsQueue;
        VkQueue m_presentQueue;
        VkFormat m_imageFormat;
	    VkDescriptorPool m_descriptorPool;
	    VkDescriptorSetLayout m_descriptorSetLayout;

        const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
        const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	};


}