/** @file
 *  @brief Vulkan render engine */
#pragma once

#include <Obsidian/Defines.h>
#include <Obsidian/Renderer/RenderEngine.h>

/**
 * Initialize the Vulkan render engine.
 * @param engine The RenderEngine object to initialize.
 * @param appName The name of the application.
 * @param platform The PlatformState object for the current platform.
 * @return TRUE on success, FALSE on failure.
 */
B8 RenderEngine_Vulkan_Initialize(RenderEngine engine, const char* appName, struct PlatformStateT* platform);

/**
 * Shut down the Vulkan render engine.
 * @param engine The RenderEngine object to shut down.
 */
void RenderEngine_Vulkan_Shutdown(RenderEngine engine);

/**
 * Begin rendering a new frame.
 * @param engine The RenderEngine object to use.
 * @param deltaTime The amount of time, in seconds, since the last render.
 * @return FALSE if the frame should not be rendered, TRUE otherwise.
 */
B8 RenderEngine_Vulkan_BeginFrame(RenderEngine engine, F64 deltaTime);

/**
 * End rendering the new frame.
 * @param engine The RenderEngine object to use.
 * @param deltaTime The amount of time, in seconds, since the last render.
 * @return TRUE upon a successful render, FALSE if an error occurred.
 */
B8 RenderEngine_Vulkan_EndFrame(RenderEngine engine, F64 deltaTime);
