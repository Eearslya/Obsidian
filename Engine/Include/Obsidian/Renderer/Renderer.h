/** @file
 *  @brief Renderer subsystem */
#pragma once

#include <Obsidian/Defines.h>
#include <Obsidian/Renderer/Common.h>

struct PlatformStateT;

/**
 * Initialize the renderer subsystem.
 * @param appName The name of the application.
 * @param platform The current PlatformState object for the platform.
 * @return TRUE on success, FALSE otherwise.
 */
B8 Renderer_Initialize(const char* appName, struct PlatformStateT* platform);

/**
 * Shut down the renderer subsystem.
 */
void Renderer_Shutdown();

/**
 * Render a single frame.
 * @param packet A RenderPacket object containing the information needed to render.
 * @return TRUE upon success, FALSE on failure.
 */
B8 Renderer_DrawFrame(const RenderPacket* packet);
