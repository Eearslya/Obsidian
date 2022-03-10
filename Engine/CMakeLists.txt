include(FindVulkan)

add_library(Obsidian-Engine SHARED)
target_compile_definitions(Obsidian-Engine PRIVATE OBSIDIAN_BUILD)
target_include_directories(Obsidian-Engine PRIVATE Source PUBLIC Include)
target_link_libraries(Obsidian-Engine PRIVATE Vulkan::Vulkan)

add_subdirectory(Source)
