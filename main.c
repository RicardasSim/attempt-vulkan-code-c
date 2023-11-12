/**********************************************************************
 * FILE NAME: main.c
 *
 *
 **********************************************************************/

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cglm/cglm.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include "tinyobj_loader_c.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <float.h>
#include <sys/time.h>

const uint32_t width_init = 800;
const uint32_t height_init = 600;

const char model_path[] = "models/GeneratorShack.obj";
const char texture_path[] = "textures/GeneratorShack_Base_Color.png";

const int max_frames_in_flight = 2;

const char *validationLayers[] = {
    "VK_LAYER_KHRONOS_validation"
};

const char *deviceExtensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

typedef struct QueueFamilyIndices {
    uint32_t graphicsFamily;
    bool graphicsFamilyHasValue;
    uint32_t presentFamily;
    bool presentFamilyHasValue;
} QueueFamilyIndices;

typedef struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR *formats;
    uint32_t format_count;
    VkPresentModeKHR *presentModes;
    uint32_t presentmode_count;
} SwapChainSupportDetails;

typedef struct s_vertex {
    vec3 pos;
    vec3 color;
    vec2 texCoord;
} Vertex;

typedef struct UniformBufferObject {
    mat4 model  __attribute__ ((aligned (16)));
    mat4 view   __attribute__ ((aligned (16)));
    mat4 proj   __attribute__ ((aligned (16)));
} UniformBufferObject;

GLFWwindow *window;

VkInstance instance;
VkDebugUtilsMessengerEXT debugMessenger;
VkSurfaceKHR surface;
VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
VkDevice logicalDevice;

VkQueue graphicsQueue;
VkQueue presentQueue;

VkSwapchainKHR swapChain;

VkImage *swapChainImages;
uint32_t swapchain_image_count;
VkFormat swapChainImageFormat;
VkExtent2D swapChainExtent;
VkImageView *swapChainImageViews;
uint32_t swapchain_imageviews_count;
VkFramebuffer *swapChainFramebuffers;
uint32_t swapchain_framebuffers_count;

VkRenderPass renderPass;
VkDescriptorSetLayout descriptorSetLayout;
VkPipelineLayout pipelineLayout;
VkPipeline graphicsPipeline;

VkCommandPool commandPool;

VkImage colorImage;
VkDeviceMemory colorImageMemory;
VkImageView colorImageView;

VkImage depthImage;
VkDeviceMemory depthImageMemory;
VkImageView depthImageView;

uint32_t mipmapLevels;
VkImage textureImage;
VkDeviceMemory textureImageMemory;
VkImageView textureImageView;
VkSampler textureSampler;

Vertex *vertices;
size_t vertices_size = 0;
uint32_t *indices;
size_t indices_size = 0;
VkBuffer vertexBuffer;
VkDeviceMemory vertexBufferMemory;
VkBuffer indexBuffer;
VkDeviceMemory indexBufferMemory;

VkBuffer *uniformBuffers;
VkDeviceMemory *uniformBuffersMemory;
void **uniformBuffersMapped;

VkDescriptorPool descriptorPool;
VkDescriptorSet *descriptorSets;

VkCommandBuffer *commandBuffers;

VkSemaphore *imageAvailableSemaphores;
VkSemaphore *renderFinishedSemaphores;
VkFence *inFlightFences;
uint32_t currentFrame = 0;

bool framebufferResized = false;

typedef struct s_Extensions {
    char **data;
    uint32_t size;
} S_Extensions;

typedef struct s_read_buffer {
    char *p_buffer;
    size_t size;
} S_read_buffer;

typedef struct s_ctx {
    char *p_buffer[2];
    unsigned int num;
} S_ctx;

/*
================================================
 Function Prototypes
================================================
*/

VkCommandBuffer beginSingleTimeCommands();
bool checkDeviceExtensionSupport( VkPhysicalDevice device );
bool checkValidationLayerSupport();
VkExtent2D chooseSwapExtent( VkSurfaceCapabilitiesKHR capabilities );
VkPresentModeKHR chooseSwapPresentMode( VkPresentModeKHR *availablePresentModes,
                                        uint32_t num_of_available_present_modes );
VkSurfaceFormatKHR chooseSwapSurfaceFormat( VkSurfaceFormatKHR *availableFormats,
                                            uint32_t num_of_available_formats );
void copyBuffer( VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size );
void copyBufferToImage( VkBuffer buffer,
                        VkImage image,
                        uint32_t width,
                        uint32_t height );
bool createBuffer( VkDeviceSize size,
                   VkBufferUsageFlags usage,
                   VkMemoryPropertyFlags properties,
                   VkBuffer *buffer,
                   VkDeviceMemory *bufferMemory);
bool createColorResources();
bool createCommandPool();
bool createCommandBuffers();
bool createDepthResources();
bool createDescriptorSetLayout();
bool createDescriptorPool();
bool createDescriptorSets();
bool createFramebuffers();
bool createGraphicsPipeline();
bool createImage( uint32_t width,
                  uint32_t height,
                  uint32_t mipLevels,
                  VkSampleCountFlagBits numSamples,
                  VkFormat format,
                  VkImageTiling tiling,
                  VkImageUsageFlags usage,
                  VkMemoryPropertyFlags properties,
                  VkImage *p_image,
                  VkDeviceMemory *p_imageMemory );
VkImageView createImageView( VkImage image,
                             VkFormat format,
                             VkImageAspectFlags aspectFlags,
                             uint32_t mipLevels );
bool createImageViews();
bool createIndexBuffer();
bool createInstance();
bool createLogicalDevice();
bool createRenderPass();
bool createShaderModule( const char *code,
                         size_t code_size,
                         VkShaderModule *shaderModule );
bool createSyncObjects();
bool createSurface();
bool createSwapChain();
bool createTextureImage();
bool createTextureImageView();
bool createTextureSampler();
bool createUniformBuffers();
bool createVertexBuffer();
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                     VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                     const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                     void *pUserData );
bool drawFrame();
void endSingleTimeCommands( VkCommandBuffer commandBuffer );
bool findDepthFormat( VkFormat *pFormat );
uint32_t findMemoryType( uint32_t typeFilter, VkMemoryPropertyFlags properties );
QueueFamilyIndices findQueueFamilies( VkPhysicalDevice device );
static void framebufferResizeCallback( GLFWwindow *window, int width, int height );
bool generateMipmaps( VkImage image,
                      VkFormat imageFormat,
                      int32_t texWidth,
                      int32_t texHeight,
                      uint32_t mipLevels );
VkSampleCountFlagBits getMaxUsableSampleCount();
bool getRequiredExtensions( S_Extensions *s );
bool isDeviceSuitable( VkPhysicalDevice device );
bool loadModel();
bool pickPhysicalDevice();
void populateDebugMessengerCreateInfo( VkDebugUtilsMessengerCreateInfoEXT *createInfo );
bool querySwapChainSupport( VkPhysicalDevice device,
                            SwapChainSupportDetails *details );
bool transitionImageLayout( VkImage image,
                            VkFormat format,
                            VkImageLayout oldLayout,
                            VkImageLayout newLayout,
                            uint32_t mipLevels );
static bool readFile( S_read_buffer *pSrb, const char *filename );
bool setupDebugMessenger();


/*
================================================
 FUNCTION NAME: CreateDebugUtilsMessengerEXT
================================================
*/

VkResult CreateDebugUtilsMessengerEXT( VkInstance inst,
                                       const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                       const VkAllocationCallbacks *pAllocator,
                                       VkDebugUtilsMessengerEXT *pDebugMessenger )
{
    PFN_vkCreateDebugUtilsMessengerEXT func =
        (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr( inst, "vkCreateDebugUtilsMessengerEXT" );

    if ( func != NULL )
    {
        return func( inst, pCreateInfo, pAllocator, pDebugMessenger );
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

/*
================================================
 FUNCTION NAME: DestroyDebugUtilsMessengerEXT
================================================
*/

void DestroyDebugUtilsMessengerEXT( VkInstance inst,
                                    VkDebugUtilsMessengerEXT dbgMessenger,
                                    const VkAllocationCallbacks *pAllocator )
{
    PFN_vkDestroyDebugUtilsMessengerEXT func =
        (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr( inst, "vkDestroyDebugUtilsMessengerEXT" );

    if ( func != NULL )
    {
        func( inst, dbgMessenger, pAllocator );
    }
}

/*
================================================
 FUNCTION NAME: queueFamilyIndicesIsComplete
================================================
 */

bool queueFamilyIndicesIsComplete( struct QueueFamilyIndices s )
{
    return s.graphicsFamilyHasValue && s.presentFamilyHasValue;
}

/*
================================================
 FUNCTION NAME: freeSExtensions
================================================
 */

void freeSExtensions( S_Extensions *p_s )
{  
    for ( uint32_t i = 0; i < p_s->size; ++i )
    {
        free( p_s->data[i] );
    }

    free( p_s->data );
    
    p_s->size = 0;
    p_s->data = NULL;
}

/*
================================================
 FUNCTION NAME: clampUintVal
================================================
*/

uint32_t clampUintVal( uint32_t v, uint32_t min_v, uint32_t max_v )
{
    uint32_t tmp = v < min_v ? min_v : v;

    return tmp > max_v ? max_v : tmp;
}

/*
================================================
 FUNCTION NAME: framebufferResizeCallback
================================================
*/

static void framebufferResizeCallback( GLFWwindow *wnd,
                                       int width,
                                       int height )
{
    /* Silence compiler warning for unused parameters
       wnd, width, height */

    (void)wnd;
    (void)width;
    (void)height;

    framebufferResized = true;
}

/*
================================================
 FUNCTION NAME: initVulkan
================================================
*/

bool initVulkan()
{
    if ( !createInstance() )
    {
        printf("Error: createInstance.\n");
        return false;
    }

    if ( !setupDebugMessenger() )
    {
        printf("Error: setupDebugMessenger.\n");
        return false;
    }

    if ( !createSurface() )
    {
        printf("Error: createSurface.\n");
        return false;
    }

    if ( !pickPhysicalDevice() )
    {
        printf("Error: pickPhysicalDevice.\n");
        return false;
    }

    if ( !createLogicalDevice() )
    {
        printf("Error: createLogicalDevice.\n");
        return false;
    }

    if ( !createSwapChain() )
    {
        printf("Error: createSwapChain.\n");
        return false;
    }

    if ( !createImageViews() )
    {
        printf("Error: createImageViews.\n");
        return false;
    }

    if ( !createRenderPass() )
    {
        printf("Error: createRenderPass.\n");
        return false;
    }

    if ( !createDescriptorSetLayout() )
    {
        printf("Error: createDescriptorSetLayout.\n");
        return false;
    }

    if ( !createGraphicsPipeline() )
    {
        printf("Error: createGraphicsPipeline.\n");
        return false;
    }

    if ( !createCommandPool() )
    {
        printf("Error: createCommandPool.\n");
        return false;
    }

    if ( !createColorResources() )
    {
        printf("Error: createColorResources.\n");
        return false;
    }

    if ( !createDepthResources() )
    {
        printf("Error: createDepthResources.\n");
        return false;
    }

    if ( !createFramebuffers() )
    {
        printf("Error: createFramebuffers.\n");
        return false;
    }

    if ( !createTextureImage() )
    {
        printf("Error: createTextureImage.\n");
        return false;
    }

    if ( !createTextureImageView() )
    {
        printf("Error: createTextureImageView.\n");
        return false;
    }

    if ( !createTextureSampler() )
    {
        printf("Error: createTextureSampler.\n");
        return false;
    }

    if ( !loadModel() )
    {
        printf("Error: loadModel.\n");
        return false;
    }

    if ( !createVertexBuffer() )
    {
        printf("Error: createVertexBuffer.\n");
        return false;
    }

    if ( !createIndexBuffer() )
    {
        printf("Error: createIndexBuffer.\n");
        return false;
    }

    if ( !createUniformBuffers() )
    {
        printf("Error: createUniformBuffers.\n");
        return false;
    }

    if ( !createDescriptorPool() )
    {
        printf("Error: createDescriptorPool.\n");
        return false;
    }

    if ( !createDescriptorSets() )
    {
        printf("Error: createDescriptorSets.\n");
        return false;
    }

    if ( !createCommandBuffers() )
    {
        printf("Error: createCommandBuffers.\n");
        return false;
    }

    if ( !createSyncObjects() )
    {
        printf("Error: createSyncObjects.\n");
        return false;
    }

    return true;
}

/*
================================================
 FUNCTION NAME: mainLoop
================================================
*/

void mainLoop()
{
    while ( !glfwWindowShouldClose( window ) )
    {
        glfwPollEvents();
        
        if ( !drawFrame() )
        {
            break;
        }
    }

    vkDeviceWaitIdle( logicalDevice );
}

/*
================================================
 FUNCTION NAME: cleanupSwapChain
================================================
*/

void cleanupSwapChain()
{

    vkDestroyImageView( logicalDevice, depthImageView, NULL );
    vkDestroyImage( logicalDevice, depthImage, NULL );
    vkFreeMemory( logicalDevice, depthImageMemory, NULL );

    vkDestroyImageView( logicalDevice, colorImageView, NULL );
    vkDestroyImage( logicalDevice, colorImage, NULL );
    vkFreeMemory( logicalDevice, colorImageMemory, NULL );

    for ( uint32_t i = 0; i < swapchain_framebuffers_count; ++i )
    {
        vkDestroyFramebuffer( logicalDevice,
                              swapChainFramebuffers[i],
                              NULL );
    }

    free( swapChainFramebuffers );
    swapChainFramebuffers = NULL;
    swapchain_framebuffers_count = 0;

    for ( uint32_t i = 0; i < swapchain_imageviews_count; ++i )
    {
        vkDestroyImageView( logicalDevice,
                            swapChainImageViews[i],
                            NULL );
    }

    free( swapChainImageViews );
    swapChainImageViews = NULL;
    swapchain_imageviews_count = 0;

    free( swapChainImages );
    swapChainImages = NULL;
    swapchain_image_count = 0;

    vkDestroySwapchainKHR( logicalDevice, swapChain, NULL );

}

/*
================================================
 FUNCTION NAME: cleanupVulkan
================================================
*/

void cleanupVulkan()
{

    cleanupSwapChain();

    vkDestroyPipeline( logicalDevice, graphicsPipeline, NULL );
    vkDestroyPipelineLayout( logicalDevice, pipelineLayout, NULL );
    vkDestroyRenderPass( logicalDevice, renderPass, NULL );

    for ( int i = 0; i < max_frames_in_flight; i++ )
    {
        if ( uniformBuffers )
        {
            vkDestroyBuffer( logicalDevice, uniformBuffers[i], NULL );
        }

        if ( uniformBuffersMemory )
        {
            vkFreeMemory( logicalDevice, uniformBuffersMemory[i], NULL );
        }
    }

    //TODO:
    free( commandBuffers );
    free( uniformBuffers );
    free( uniformBuffersMemory );
    free( uniformBuffersMapped );

    vkDestroyDescriptorPool( logicalDevice, descriptorPool, NULL );

    vkDestroySampler( logicalDevice, textureSampler, NULL );
    vkDestroyImageView( logicalDevice, textureImageView, NULL );

    vkDestroyImage( logicalDevice, textureImage, NULL );
    vkFreeMemory( logicalDevice, textureImageMemory, NULL );

    vkDestroyDescriptorSetLayout( logicalDevice,
                                  descriptorSetLayout,
                                  NULL );

    vkDestroyBuffer( logicalDevice, indexBuffer, NULL );
    vkFreeMemory( logicalDevice, indexBufferMemory, NULL );

    vkDestroyBuffer(logicalDevice, vertexBuffer, NULL );
    vkFreeMemory( logicalDevice, vertexBufferMemory, NULL );

    free( descriptorSets );
    free( vertices );
    free( indices );

    for ( int i = 0; i < max_frames_in_flight; i++ )
    {
        if( renderFinishedSemaphores )
        {
            vkDestroySemaphore( logicalDevice, renderFinishedSemaphores[i], NULL );
        }

        if( imageAvailableSemaphores )
        {
            vkDestroySemaphore( logicalDevice, imageAvailableSemaphores[i], NULL );
        }

        if( inFlightFences )
        {
            vkDestroyFence( logicalDevice, inFlightFences[i], NULL );
        }
    }

    free( imageAvailableSemaphores );
    free( renderFinishedSemaphores );
    free( inFlightFences );

    vkDestroyCommandPool( logicalDevice, commandPool, NULL );

    vkDestroyDevice( logicalDevice, NULL );

    if ( enableValidationLayers )
    {
        DestroyDebugUtilsMessengerEXT( instance, debugMessenger, NULL );
    }

    vkDestroySurfaceKHR( instance, surface, NULL );

    vkDestroyInstance( instance, NULL );

}

/*
================================================
 FUNCTION NAME: recreateSwapChain
================================================
*/

bool recreateSwapChain()
{
    int width = 0, height = 0;

    glfwGetFramebufferSize( window, &width, &height );

    while ( width == 0 || height == 0 )
    {
        glfwGetFramebufferSize( window, &width, &height );
        glfwWaitEvents();
    }

    vkDeviceWaitIdle( logicalDevice );

    cleanupSwapChain();

    if ( !createSwapChain() )
    {
        printf("Error: recreate swap chain, createSwapChain.\n");
        return false;
    }

    if ( !createImageViews() )
    {
        printf("Error: recreate swap chain, createImageViews.\n");
        return false;
    }

    if ( !createColorResources() )
    {
        printf("Error: recreate swap chain, createColorResources.\n");
        return false;
    }

    if ( !createDepthResources() )
    {
        printf("Error: recreate swap chain, createDepthResources.\n");
        return false;
    }

    if ( !createFramebuffers() )
    {
        printf("Error: recreate swap chain, createFramebuffers.\n");
        return false;
    }

    return true;
}

/*
================================================
 FUNCTION NAME: createInstance
================================================
*/

bool createInstance()
{

    if ( enableValidationLayers && !checkValidationLayerSupport() )
    {
        printf("Error: validation layers requested, but not available!\n");
        return false;
    }

    VkApplicationInfo appInfo = {0};

    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {0};

    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    S_Extensions extensions;
    
    if ( !getRequiredExtensions( &extensions ) )
    {
        printf("Error: createInstance, getRequiredExtensions.\n");
        return false;
    }

    createInfo.enabledExtensionCount = extensions.size;
    createInfo.ppEnabledExtensionNames = (const char* const*) extensions.data;

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {0};

    if ( enableValidationLayers )
    {
        uint32_t numOfValLayers = sizeof validationLayers / sizeof validationLayers[0];

        if( numOfValLayers  == 1 && validationLayers[0] == NULL )
        {
            numOfValLayers = 0;
        }

        createInfo.enabledLayerCount = numOfValLayers;
        createInfo.ppEnabledLayerNames = validationLayers;

        populateDebugMessengerCreateInfo( &debugCreateInfo );

        createInfo.pNext = &debugCreateInfo;
    }
    else
    {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = NULL;
    }

    if ( vkCreateInstance( &createInfo, NULL, &instance ) != VK_SUCCESS )
    {
        printf("Error: failed to create instance!\n");
        freeSExtensions( &extensions );
        return false;
    }

    freeSExtensions( &extensions );

    return true;
}

/*
==================================================
 FUNCTION NAME: populateDebugMessengerCreateInfo
==================================================
*/

void populateDebugMessengerCreateInfo( VkDebugUtilsMessengerCreateInfoEXT *createInfo )
{

    memset( createInfo, 0, sizeof (VkDebugUtilsMessengerCreateInfoEXT) );

    createInfo->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

    createInfo->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

    createInfo->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

    createInfo->pfnUserCallback = debugCallback;

}

/*
================================================
 FUNCTION NAME: setupDebugMessenger
================================================
*/

bool setupDebugMessenger()
{
    if ( !enableValidationLayers )
    {
        return true;
    }

    VkDebugUtilsMessengerCreateInfoEXT createInfo;

    populateDebugMessengerCreateInfo( &createInfo );

    if ( CreateDebugUtilsMessengerEXT( instance, &createInfo, NULL, &debugMessenger ) != VK_SUCCESS )
    {
        printf("Error: failed to set up debug messenger!\n");
        return false;
    }

    return true;
}

/*
================================================
 FUNCTION NAME: createSurface
================================================
*/

bool createSurface()
{
    if ( glfwCreateWindowSurface( instance, window, NULL, &surface ) != VK_SUCCESS )
    {
        printf("Error: failed to create window surface!\n");
        return false;
    }

    return true;
}

/*
================================================
 FUNCTION NAME: pickPhysicalDevice
================================================
*/

bool pickPhysicalDevice()
{

    uint32_t deviceCount = 0;

    vkEnumeratePhysicalDevices( instance, &deviceCount, NULL );

    if ( deviceCount == 0 )
    {
        printf("Error: failed to find GPUs with Vulkan support!\n");
        return false;
    }

    VkPhysicalDevice *devices = malloc( deviceCount * sizeof(VkPhysicalDevice) );

    if ( !devices )
    {
        printf("Error: cannot allocate memory (1).\n");
        return false;
    }


    vkEnumeratePhysicalDevices( instance, &deviceCount, devices );

    for ( uint32_t i = 0; i < deviceCount; ++i )
    {
        VkPhysicalDevice device = devices[i];

        if ( isDeviceSuitable( device ) )
        {
            physicalDevice = device;
            msaaSamples = getMaxUsableSampleCount();
            break;
        }
    }

    free ( devices );

    if ( physicalDevice == VK_NULL_HANDLE )
    {
        printf("Error: failed to find a suitable GPU!\n");
        return false;
    }

    return true;
}

/*
================================================
 FUNCTION NAME: createLogicalDevice
================================================
*/

bool createLogicalDevice()
{

    QueueFamilyIndices qfIndices = findQueueFamilies(physicalDevice);

    uint32_t queue_info_count = 0;

    if ( qfIndices.graphicsFamily != qfIndices.presentFamily )
    {
        queue_info_count = 2;
    }
    else
    {
        queue_info_count = 1;
    }

    VkDeviceQueueCreateInfo *queueCreateInfos = calloc( queue_info_count, sizeof(VkDeviceQueueCreateInfo) );

    if ( !queueCreateInfos )
    {
        printf("Error: cannot allocate memory (2).\n");
        return false;
    }

    uint32_t uniqueQueueFamilies[queue_info_count];

    if ( qfIndices.graphicsFamily != qfIndices.presentFamily )
    {
        uniqueQueueFamilies[0] = qfIndices.graphicsFamily;
        uniqueQueueFamilies[1] = qfIndices.presentFamily;
    }
    else
    {
        uniqueQueueFamilies[0] = qfIndices.graphicsFamily;
    }

    float queuePriority = 1.0f;

    for ( uint32_t i = 0; i < queue_info_count; ++i )
    {
        queueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfos[i].queueFamilyIndex = uniqueQueueFamilies[i];
        queueCreateInfos[i].queueCount = 1;
        queueCreateInfos[i].pQueuePriorities = &queuePriority;
    }

    VkPhysicalDeviceFeatures deviceFeatures = {0};

    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo createInfo = {0};

    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = queue_info_count;
    createInfo.pQueueCreateInfos = queueCreateInfos;

    createInfo.pEnabledFeatures = &deviceFeatures;

    uint32_t numOfDvcExt = sizeof deviceExtensions / sizeof deviceExtensions[0];

    if( numOfDvcExt  == 1 && deviceExtensions[0] == NULL )
    {
        numOfDvcExt = 0;
    }

    createInfo.enabledExtensionCount = numOfDvcExt;
    createInfo.ppEnabledExtensionNames = deviceExtensions;

    if (enableValidationLayers)
    {
        uint32_t numOfValLayers = sizeof validationLayers / sizeof validationLayers[0];

        if( numOfValLayers  == 1 && validationLayers[0] == NULL )
        {
            numOfValLayers = 0;
        }

        createInfo.enabledLayerCount = numOfValLayers;
        createInfo.ppEnabledLayerNames = validationLayers;
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    if ( vkCreateDevice( physicalDevice,
                         &createInfo,
                         NULL,
                         &logicalDevice )
        != VK_SUCCESS)
    {
        printf("Error: failed to create logical device!\n");
        free(queueCreateInfos);
        return false;
    }

    vkGetDeviceQueue( logicalDevice,
                      qfIndices.graphicsFamily,
                      0,
                      &graphicsQueue );

    vkGetDeviceQueue( logicalDevice,
                      qfIndices.presentFamily,
                      0,
                      &presentQueue );

    free(queueCreateInfos);

    return true;
}

/*
================================================
 FUNCTION NAME: createSwapChain
================================================
*/

bool createSwapChain()
{

    SwapChainSupportDetails swapChainSupport = {0};
    
    querySwapChainSupport( physicalDevice, &swapChainSupport );

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat( swapChainSupport.formats,
                                                                swapChainSupport.format_count );

    VkPresentModeKHR presentMode = chooseSwapPresentMode( swapChainSupport.presentModes,
                                                          swapChainSupport.presentmode_count );

    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    if ( swapChainSupport.capabilities.maxImageCount > 0 &&
         imageCount > swapChainSupport.capabilities.maxImageCount )
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {0};

    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices qfIndices = findQueueFamilies( physicalDevice );

    uint32_t queueFamilyIndices[] = { qfIndices.graphicsFamily,
                                      qfIndices.presentFamily };

    if ( qfIndices.graphicsFamily != qfIndices.presentFamily )
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    if ( vkCreateSwapchainKHR( logicalDevice,
                               &createInfo,
                               NULL,
                               &swapChain ) != VK_SUCCESS )
    {
        printf("Error: failed to create swap chain!\n");
        return false;
    }

    vkGetSwapchainImagesKHR( logicalDevice,
                             swapChain,
                             &imageCount,
                             NULL );

    swapChainImages =  malloc( imageCount * sizeof *swapChainImages );

    if ( !swapChainImages )
    {
        printf("Error: cannot allocate memory (3).\n");
        return false;
    }

    swapchain_image_count = imageCount;

    vkGetSwapchainImagesKHR( logicalDevice,
                             swapChain,
                             &imageCount,
                             swapChainImages );

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;

    // TODO:
    free( swapChainSupport.formats );
    free( swapChainSupport.presentModes );

    return true;
}

/*
================================================
 FUNCTION NAME: createImageViews
================================================
*/

bool createImageViews()
{
    swapChainImageViews = calloc( swapchain_image_count, sizeof *swapChainImageViews );

    if ( !swapChainImageViews )
    {
        printf("Error: cannot allocate memory (4).\n");
        return false;
    }

    for ( uint32_t i = 0; i < swapchain_image_count; i++ )
    {
        swapChainImageViews[i] = createImageView( swapChainImages[i],
                                                  swapChainImageFormat,
                                                  VK_IMAGE_ASPECT_COLOR_BIT,
                                                  1 );

        if ( !swapChainImageViews[i] )
        {
            printf("Error: cannot create swap chain image view.\n");
            return false;
        }

        swapchain_imageviews_count++;
    }

    //swapchain_imageviews_count = swapchain_image_count;

    return true;
}

/*
================================================
 FUNCTION NAME: createRenderPass
================================================
*/

bool createRenderPass()
{

    VkAttachmentDescription colorAttachment = {0};

    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = msaaSamples;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkFormat depthFormat;

    if ( !findDepthFormat( &depthFormat ) )
    {
        printf("Error: createRenderPass, findDepthFormat\n");
        return false;
    }

    VkAttachmentDescription depthAttachment = {0};

    depthAttachment.format = depthFormat;
    depthAttachment.samples = msaaSamples;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription colorAttachmentResolve = {0};

    colorAttachmentResolve.format = swapChainImageFormat;
    colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {0};

    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef = {0};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentResolveRef = {0};
    colorAttachmentResolveRef.attachment = 2;
    colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {0};

    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;
    subpass.pResolveAttachments = &colorAttachmentResolveRef;

    VkSubpassDependency dependency = {0};

    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

    dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkAttachmentDescription attachments[3] = { colorAttachment,
                                               depthAttachment,
                                               colorAttachmentResolve };

    VkRenderPassCreateInfo renderPassInfo = {0};

    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = sizeof attachments / sizeof attachments[0];
    renderPassInfo.pAttachments = attachments;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if ( vkCreateRenderPass( logicalDevice,
                             &renderPassInfo,
                             NULL,
                             &renderPass )
        != VK_SUCCESS )
    {
        printf("Error: failed to create render pass!\n");
        return false;
    }

    return true;
}

/*
================================================
 FUNCTION NAME: createDescriptorSetLayout
================================================
*/

bool createDescriptorSetLayout()
{

    VkDescriptorSetLayoutBinding uboLayoutBinding = {0};

    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.pImmutableSamplers = NULL;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding samplerLayoutBinding = {0};

    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = NULL;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding bindings[2] = { uboLayoutBinding,
                                                 samplerLayoutBinding };

    VkDescriptorSetLayoutCreateInfo layoutInfo = {0};

    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = sizeof bindings / sizeof bindings[0];
    layoutInfo.pBindings = bindings;

    if (vkCreateDescriptorSetLayout( logicalDevice,
                                     &layoutInfo,
                                     NULL,
                                     &descriptorSetLayout )
        != VK_SUCCESS)
    {
        printf("Error: failed to create descriptor set layout!\n");
        return false;
    }

    return true;
}

/*
================================================
 FUNCTION NAME: loadShader
================================================
*/

bool loadShader( const char *shader_file,
                 VkShaderModule *shader_module )
{

    S_read_buffer s_ret;

    if ( !readFile( &s_ret, shader_file ) )
    {
        printf("Error: cannot read shader file: %s\n", shader_file );
        return false;
    }

    char *shader_code = s_ret.p_buffer;
    size_t shader_code_size = s_ret.size;

    if ( !createShaderModule( shader_code,
                              shader_code_size,
                              shader_module ))
    {
        printf("Error: failed to create shader module.\n");
        free( shader_code );
        return false;
    }

    free( shader_code );

    return true;
}

/*
================================================
 FUNCTION NAME: createGraphicsPipeline
================================================
*/

bool createGraphicsPipeline()
{

    VkShaderModule vertShaderModule;

    if ( !loadShader( "shaders/vert.spv", &vertShaderModule ) )
    {
        printf("Error: cannot load vert.spv\n");
        return false;
    }

    VkShaderModule fragShaderModule;

    if ( !loadShader( "shaders/frag.spv", &fragShaderModule ) )
    {
        printf("Error: cannot load frag.spv\n");
        vkDestroyShaderModule(logicalDevice, vertShaderModule, NULL);
        return false;
    }

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {0};

    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {0};

    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo,
                                                       fragShaderStageInfo };

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {0};

    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    // Binding Description
    
    VkVertexInputBindingDescription bindingDescription = {0};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof( Vertex );
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;


    // Attribute Descriptions

    #define ATTRIBUTE_DESCRIPTION_COUNT 3

    VkVertexInputAttributeDescription attributeDescriptions[ATTRIBUTE_DESCRIPTION_COUNT] = {0};

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, texCoord);


    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = ATTRIBUTE_DESCRIPTION_COUNT;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {0};

    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState = {0};

    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer = {0};

    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling = {0};

    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = msaaSamples;

    VkPipelineDepthStencilStateCreateInfo depthStencil = {0};

    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {0};

    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                          VK_COLOR_COMPONENT_G_BIT |
                                          VK_COLOR_COMPONENT_B_BIT |
                                          VK_COLOR_COMPONENT_A_BIT;

    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending = {0};

    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState = {0};

    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = sizeof dynamicStates / sizeof dynamicStates[0];
    dynamicState.pDynamicStates = dynamicStates;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {0};

    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

    if ( vkCreatePipelineLayout( logicalDevice,
                                 &pipelineLayoutInfo,
                                 NULL,
                                 &pipelineLayout )
        != VK_SUCCESS)
    {
        printf("Error: failed to create pipeline layout!\n");
        return false;
    }

    VkGraphicsPipelineCreateInfo pipelineInfo = {0};

    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if ( vkCreateGraphicsPipelines( logicalDevice,
                                    VK_NULL_HANDLE,
                                    1,
                                    &pipelineInfo,
                                    NULL,
                                    &graphicsPipeline )
        != VK_SUCCESS)
    {
        printf("Error: failed to create graphics pipeline!\n");
        return false;
    }

    vkDestroyShaderModule(logicalDevice, fragShaderModule, NULL);
    vkDestroyShaderModule(logicalDevice, vertShaderModule, NULL);

    return true;

}

/*
================================================
 FUNCTION NAME: createFramebuffers
================================================
*/

bool createFramebuffers()
{

    swapChainFramebuffers = malloc( swapchain_imageviews_count *
                                    sizeof *swapChainFramebuffers );

    if ( !swapChainFramebuffers )
    {
        printf("Error: cannot allocate memory (5).\n");
        return false;
    }

    swapchain_framebuffers_count = swapchain_imageviews_count;

    for (size_t i = 0; i < swapchain_imageviews_count; i++)
    {
        VkImageView attachments[] = { colorImageView,
                                      depthImageView,
                                      swapChainImageViews[i] };

        VkFramebufferCreateInfo framebufferInfo = {0};

        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = sizeof attachments / sizeof attachments[0];
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer( logicalDevice,
                                 &framebufferInfo,
                                 NULL,
                                 &swapChainFramebuffers[i] )
            != VK_SUCCESS )
        {
            printf("Error: failed to create framebuffer!\n");
            return false;
        }
    }

    return true;
}

/*
================================================
 FUNCTION NAME: createCommandPool
================================================
*/

bool createCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

    VkCommandPoolCreateInfo poolInfo = {0};

    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;

    if ( vkCreateCommandPool( logicalDevice,
                              &poolInfo,
                              NULL,
                              &commandPool )
            != VK_SUCCESS)
    {
        printf("Error: failed to create graphics command pool!\n");
        return false;
    }

    return true;
}

/*
================================================
 FUNCTION NAME: createColorResources
================================================
*/

bool createColorResources()
{

    VkFormat colorFormat = swapChainImageFormat;

    if ( !createImage( swapChainExtent.width,
                 swapChainExtent.height,
                 1,
                 msaaSamples,
                 colorFormat,
                 VK_IMAGE_TILING_OPTIMAL,
                 VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
                 VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 &colorImage,
                 &colorImageMemory ) )
    {
        printf("Error: createColorResources, createImage.\n");
        return false;
    }

    colorImageView = createImageView( colorImage,
                                      colorFormat,
                                      VK_IMAGE_ASPECT_COLOR_BIT,
                                      1 );

    if ( !colorImageView )
    {
        printf("Error: createColorResources, createImageView.\n");
        return false;
    }

    return true;
}

/*
================================================
 FUNCTION NAME: createDepthResources
================================================
*/

bool createDepthResources()
{

    VkFormat depthFormat;

    if ( !findDepthFormat( &depthFormat ) )
    {
        printf("Error: createDepthResources, findDepthFormat.\n");
        return false;
    }    

    if ( !createImage( swapChainExtent.width,
                 swapChainExtent.height,
                 1,
                 msaaSamples,
                 depthFormat,
                 VK_IMAGE_TILING_OPTIMAL,
                 VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 &depthImage,
                 &depthImageMemory ) )
    {
        printf("Error: createDepthResources, createImage.\n");
        return false;
    }

    depthImageView = createImageView( depthImage,
                                      depthFormat,
                                      VK_IMAGE_ASPECT_DEPTH_BIT,
                                      1 );

    if ( depthImageView )
    {
        return true;
    }

    printf("Error: createDepthResources, createImageView.\n");

    return false;
}

/*
================================================
 FUNCTION NAME: findSupportedFormat
================================================
*/

bool findSupportedFormat( VkFormat *pFormat,
                          VkFormat *candidates,
                          uint32_t format_num,
                          VkImageTiling tiling,
                          VkFormatFeatureFlags features )
{

    VkFormat format = 0;

    for ( uint32_t i = 0; i < format_num; ++i )
    {
        format = candidates[i];

        VkFormatProperties props;

        vkGetPhysicalDeviceFormatProperties( physicalDevice,
                                             format,
                                             &props );

        if ( tiling == VK_IMAGE_TILING_LINEAR &&
             ( props.linearTilingFeatures & features )
            == features )
        {
            *pFormat = format;
            return true;
        }
        else if ( tiling == VK_IMAGE_TILING_OPTIMAL &&
                  ( props.optimalTilingFeatures & features )
            == features )
        {
            *pFormat = format;
            return true;
        }
    }

    printf("Error: failed to find supported format!\n");

    *pFormat = format;
    return false;
}

/*
================================================
 FUNCTION NAME: findDepthFormat
================================================
*/

bool findDepthFormat( VkFormat *pFormat )
{
    VkFormat format;

    if ( !findSupportedFormat( &format,
                               (VkFormat[]){ VK_FORMAT_D32_SFLOAT,
                                              VK_FORMAT_D32_SFLOAT_S8_UINT,
                                              VK_FORMAT_D24_UNORM_S8_UINT },
                                3,
                                VK_IMAGE_TILING_OPTIMAL,
                                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT ) )
    {
        printf("Error: findDepthFormat, findSupportedFormat.\n");
        *pFormat = 0;
        return false;
    }

    *pFormat = format;
    return true;
}

/*
================================================
 FUNCTION NAME: hasStencilComponent
================================================
*/

bool hasStencilComponent( VkFormat format )
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
           format == VK_FORMAT_D24_UNORM_S8_UINT;
}

/*
================================================
 FUNCTION NAME: createTextureImage
================================================
*/

bool createTextureImage()
{
    int texWidth, texHeight, texChannels;

    stbi_uc *pixels = stbi_load( texture_path,
                                 &texWidth,
                                 &texHeight,
                                 &texChannels,
                                 STBI_rgb_alpha );

    if ( !pixels )
    {
        printf("Error: failed to load texture image!\n");
        return false;
    }

    VkDeviceSize imageSize = texWidth * texHeight * 4;

    mipmapLevels = (uint32_t) floor( log2( fmax( texWidth, texHeight ) ) ) + 1;

    VkBuffer stagingBuffer;

    VkDeviceMemory stagingBufferMemory;

    if ( !createBuffer( imageSize,
                        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        &stagingBuffer,
                        &stagingBufferMemory ) )
    {
        return false;
    }

    void *data;

    vkMapMemory( logicalDevice,
                 stagingBufferMemory,
                 0,
                 imageSize,
                 0,
                 &data );

    memcpy( data, pixels, imageSize );

    vkUnmapMemory( logicalDevice,
                   stagingBufferMemory );

    stbi_image_free( pixels );

    if ( !createImage( texWidth,
                 texHeight,
                 mipmapLevels,
                 VK_SAMPLE_COUNT_1_BIT,
                 VK_FORMAT_R8G8B8A8_SRGB,
                 VK_IMAGE_TILING_OPTIMAL,
                 VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                 VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                 VK_IMAGE_USAGE_SAMPLED_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 &textureImage,
                 &textureImageMemory ) )
    {
        printf("Error: createTextureImage, createImage.\n");
        return false;
    }

    if ( !transitionImageLayout( textureImage,
                                 VK_FORMAT_R8G8B8A8_SRGB,
                                 VK_IMAGE_LAYOUT_UNDEFINED,
                                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                 mipmapLevels ) )
    {
        return false;
    }

    copyBufferToImage( stagingBuffer,
                       textureImage,
                       (uint32_t) texWidth,
                       (uint32_t) texHeight );

    //transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps

    vkDestroyBuffer( logicalDevice, stagingBuffer, NULL );
    vkFreeMemory( logicalDevice, stagingBufferMemory, NULL );

    if ( !generateMipmaps( textureImage,
                           VK_FORMAT_R8G8B8A8_SRGB,
                           texWidth,
                           texHeight,
                           mipmapLevels ) )
    {
        printf("Error: createTextureImage, generateMipmaps.\n");
        return false;
    }

    return true;
}

/*
================================================
 FUNCTION NAME: generateMipmaps
================================================
*/

bool generateMipmaps( VkImage image,
                      VkFormat imageFormat,
                      int32_t texWidth,
                      int32_t texHeight,
                      uint32_t mipLevels )
{
    // Check if image format supports linear blitting
    VkFormatProperties formatProperties;

    vkGetPhysicalDeviceFormatProperties( physicalDevice,
                                         imageFormat,
                                         &formatProperties );

    if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
    {
        printf("Error: texture image format does not support linear blitting!\n");
        return false;
    }

    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkImageMemoryBarrier barrier = {0};

    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    int32_t mipWidth = texWidth;
    int32_t mipHeight = texHeight;

    for (uint32_t i = 1; i < mipLevels; i++)
    {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier( commandBuffer,
                              VK_PIPELINE_STAGE_TRANSFER_BIT,
                              VK_PIPELINE_STAGE_TRANSFER_BIT,
                              0,
                              0,
                              NULL,
                              0,
                              NULL,
                              1,
                              &barrier );

        VkImageBlit blit = {0};

        blit.srcOffsets[0] = (VkOffset3D){0, 0, 0};
        blit.srcOffsets[1] = (VkOffset3D){ mipWidth, mipHeight, 1 };
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = (VkOffset3D){0, 0, 0};
        blit.dstOffsets[1] = (VkOffset3D){ mipWidth > 1 ? mipWidth / 2 : 1,
                                           mipHeight > 1 ? mipHeight / 2 : 1,
                                           1 };
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        vkCmdBlitImage( commandBuffer,
                        image,
                        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                        image,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        1,
                        &blit,
                        VK_FILTER_LINEAR );

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier( commandBuffer,
                              VK_PIPELINE_STAGE_TRANSFER_BIT,
                              VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                              0,
                              0,
                              NULL,
                              0,
                              NULL,
                              1,
                              &barrier );

        if (mipWidth > 1)
        {
            mipWidth /= 2;
        }

        if (mipHeight > 1)
        {
            mipHeight /= 2;
        }
    }

    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier( commandBuffer,
                          VK_PIPELINE_STAGE_TRANSFER_BIT,
                          VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                          0,
                          0,
                          NULL,
                          0,
                          NULL,
                          1,
                          &barrier );

    endSingleTimeCommands( commandBuffer );

    return true;
}

/*
================================================
 FUNCTION NAME: getMaxUsableSampleCount
================================================
*/

VkSampleCountFlagBits getMaxUsableSampleCount()
{
    VkPhysicalDeviceProperties physicalDeviceProperties;

    vkGetPhysicalDeviceProperties( physicalDevice, &physicalDeviceProperties );

    VkSampleCountFlags counts =
        physicalDeviceProperties.limits.framebufferColorSampleCounts &
        physicalDeviceProperties.limits.framebufferDepthSampleCounts;

    if ( counts & VK_SAMPLE_COUNT_64_BIT )
    {
        return VK_SAMPLE_COUNT_64_BIT;
    }

    if ( counts & VK_SAMPLE_COUNT_32_BIT )
    {
        return VK_SAMPLE_COUNT_32_BIT;
    }

    if ( counts & VK_SAMPLE_COUNT_16_BIT )
    {
        return VK_SAMPLE_COUNT_16_BIT;
    }

    if ( counts & VK_SAMPLE_COUNT_8_BIT )
    {
        return VK_SAMPLE_COUNT_8_BIT;
    }

    if ( counts & VK_SAMPLE_COUNT_4_BIT )
    {
        return VK_SAMPLE_COUNT_4_BIT;
    }

    if ( counts & VK_SAMPLE_COUNT_2_BIT )
    {
        return VK_SAMPLE_COUNT_2_BIT;
    }

    return VK_SAMPLE_COUNT_1_BIT;
}

/*
================================================
 FUNCTION NAME: createTextureImageView
================================================
*/

bool createTextureImageView()
{
    textureImageView = createImageView( textureImage,
                                        VK_FORMAT_R8G8B8A8_SRGB,
                                        VK_IMAGE_ASPECT_COLOR_BIT,
                                        mipmapLevels );

    if ( !textureImageView )
    {
        printf("Error: failed to create texture image view.\n");
        return false;
    }

    return true;
}

/*
================================================
 FUNCTION NAME: createTextureSampler
================================================
*/

bool createTextureSampler()
{
    VkPhysicalDeviceProperties properties = {0};

    vkGetPhysicalDeviceProperties( physicalDevice, &properties );

    VkSamplerCreateInfo samplerInfo = {0};

    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = VK_LOD_CLAMP_NONE;
    samplerInfo.mipLodBias = 0.0f;

    if ( vkCreateSampler( logicalDevice,
                          &samplerInfo,
                          NULL,
                          &textureSampler ) != VK_SUCCESS )
    {
        printf("Error: failed to create texture sampler!\n");
        return false;
    }

    return true;
}

/*
================================================
 FUNCTION NAME: createImageView
================================================
*/

VkImageView createImageView( VkImage image,
                             VkFormat format,
                             VkImageAspectFlags aspectFlags,
                             uint32_t mipLevels )
{
    VkImageViewCreateInfo viewInfo = {0};

    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;

    if ( vkCreateImageView( logicalDevice,
                            &viewInfo,
                            NULL,
                            &imageView ) != VK_SUCCESS )
    {
        printf("Error: failed to create image view!\n");
        return NULL;
    }

    return imageView;
}

/*
================================================
 FUNCTION NAME: createImage
================================================
*/

bool createImage( uint32_t width,
                  uint32_t height,
                  uint32_t mipLevels,
                  VkSampleCountFlagBits numSamples,
                  VkFormat format,
                  VkImageTiling tiling,
                  VkImageUsageFlags usage,
                  VkMemoryPropertyFlags properties,
                  VkImage *p_image,
                  VkDeviceMemory *p_imageMemory )
{
    VkImageCreateInfo imageInfo = {0};

    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = numSamples;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if ( vkCreateImage( logicalDevice,
                        &imageInfo,
                        NULL,
                        p_image ) != VK_SUCCESS )
    {
        printf("Error: failed to create image!\n");
        return false;
    }

    VkMemoryRequirements memRequirements;

    vkGetImageMemoryRequirements( logicalDevice,
                                  *p_image,
                                  &memRequirements );

    VkMemoryAllocateInfo allocInfo = {0};

    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType( memRequirements.memoryTypeBits,
                                                properties );

    if ( vkAllocateMemory( logicalDevice, &allocInfo, NULL, p_imageMemory) != VK_SUCCESS )
    {
        printf("Error: failed to allocate image memory!\n");
        return false;
    }

    vkBindImageMemory( logicalDevice, *p_image, *p_imageMemory, 0 );

    return true;
}

/*
================================================
 FUNCTION NAME: transitionImageLayout
================================================
*/

bool transitionImageLayout( VkImage image,
                            VkFormat format,
                            VkImageLayout oldLayout,
                            VkImageLayout newLayout,
                            uint32_t mipLevels )
{
    /* Silence compiler warning for unused parameter format */
    (void)format;

    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkImageMemoryBarrier barrier = {0};

    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage = 0;
    VkPipelineStageFlags destinationStage = 0;

    if ( oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
         newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL )
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if ( oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
              newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL )
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        printf("Error: unsupported layout transition!\n");
        return false;
    }

    vkCmdPipelineBarrier( commandBuffer,
                          sourceStage,
                          destinationStage,
                          0,
                          0,
                          NULL,
                          0,
                          NULL,
                          1,
                          &barrier );

    endSingleTimeCommands( commandBuffer );

    return true;
}

/*
================================================
 FUNCTION NAME: copyBufferToImage
================================================
*/

void copyBufferToImage( VkBuffer buffer,
                        VkImage image,
                        uint32_t width,
                        uint32_t height )
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferImageCopy region = {0};

    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = (VkOffset3D){ 0, 0, 0 };
    region.imageExtent = (VkExtent3D){ width, height, 1 };

    vkCmdCopyBufferToImage( commandBuffer,
                            buffer,
                            image,
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                            1,
                            &region );

    endSingleTimeCommands( commandBuffer );
}

/*
================================================
 FUNCTION NAME: fileReaderCallback
================================================
*/

static void fileReaderCallback( void *context,
                                const char *file_name,
                                const int mtl_flag,
                                const char *obj_file_name,
                                char **content_buffer,
                                size_t *size_of_content )
{
    printf( "fileReaderCallback().\n" );
    printf( "mtl flag %d\n", mtl_flag );
    printf( "file name %s\n", file_name );
    printf( "obj file name %s\n\n", obj_file_name );

    FILE *fp;

    fp = fopen( file_name, "rb" );

    if ( !fp )
    {
        printf( "Error: failed to open file \"%s\" !\n", file_name );
        *content_buffer = NULL;
        *size_of_content = 0;
        return;
    }

    fseek( fp, 0L, SEEK_END );

    size_t file_size = ftell(fp);

    char *buffer = malloc( file_size );

    if ( !buffer )
    {
        printf( "Error: cannot allocate memory (6).\n" );
        *content_buffer = NULL;
        *size_of_content = 0;
        return;
    }

    fseek( fp, 0, SEEK_SET );

    size_t num = fread( buffer, sizeof *buffer, file_size, fp );

    fclose(fp);

    if ( num != file_size )
    {
        printf("Error: an error occurred while reading the file.\n");
        free( buffer );
        *content_buffer = NULL;
        *size_of_content = 0;
        return;
    }

    *content_buffer = buffer;
    *size_of_content = file_size;

    ((S_ctx*) context)->p_buffer[((S_ctx*) context)->num] = buffer;
    ((S_ctx*) context)->num++;
}

/*
================================================
 FUNCTION NAME: fltAEqual
================================================
*/

bool fltAEqual( float a, float b, float epsilon )
{
    return fabsf( a - b ) < epsilon;
}

/*
================================================
 FUNCTION NAME: loadModel
================================================
*/

bool loadModel()
{
    tinyobj_attrib_t attrib;
    tinyobj_shape_t *shapes;
    size_t num_shapes;
    tinyobj_material_t *materials;
    size_t num_materials;
    void *context;
    unsigned int flags = TINYOBJ_FLAG_TRIANGULATE;

    S_ctx sctx = {0};

    context = &sctx;

    int retv = tinyobj_parse_obj( &attrib,
                                  &shapes,
                                  &num_shapes,
                                  &materials,
                                  &num_materials,
                                  model_path,
                                  fileReaderCallback,
                                  context,
                                  flags );

    if ( retv != TINYOBJ_SUCCESS )
    {
        printf("Error: tinyobj parse error.\n");
        return false;
    }

    printf( "Number of vertices: %u\n", attrib.num_vertices );
    printf( "Number of normals: %u\n", attrib.num_normals );
    printf( "Number of texcoords: %u\n", attrib.num_texcoords );
    printf( "Number of vertex indices: %u\n", attrib.num_faces );
    printf( "Number of faces: %u\n", attrib.num_face_num_verts );
    printf( "Number of materials: %ld\n", num_materials );
    printf( "Number of shapes: %ld\n\n", num_shapes );

    //TODO: use shape
/*
    for ( size_t i = 0; i < num_shapes; ++i )
    {
        printf( "Shape [%ld]:\n", i );
        printf( "name: %s\n", shapes[i].name );

        unsigned int offset = shapes[i].face_offset;

        unsigned int length = shapes[i].length;

        printf( "face offset: %u\n", offset );
        printf( "length: %u\n\n", length );

        // ...
    }
*/

    unsigned int offset = 0;

    unsigned int number_of_faces = attrib.num_face_num_verts;

    for( unsigned int i = offset; i < offset + number_of_faces; ++i )
    {
        // triangulated, every face has 3 vertices

        for ( unsigned int j = 0; j < 3; ++j )
        {

            Vertex vertex = {0};
            
            tinyobj_vertex_index_t vert_idx = attrib.faces[ i * 3 + j ];

            int indx_v = vert_idx.v_idx;

            // every vertice float x, y, z
            // vertex array offset indx_v * 3

            float x = attrib.vertices[ indx_v * 3 ];
            float y = attrib.vertices[ indx_v * 3 + 1 ];
            float z = attrib.vertices[ indx_v * 3 + 2 ];

            vertex.pos[0] = x;
            vertex.pos[1] = y;
            vertex.pos[2] = z;

            vertex.color[0] = 1.0f;
            vertex.color[1] = 1.0f;
            vertex.color[2] = 1.0f;

            int indx_vt = vert_idx.vt_idx;

            float u = attrib.texcoords[ indx_vt * 2 ];
            float v = 1.0f - attrib.texcoords[ indx_vt * 2 + 1 ];

            vertex.texCoord[0] = u;
            vertex.texCoord[1] = v;

            bool flag_sutampa = false;

            uint32_t vertices_indeksas = 0;

            for ( size_t bi = 0; bi < vertices_size; ++bi )
            {
                if ( fltAEqual ( vertices[bi].pos[0], vertex.pos[0], FLT_EPSILON ) &&
                     fltAEqual ( vertices[bi].pos[1], vertex.pos[1], FLT_EPSILON ) &&
                     fltAEqual ( vertices[bi].pos[2], vertex.pos[2], FLT_EPSILON ) &&
                     fltAEqual ( vertices[bi].texCoord[0], vertex.texCoord[0], FLT_EPSILON ) &&
                     fltAEqual ( vertices[bi].texCoord[1], vertex.texCoord[1], FLT_EPSILON )
                   )
                {
                    flag_sutampa = true;
                    vertices_indeksas = bi;
                    break;
                }
            }       


            if( !flag_sutampa )
            {
                vertices_indeksas = vertices_size;

                Vertex *p_tmp_vertices = NULL;

                vertices_size++;

                p_tmp_vertices = realloc( vertices, vertices_size * sizeof *vertices );

                if( !p_tmp_vertices )
                {
                    printf("Error: memory reallocation error (1).\n");
                    free(vertices);
                    vertices = NULL;
                    vertices_size = 0;
                }
                else
                {
                    vertices = p_tmp_vertices;
                
                    vertices[vertices_indeksas].pos[0] = vertex.pos[0];
                    vertices[vertices_indeksas].pos[1] = vertex.pos[1];
                    vertices[vertices_indeksas].pos[2] = vertex.pos[2];
                    vertices[vertices_indeksas].color[0] = vertex.color[0];
                    vertices[vertices_indeksas].color[1] = vertex.color[1];
                    vertices[vertices_indeksas].color[2] = vertex.color[2];
                    vertices[vertices_indeksas].texCoord[0] = vertex.texCoord[0];
                    vertices[vertices_indeksas].texCoord[1] = vertex.texCoord[1];
                }
            }

            uint32_t *p_tmp_indices = NULL;

            indices_size = i * 3 + j + 1;

            p_tmp_indices = realloc( indices, indices_size * sizeof *indices );
                
            if( !p_tmp_indices )
            {
                printf("Error: memory reallocation error (2).\n");
                free(indices);
                indices = NULL;
                indices_size = 0;
            }
            else
            {
                indices = p_tmp_indices;
                indices[ i * 3 + j ] = vertices_indeksas;
            }

        }
    }

    tinyobj_attrib_free( &attrib );
    tinyobj_shapes_free( shapes, num_shapes );
    tinyobj_materials_free( materials, num_materials );

    for ( uint32_t i = 0; i < sctx.num; ++i )
    {
        if ( sctx.p_buffer[i] )
        {
            printf("free buffer (%u).\n", i );
            free( sctx.p_buffer[i] );
        }
    }

    return true;
}

/*
================================================
 FUNCTION NAME: createVertexBuffer
================================================
*/

bool createVertexBuffer()
{

    VkDeviceSize bufferSize = sizeof *vertices * vertices_size;

    VkBuffer stagingBuffer;

    VkDeviceMemory stagingBufferMemory;

    if ( !createBuffer( bufferSize,
                  VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                  &stagingBuffer,
                  &stagingBufferMemory ) )
    {
        return false;
    }

    void* data;

    vkMapMemory( logicalDevice,
                 stagingBufferMemory,
                 0,
                 bufferSize,
                 0,
                 &data );

    memcpy( data, vertices, (size_t) bufferSize );

    vkUnmapMemory( logicalDevice, stagingBufferMemory );

    if ( !createBuffer( bufferSize,
                  VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                  VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                  &vertexBuffer,
                  &vertexBufferMemory ) )
    {
        return false;
    }

    copyBuffer( stagingBuffer, vertexBuffer, bufferSize );

    vkDestroyBuffer( logicalDevice, stagingBuffer, NULL );

    vkFreeMemory( logicalDevice, stagingBufferMemory, NULL );

    return true;
}

/*
================================================
 FUNCTION NAME: createIndexBuffer
================================================
*/

bool createIndexBuffer()
{
    VkDeviceSize bufferSize = sizeof *indices * indices_size;

    VkBuffer stagingBuffer;

    VkDeviceMemory stagingBufferMemory;

    if ( !createBuffer( bufferSize,
                  VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                  &stagingBuffer,
                  &stagingBufferMemory ) )
    {
        return false;
    }

    void* data;

    vkMapMemory( logicalDevice,
                 stagingBufferMemory,
                 0,
                 bufferSize,
                 0,
                 &data );

    memcpy( data, indices, (size_t) bufferSize );

    vkUnmapMemory( logicalDevice, stagingBufferMemory );

    if ( !createBuffer( bufferSize,
                  VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                  VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                  &indexBuffer,
                  &indexBufferMemory ) )
    {
        return false;
    }

    copyBuffer( stagingBuffer, indexBuffer, bufferSize );

    vkDestroyBuffer( logicalDevice, stagingBuffer, NULL );

    vkFreeMemory( logicalDevice, stagingBufferMemory, NULL );

    return true;
}

/*
================================================
 FUNCTION NAME: createUniformBuffers
================================================
*/

bool createUniformBuffers()
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    //TODO: check malloc
    uniformBuffers = malloc( sizeof(VkBuffer) * max_frames_in_flight );
    uniformBuffersMemory = malloc( sizeof(VkDeviceMemory) * max_frames_in_flight );
    uniformBuffersMapped = malloc( sizeof (void*) * max_frames_in_flight );

    for ( int i = 0; i < max_frames_in_flight; i++)
    {
        //TODO: check for errors
        if ( !createBuffer( bufferSize,
                      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                      VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                      &uniformBuffers[i],
                      &uniformBuffersMemory[i] ) )
        {
            return false;
        }

        vkMapMemory( logicalDevice,
                     uniformBuffersMemory[i],
                     0,
                     bufferSize,
                     0,
                     &uniformBuffersMapped[i] );
    }

    return true;
}

/*
================================================
 FUNCTION NAME: createDescriptorPool
================================================
*/

bool createDescriptorPool()
{
    VkDescriptorPoolSize poolSizes[2] = {0};

    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = (uint32_t)max_frames_in_flight;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = (uint32_t)max_frames_in_flight;

    VkDescriptorPoolCreateInfo poolInfo = {0};

    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = sizeof poolSizes / sizeof poolSizes[0];
    poolInfo.pPoolSizes = poolSizes;
    poolInfo.maxSets = (uint32_t)max_frames_in_flight;

    if ( vkCreateDescriptorPool( logicalDevice,
                                 &poolInfo,
                                 NULL,
                                 &descriptorPool ) != VK_SUCCESS )
    {
        printf("Error: failed to create descriptor pool!\n");
        return false;
    }

    return true;
}

/*
================================================
 FUNCTION NAME: createDescriptorSets
================================================
*/

bool createDescriptorSets()
{
    VkDescriptorSetLayout layouts[max_frames_in_flight];

    for ( int i = 0; i < max_frames_in_flight; ++i )
    {
        layouts[i] = descriptorSetLayout;
    }

    VkDescriptorSetAllocateInfo allocInfo = {0};

    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = (uint32_t)max_frames_in_flight;
    allocInfo.pSetLayouts = layouts;

    descriptorSets = malloc( sizeof(VkDescriptorSet) * max_frames_in_flight );
    
    if ( !descriptorSets )
    {
        printf("Error: cannot allocate memory (7).\n");
        return false;
    }

    if ( vkAllocateDescriptorSets( logicalDevice,
                                   &allocInfo,
                                   descriptorSets )
        != VK_SUCCESS )
    {
        printf("Error: failed to allocate descriptor sets!\n");
        return false;
    }

    for ( int i = 0; i < max_frames_in_flight; i++ )
    {
        VkDescriptorBufferInfo bufferInfo = {0};

        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkDescriptorImageInfo imageInfo = {0};

        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = textureImageView;
        imageInfo.sampler = textureSampler;

        VkWriteDescriptorSet descriptorWrites[2] = {0};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;

        vkUpdateDescriptorSets( logicalDevice,
                                2,
                                descriptorWrites,
                                0,
                                NULL );
    }

    return true;
}

/*
================================================
 FUNCTION NAME: createBuffer
================================================
*/

bool createBuffer( VkDeviceSize size,
                   VkBufferUsageFlags usage,
                   VkMemoryPropertyFlags properties,
                   VkBuffer *buffer,
                   VkDeviceMemory *bufferMemory )
{
    VkBufferCreateInfo bufferInfo = {0};

    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer( logicalDevice, &bufferInfo, NULL, buffer) != VK_SUCCESS )
    {
        printf("Error: failed to create buffer!\n");
        return false;
    }

    VkMemoryRequirements memRequirements;

    vkGetBufferMemoryRequirements(logicalDevice, *buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {0};

    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType( memRequirements.memoryTypeBits,
                                                properties );

    if (vkAllocateMemory( logicalDevice, &allocInfo, NULL, bufferMemory ) != VK_SUCCESS)
    {
        printf("Error: failed to allocate buffer memory!\n");
        return false;
    }

    vkBindBufferMemory( logicalDevice, *buffer, *bufferMemory, 0 );

    return true;
}

/*
================================================
 FUNCTION NAME: beginSingleTimeCommands
================================================
*/

VkCommandBuffer beginSingleTimeCommands()
{
    VkCommandBufferAllocateInfo allocInfo = {0};

    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;

    vkAllocateCommandBuffers( logicalDevice, &allocInfo, &commandBuffer );

    VkCommandBufferBeginInfo beginInfo = {0};

    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer( commandBuffer, &beginInfo );

    return commandBuffer;
}

/*
================================================
 FUNCTION NAME: endSingleTimeCommands
================================================
*/

void endSingleTimeCommands( VkCommandBuffer commandBuffer )
{
    vkEndCommandBuffer( commandBuffer );

    VkSubmitInfo submitInfo = {0};

    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit( graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE );
    vkQueueWaitIdle( graphicsQueue );

    vkFreeCommandBuffers( logicalDevice, commandPool, 1, &commandBuffer );
}

/*
================================================
 FUNCTION NAME: copyBuffer
================================================
*/

void copyBuffer( VkBuffer srcBuffer,
                 VkBuffer dstBuffer,
                 VkDeviceSize size )
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferCopy copyRegion = {0};

    copyRegion.size = size;

    vkCmdCopyBuffer( commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion );

    endSingleTimeCommands( commandBuffer );
}

/*
================================================
 FUNCTION NAME: findMemoryType
================================================
*/

uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;

    vkGetPhysicalDeviceMemoryProperties( physicalDevice, &memProperties );

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ( ( typeFilter & (1 << i) ) &&
             ( memProperties.memoryTypes[i].propertyFlags & properties )
            == properties )
        {
            return i;
        }
    }

    //TODO: on error

    printf("Error: failed to find suitable memory type!\n");

    return 0;
}

/*
================================================
 FUNCTION NAME: createCommandBuffers
================================================
*/

bool createCommandBuffers()
{
    //TODO: check malloc
    commandBuffers = malloc( sizeof(VkCommandBuffer) * max_frames_in_flight );

    VkCommandBufferAllocateInfo allocInfo = {0};

    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = max_frames_in_flight;

    if (vkAllocateCommandBuffers( logicalDevice,
                                  &allocInfo,
                                  commandBuffers )
        != VK_SUCCESS )
    {
        printf("Error: failed to allocate command buffers!\n");
        return false;
    }

    return true;
}

/*
================================================
 FUNCTION NAME: recordCommandBuffer
================================================
*/

void recordCommandBuffer( VkCommandBuffer commandBuffer,
                          uint32_t imageIndex )
{
    VkCommandBufferBeginInfo beginInfo = {0};

    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        //TODO: exit on error
        printf("Error: failed to begin recording command buffer!\n");
    }

    VkRenderPassBeginInfo renderPassInfo = {0};

    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = (VkOffset2D){0, 0};
    renderPassInfo.renderArea.extent = swapChainExtent;

    VkClearValue clearValues[2] = {0};
    
    clearValues[0].color = (VkClearColorValue){{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = (VkClearDepthStencilValue){1.0f, 0};

    renderPassInfo.clearValueCount = 2;
    renderPassInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass( commandBuffer,
                          &renderPassInfo,
                          VK_SUBPASS_CONTENTS_INLINE );

    vkCmdBindPipeline( commandBuffer,
                       VK_PIPELINE_BIND_POINT_GRAPHICS,
                       graphicsPipeline );

    VkViewport viewport = {0};

    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) swapChainExtent.width;
    viewport.height = (float) swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vkCmdSetViewport( commandBuffer, 0, 1, &viewport );

    VkRect2D scissor = {0};

    scissor.offset = (VkOffset2D){ 0, 0 };
    scissor.extent = swapChainExtent;
    
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    VkBuffer vertexBuffers[] = {vertexBuffer};

    VkDeviceSize offsets[] = {0};
    
    vkCmdBindVertexBuffers( commandBuffer,
                            0,
                            1,
                            vertexBuffers,
                            offsets );

    vkCmdBindIndexBuffer( commandBuffer,
                          indexBuffer,
                          0,
                          VK_INDEX_TYPE_UINT32 );

    vkCmdBindDescriptorSets( commandBuffer,
                             VK_PIPELINE_BIND_POINT_GRAPHICS,
                             pipelineLayout,
                             0,
                             1,
                             &descriptorSets[currentFrame],
                             0,
                             NULL );

    vkCmdDrawIndexed( commandBuffer,
                      indices_size,
                      1,
                      0,
                      0,
                      0 );

    vkCmdEndRenderPass( commandBuffer );

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        //TODO: exit on error
        printf("Error: failed to record command buffer!\n");
    }
}

/*
================================================
 FUNCTION NAME: createSyncObjects
================================================
*/

bool createSyncObjects()
{

    imageAvailableSemaphores = malloc( max_frames_in_flight * sizeof *imageAvailableSemaphores );

    if( !imageAvailableSemaphores )
    {
        printf("Error: cannot allocate memory (8).\n");
        return false;
    }

    renderFinishedSemaphores =  malloc( max_frames_in_flight * sizeof *renderFinishedSemaphores );

    if( !renderFinishedSemaphores )
    {
        printf("Error: cannot allocate memory (9).\n");
        return false;
    }

    inFlightFences = malloc( max_frames_in_flight * sizeof *inFlightFences );

    if( !inFlightFences )
    {
        printf("Error: cannot allocate memory (10).\n");
        return false;
    }

    VkSemaphoreCreateInfo semaphoreInfo = {0};

    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {0};

    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for ( int i = 0; i < max_frames_in_flight; i++ )
    {
        if ( vkCreateSemaphore( logicalDevice,
                                &semaphoreInfo,
                                NULL,
                                &imageAvailableSemaphores[i] )
                != VK_SUCCESS ||

             vkCreateSemaphore( logicalDevice,
                                &semaphoreInfo,
                                NULL,
                                &renderFinishedSemaphores[i] )
                != VK_SUCCESS ||

             vkCreateFence( logicalDevice,
                            &fenceInfo,
                            NULL,
                            &inFlightFences[i] )
                != VK_SUCCESS)
        {
            printf("Error: failed to create synchronization objects for a frame!\n");
            return false;
        }
    }

    return true;
}

/*
================================================
 FUNCTION NAME: updateUniformBuffer
================================================
*/

void updateUniformBuffer( uint32_t currentImage )
{
    //TODO: windows compatible

    /*
      C++ from tutorial file
      static auto startTime = std::chrono::high_resolution_clock::now();

      auto currentTime = std::chrono::high_resolution_clock::now();
      float time = std::chrono::duration<float,
                   std::chrono::seconds::period>(currentTime - startTime).count();
    */

    struct timeval tv;

    double seconds;
    double microseconds;
    double tm_current;
    double tm_delta;
    static double tm_old = 0.0;

    gettimeofday(&tv, NULL);

    seconds = (double) tv.tv_sec;
    microseconds = (double) tv.tv_usec / 1000000;
    tm_current = seconds + microseconds;
    tm_delta = tm_current - tm_old;
    tm_old = tm_current;

    UniformBufferObject ubo = {0};

    static mat4 m = { { 1.0f, 0.0f, 0.0f, 0.0f },
                      { 0.0f, 1.0f, 0.0f, 0.0f },
                      { 0.0f, 0.0f, 1.0f, 0.0f },
                      { 0.0f, 0.0f, 0.0f, 1.0f } };

    glm_rotate ( m, tm_delta * glm_rad(90.0f), (vec3){0.0f, 0.0f, 1.0f} );

    glm_mat4_copy( m, ubo.model );

    glm_lookat( (vec3){7.0f, 7.0f, 4.0f},
                (vec3){0.0f, 0.0f, 1.0f},
                (vec3){0.0f, 0.0f, 1.0f},
                ubo.view );

    glm_perspective( glm_rad(45.0f),
                     swapChainExtent.width / (float) swapChainExtent.height,
                     0.1f,
                     15.0f,
                     ubo.proj
    ); 

    ubo.proj[1][1] *= -1;

    memcpy( uniformBuffersMapped[ currentImage ],
            &ubo,
            sizeof(ubo) );

}

/*
================================================
 FUNCTION NAME: drawFrame
================================================
*/

bool drawFrame()
{

    vkWaitForFences( logicalDevice,
                     1,
                     &inFlightFences[currentFrame],
                     VK_TRUE,
                     UINT64_MAX );

    uint32_t imageIndex;

    VkResult result = vkAcquireNextImageKHR( logicalDevice,
                                             swapChain,
                                             UINT64_MAX,
                                             imageAvailableSemaphores[currentFrame],
                                             VK_NULL_HANDLE,
                                             &imageIndex );

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        if ( !recreateSwapChain() )
        {
            return false;
        }

        return true;
    }
    else if ( result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR )
    {
        printf("Error: failed to acquire swap chain image!\n");
        return false;
    }

    updateUniformBuffer( currentFrame );

    vkResetFences( logicalDevice,
                   1,
                   &inFlightFences[currentFrame] );

    vkResetCommandBuffer( commandBuffers[currentFrame], 0 );

    recordCommandBuffer( commandBuffers[currentFrame],
                         imageIndex );

    VkSubmitInfo submitInfo = {0};

    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if ( vkQueueSubmit( graphicsQueue,
                        1,
                        &submitInfo,
                        inFlightFences[currentFrame] )
        != VK_SUCCESS)
    {
        printf("Error: failed to submit draw command buffer!\n");
        return false;
    }

    VkPresentInfoKHR presentInfo = {0};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR( presentQueue, &presentInfo );

    if ( result == VK_ERROR_OUT_OF_DATE_KHR ||
         result == VK_SUBOPTIMAL_KHR ||
         framebufferResized )
    {
        framebufferResized = false;
        
        if ( !recreateSwapChain() )
        {
            return false;
        }
    }
    else if (result != VK_SUCCESS)
    {
        printf("Error: failed to present swap chain image!\n");
        return false;
    }

    currentFrame = (currentFrame + 1) % max_frames_in_flight;

    return true;
}

/*
================================================
 FUNCTION NAME: createShaderModule
================================================
*/

bool createShaderModule( const char *code,
                         size_t code_size,
                         VkShaderModule *shaderModule )
{
    VkShaderModuleCreateInfo createInfo = {0};

    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code_size;
    createInfo.pCode = (const uint32_t*) code;

    if ( vkCreateShaderModule(logicalDevice, &createInfo, NULL, shaderModule) != VK_SUCCESS)
    {
        printf("Error: failed to create shader module!\n");
        return false;
    }

    return true;
}

/*
================================================
 FUNCTION NAME: chooseSwapSurfaceFormat
================================================
*/

VkSurfaceFormatKHR chooseSwapSurfaceFormat( VkSurfaceFormatKHR *availableFormats,
                                            uint32_t num_of_available_formats )
{
    for ( uint32_t i = 0; i < num_of_available_formats; ++i )
    {
        VkSurfaceFormatKHR availableFormat = availableFormats[i];

        if ( availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
             availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR )
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

/*
================================================
 FUNCTION NAME: chooseSwapPresentMode
================================================
*/

VkPresentModeKHR chooseSwapPresentMode( VkPresentModeKHR *availablePresentModes,
                                        uint32_t num_of_available_present_modes )
{
    for ( uint32_t i = 0; i < num_of_available_present_modes; ++i )
    {
        VkPresentModeKHR availablePresentMode = availablePresentModes[i];

        if ( availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR )
        {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

/*
================================================
 FUNCTION NAME: chooseSwapExtent
================================================
*/

VkExtent2D chooseSwapExtent( VkSurfaceCapabilitiesKHR capabilities )
{
    if ( capabilities.currentExtent.width != UINT32_MAX )
    {
        return capabilities.currentExtent;
    }
    else
    {
        int width, height;

        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = {
            width,
            height
        };

        actualExtent.width = clampUintVal( actualExtent.width,
                                           capabilities.minImageExtent.width,
                                           capabilities.maxImageExtent.width );

        actualExtent.height = clampUintVal( actualExtent.height,
                                            capabilities.minImageExtent.height,
                                            capabilities.maxImageExtent.height );

        return actualExtent;
    }
}

/*
================================================
 FUNCTION NAME: querySwapChainSupport
================================================
*/

bool querySwapChainSupport( VkPhysicalDevice device,
                            SwapChainSupportDetails *details )
{

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR( device,
                                               surface,
                                               &details->capabilities );

    uint32_t formatCount;

    vkGetPhysicalDeviceSurfaceFormatsKHR( device,
                                          surface,
                                          &formatCount,
                                          NULL );

    //TODO: if formatCount zero
    details->format_count = formatCount;

    if ( formatCount != 0 )
    {
        //C++ from tutorial file: details.formats.resize(formatCount);
        //TODO: check malloc
        details->formats = malloc( formatCount * sizeof( VkSurfaceFormatKHR ) );

        vkGetPhysicalDeviceSurfaceFormatsKHR( device,
                                              surface,
                                              &formatCount,
                                              details->formats );
    }

    uint32_t presentModeCount;

    vkGetPhysicalDeviceSurfacePresentModesKHR( device,
                                               surface,
                                               &presentModeCount,
                                               NULL );

    details->presentmode_count = presentModeCount;

    if ( presentModeCount != 0 )
    {
        //C++ from tutorial file: details.presentModes.resize(presentModeCount);
        //TODO: check malloc
        details->presentModes = malloc( presentModeCount * sizeof( VkPresentModeKHR ) );
 
        vkGetPhysicalDeviceSurfacePresentModesKHR( device,
                                                   surface,
                                                   &presentModeCount,
                                                   details->presentModes );
    }

    return true;
}

/*
================================================
 FUNCTION NAME: isDeviceSuitable
================================================
*/

bool isDeviceSuitable( VkPhysicalDevice device )
{
    QueueFamilyIndices qfIndices = findQueueFamilies( device );

    bool extensionsSupported = checkDeviceExtensionSupport( device );

    bool swapChainAdequate = false;

    if ( extensionsSupported )
    {
        SwapChainSupportDetails swapChainSupport = {0};
        // TODO: check querySwapChainSupport return value (bool)
        querySwapChainSupport( device, &swapChainSupport );
        swapChainAdequate = !( swapChainSupport.formats == NULL ) &&
                            !( swapChainSupport.presentModes == NULL );
        // TODO:
        free( swapChainSupport.formats );
        free( swapChainSupport.presentModes );
    }

    VkPhysicalDeviceFeatures supportedFeatures;

    vkGetPhysicalDeviceFeatures( device, &supportedFeatures );

    return queueFamilyIndicesIsComplete(qfIndices) &&
           extensionsSupported &&
           swapChainAdequate &&
           supportedFeatures.samplerAnisotropy;
}

/*
================================================
 FUNCTION NAME: checkDeviceExtensionSupport
================================================
*/

bool checkDeviceExtensionSupport( VkPhysicalDevice device )
{
    uint32_t extensionCount;

    vkEnumerateDeviceExtensionProperties( device,
                                          NULL,
                                          &extensionCount,
                                          NULL );

    VkExtensionProperties availableExtensions[ extensionCount ];

    vkEnumerateDeviceExtensionProperties( device,
                                          NULL,
                                          &extensionCount,
                                          availableExtensions );

    uint32_t numOfDvcExt = sizeof deviceExtensions / sizeof deviceExtensions[0];

    if( numOfDvcExt  == 1 && deviceExtensions[0] == NULL )
    {
        numOfDvcExt = 0;
    }

    bool is_available_flag = false;

    for ( uint32_t i = 0; i < numOfDvcExt; ++i )
    {
        is_available_flag = false;

        for ( uint32_t j = 0 ; j < extensionCount; ++j )
        {
            if ( strcmp( deviceExtensions[i],
                 availableExtensions[j].extensionName )
                 == 0 )
            {
                is_available_flag = true;
                break;
            }
        }

        if ( !is_available_flag )
        {
            return false;
        }

    }

    return is_available_flag;
}

/*
================================================
 FUNCTION NAME: findQueueFamilies
================================================
*/

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices qfIndices = {0};

    uint32_t queueFamilyCount = 0;

    vkGetPhysicalDeviceQueueFamilyProperties( device, &queueFamilyCount, NULL );

    VkQueueFamilyProperties queueFamilies[queueFamilyCount];

    vkGetPhysicalDeviceQueueFamilyProperties( device, &queueFamilyCount, queueFamilies );

    int i = 0;

    for ( uint32_t j = 0; j < queueFamilyCount; ++j )
    {
        if ( queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT )
        {
            qfIndices.graphicsFamily = i;
            qfIndices.graphicsFamilyHasValue = true;
        }

        VkBool32 presentSupport = false;

        vkGetPhysicalDeviceSurfaceSupportKHR( device, i, surface, &presentSupport );

        if (presentSupport)
        {
            qfIndices.presentFamily = i;
            qfIndices.presentFamilyHasValue = true;
        }

        if ( queueFamilyIndicesIsComplete(qfIndices) )
        {
            break;
        }

        i++;
    }

    return qfIndices;
}

/*
================================================
 FUNCTION NAME: getRequiredExtensions
================================================
*/

bool getRequiredExtensions( S_Extensions *pS )
{
    pS->size = 0;
    pS->data = NULL;
    
    uint32_t glfwExtensionCount = 0;

    const char **glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions( &glfwExtensionCount );

    pS->data = malloc ( glfwExtensionCount * sizeof (char*) );
    
    if( !pS->data )
    {
        //TODO: description
        printf("Error: getRequiredExtensions (1).\n");
        return false;
    }

    for ( uint32_t i = 0; i < glfwExtensionCount; ++i )
    {
        pS->data[i] = malloc( strlen( glfwExtensions[i] ) + 1 );
        
        if( !pS->data[i] )
        {
            //TODO: description
            printf("Error: getRequiredExtensions (2).\n");
            freeSExtensions( pS );
            return false;
        }

        pS->size++;

        strcpy( pS->data[i], glfwExtensions[i] );
    }

    //pS->size = glfwExtensionCount;

    /*
      if enableValidationLayers enabled
      append VK_EXT_DEBUG_UTILS_EXTENSION_NAME
    */

    if ( enableValidationLayers )
    {
        uint32_t index = pS->size;

        char **pTmp = (char**) realloc( pS->data, sizeof (char*) * ( pS->size + 1 ) );

        if( !**pTmp )
        {
            //TODO: description
            printf("Error:  getRequiredExtensions (3).\n");
            freeSExtensions( pS );
            return false;
        }

        pS->data = pTmp;

        pS->data[ index ] = malloc( strlen(VK_EXT_DEBUG_UTILS_EXTENSION_NAME) + 1 );

        if( !pS->data[ index ] )
        {
            //TODO: description
            printf("Error:  getRequiredExtensions (4).\n");
            freeSExtensions( pS );
            return false;
        }

        pS->size++;

        strcpy( pS->data[ index ], VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
    }

    return true;
}

/*
================================================
 FUNCTION NAME: checkValidationLayerSupport
================================================
*/

bool checkValidationLayerSupport()
{

    uint32_t layerCount;

    vkEnumerateInstanceLayerProperties( &layerCount, NULL );

    VkLayerProperties *availableLayers;

    availableLayers = malloc( layerCount * sizeof (VkLayerProperties) );

    if ( !availableLayers )
    {
        printf("Error: cannot allocate memory for instance layers.\n");
        return false;
    }

    vkEnumerateInstanceLayerProperties( &layerCount, availableLayers );

    uint32_t numOfValLayers = sizeof validationLayers / sizeof validationLayers[0];

    if( numOfValLayers  == 1 && validationLayers[0] == NULL )
    {
        // numOfValLayers = 0;
        return false;
    }

    for ( uint32_t i = 0; i < numOfValLayers; ++i )
    {
        const char *layerName = validationLayers[i];

        bool layerFound = false;

        for ( uint32_t j = 0; j < layerCount; ++j )
        {
            VkLayerProperties layerProperties = availableLayers[j];

            if ( strcmp( layerName, layerProperties.layerName ) == 0 )
            {
                layerFound = true;
                break;
            }
        }

        if ( !layerFound )
        {
            free( availableLayers );
            return false;
        }
    }

    free( availableLayers );

    return true;
}

/*
================================================
 FUNCTION NAME: readFile
================================================
*/

static bool readFile( S_read_buffer *pSrb, const char *filename )
{
    S_read_buffer s_ret = {0};

    FILE *fp;

    fp = fopen( filename, "rb" );

    if ( !fp )
    {
        printf( "Error: failed to open file: %s.\n", filename );
        return false;
    }

    fseek( fp, 0L, SEEK_END );

    size_t fileSize = ftell(fp);

    char *buffer = malloc( fileSize );

    if ( !buffer )
    {
        printf("Error: cannot allocate memory (11).\n");
        fclose(fp);
        return false;
    }

    fseek( fp, 0, SEEK_SET );

    size_t num = fread( buffer, sizeof *buffer, fileSize, fp );

    fclose(fp);

    if ( num != fileSize )
    {
        printf("Error: an error occurred while reading the file.\n");
        free( buffer );
        return false;
    }

    s_ret.p_buffer = buffer;
    s_ret.size = fileSize;

    *pSrb = s_ret;

    return true;
}

/*
================================================
 FUNCTION NAME: debugCallback
================================================
*/

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                     VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                     const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                     void *pUserData )
{
    /*
      Silence compiler warning for unused parameters
      messageSeverity, messageType, pUserData
    */
    (void)messageSeverity;
    (void)messageType;
    (void)pUserData;

    printf( "Validation layer debug: %s\n", pCallbackData->pMessage );

    return VK_FALSE;
}

/*
================================================
 FUNCTION NAME: main
================================================
*/

int main()
{

    if( !glfwInit() )
    {
        printf("Error: cannot init gltw.\n");
        return EXIT_FAILURE;
    }

    glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );

    window = glfwCreateWindow( width_init,
                               height_init,
                               "Vulkan",
                               NULL,
                               NULL );

    if ( !window )
    {
        printf("Error: unable to create window.\n");
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwSetFramebufferSizeCallback( window, framebufferResizeCallback );


    if ( !initVulkan() )
    {
        printf("Error: unable to initialize Vulkan.\n");
        cleanupVulkan();
        glfwDestroyWindow( window );
        glfwTerminate();
        return EXIT_FAILURE;
    }

    mainLoop();

    cleanupVulkan();

    glfwDestroyWindow( window );

    glfwTerminate();

    return EXIT_SUCCESS;
}
