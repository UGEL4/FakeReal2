/*
 * Basic camera class
 *
 * Copyright (C) 2016 by Sascha Willems - www.saschawillems.de
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */
#pragma once
//#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "Math/Matrix.h"
#include "frustum.hpp"
#include "Math/Quaternion.h"

#if defined(_WIN32)
    #define KEY_ESCAPE VK_ESCAPE
    #define KEY_F1 VK_F1
    #define KEY_F2 VK_F2
    #define KEY_F3 VK_F3
    #define KEY_F4 VK_F4
    #define KEY_F5 VK_F5
    #define KEY_W 0x57
    #define KEY_A 0x41
    #define KEY_S 0x53
    #define KEY_D 0x44
    #define KEY_P 0x50
    #define KEY_SPACE 0x20
    #define KEY_KPADD 0x6B
    #define KEY_KPSUB 0x6D
    #define KEY_B 0x42
    #define KEY_F 0x46
    #define KEY_L 0x4C
    #define KEY_N 0x4E
    #define KEY_O 0x4F
    #define KEY_T 0x54
#endif

class Camera
{
private:
    float fov;
    float znear, zfar, aspect;

    void updateViewMatrix()
    {
        using namespace FakeReal;
        /* math::Matrix4X4 rotM(1.0f);
        math::Matrix4X4 transM;

        rotM = glm::rotate(rotM, glm::radians(rotation.x * (flipY ? -1.0f : 1.0f)), math::Vector3(1.0f, 0.0f, 0.0f));
        rotM = glm::rotate(rotM, glm::radians(rotation.y), math::Vector3(0.0f, 1.0f, 0.0f));
        rotM = glm::rotate(rotM, glm::radians(rotation.z), math::Vector3(0.0f, 0.0f, 1.0f)); */

        math::Vector3 angle(glm::radians(rotation.x), glm::radians(rotation.y), glm::radians(rotation.z));
        math::Matrix4X4 rot    = glm::toMat4(math::Quaternion(angle));
        math::Matrix4X4 transM = glm::translate(math::Matrix4X4(1.0f), position);
        matrices.view          = glm::inverse(transM * rot);

        /* if (type == CameraType::firstperson)
        {
            matrices.view = rotM * transM;
        }
        else
        {
            matrices.view = transM * rotM;
        } */

        viewPos = math::Vector4(position, 0.0f) * math::Vector4(-1.0f, 1.0f, -1.0f, 1.0f);

        updated = true;
    };

public:
    enum CameraType
    {
        lookat,
        firstperson
    };
    CameraType type = CameraType::lookat;

    FakeReal::math::Vector3 rotation =  FakeReal::math::Vector3();
    FakeReal::math::Vector3 position =  FakeReal::math::Vector3();
    FakeReal::math::Vector4 viewPos  =  FakeReal::math::Vector4();

    float rotationSpeed = 1.0f;
    float movementSpeed = 1.0f;

    bool updated = false;
    bool flipY   = false;

    struct
    {
        FakeReal::math::Matrix4X4 perspective;
        FakeReal::math::Matrix4X4 view;
    } matrices;

    struct
    {
        bool left  = false;
        bool right = false;
        bool up    = false;
        bool down  = false;
    } keys;

    bool moving()
    {
        return keys.left || keys.right || keys.up || keys.down;
    }

    float getNearClip() const
    {
        return znear;
    }

    float getFarClip() const
    {
        return zfar;
    }

    float getAspect() const
    {
        return aspect;
    }

    float getFov() const
    {
        return this->fov;
    }

    void setPerspective(float fov, float aspect, float znear, float zfar)
    {
        this->fov            = fov;
        this->znear          = znear;
        this->zfar           = zfar;
        this->aspect         = aspect;
        matrices.perspective = glm::perspective(glm::radians(fov), aspect, znear, zfar);
        if (flipY)
        {
            matrices.perspective[1][1] *= -1.0f;
        }
    };

    void updateAspectRatio(float aspect)
    {
        this->aspect         = aspect;
        matrices.perspective = glm::perspective(glm::radians(fov), aspect, znear, zfar);
        if (flipY)
        {
            matrices.perspective[1][1] *= -1.0f;
        }
    }

    void setPosition(FakeReal::math::Vector3 position)
    {
        this->position = position;
        updateViewMatrix();
    }

    void setRotation(FakeReal::math::Vector3 rotation)
    {
        this->rotation = rotation;
        updateViewMatrix();
    }

    void rotate(FakeReal::math::Vector3 delta)
    {
        this->rotation += delta;
        updateViewMatrix();
    }

    void setTranslation(FakeReal::math::Vector3 translation)
    {
        this->position = translation;
        updateViewMatrix();
    };

    void translate(FakeReal::math::Vector3 delta)
    {
        this->position += delta;
        updateViewMatrix();
    }

    void setRotationSpeed(float rotationSpeed)
    {
        this->rotationSpeed = rotationSpeed;
    }

    void setMovementSpeed(float movementSpeed)
    {
        this->movementSpeed = movementSpeed;
    }

    void update(float deltaTime)
    {
        using namespace FakeReal;
        updated = false;
        if (type == CameraType::firstperson)
        {
            if (moving())
            {
                /* math::Vector3 camFront;
                camFront.x = -cos(glm::radians(rotation.x)) * sin(glm::radians(rotation.y));
                camFront.y = sin(glm::radians(rotation.x));
                camFront.z = cos(glm::radians(rotation.x)) * cos(glm::radians(rotation.y));
                camFront   = glm::normalize(camFront); */

                math::Vector3 angle(glm::radians(rotation.x), glm::radians(rotation.y), glm::radians(rotation.z));
                math::Matrix4X4 rot    = glm::toMat4(math::Quaternion(angle));
                math::Vector3 camFront = -glm::normalize(math::Vector3(rot[2][0], rot[2][1], rot[2][2]));

                float moveSpeed = deltaTime * movementSpeed;

                if (keys.up)
                    position += camFront * moveSpeed;
                if (keys.down)
                    position -= camFront * moveSpeed;
                if (keys.left)
                    position -= glm::normalize(glm::cross(camFront, math::Vector3(0.0f, 1.0f, 0.0f))) * moveSpeed;
                if (keys.right)
                    position += glm::normalize(glm::cross(camFront, math::Vector3(0.0f, 1.0f, 0.0f))) * moveSpeed;
            }
        }
        updateViewMatrix();
    };

    void GetPlane(Plane planes[6]) const
    {
        Frustum frustum(matrices.perspective * matrices.view);
        for (uint32_t i = 0; i < 6; i++)
        {
            planes[i] = frustum.planes[i];
        }
    }

    // Update camera passing separate axis data (gamepad)
    // Returns true if view or position has been changed
    /* bool updatePad(glm::vec2 axisLeft, glm::vec2 axisRight, float deltaTime)
    {
        bool retVal = false;

        if (type == CameraType::firstperson)
        {
            // Use the common console thumbstick layout
            // Left = view, right = move

            const float deadZone = 0.0015f;
            const float range    = 1.0f - deadZone;

            glm::vec3 camFront;
            camFront.x = -cos(glm::radians(rotation.x)) * sin(glm::radians(rotation.y));
            camFront.y = sin(glm::radians(rotation.x));
            camFront.z = cos(glm::radians(rotation.x)) * cos(glm::radians(rotation.y));
            camFront   = glm::normalize(camFront);

            float moveSpeed = deltaTime * movementSpeed * 2.0f;
            float rotSpeed  = deltaTime * rotationSpeed * 50.0f;

            // Move
            if (fabsf(axisLeft.y) > deadZone)
            {
                float pos = (fabsf(axisLeft.y) - deadZone) / range;
                position -= camFront * pos * ((axisLeft.y < 0.0f) ? -1.0f : 1.0f) * moveSpeed;
                retVal = true;
            }
            if (fabsf(axisLeft.x) > deadZone)
            {
                float pos = (fabsf(axisLeft.x) - deadZone) / range;
                position += glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * pos * ((axisLeft.x < 0.0f) ? -1.0f : 1.0f) * moveSpeed;
                retVal = true;
            }

            // Rotate
            if (fabsf(axisRight.x) > deadZone)
            {
                float pos = (fabsf(axisRight.x) - deadZone) / range;
                rotation.y += pos * ((axisRight.x < 0.0f) ? -1.0f : 1.0f) * rotSpeed;
                retVal = true;
            }
            if (fabsf(axisRight.y) > deadZone)
            {
                float pos = (fabsf(axisRight.y) - deadZone) / range;
                rotation.x -= pos * ((axisRight.y < 0.0f) ? -1.0f : 1.0f) * rotSpeed;
                retVal = true;
            }
        }
        else
        {
            // todo: move code from example base class for look-at
        }

        if (retVal)
        {
            updateViewMatrix();
        }

        return retVal;
    } */
};


class FPSCamera
{
public:
    float rotationSpeed = 1.0f;
    float movementSpeed = 1.0f;

    bool updated = false;
    bool flipY   = false;

    struct
    {
        FakeReal::math::Matrix4X4 perspective;
        FakeReal::math::Matrix4X4 view;
    } matrices;

    struct
    {
        bool left  = false;
        bool right = false;
        bool up    = false;
        bool down  = false;
    } keys;

public:
    void Update(float deltaTime)
    {
        using namespace FakeReal;
        updated = false;
        if (moving())
        {
            math::Vector3 angle(glm::radians(rotation.x), glm::radians(rotation.y), glm::radians(rotation.z));
            math::Matrix4X4 rot    = glm::toMat4(math::Quaternion(angle));
            math::Vector3 camFront = glm::normalize(math::Vector3(rot[2][0], rot[2][1], rot[2][2]));

            float moveSpeed = deltaTime * movementSpeed;

            if (keys.up)
                position += camFront * moveSpeed;
            if (keys.down)
                position -= camFront * moveSpeed;
            if (keys.left)
                position -= glm::normalize(glm::cross(camFront, math::Vector3(0.0f, 1.0f, 0.0f))) * moveSpeed;
            if (keys.right)
                position += glm::normalize(glm::cross(camFront, math::Vector3(0.0f, 1.0f, 0.0f))) * moveSpeed;
        }
        updateViewMatrix();
    }

    void updateViewMatrix()
    {
        using namespace FakeReal;
        math::Vector3 angle(glm::radians(rotation.x), glm::radians(rotation.y), glm::radians(rotation.z));
        math::Matrix4X4 rotM = glm::toMat4(math::Quaternion(angle));
        math::Matrix4X4 transM;
        math::Vector3 translation = position;
        if (flipY)
        {
            translation.y *= -1.0f;
        }
        transM = glm::translate(math::Matrix4X4(1.0f), translation);
        {
            matrices.view = glm::inverse(transM * rotM);
        }
        /* else
        {
            matrices.view = transM * rotM;
        } */
        //matrices.view = glm::lookAt(math::Vector3(-25.f, 5.f, 0.f), math::Vector3(0.f, 5.f, 0.f), math::Vector3(0.f, 1.f, 0.f));

        viewPos = math::Vector4(position, 0.0f) * math::Vector4(-1.0f, -1.0f, -1.0f, 1.0f);

        updated = true;
    };

    void setPosition(FakeReal::math::Vector3 position)
    {
        this->position = position;
        updateViewMatrix();
    }

    void setRotation(FakeReal::math::Vector3 rotation)
    {
        this->rotation = rotation;
        updateViewMatrix();
    }

    void setPerspective(float fov, float aspect, float znear, float zfar)
    {
        this->fov            = fov;
        this->znear          = znear;
        this->zfar           = zfar;
        matrices.perspective = glm::perspective(glm::radians(fov), aspect, znear, zfar);
        /* if (flipY)
        {
            matrices.perspective[1][1] *= -1.0f;
        } */
    }

    bool moving()
    {
        return keys.left || keys.right || keys.up || keys.down;
    }

    void GetPlane(Plane planes[6]) const
    {
        //Frustum frustum(matrices.perspective * matrices.view);
        Frustum frustum;
        frustum.Initialize(matrices.perspective * matrices.view, true);
        for (uint32_t i = 0; i < 6; i++)
        {
            planes[i] = frustum.planes[i];
        }
    }

public:
    FakeReal::math::Vector3 rotation =  FakeReal::math::Vector3();
    FakeReal::math::Vector3 position =  FakeReal::math::Vector3();
    FakeReal::math::Vector4 viewPos  =  FakeReal::math::Vector4();
private:
    float fov;
    float znear, zfar;
};