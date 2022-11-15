## Self-elevate the script if required
#if (-Not ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole] 'Administrator')) {
# if ([int](Get-CimInstance -Class Win32_OperatingSystem | Select-Object -ExpandProperty BuildNumber) -ge 6000) {
#  $CommandLine = "-File `"" + $MyInvocation.MyCommand.Path + "`" " + $MyInvocation.UnboundArguments
#  Start-Process -FilePath PowerShell.exe -Verb Runas -ArgumentList $CommandLine
#  Exit
# }
#}

Write-Output ">> Updating git submodules"
git submodule update --init --recursive



# SDL
Write-Output "`n>> Configuring SDL"
cmake -S .\third_party\SDL -B .\third_party\SDL\build -DCMAKE_INSTALL_PREFIX=".\third_party\SDL\sdk\" -DSDL_TEST=FALSE

Write-Output "`n>> Building SDL"
cmake --build .\third_party\SDL\build --config Release

Write-Output "`n>> Installing SDL"
cmake -DBUILD_TYPE=Release -P .\third_party\SDL\build\cmake_install.cmake


# zlib
Write-Output "`n>> Configuring zlib"
cmake -S .\third_party\zlib -B .\third_party\zlib\build -DCMAKE_INSTALL_PREFIX=".\third_party\zlib\sdk\"

Write-Output "`n>> Building zlib"
cmake --build .\third_party\zlib\build --config Release

Write-Output "`n>> Installing zlib"
cmake -DBUILD_TYPE=Release -P .\third_party\zlib\build\cmake_install.cmake



# Ogre
Write-Output "`n>> Configuring Ogre"
$CXXFLAGS = $env:CXXFLAGS
$env:CXXFLAGS += "/I`"$env:CG_INC_PATH`""
cmake -S .\third_party\ogre -B .\third_party\ogre\build -DCMAKE_INSTALL_PREFIX=".\third_party\ogre\sdk\" -DOGRE_STATIC=TRUE -DOGRE_BUILD_RENDERSYSTEM_VULKAN=TRUE -DOGRE_BUILD_SAMPLES=FALSE -DOGRE_INSTALL_SAMPLES=FALSE -DOGRE_BUILD_COMPONENT_BULLET=FALSE -DOGRE_BUILD_COMPONENT_TERRAIN=FALSE -DOGRE_BUILD_TOOLS=FALSE -DOGRE_INSTALL_TOOLS=FALSE -DOGRE_BUILD_COMPONENT_OVERLAY_IMGUI=FALSE -DOGRE_BUILD_RENDERSYSTEM_GLES2=FALSE -DOGRE_BUILD_PLUGIN_STBI=FALSE -DOGRE_BUILD_PLUGIN_ASSIMP=FALSE -DOGRE_BUILD_PLUGIN_BSP=FALSE -DSDL2_DIR=".\third_party\SDL\sdk\cmake"
$env:CXXFLAGS = $CXXFLAGS

Write-Output "`n>> Building Ogre"
cmake --build .\third_party\ogre\build --config Release

Write-Output "`n>> Installing Ogre"
cmake --build .\third_party\ogre\build --config Release --target install



# reactphysics3d
Write-Output "`n>> Configuring reactphysics3d"
cmake -S .\third_party\reactphysics3d -B .\third_party\reactphysics3d\build -DCMAKE_INSTALL_PREFIX=".\third_party\reactphysics3d\sdk\"

Write-Output "`n>> Building reactphysics3d"
cmake --build .\third_party\reactphysics3d\build --config Release

Write-Output "`n>> Installing reactphysics3d"
cmake -DBUILD_TYPE=Release -P .\third_party\reactphysics3d\build\cmake_install.cmake



# soloud
Copy-Item .\setup_soloud.cmake .\third_party\soloud\CMakeLists.txt
Write-Output "`n>> Configuring soloud"
cmake -S .\third_party\soloud -B .\third_party\soloud\build -DCMAKE_INSTALL_PREFIX=".\third_party\soloud\sdk\"

Write-Output "`n>> Building soloud"
cmake --build .\third_party\soloud\build --config Release

Write-Output "`n>> Installing soloud"
cmake -DBUILD_TYPE=Release -P .\third_party\soloud\build\cmake_install.cmake



Read-Host -Prompt "Press any key to continue"