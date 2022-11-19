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


function Print-Stage {
    param(
        [String]
        $stage_name
    )
    Write-Host ""
    Write-Host ("+" * ($stage_name.Length + 4)) -ForegroundColor Cyan
    Write-Host ("x " + $stage_name + " x")      -ForegroundColor Cyan
    Write-Host ("+" * ($stage_name.Length + 4)) -ForegroundColor Cyan
}



$t_sdl = "sdl"
$t_zlib = "zlib"
$t_ogre = "ogre"
$t_rp3d = "reactphysics3d"
$t_soloud = "soloud"

$a_config = "configure"
$a_build = "build"
$a_install = "install"



Write-Host ">> Update submodules: $submodules"
Write-Host ">> Targets: $target"
Write-Host ">> Actions: $action"



#submodules
if ($submodules -eq "true") {
    Print-Stage "Updating git submodules"
    git submodule update --init --recursive
}

# SDL
if ($target.Contains($t_sdl)) {
    if ($action.Contains($a_config)) {
        Print-Stage "Configuring SDL"
        cmake -S .\third_party\SDL -B .\third_party\SDL\build -DCMAKE_INSTALL_PREFIX=".\third_party\SDL\sdk\" -DSDL_TEST=FALSE -DSDL_SHARED=FALSE
    }

    if ($action.Contains($a_build)) {
        Print-Stage "Building SDL"
        cmake --build .\third_party\SDL\build --config Release
    }

    if ($action.Contains($a_install)) {
        Print-Stage "Installing SDL"
        cmake --build .\third_party\SDL\build --config Release --target install
    }
}

# zlib
if ($target.contains($t_zlib))
{
    if ($action.contains($a_config)) {
        Print-Stage "Configuring zlib"
        cmake -S .\third_party\zlib -B .\third_party\zlib\build -DCMAKE_INSTALL_PREFIX=".\third_party\zlib\sdk\"
    }

    if ($action.contains($a_build)) {
        Print-Stage "Building zlib"
        cmake --build .\third_party\zlib\build --config Release
    }

    if ($action.contains($a_install)) {
        Print-Stage "Installing zlib"
        cmake --build .\third_party\zlib\build --config Release --target install
    }
}

# Ogre
if ($target.Contains($t_ogre)) {
    if ($action.Contains($a_config)) {
        Print-Stage "Configuring Ogre"
        $CXXFLAGS = $env:CXXFLAGS
        $env:CXXFLAGS += "/I`"$env:CG_INC_PATH`""
        cmake -S .\third_party\ogre -B .\third_party\ogre\build -DCMAKE_INSTALL_PREFIX=".\third_party\ogre\sdk\" -DOGRE_STATIC=TRUE -DCg_LIBRARY_REL="$env:CG_LIB64_PATH\cg.lib" -DCg_LIBRARY_DBG="$env:CG_LIB64_PATH\cg.lib" -DOGRE_BUILD_RENDERSYSTEM_VULKAN=FALSE -DOGRE_BUILD_SAMPLES=FALSE -DOGRE_INSTALL_SAMPLES=FALSE -DOGRE_BUILD_COMPONENT_BULLET=FALSE -DOGRE_BUILD_COMPONENT_TERRAIN=FALSE -DOGRE_BUILD_TOOLS=FALSE -DOGRE_INSTALL_TOOLS=FALSE -DOGRE_BUILD_COMPONENT_OVERLAY_IMGUI=FALSE -DOGRE_BUILD_RENDERSYSTEM_GLES2=FALSE -DOGRE_BUILD_PLUGIN_STBI=FALSE -DOGRE_BUILD_PLUGIN_ASSIMP=FALSE -DOGRE_BUILD_PLUGIN_BSP=FALSE -DSDL2_DIR=".\third_party\SDL\sdk\cmake"
        $env:CXXFLAGS = $CXXFLAGS
    }

    if ($action.Contains($a_build)) {
        Print-Stage "Building Ogre"
        cmake --build .\third_party\ogre\build --config Release
    }

    if ($action.Contains($a_install)) {
        Print-Stage "Installing Ogre"
        cmake --build .\third_party\ogre\build --config Release --target install
    }
}

# reactphysics3d
if ($target.contains($t_rp3d)) {
    if ($action.contains($a_config)) {
        Print-Stage "Configuring reactphysics3d"
        cmake -S .\third_party\reactphysics3d -B .\third_party\reactphysics3d\build -DCMAKE_INSTALL_PREFIX=".\third_party\reactphysics3d\sdk\"
    }

    if ($action.contains($a_build)) {
        Print-Stage "Building reactphysics3d"
        cmake --build .\third_party\reactphysics3d\build --config Release
    }

    if ($action.contains($a_install)) {
        Print-Stage "Installing reactphysics3d"
        cmake --build .\third_party\reactphysics3d\build --config Release --target install
    }
}

# soloud
if ($target.contains($t_soloud)) {
    if ($action.contains($a_config)) {
        Print-Stage "Configuring soloud"
        Copy-Item .\install\soloud\CMakeLists.txt .\third_party\soloud\CMakeLists.txt
        Copy-Item .\install\soloud\soloudConfig.cmake.in .\third_party\soloud\soloudConfig.cmake.in
        cmake -S .\third_party\soloud -B .\third_party\soloud\build -DCMAKE_INSTALL_PREFIX=".\third_party\soloud\sdk\" -DSDL2_DIR="$(Resolve-Path ".\third_party\SDL\sdk\cmake")"
    }

    if ($action.contains($a_build)) {
        Print-Stage "Building soloud"
        cmake --build .\third_party\soloud\build --config Release
    }

    if ($action.contains($a_install)) {
        Print-Stage "Installing soloud"
        cmake --build .\third_party\soloud\build --config Release --target install
    }
}
