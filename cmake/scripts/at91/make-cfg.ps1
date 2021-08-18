$cfg_script_name = 'cmake-build-cfg.py'
$counter = 0
while ($counter -lt 5) {
	if (Test-Path ${cfg_script_name}) {
		break
	}
	cd ..
	$counter++
}

if ("${counter}" -ge 5) {
	echo "${cfg_script_name} not found in upper directories!"
    exit 1
}

Set-PSDebug -Trace 1
py ${cfg_script_name} -o "freertos" -g "Ninja" -l "build-Debug-AT91" -b "debug"
Set-PSDebug -Trace 0
