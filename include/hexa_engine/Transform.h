#pragma once

#include <base_lib/Quaternion.h>
#include <base_lib/Vector3.h>

struct Transform
{
    Transform()
    {}

    Transform(const Vector3& location)
        : location(location)
    {}

    Transform(const Vector3& location, const Quaternion& rotation)
        : location(location)
        , rotation(rotation)
    {}

    Transform(const Vector3& location, const Quaternion& rotation, const Vector3& scale)
        : location(location)
        , rotation(rotation)
        , scale(scale)
    {}

    Transform(const Vector3& location, const Vector3& scale)
        : location(location)
        , scale(scale)
    {}

    Vector3 location;
    Quaternion rotation;
    Vector3 scale = Vector3::one();
};
