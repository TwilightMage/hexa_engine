#pragma once

#include <base_lib/BasicTypes.h>

struct TimerHandle
{
    constexpr bool operator==(const TimerHandle& rhs) const { return id == rhs.id; }
    constexpr bool operator<(const TimerHandle& rhs) const { return id < rhs.id; }
    
    inline static uint id_generator = 0;
    uint id;
};
