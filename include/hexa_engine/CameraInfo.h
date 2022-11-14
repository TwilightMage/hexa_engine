#pragma once

#include <base_lib/Quaternion.h>
#include <base_lib/Vector3.h>

struct CameraInfo
{
    Vector3 position = Vector3::zero();
    Quaternion rotation = Vector3::zero();
    float fov = 45.0f;
};
