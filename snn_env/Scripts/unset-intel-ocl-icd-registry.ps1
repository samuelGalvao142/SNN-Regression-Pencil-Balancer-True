#Requires -RunAsAdministrator

if (Test-Path -Path HKLM:\SOFTWARE\Khronos) {
    if (Test-Path -Path HKLM:\SOFTWARE\Khronos\OpenCL) {
        if (Test-Path -Path HKLM:\SOFTWARE\Khronos\OpenCL\vendors) {
            $conda_env_library = "$env:CONDA_PREFIX\Library"
            $expected_name = "$conda_env_library\bin\intelocl64.dll"
            if ((Get-ItemProperty -Path HKLM:\SOFTWARE\Khronos\OpenCL\vendors).PSObject.Properties.name -contains $expected_name) {
                Remove-ItemProperty -Path HKLM:\SOFTWARE\Khronos\OpenCL\Vendors -Name $expected_name
                Write-Host "Registry entry removed"
            }
        }
    }
}

