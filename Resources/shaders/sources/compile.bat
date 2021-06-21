

@echo off
for %%i in (*.vert *.frag) do "C:\VulkanSDK\1.2.162.1\Bin\glslangValidator.exe " -V "%%~i" -o "../spv/%%~i.spv"