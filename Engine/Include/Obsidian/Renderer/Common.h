/** @file
 *  @brief Types and information common to all renderer systems */
#pragma once

#include <Obsidian/Defines.h>

struct PlatformStateT;

/** Specifies the API the render engine should use. */
typedef enum RenderEngineType {
	RenderEngineType_Vulkan,
	RenderEngineType_OpenGL,
	RenderEngineType_DirectX
} RenderEngineType;

/** Contains the information needed to render a frame. */
typedef struct RenderPacketT {
	F64 DeltaTime; /**< The time in seconds since the last render. */
} RenderPacket;

struct RenderEngineT {
	B8 (*Initialize)(struct RenderEngineT* engine, const char* appName, struct PlatformStateT* platform);
	void (*Shutdown)(struct RenderEngineT* engine);
	B8 (*BeginFrame)(struct RenderEngineT* engine, F64 deltaTime);
	B8 (*EndFrame)(struct RenderEngineT* engine, F64 deltaTime);
};
typedef struct RenderEngineT* RenderEngine;
