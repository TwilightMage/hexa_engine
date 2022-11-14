#pragma once

#include "hexa_engine/physics/RaycastResult.h"

#include <base_lib/List.h>
#include <reactphysics3d/collision/RaycastInfo.h>

class RaycastCallback : public reactphysics3d::RaycastCallback
{
public:
    reactphysics3d::decimal notifyRaycastHit(const reactphysics3d::RaycastInfo& raycastInfo) override;

    List<RaycastResult> results;
};
