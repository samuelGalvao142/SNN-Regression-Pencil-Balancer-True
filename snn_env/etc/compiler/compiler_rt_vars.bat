@echo off
:: ============================================================================
:: Copyright 1985 Intel Corporation All Rights Reserved.
::
:: The source code,  information and material ("Material")  contained herein is
:: owned by Intel Corporation or its suppliers or licensors,  and title to such
:: Material remains with Intel Corporation or its suppliers  or licensors.  The
:: Material contains  proprietary  information  of Intel  or its  suppliers and
:: licensors.  The Material is protected by worldwide copyright laws and treaty
:: provisions.  No part  of the  Material   may be  used,  copied,  reproduced,
:: modified,  published,   uploaded,   posted,   transmitted,   distributed  or
:: disclosed in any way without Intel's prior  express written  permission.  No
:: license under any patent,  copyright  or other intellectual  property rights
:: in the Material is granted to or conferred upon you,  either  expressly,  by
:: implication,  inducement,  estoppel  or otherwise.  Any  license  under such
:: intellectual  property  rights must  be  express and  approved  by  Intel in
:: writing.
::
:: Unless otherwise  agreed by  Intel in writing,  you may not  remove or alter
:: this notice or  any other notice  embedded in Materials by  Intel or Intel's
:: suppliers or licensors in any way.
:: ============================================================================

set "VARSDIR=%~dp0"
if not defined CMPLR_ROOT for /f "delims=" %%F in ("%VARSDIR%..") do set "CMPLR_ROOT=%%~fF"

set "SCRIPT_NAME=%~nx0"
set "INTEL_TARGET_ARCH="
set "INTEL_TARGET_PLATFORM=windows"

:ParseArgs
:: Parse the incoming arguments
if /i "%1"==""              goto CheckArgs
if /i "%1"=="ia32"          (set INTEL_TARGET_ARCH=ia32)     & (set TARGET_VS_ARCH=x86)     & shift & goto ParseArgs
if /i "%1"=="intel64"       (set INTEL_TARGET_ARCH=intel64)  & (set TARGET_VS_ARCH=amd64)   & shift & goto ParseArgs
shift & goto ParseArgs

:CheckArgs
:: set correct defaults
if /i "%INTEL_TARGET_ARCH%"==""   (set INTEL_TARGET_ARCH=intel64) & (set TARGET_VS_ARCH=amd64)

:: setup intel compiler environment
:: ============================================================================
:SetIntelEnv
if /i "%INTEL_TARGET_ARCH%"=="ia32" (
    set "INTEL_TARGET_ARCH_IA32=ia32"
) else (
    if defined INTEL_TARGET_ARCH_IA32 (set "INTEL_TARGET_ARCH_IA32=")
)

set "PATH=%CMPLR_ROOT%\%INTEL_TARGET_PLATFORM%\lib;%PATH%"

set "OCL_ICD_FILENAMES=%CMPLR_ROOT%\%INTEL_TARGET_PLATFORM%\lib\x64\intelocl64.dll"

goto End

:End
exit /B 0

:: ============================================================================
:GetFullPath
SET %2=%~f1
GOTO :EOF


