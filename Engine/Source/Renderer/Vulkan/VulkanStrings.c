#include <Obsidian/Renderer/Vulkan/VulkanStrings.h>

const char* VulkanString_VkResult(VkResult value) {
	switch (value) {
		case VK_SUCCESS:
			return "Success";
		case VK_NOT_READY:
			return "NotReady";
		case VK_TIMEOUT:
			return "Timeout";
		case VK_EVENT_SET:
			return "EventSet";
		case VK_EVENT_RESET:
			return "EventReset";
		case VK_INCOMPLETE:
			return "Incomplete";
		case VK_ERROR_OUT_OF_HOST_MEMORY:
			return "OutOfHostMemory";
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
			return "OutOfDeviceMemory";
		case VK_ERROR_INITIALIZATION_FAILED:
			return "InitializationFailed";
		case VK_ERROR_DEVICE_LOST:
			return "DeviceLost";
		case VK_ERROR_MEMORY_MAP_FAILED:
			return "MemoryMapFailed";
		case VK_ERROR_LAYER_NOT_PRESENT:
			return "LayerNotPresent";
		case VK_ERROR_EXTENSION_NOT_PRESENT:
			return "ExtensionNotPresent";
		case VK_ERROR_FEATURE_NOT_PRESENT:
			return "FeatureNotPresent";
		case VK_ERROR_INCOMPATIBLE_DRIVER:
			return "IncompatibleDriver";
		case VK_ERROR_TOO_MANY_OBJECTS:
			return "TooManyObjects";
		case VK_ERROR_FORMAT_NOT_SUPPORTED:
			return "FormatNotSupported";
		case VK_ERROR_FRAGMENTED_POOL:
			return "FragmentedPool";
		case VK_ERROR_OUT_OF_POOL_MEMORY:
			return "OutOfPoolMemory";
		case VK_ERROR_INVALID_EXTERNAL_HANDLE:
			return "InvalidExternalHandle";
		case VK_ERROR_FRAGMENTATION:
			return "Fragmentation";
		case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
			return "InvalidOpaqueCaptureAddress";
		case VK_PIPELINE_COMPILE_REQUIRED:
			return "PipelineCompileRequired";
		case VK_ERROR_SURFACE_LOST_KHR:
			return "SurfaceLostKHR";
		case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
			return "NativeWindowInUseKHR";
		case VK_SUBOPTIMAL_KHR:
			return "SuboptimalKHR";
		case VK_ERROR_OUT_OF_DATE_KHR:
			return "OutOfDateKHR";
		case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
			return "IncompatibleDisplayKHR";
		case VK_ERROR_VALIDATION_FAILED_EXT:
			return "ValidationFailedEXT";
		case VK_ERROR_INVALID_SHADER_NV:
			return "InvalidShaderNV";
		case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
			return "InvalidDrmFormatModifierPlaneLayoutEXT";
		case VK_ERROR_NOT_PERMITTED_EXT:
			return "NotPermittedEXT";
		case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
			return "FullscreenExclusiveModeLostEXT";
		case VK_THREAD_IDLE_KHR:
			return "ThreadIdleKHR";
		case VK_THREAD_DONE_KHR:
			return "ThreadDoneKHR";
		case VK_OPERATION_DEFERRED_KHR:
			return "OperationDeferredKHR";
		case VK_OPERATION_NOT_DEFERRED_KHR:
			return "OperationNotDeferredKHR";
		case VK_ERROR_UNKNOWN:
		default:
			return "Unknown";
	}
}

const char* VulkanString_VkPhysicalDeviceType(VkPhysicalDeviceType value) {
	switch (value) {
		case VK_PHYSICAL_DEVICE_TYPE_OTHER:
			return "Other";
		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
			return "IntegratedGpu";
		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
			return "DiscreteGpu";
		case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
			return "VirtualGpu";
		case VK_PHYSICAL_DEVICE_TYPE_CPU:
			return "Cpu";
		default:
			return "Unknown";
	}
}