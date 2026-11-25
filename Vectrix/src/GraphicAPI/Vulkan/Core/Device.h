#pragma once
#include "volk.h"
#include "vk_mem_alloc.h"

#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

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
        uint32_t graphicsFamily{};
        uint32_t presentFamily{};
        bool graphicsFamilyHasValue = false;
        bool presentFamilyHasValue = false;
        [[nodiscard]] bool isComplete() const { return graphicsFamilyHasValue && presentFamilyHasValue; }
    };

	class Window;

    /**
     * The Vulkan instance, physical and logical device, queues and pools.
     *
     * Threads: Vulkan leaves the synchronization of queues and pools to the application. Here:
     * - every use of a queue goes through submit/present/waitIdle, or holds lockQueue (vkDeviceWaitIdle needs all
     *   queues, hence waitIdle): any thread may submit;
     * - beginSingleTimeCommands/endSingleTimeCommands work on any thread, each thread having its own command pool;
     * - descriptor sets from descriptorPool() are allocated and freed with allocateDescriptorSets/freeDescriptorSets;
     * - VMA locks its allocators itself, and pipelines are created with pipelineCache(), which Vulkan synchronizes.
     * getCommandPool() is the main thread's (the frames' command buffers).
     */
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
        [[nodiscard]] VkDescriptorPool descriptorPool() const { return m_descriptorPool; }

        SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport(m_physicalDevice); }
        QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(m_physicalDevice); }
        [[nodiscard]] VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;

        /// Hold it around any direct use of graphicsQueue()/presentQueue() (other threads submit too)
        [[nodiscard]] std::unique_lock<std::mutex> lockQueue() { return std::unique_lock(m_queueMutex); }
        /// The mutex lockQueue takes, for code that can only take a mutex (the ImGui backend)
        [[nodiscard]] std::mutex& queueMutex() { return m_queueMutex; }
        /// vkQueueSubmit on the graphics queue, locked
        VkResult submit(const VkSubmitInfo& submitInfo, VkFence fence);
        /// vkQueuePresentKHR on the present queue, locked
        VkResult present(const VkPresentInfoKHR& presentInfo);
        /// vkDeviceWaitIdle, with the queues locked as it requires
        void waitIdle();

        /// vkAllocateDescriptorSets from descriptorPool(), locked: any thread
        VkResult allocateDescriptorSets(const VkDescriptorSetAllocateInfo& allocateInfo, VkDescriptorSet* sets);
        /// vkFreeDescriptorSets to descriptorPool(), locked: any thread
        void freeDescriptorSets(uint32_t count, const VkDescriptorSet* sets);

        /// Shared by every pipeline (and ImGui's), loaded from and saved to the state folder: pipelines already
        /// built by a previous run are created much faster. Vulkan synchronizes it itself
        [[nodiscard]] VkPipelineCache pipelineCache() const { return m_pipelineCache; }

        // Buffer Helper Functions
        void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryProperties, VkBuffer& buffer, VmaAllocation& allocation,VmaAllocator allocator=nullptr);
        /// A one-time command buffer from the calling thread's command pool: any thread
        VkCommandBuffer beginSingleTimeCommands();
        /// Submit it and wait for it to complete, only blocking the calling thread
        void endSingleTimeCommands(VkCommandBuffer commandBuffer);
        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkDeviceSize srcOffset, VkDeviceSize dstOffset);

        void createImageWithInfo(const VkImageCreateInfo& imageInfo, VkMemoryPropertyFlags properties, VkImage& image, VmaAllocation& allocation);

        VkPhysicalDeviceProperties properties;


        void destroyBuffer(VkBuffer buffer, VmaAllocation allocation,VmaAllocator allocator=nullptr);
        void destroyImage(VkImage image, VmaAllocation allocation);

        [[nodiscard]] VmaAllocator getBufferAllocator() const {return m_bufferAllocator;}
        [[nodiscard]] VmaAllocator getTextureAllocator() const {return m_textureAllocator;}
        [[nodiscard]] VmaAllocator getSSBOAllocator() const {return m_SSBOAllocator;}

        /// @brief Whether the logical device was created with the fillModeNonSolid feature (wireframe).
        [[nodiscard]] bool supportsFillModeNonSolid() const { return m_fillModeNonSolid; }
    private:
        void createInstance();
        void setupDebugMessenger();
        void createSurface();
        void pickPhysicalDevice();
        void createLogicalDevice();
        void createCommandPool();
	    void createDescriptorPool(const DescriptorPoolConfig& cfg);
        void createPipelineCache();
        void savePipelineCache() const;
        /// The calling thread's pool for single-time commands (the main thread's is m_commandPool)
        VkCommandPool threadCommandPool();
        // helper functions
        bool isDeviceSuitable(VkPhysicalDevice physicalDevice);
        static std::vector<const char*> getRequiredExtensions();
        [[nodiscard]] bool checkValidationLayerSupport() const;
        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) const;

        static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
        static void hasGflwRequiredInstanceExtensions();
        bool checkDeviceExtensionSupport(VkPhysicalDevice device) const;
        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) const;


        VkDebugUtilsMessengerEXT m_debugMessenger;
        Window& m_window;
        VkCommandPool m_commandPool;

	    VmaAllocator m_bufferAllocator;
	    VmaAllocator m_textureAllocator;
	    VmaAllocator m_SSBOAllocator;

        VkDevice m_device;
        VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
        VkInstance m_instance;
        VkSurfaceKHR m_surface;
        VkQueue m_graphicsQueue;
        VkQueue m_presentQueue;
	    VkDescriptorPool m_descriptorPool;
	    bool m_fillModeNonSolid = false;

        std::mutex m_queueMutex;          ///< graphics and present queues (one lock: they may be the same VkQueue)
        std::mutex m_descriptorPoolMutex; ///< m_descriptorPool
        std::mutex m_threadPoolsMutex;    ///< m_threadCommandPools (the map, not the pools: each is used by its thread)
        std::unordered_map<std::thread::id, VkCommandPool> m_threadCommandPools;
        VkPipelineCache m_pipelineCache = VK_NULL_HANDLE;


        const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
        const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME };
	};
}