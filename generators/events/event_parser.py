#! /usr/bin/python3
"""
@file   event_parser.py
@brief  Part of the Mission Information Base Exporter for the SOURCE project by KSat.
@details
Event exporter.

To use MySQLdb, run pip install mysqlclient or install in IDE.
On Windows, Build Tools installation might be necessary
@data   21.11.2019
"""
import datetime

from modgen.events.event_parser import handle_csv_export, handle_cpp_export, SubsystemDefinitionParser, EventParser
from modgen.parserbase.file_list_parser import FileListParser
from modgen.utility.printer import PrettyPrinter
from modgen.utility.file_management import copy_file, move_file

DATE_TODAY = datetime.datetime.now()
DATE_STRING_FULL = DATE_TODAY.strftime("%Y-%m-%d %H:%M:%S")

GENERATE_CPP = True
GENERATE_CPP_H = True
GENERATE_CSV = True
COPY_CPP_FILE = True
COPY_CPP_H_FILE = True
MOVE_CSV_FILE = True

PARSE_HOST_BSP = False

CSV_MOVE_DESTINATION = "../"

CPP_FILENAME = "translateEvents.cpp"
CPP_H_FILENAME = "translateEvents.h"

if PARSE_HOST_BSP:
    BSP_FOLDER = "bsp_hosted"
else:
    BSP_FOLDER = "bsp_sam9g20"

CSV_FILENAME = f"{BSP_FOLDER}_events.csv"
CPP_COPY_DESTINATION = f"../../{BSP_FOLDER}/fsfwconfig/events/"

FILE_SEPARATOR = ";"
SUBSYSTEM_DEFINITION_DESTINATIONS = [
    f"../../{BSP_FOLDER}/fsfwconfig/events/subsystemIdRanges.h",
    "../../fsfw/events/fwSubsystemIdRanges.h",
    "../../common/events/commonSubsystemIds.h"
]
HEADER_DEFINITION_DESTINATIONS = ["../../mission/", "../../fsfw/", f"../../{BSP_FOLDER}", "../../test/"]


def main():
    print("EventParser: Parsing events: ")
    event_list = parse_events()
    if GENERATE_CSV:
        handle_csv_export(file_name=CSV_FILENAME, event_list=event_list, file_separator=FILE_SEPARATOR)
        if MOVE_CSV_FILE:
            move_file(file_name=CSV_FILENAME, destination=CSV_MOVE_DESTINATION)
    if GENERATE_CPP:
        handle_cpp_export(
            event_list=event_list, date_string=DATE_STRING_FULL, file_name=CPP_FILENAME,
            generate_header=GENERATE_CPP_H, header_file_name=CPP_H_FILENAME
        )
        if COPY_CPP_FILE:
            print(f"EventParser: Copying file to {CPP_COPY_DESTINATION}")
            copy_file(CPP_FILENAME, CPP_COPY_DESTINATION)
            copy_file(CPP_H_FILENAME, CPP_COPY_DESTINATION)
    print("")


def parse_events():
    subsystem_parser = SubsystemDefinitionParser(SUBSYSTEM_DEFINITION_DESTINATIONS)
    subsystem_table = subsystem_parser.parse_files()
    print(f"Found {len(subsystem_table)} subsystem definitions.")
    PrettyPrinter.pprint(subsystem_table)
    event_header_parser = FileListParser(HEADER_DEFINITION_DESTINATIONS)
    event_headers = event_header_parser.parse_header_files(
        True, "Parsing event header file list:\n", True
    )
    # PrettyPrinter.pprint(event_headers)
    # myEventList = parseHeaderFiles(subsystem_table, event_headers)
    event_parser = EventParser(event_headers, subsystem_table)
    event_parser.set_moving_window_mode(moving_window_size=7)
    event_table = event_parser.parse_files()
    list_items = sorted(event_table.items())
    print(f"Found {len(list_items)} entries:")
    PrettyPrinter.pprint(list_items)
    return list_items


if __name__ == "__main__":
    main()
