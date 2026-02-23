:: ============================================================================
:: Copyright 2023 Intel Corporation All Rights Reserved.
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

setlocal EnableDelayedExpansion
set "cl_cfg_orig=%PREFIX%\Library\lib\cl.cfg"
set "cl_cfg_temp=%PREFIX%\Library\lib\cl.cfg.temp"
set "lib_bin=%PREFIX%\Library\bin"
set "PATH=%PATH%;C:\Windows\System32"
%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell -Command "(gc '%cl_cfg_orig%') -replace 'CL_CONFIG_TBB_DLL_PATH =.*', 'CL_CONFIG_TBB_DLL_PATH = %lib_bin%' | Out-File -Encoding ASCII -FilePath '%cl_cfg_temp%'"
move /Y "%cl_cfg_temp%" "%cl_cfg_orig%"
