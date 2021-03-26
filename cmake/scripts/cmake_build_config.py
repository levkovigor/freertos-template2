#!/usr/bin/env python3
# -*- coding: utf-8 -*
"""
@brief  CMake configuration helper
@details
This script was written to have a portable way to perform the CMake configuration with various parameters on
different OSes. It was first written for the FSFW Example, but could be adapted to be more generic
in the future.

Run cmake_build_config.py --help to get more information.
"""
import os
import platform
import sys
import argparse
import shutil


def main():
    print("-- Python CMake build configurator utility --")

    print("Parsing command line arguments..")
    parser = argparse.ArgumentParser(description="Processing arguments for CMake build configuration.")
    parser.add_argument("-o", "--osal", type=str, choices=["freertos", "linux", "rtems", "host"],
                        help="FSFW OSAL. Valid arguments: host, linux, rtems, freertos")
    parser.add_argument("-b", "--buildtype", type=str, choices=["debug", "release", "size", "reldeb"],
                        help="CMake build type. Valid arguments: debug, release, size, reldeb "
                             "(Release with Debug Information)", default="debug")
    parser.add_argument("-l", "--builddir", type=str, help="Specify build directory.")
    parser.add_argument("-g", "--generator", type=str, help="CMake Generator")
    parser.add_argument(
        "-d", "--defines", 
        help="Additional custom defines passed to CMake (supply without -D prefix!)",
        nargs="*", type=str
    )
    parser.add_argument("-t", "--target-bsp", type=str, help="Target BSP, combination of "
                                                             "architecture and machine")

    args = parser.parse_args()

    print("Determining source location..")
    source_location = determine_source_location()
    print(f"Determined source location: {source_location}")

    print("Building cmake configuration command..")

    if args.generator is None:
        generator = determine_build_generator()
        generator_cmake_arg = f"-G \"{generator}\""
    else:
        generator_cmake_arg = f"-G \"{args.generator}\""

    if args.osal is None:
        print("No FSFW OSAL specified.")
        cmake_fsfw_osal = determine_fsfw_osal()
    else:
        cmake_fsfw_osal = args.osal

    cmake_build_type = determine_build_type(args.buildtype)

    """
    if args.target_bsp is not None:
        cmake_target_cfg_cmd = f"-DTGT_BSP=\"{args.target_bsp}\""
    else:
        cmake_target_cfg_cmd = determine_tgt_bsp(cmake_fsfw_osal)
    """
    
    define_string = ""
    if args.defines is not None:
        for define in args.defines:
            define_string += f"-D{define} "
    
    if args.builddir is None:
        cmake_build_folder = determine_build_folder(cmake_build_type)
    else:
        cmake_build_folder = args.builddir

    build_path = source_location + os.path.sep + cmake_build_folder
    if os.path.isdir(build_path):
        remove_old_dir = input(f"{cmake_build_folder} folder already exists. "
                               f"Remove old directory? [y/n]: ")
        if str(remove_old_dir).lower() in ["yes", "y", 1]:
            remove_old_dir = True
        else:
            cmake_build_folder = determine_new_folder()
            build_path = source_location + os.path.sep + cmake_build_folder
            remove_old_dir = False
        if remove_old_dir:
            try:
                shutil.rmtree(build_path)
            except PermissionError as error:
                print(error)
                print("File might still be opened!")
                sys.exit(0)
    os.chdir(source_location)
    os.mkdir(cmake_build_folder)
    print(f"Navigating into build directory: {build_path}")
    os.chdir(cmake_build_folder)

    cmake_command = f"cmake {generator_cmake_arg} -DOS_FSFW=\"{cmake_fsfw_osal}\" " \
                    f"-DCMAKE_BUILD_TYPE=\"{cmake_build_type}\" {define_string} {source_location}"
    # Remove redundant spaces
    cmake_command = ' '.join(cmake_command.split())
    print("Running CMake command (without +): ")
    print(f"+ {cmake_command}")
    os.system(cmake_command)
    print("-- CMake configuration done. --")


def determine_build_generator() -> str:
    print("No generator specified. ")
    print("Please select from the following list of build types or type "
          "in desired system directly [h for help]: ")
    while True:
        user_input = input("Enter your selection: ")
        if user_input == "h":
            os.system("cmake --help")
        else:
            build_generator = user_input
            confirmation = input(f"Confirm your generator: {build_generator} [y/n]: ")
            if confirmation in ["y", "yes", 1]:
                break
    return build_generator


def determine_build_folder(cmake_build_type: str) -> str:
    confirm = input(f"No build folder specified. Set to build type name {cmake_build_type}? [y/n]: ")
    if confirm in ["yes", "y", 1]:
        return cmake_build_type
    else:
        new_folder_name = input("Please enter folder name, will be created in source folder: ")
        return new_folder_name


def determine_source_location() -> str:
    index = 0
    while not os.path.isdir("fsfw"):
        index += 1
        os.chdir("..")
        if index >= 5:
            print("Error: Could not find source directory (determined by looking for fsfw folder!)")
            sys.exit(1)
    return os.getcwd()


def determine_fsfw_osal() -> str:
    select_dict = dict({
        1: "host",
        2: "linux",
        3: "freertos",
        4: "rtems"
    })
    print("No build type specified. Please select from the following list of build types: ")
    for key, item in select_dict.items():
        print(f"{key}: {item}")
    select = input("Enter your selection: ")
    while True:
        if select.isdigit():
            select_num = int(select)
            if select_num >= 1 or select_num <= 4:
                return select_dict[select_num]
            else:
                print("Input digit is invalid!")
        else:
            print("Input is not a digit!")


def determine_build_type(build_type_arg) -> str:
    if build_type_arg is None:
        select_dict = dict({
            1: "Debug",
            2: "Release",
            3: "Release with Debug Information",
            4: "Size"
        })
        print("No build type specified. Please select from the following list of build types")
        for key, item in select_dict.items():
            print(f"{key}: {item}")
        select = input("Enter your selection: ")
        while True:
            if select.isdigit():
                select_num = int(select)
                if select_num >= 1 or select_num <= 4:
                    cmake_build_type = select_dict[select_num]
                    break
                else:
                    print("Input digit is invalid!")
            else:
                print("Input is not a digit!")
    else:
        if build_type_arg == "debug":
            cmake_build_type = "Debug"
        elif build_type_arg == "release":
            cmake_build_type = "Release"
        elif build_type_arg == "size":
            cmake_build_type = "MinSizeRel"
        elif build_type_arg == "reldeb":
            cmake_build_type = "RelWithDebInfo"
        else:
            print("Unknown buildtype.")
            cmake_build_type = determine_build_type(None)
    return cmake_build_type


def determine_new_folder() -> str:
    new_folder = input(f"Use different folder name? [y/n]: ")
    if str(new_folder).lower() in ["yes", "y", 1]:
        new_folder_name = input("New folder name: ")
        return new_folder_name
    else:
        print("Aborting configuration.")
        sys.exit(0)


"""
def determine_tgt_bsp(osal: str) -> str:
    if osal == "freertos":
        print("Target BSP set to arm/stm32h743zi-nucleo")
        osal = "arm/stm32h743zi-nucleo"
    elif osal == "linux":
        print("No target BSP specified. Please select from the following list of build types.")
        select_dict = dict({
            1: "arm/raspberrypi",
            2: "none/hosted"
        })
        for key, item in select_dict.items():
            print(f"{key}: {item}")
        select = input("Enter your selection: ")
        while True:
            if select.isdigit():
                select_num = int(select)
                if select_num >= 1 or select_num <= 2:
                    osal = select_dict[select_num]
                    break
                else:
                    print("Input digit is invalid!")
            else:
                print("Input is not a digit!")
    return osal
"""

if __name__ == "__main__":
    main()
