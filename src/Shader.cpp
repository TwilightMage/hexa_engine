#include "hexa_engine/Shader.h"

Shader::Shader()
    : regular_(nullptr)
    , instanced_(nullptr)
{}

Shader::Shader(Shared<Ogre::GpuProgram> regular)
    : regular_(regular)
    , instanced_(nullptr)
{}

Shader::Shader(Shared<Ogre::GpuProgram> regular, Shared<Ogre::GpuProgram> instanced)
    : regular_(regular)
    , instanced_(instanced)
{}

Shader::operator bool() const { return is_valid(); }

bool Shader::is_valid() const { return regular_ != nullptr; }

bool Shader::has_instanced() const { return instanced_ != nullptr; }

Shared<Ogre::GpuProgram> Shader::try_get_instanced() const { return instanced_ ? instanced_ : regular_; }
