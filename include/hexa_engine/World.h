#pragma once

#include "Entity.h"
#include "SoundHandle.h"
#include "TimerHandle.h"
#include "Transform.h"

#include <base_lib/Color.h>
#include <base_lib/List.h>
#include <base_lib/Set.h>
#include <base_lib/Vector3.h>

class StaticMesh;
class Audio;
class CameraComponent;
class ItemDrop;
class AudioChannel;
struct RaycastResult;
class Game;
class Entity;

namespace reactphysics3d
{
    class PhysicsWorld;
}

namespace Ogre
{
    class InstanceManager;
    class Light;
    class SceneNode;
    class SceneManager;
} // namespace Ogre

class EXPORT World : public EnableSharedFromThis<World>
{
    friend Game;
    friend MeshComponent;
    friend CameraComponent;
    friend Entity;

    struct TimerEntry
    {
        float time;
        std::function<void()> func;
    };

public:
    bool spawn_entity(const Shared<Entity>& entity, const Transform& transform);
    bool spawn_entity(const Shared<Entity>& entity);
    template<typename T, typename... Args>
    Shared<T> spawn_entity(const Transform& transform, Args... args)
    {
        auto entity = MakeShared<T>(std::forward<Args>(args)...);
        entity->transform_ = transform;
        spawn_entity_internal(entity);
        return entity;
    }

    SoundHandle play_sound(const Shared<Audio>& audio, const Shared<AudioChannel>& channel = nullptr);
    SoundHandle play_sound_3d(const Shared<Audio>& audio, const Vector3& location, const Shared<AudioChannel>& channel = nullptr);

    Shared<const RaycastResult> raycast(const Vector3& from, const Vector3& to) const;
    Shared<const RaycastResult> raycast(const Vector3& from, const Vector3& to, byte16 collision_mask) const;
    List<RaycastResult> raycast_all(const Vector3& from, const Vector3& to, bool sort_by_distance = false) const;
    List<RaycastResult> raycast_all(const Vector3& from, const Vector3& to, byte16 collision_mask, bool sort_by_distance = false) const;

    void init();
    void start();
    void tick(float delta_time);

    const Set<Shared<Entity>>& get_entities() const;

    float get_time_scale() const;
    void set_time_scale(float val);

    Vector3 get_gravity() const;
    void set_gravity(const Vector3& val) const;

    FORCEINLINE float get_time() const { return time_; }

    TimerHandle delay(float time, std::function<void()> func);

    void set_ambient_light(const Color& color, float intensity);

    void set_directional_light(const Color& color, float intensity, const Quaternion& rotation);

protected:
    virtual void on_start();
    virtual void on_tick(float delta_time);
    virtual void on_close();

private:
    void close();
    void spawn_entity_internal(const Shared<Entity>& entity);

    void do_destroy(const Shared<Entity>& entity);

    void set_entity_tick_enabled(const Shared<Entity>& entity, bool state);
    void mark_entity_for_destroy(const Shared<Entity>& entity, bool state);

    List<Ogre::InstanceManager*> get_or_create_instance_managers(const Shared<StaticMesh>& mesh, uint batch_instance_count, uint instanced_params_count);

    Set<Shared<Entity>> entities_;
    Set<Shared<Entity>> tick_list_;
    Set<Shared<Entity>> destroy_list_;
    reactphysics3d::PhysicsWorld* physics_world_;
    float physics_tick_accum_;
    float time_scale_ = 1.0f;
    Ogre::SceneManager* manager_;
    Ogre::SceneNode* world_root_;

    Color ambient_color_;
    float ambient_intensity_;

    Color directional_color_;
    float directional_intensity_;
    Quaternion directional_rotation_;
    Ogre::SceneNode* directional_light_node_;
    Ogre::Light* directional_light_;
    Map<Shared<StaticMesh>, List<Ogre::InstanceManager*>> instance_managers_;

    Map<TimerHandle, TimerEntry> timer_entries_;

    float time_ = 0.0f;
};
