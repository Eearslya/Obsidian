add_library(Obsidian-Engine SHARED)
target_compile_definitions(Obsidian-Engine PRIVATE OBSIDIAN_BUILD)
target_include_directories(Obsidian-Engine PRIVATE Source PUBLIC Include)

add_subdirectory(Source)
