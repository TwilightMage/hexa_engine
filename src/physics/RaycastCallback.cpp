#include "hexa_engine/physics/RaycastCallback.h"

#include <reactphysics3d/body/CollisionBody.h>

#include "hexa_engine/Entity.h"

reactphysics3d::decimal RaycastCallback::notifyRaycastHit(const reactphysics3d::RaycastInfo& raycastInfo)
{
    RaycastResult result;
    result.location = cast_object<Vector3>(raycastInfo.worldPoint);
    result.normal = -cast_object<Vector3>(raycastInfo.worldNormal);
    result.triangle_index = raycastInfo.triangleIndex;
    result.entity = static_cast<Entity*>(raycastInfo.body->getUserData())->shared_from_this();
    results.add(result);
    
    return 1.0f;
}
