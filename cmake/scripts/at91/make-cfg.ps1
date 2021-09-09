<#
.SYNOPSIS
    Script to help generate a build system for the AT91 development board
.DESCRIPTION
    .
.PARAMETER Generator
	CMake generator to use. Defaults to ninja, make is also possible
.PARAMETER BuildType
	Build type to use. Defaults to debug.
#>
param (
    # Build system generator to use
    [ValidateSet("ninja","make")]
    [string]$Generator = 'ninja',
    [ValidateSet("debug", "release", "size")]
    [string]$BuildType = 'debug'
)
#>

$orig_loc = ${pwd}
$cfg_script_name = 'cmake-build-cfg.py'
$script_loc = ''
$root_path = ''
$counter = 0
while ($counter -lt 5) {
	if (Test-Path ${cfg_script_name}) {
		$script_loc = ${pwd}
	}
	if (Test-Path 'CMakeLists.txt') {
		$root_path = ${pwd}
		break
	}
	cd ..
	$counter++
}

if ("${counter}" -ge 5) {
	echo "${cfg_script_name} not found in upper directories!"
    exit 1
}

if ($BuildType -eq 'debug')  {
	$BuildDir = 'build-Debug-AT91EK'
}
else {
	$BuildDir = 'build-Release-AT91EK'
}
cd ${script_loc}

$command = "py ${cfg_script_name} -o 'freertos' -g ${Generator} -l ${BuildDir}  -b ${BuildType}"
echo "Executing ${command}"
Invoke-Expression ${command}
$result = "${LASTEXITCODE}"
if ($result -eq 0) {
	cd ${root_path}
}
else {
	cd ${orig_loc}
}

Read-Host -Prompt "Press Enter to exit"
