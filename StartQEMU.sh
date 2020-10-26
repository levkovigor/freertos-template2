#!/bin/bash 

# -- Command Line Interface Definitions ----------------------------------------

scriptname="StartQEMU.sh"

# help text
read -r -d '' cli_help <<EOD
QEMU startup helper for IOBC/AT91.

Usage:
    ${scriptname} [FLAGS] [OPTIONS]

Flags:
	-h, --help  	Print this help message
	-v, --verbose 	Enable verbose output

Options:
	-g, --waitdbgu  Wait for debugger connection
	-f, --profile <profile-name>

Supported Profiles:
	sdram    		Debug configuration for SDRAM
	norflash		NOR-Flash boot configuration

Supported Register Overrides:
	pmc-mclk		Override PMC master clock for debug-boot

Examples:
	${scriptname} -g 
	${scriptname} -f norflash
EOD


# -- QEMU IOBC Definitions -----------------------------------------------------

declare -A iobc_mem_addr=(
    # ["bootmem"]=$((   0x00000000 ))
    # ["rom"]=$((       0x00100000 ))
    # ["sram0"]=$((     0x00200000 ))
    # ["sram1"]=$((     0x00300000 ))
    ["norflash"]=$((  0x10000000 ))
    ["sdram"]=$((     0x20000000 ))
)

# -- Helper Functions ----------------------------------------------------------

# check if ${1} is an integer
function is_integer() {
    printf "%x" "${1}" > /dev/null 2>&1
    return $?
}

# try to convert ${1} to a valid memory address
function iobc_mem_addr_to_integer() {
    addr="${1}"

    if [ ${iobc_mem_addr["${addr}"]+x} ]
    then
        echo "${iobc_mem_addr["${addr}"]}"
        return 0

    elif is_integer "${addr}"
    then
        echo "${addr}"
        return 0

    else
        return 1
    fi
}

# -- Command Line Parser -------------------------------------------------------

arg_positionals=()
arg_help=n
arg_verbose=n
arg_debug=n
arg_program_counter=
arg_load_profile="sdram"
arg_overrides=()
arg_qemu_args=()

while (( "${#}" ))
do
    case "${1}" in
        -h|--help)
            arg_help=y
            shift
            ;;
        -v|--verbose)
            arg_verbose=y
            shift
            ;;
        -g|--waitdbgu)
        	arg_debug=y
        	shift
        	;;
        -f|--load)
            if [ "${#}" -ge 2 ]
            then
                arg_load_profile="${2}"
            else
                echo "error: Missing argument for ${1}"
                exit 1
            fi
            shift 2
            ;;
    esac
done

# handle help
if [ ${arg_help} = y ]
then
    echo "${cli_help}"
    exit 0
fi

[ ${arg_verbose} = y ] && printf "Info: Specified profile %s\n" "$arg_load_profile"

# echo ${iobc_mem_addr["${arg_load_profile}"]}

# if $arg_load_profile != "sdram" && ! addr=$(iobc_mem_addr_to_integer "${arg_load_profile}"); then
#   echo "error: Invalid memory address for program memory."
#   echo "       Expected integer or region, got '${arg_load_profile}'."
#   exit 1
# fi


# -- Main Logic ----------------------------------------------------------------

req_directory="../obc-qemu"
if [ ! -d "$req_directory" ]; then
	echo "Requirements to start QEMU not met."
	echo "obc-qemu directory has to exist in same folder as the OBSW folder."
	echo "Please make sure to clone and build QEMU properly."
	exit
fi

directory=_bin
files=$(find . -type f -name "*.bin")

index=0
declare -a fileArray

for file in $files; do
printf "File $((index)): %s\r\n" "$file"
fileArray=("${fileArray[@]}" "$file")
index=$index+1
done;

echo "Select file with given index: "
read selection
if [[ -n ${selection//[0-9]/} ]]; then
echo "Selection contains letters!"
exit 
fi

if [[ $selection -lt  0 || $selection -gt ${#fileArray[@]} ]]; then
echo "Selection is invalid!"
exit
fi

target_binary=${fileArray[0]}
echo "Selected target file: $targetFile"

echo Launching QEMU:
echo

dbgu_flags=
loader_flags=

# Launch without waiting for debugger connection
if [ "$arg_debug" == "n" ]; then
	dbgu_flags="-s"
else
	dbgu_flags="-s -S"
fi

if [ "$arg_load_profile" == "sdram" ]; then
	loader_flags="-f sdram "${target_binary}" -s sdram -o pmc-mclk"
else
	loader_flags="-f norflash ${target_binary} -s bootmem -o pmc-mclk"
fi

# TODO: Generate SD card image automatically after checking whether it already exists.
	

# Everything after -- is passed to QEMU directly, everything before
# is passed to separate loader program.
../obc-qemu/iobc-loader ${loader_flags} \
	-- -serial stdio -monitor none \
	-drive if=sd,index=0,format=raw,file=../obc-qemu/build/sd0.img \
	${dbgu_flags}



