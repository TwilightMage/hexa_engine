#include "hexa_engine/CameraComponent.h"

#include "hexa_engine/Entity.h"
#include "hexa_engine/Game.h"
#include "hexa_engine/World.h"

#include <OgreSceneManager.h>
#include <glm/ext/matrix_transform.hpp>

Ogre::Affine3 makeViewMatrix(const Ogre::Vector3& position, const Ogre::Quaternion& orientation, const Ogre::Affine3* reflectMatrix = 0)
{
    auto mat4x4_0 = lookAtLH(cast_object<glm::vec3>(position), cast_object<glm::vec3>(position - orientation.xAxis()), glm::vec3(0, 0, 1));
    auto mat4x4_1 = transpose(mat4x4_0);
    auto result = Ogre::Affine3(cast_object<Ogre::Matrix4>(mat4x4_1));

    if (reflectMatrix)
    {
        result = result * *reflectMatrix;
    }

    return result;
}

void CameraComponent::on_start()
{
    if (auto owner = get_owner())
    {
        if (auto world = owner->get_world())
        {
            ogre_camera_ = world->manager_->createCamera("Camera");
            ogre_camera_->setNearClipDistance(1);
            ogre_camera_->setFarClipDistance(10000);
            ogre_camera_->setAutoAspectRatio(true);
            ogre_camera_->viewMatrixCalcDelegate = &makeViewMatrix;

            owner->scene_node_->attachObject(ogre_camera_);
        }
    }
}

void CameraComponent::on_destroy()
{
    if (auto owner = get_owner())
    {
        if (auto world = owner->get_world())
        {
            world->manager_->destroyCamera(ogre_camera_);
        }
    }
}

bool CameraComponent::WorldToViewport(const Vector3& world, Vector2& out_screen) const
{
    const Ogre::Vector3 hcsPosition = ogre_camera_->getProjectionMatrix() * (ogre_camera_->getViewMatrix() * cast_object<Ogre::Vector3>(world));

    if ((hcsPosition.x < -1.0f) || (hcsPosition.x > 1.0f) || (hcsPosition.y < -1.0f) || (hcsPosition.y > 1.0f))
        return false;

    const int nCWidth = (Game::get_instance()->get_screen_width() / 2);
    const int nCHeight = (Game::get_instance()->get_screen_height() / 2);

    out_screen.x = nCWidth + (nCWidth * -hcsPosition.x);
    out_screen.y = nCHeight + (nCHeight * hcsPosition.y);

    return true;
}

Vector3 CameraComponent::ViewportToWorld(const Vector2& screen) const
{
    const auto ray = ogre_camera_->getCameraToViewportRay(screen.x, screen.y);
    return cast_object<Vector3>(ray.getPoint(1) - ray.getPoint(0));
}
