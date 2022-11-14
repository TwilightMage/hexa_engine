#include "hexa_engine/EntityComponent.h"

#include "Entity.h"

Shared<Entity> EntityComponent::get_owner() const
{
    return owner_->shared_from_this();
}

void EntityComponent::on_start()
{
}

void EntityComponent::on_tick(float delta_time)
{
}

void EntityComponent::on_destroy()
{
}

void EntityComponent::start()
{
    started_ = true;

    on_start();
}
