## Self-elevate the script if required
#if (-Not ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole] 'Administrator')) {
# if ([int](Get-CimInstance -Class Win32_OperatingSystem | Select-Object -ExpandProperty BuildNumber) -ge 6000) {
#  $CommandLine = "-File `"" + $MyInvocation.MyCommand.Path + "`" " + $MyInvocation.UnboundArguments
#  Start-Process -FilePath PowerShell.exe -Verb Runas -ArgumentList $CommandLine
#  Exit
# }
#}

param(
    [Parameter()]
    [ValidateSet("release", "debug")]
    [String]
    $config = "release",

    [Parameter()]
    [ValidateSet("sdl", "zlib", "ogre", "reactphysics3d", "soloud")]
    [String[]]
    $target = ("sdl", "zlib", "ogre", "reactphysics3d", "soloud"),

    [Parameter()]
    [ValidateSet("configure", "build", "install")]
    [String[]]
    $action = ("configure", "build", "install"),

    [Parameter()]
    [ValidateSet("true", "false")]
    [String]
    $submodules = "true"
)



$all_targets = ("sdl", "zlib", "ogre", "reactphysics3d", "soloud")
$t_sdl = "sdl"
$t_zlib = "zlib"
$t_ogre = "ogre"
$t_rp3d = "reactphysics3d"
$t_soloud = "soloud"

$all_actions = ("configure", "build", "install")
$a_config = "configure"
$a_build = "build"
$a_install = "install"


if ($config -eq "debug") {
    $config = "Debug"
} else {
    $config = "Release"
}



Write-Output ">> Update submodules: $submodules"
Write-Output ">> Configuration: $config"
Write-Output ">> Targets: $target"
Write-Output ">> Actions: $action"



#submodules
if ($submodules -eq "true") {
    Write-Output ">> Updating git submodules"
    git submodule update --init --recursive
}

# SDL
if ($target.Contains($t_sdl)) {
    if ($action.Contains($a_config)) {
        Write-Output "`n>> Configuring SDL"
        cmake -S .\third_party\SDL -B .\third_party\SDL\build\$config -DCMAKE_BUILD_TYPE="$config" -DCMAKE_INSTALL_PREFIX=".\third_party\SDL\sdk\$config\" -DSDL_TEST=FALSE -DSDL_SHARED=FALSE
    }

    if ($action.Contains($a_build)) {
        Write-Output "`n>> Building SDL"
        cmake --build .\third_party\SDL\build\$config --config $config
    }

    if ($action.Contains($a_install)) {
        Write-Output "`n>> Installing SDL"
        cmake --build .\third_party\SDL\build\$config --config $config --target install
    }
}

# zlib
if ($target.contains($t_zlib))
{
    if ($action.contains($a_config)) {
        Write-Output "`n>> Configuring zlib"
        cmake -S .\third_party\zlib -B .\third_party\zlib\build\$config -DCMAKE_BUILD_TYPE="$config" -DCMAKE_INSTALL_PREFIX=".\third_party\zlib\sdk\$config\"
    }

    if ($action.contains($a_build)) {
        Write-Output "`n>> Building zlib"
        cmake --build .\third_party\zlib\build\$config --config $config
    }

    if ($action.contains($a_install)) {
        Write-Output "`n>> Installing zlib"
        cmake --build .\third_party\zlib\build\$config --config $config --target install
    }
}

# Ogre
if ($target.Contains($t_ogre)) {
    if ($action.Contains($a_config)) {
        Write-Output "`n>> Configuring Ogre"
        $CXXFLAGS = $env:CXXFLAGS
        $env:CXXFLAGS += "/I`"$env:CG_INC_PATH`""
        Write-Output $env:CXXFLAGS
        cmake -S .\third_party\ogre -B .\third_party\ogre\build\$config -DCMAKE_BUILD_TYPE="$config" -DCMAKE_INSTALL_PREFIX=".\third_party\ogre\sdk\$config\" -DOGRE_LIB_DEBUG_PATH="/Debug" -DOGRE_STATIC=TRUE -DCg_LIBRARY_REL="$env:CG_LIB64_PATH\cg.lib" -DCg_LIBRARY_DBG="$env:CG_LIB64_PATH\cg.lib" -DOGRE_BUILD_RENDERSYSTEM_VULKAN=FALSE -DOGRE_BUILD_SAMPLES=FALSE -DOGRE_INSTALL_SAMPLES=FALSE -DOGRE_BUILD_COMPONENT_BULLET=FALSE -DOGRE_BUILD_COMPONENT_TERRAIN=FALSE -DOGRE_BUILD_TOOLS=FALSE -DOGRE_INSTALL_TOOLS=FALSE -DOGRE_BUILD_COMPONENT_OVERLAY_IMGUI=FALSE -DOGRE_BUILD_RENDERSYSTEM_GLES2=FALSE -DOGRE_BUILD_PLUGIN_STBI=FALSE -DOGRE_BUILD_PLUGIN_ASSIMP=FALSE -DOGRE_BUILD_PLUGIN_BSP=FALSE -DSDL2_DIR=".\third_party\SDL\sdk\cmake"
        $env:CXXFLAGS = $CXXFLAGS
    }

    if ($action.Contains($a_build)) {
        Write-Output "`n>> Building Ogre"
        cmake --build .\third_party\ogre\build\$config --config $config
    }

    if ($action.Contains($a_install)) {
        Write-Output "`n>> Installing Ogre"
        cmake --build .\third_party\ogre\build\$config --config $config --target install
    }
}

# reactphysics3d
if ($target.contains($t_rp3d)) {
    if ($action.contains($a_config)) {
        Write-Output "`n>> Configuring reactphysics3d"
        cmake -S .\third_party\reactphysics3d -B .\third_party\reactphysics3d\build\$config -DCMAKE_BUILD_TYPE="$config" -DCMAKE_INSTALL_PREFIX=".\third_party\reactphysics3d\sdk\$config\"
    }

    if ($action.contains($a_build)) {
        Write-Output "`n>> Building reactphysics3d"
        cmake --build .\third_party\reactphysics3d\build\$config --config $config
    }

    if ($action.contains($a_install)) {
        Write-Output "`n>> Installing reactphysics3d"
        cmake --build .\third_party\reactphysics3d\build\$config --config $config --target install
    }
}

# soloud
if ($target.contains($t_soloud)) {
    if ($action.contains($a_config)) {
        Write-Output "`n>> Configuring soloud"
        Copy-Item .\setup_soloud.cmake .\third_party\soloud\CMakeLists.txt
        cmake -S .\third_party\soloud -B .\third_party\soloud\build\$config -DCMAKE_BUILD_TYPE="$config" -DCMAKE_INSTALL_PREFIX=".\third_party\soloud\sdk\$config\"
    }

    if ($action.contains($a_build)) {
        Write-Output "`n>> Building soloud"
        cmake --build .\third_party\soloud\build\$config --config $config
    }

    if ($action.contains($a_install)) {
        Write-Output "`n>> Installing soloud"
        cmake --build .\third_party\soloud\build\$config --config $config --target install
    }
}
