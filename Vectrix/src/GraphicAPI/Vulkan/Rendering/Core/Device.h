#pragma once
#include "vk_mem_alloc.h"
#include "vector"

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
        bool isComplete() const { return graphicsFamilyHasValue && presentFamilyHasValue; }
    };

	class Window;

    class Device {
    public:
        static bool enableValidationLayers;

        Device(Window& window,DescriptorPoolConfig cfg);
        ~Device();

        [[nodiscard]] VkCommandPool getCommandPool() const { return m_commandPool; }
        [[nodiscard]] VkDevice device() const { return m_device; }
        [[nodiscard]] VkPhysicalDevice physicalDevice() const { return m_physicalDevice; }
        [[nodiscard]] VkInstance instance() const { return m_instance; }
        [[nodiscard]] VkSurfaceKHR surface() const { return m_surface; }
        [[nodiscard]] VkQueue graphicsQueue() const { return m_graphicsQueue; }
        [[nodiscard]] VkQueue presentQueue() const { return m_presentQueue; }
        [[nodiscard]] VkFormat imageFormat() const { return m_imageFormat; }
        [[nodiscard]] VkDescriptorPool descriptorPool() const { return m_descriptorPool; }
        [[nodiscard]] VkDescriptorSetLayout descriptorSetLayout() const {return m_descriptorSetLayout;}

        SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport(m_physicalDevice); }
        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
        QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(m_physicalDevice); }
        VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;

        // Buffer Helper Functions
        void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryProperties, VkBuffer& buffer, VmaAllocation& allocation);
        VkCommandBuffer beginSingleTimeCommands();
        void endSingleTimeCommands(VkCommandBuffer commandBuffer);
        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
        void copyBufferToImage( VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);

        void createImageWithInfo(const VkImageCreateInfo& imageInfo, VkMemoryPropertyFlags properties, VkImage& image, VmaAllocation& allocation);

        VkPhysicalDeviceProperties properties;

        void setImageFormat(VkFormat format) {
            m_imageFormat = format;
        }

        VkDescriptorSetLayout createFrameSSBOLayout() const;

        void destroyBuffer(VkBuffer buffer, VmaAllocation allocation);
        void destroyImage(VkImage image, VmaAllocation allocation);

        [[nodiscard]] VmaAllocator getAllocator() const {return m_allocator;}
    private:
        void createInstance();
        void setupDebugMessenger();
        void createSurface();
        void pickPhysicalDevice();
        void createLogicalDevice();
        void createCommandPool();
	    void createDescriptorPool(const DescriptorPoolConfig& cfg);
        // helper functions
        bool isDeviceSuitable(VkPhysicalDevice physicalDevice);
        std::vector<const char*> getRequiredExtensions();
        bool checkValidationLayerSupport();
        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) const;

        static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
        void hasGflwRequiredInstanceExtensions();
        bool checkDeviceExtensionSupport(VkPhysicalDevice device) const;
        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) const;


        VkDebugUtilsMessengerEXT m_debugMessenger;
        Window& m_window;
        VkCommandPool m_commandPool;

	    VmaAllocator m_allocator;

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