#pragma once

#include "Platform/Config.h"

typedef struct Vector3_t
{
    float x = 0.f;
    float y = 0.f;
    float z = 0.f;
} Vector3_t;

typedef struct FR_ALIGNAS(16) Vector4_t
{
    float x = 0.f;
    float y = 0.f;
    float z = 0.f;
    float w = 0.f;
} Vector4_t;