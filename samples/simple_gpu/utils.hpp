#pragma once
#include <fstream>
#include <filesystem>

inline static void ReadBytes(const char8_t* file_name, uint32_t** bytes, uint32_t* length)
{
    FILE* f = fopen((const char*)file_name, "rb");
    fseek(f, 0, SEEK_END);
    *length = ftell(f);
    fseek(f, 0, SEEK_SET);
    *bytes = (uint32_t*)malloc(*length);
    fread(*bytes, *length, 1, f);
    fclose(f);
}

inline static void ReadShaderBytes(const char8_t* virtual_path, uint32_t** bytes, uint32_t* length)
{
    std::filesystem::path cur_path = std::filesystem::current_path();
    std::u8string shader_path      = cur_path.u8string();
    shader_path.append(u8"/");
    char8_t shader_file[256];
    strcpy((char*)shader_file, (const char*)shader_path.c_str());
    strcat((char*)shader_file, (const char*)virtual_path);
    strcat((char*)shader_file, (const char*)(u8".spv"));
           
    ReadBytes(shader_file, bytes, length);
}