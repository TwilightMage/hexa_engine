#pragma once

#include <base_lib/Pointers.h>

class Module;

namespace Ogre {
    class GpuProgram;
    
}

class Shader {
private:
    friend Module;
    
public:
    Shader();
    Shader(Shared<Ogre::GpuProgram> regular);
    Shader(Shared<Ogre::GpuProgram> regular, Shared<Ogre::GpuProgram> instanced);

    operator bool() const;

    bool is_valid() const;
    bool has_instanced() const;
    
    Shared<Ogre::GpuProgram> try_get_instanced() const;

private:
    Shared<Ogre::GpuProgram> regular_;
    Shared<Ogre::GpuProgram> instanced_;
};
