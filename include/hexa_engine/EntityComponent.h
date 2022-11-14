#pragma once

#include <base_lib/BasicTypes.h>
#include <base_lib/Pointers.h>
#include <base_lib/framework.h>

class World;
class Entity;

class EXPORT EntityComponent : public EnableSharedFromThis<EntityComponent>
{
    friend Entity;
    friend World;

public:
    FORCEINLINE Shared<Entity> get_owner() const;
    FORCEINLINE bool is_started() const { return started_; }

protected:
    virtual void on_start();
    virtual void on_tick(float delta_time);
    virtual void on_destroy();

private:
    void start();

    Entity* owner_;
    bool started_ = false;
};
