#pragma once

#include "EntityComponent.h"
#include "Transform.h"

#include <base_lib/BasicTypes.h>
#include <base_lib/Delegate.h>
#include <base_lib/Quaternion.h>
#include <base_lib/Vector3.h>
#include <base_lib/framework.h>

class CameraComponent;
class MeshComponent;

namespace Ogre
{
    class SceneNode;
}

class Collision;
class Material3DInstance;

namespace reactphysics3d
{
    class RigidBody;
    class Collider;
} // namespace reactphysics3d

class EXPORT Entity : public EnableSharedFromThis<Entity>
{
    friend World;
    friend MeshComponent;
    friend CameraComponent;

public:
    Entity();

    Shared<World> get_world() const;

    virtual void on_start();
    virtual void on_tick(float delta_time);
    virtual void on_destroy();

    void destroy();

    bool is_started() const;

    FORCEINLINE const Transform& get_transform() const { return transform_; }
    void set_transform(const Transform& transform);

    FORCEINLINE Vector3 get_location() const { return get_transform().location; }
    void set_location(const Vector3& location);
    FORCEINLINE void translate(const Vector3& translation);

    FORCEINLINE Quaternion get_rotation() const { return get_transform().rotation; }
    void set_rotation(const Quaternion& rot);
    void rotate(const Vector3& axis, float angle);

    FORCEINLINE Vector3 get_scale() const { return get_transform().scale; }
    void set_scale(const Vector3& scale);

    void set_collision(const Shared<Collision>& collision, const Vector3& offset = Vector3::zero());
    void remove_collision();

    void set_collision_mask(byte16 bits);
    byte16 get_collision_mask() const;

    void set_gravity_enabled(bool state) const;

    void make_body_static();
    void make_body_dynamic();
    void make_body_kinematic();

    template<typename T, typename... Args>
    Shared<T> create_component(Args... args)
    {
        for (auto& existing_component : components_)
        {
            if (cast<T>(existing_component))
                return nullptr;
        }

        Shared<T> new_component = MakeShared<T>(std::forward<Args>(args)...);
        new_component->owner_ = this;
        components_.add(new_component);
        if (started_)
        {
            new_component->start();
        }
        return new_component;
    }

    template<typename T>
    void remove_component()
    {
        for (uint i = 0; i < components_.length(); i++)
        {
            if (cast<T>(components_[i]))
            {
                components_[i]->on_destroy();
                components_[i]->owner_ = nullptr;
                components_.remove_at(i);
                break;
            }
        }
    }

    void remove_all_components();
    template<class Class>
    Shared<Class> find_component()
    {
        for (auto& component : components_)
        {
            if (auto casted = cast<Class>(component))
            {
                return casted;
            }
        }

        return nullptr;
    }

    void set_tick_enabled(bool state);
    FORCEINLINE bool is_tick_enabled() const { return tick_enabled_; }

    Delegate<const Shared<Entity>&> on_destroyed;

private:
    void start();
    void tick(float delta_time);

    bool tick_enabled_ = false;

    Transform transform_;

    Weak<World> world_;
    Ogre::SceneNode* scene_node_ = nullptr;
    bool started_;
    List<Shared<EntityComponent>> components_;
};
