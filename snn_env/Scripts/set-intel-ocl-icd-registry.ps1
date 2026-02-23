#Requires -RunAsAdministrator

$conda_env_library = "$env:CONDA_PREFIX\Library"

if (-not (Test-Path -Path HKLM:\SOFTWARE\Khronos)) {
    New-Item -Path HKLM:\SOFTWARE\Khronos
}
if (-not (Test-Path -Path HKLM:\SOFTWARE\Khronos\OpenCL)) {
    New-Item -Path HKLM:\SOFTWARE\Khronos\OpenCL
}
if (-not (Test-Path -Path HKLM:\SOFTWARE\Khronos\OpenCL\Vendors)) {
    New-Item -Path HKLM:\SOFTWARE\Khronos\OpenCL\Vendors
}
New-ItemProperty -Path HKLM:\SOFTWARE\Khronos\OpenCL\Vendors -Name $conda_env_library\bin\intelocl64.dll -Value 0

Write-Host "Registry value: $(Get-Item -Path HKLM:\SOFTWARE\Khronos\OpenCL\Vendors)"

