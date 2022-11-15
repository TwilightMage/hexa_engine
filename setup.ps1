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



# OIS
Write-Output "`n>> Configuring OIS"
cmake -S .\third_party\OIS -B .\third_party\OIS\build -DCMAKE_INSTALL_PREFIX=".\third_party\OIS\sdk\"

Write-Output "`n>> Building OIS"
cmake --build .\third_party\OIS\build --config Release

Write-Output "`n>> Installing OIS"
cmake -DBUILD_TYPE=Release -P .\third_party\OIS\build\cmake_install.cmake



# Ogre
Write-Output "`n>> Configuring Ogre"
cmake -S .\third_party\ogre -B .\third_party\ogre -DOGRE_STATIC=TRUE -DOGRE_BUILD_RENDERSYSTEM_VULKAN=TRUE -DOGRE_BUILD_SAMPLES=FALSE -DOGRE_INSTALL_SAMPLES=FALSE -DOGRE_BUILD_COMPONENT_BULLET=FALSE -DOGRE_BUILD_COMPONENT_TERRAIN=FALSE -DOGRE_BUILD_TOOLS=FALSE -DOGRE_INSTALL_TOOLS=FALSE -DOGRE_BUILD_COMPONENT_OVERLAY_IMGUI=FALSE -DOGRE_BUILD_RENDERSYSTEM_GLES2=FALSE

Write-Output "`n>> Building Ogre"
#cmake --build .\third_party\ogre --config Debug
cmake --build .\third_party\ogre --config Release

Write-Output "`n>> Installing Ogre"
cmake --build .\third_party\ogre --config Release --target install



# reactphysics3d
Write-Output "`n>> Configuring reactphysics3d"
cmake -S .\third_party\reactphysics3d -B .\third_party\reactphysics3d\build -DCMAKE_INSTALL_PREFIX=".\third_party\reactphysics3d\sdk\"

Write-Output "`n>> Building reactphysics3d"
cmake --build .\third_party\reactphysics3d\build --config Release

Write-Output "`n>> Installing reactphysics3d"
cmake -DBUILD_TYPE=Release -P .\third_party\reactphysics3d\build\cmake_install.cmake



Read-Host -Prompt "Press any key to continue"