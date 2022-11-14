#pragma once

#include <base_lib/Delegate.h>
#include <base_lib/Pointers.h>
#include <base_lib/framework.h>

class World;

class EXPORT EventBus
{
public:
    Delegate<const Shared<World>&> world_opened;
    Delegate<const Shared<World>&> world_closed;
};
