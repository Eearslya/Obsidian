/** @file
 *  @brief Base render engine API */
#pragma once

#include <Obsidian/Defines.h>
#include <Obsidian/Renderer/Common.h>

struct PlatformStateT;

/**
 * Create the render engine.
 * @param type The render engine type to use.
 * @param platform The current PlatformState object for the platform.
 * @param[out] engine The created RenderEngine.
 * @return TRUE upon success, FALSE otherwise.
 */
B8 RenderEngine_Create(RenderEngineType type, struct PlatformStateT* platform, RenderEngine* engine);

/**
 * Destroy the render engine.
 * @param engine The RenderEngine to destroy.
 */
void RenderEngine_Destroy(RenderEngine engine);
