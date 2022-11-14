#pragma once

#include <base_lib/Pointers.h>
#include <base_lib/Vector3.h>

class Entity;

struct RaycastResult
{
    Vector3 location;
    Vector3 normal;
    int triangle_index;
    Shared<Entity> entity;
};
