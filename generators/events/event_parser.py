"""Part of the Mission Information Base Exporter for the SOURCE project by KSat.
Event exporter.
To use MySQLdb, run pip install mysqlclient or install in IDE.
On Windows, Build Tools installation might be necessary
@data   21.11.2019
"""
import datetime
from xml.etree import ElementTree as ElementTree
from xml.dom import minidom

from fsfwgen.events.event_parser import handle_csv_export, handle_cpp_export, \
    SubsystemDefinitionParser, EventParser
from fsfwgen.parserbase.file_list_parser import FileListParser
from fsfwgen.utility.printer import PrettyPrinter
from fsfwgen.utility.file_management import copy_file, move_file

from definitions import ROOT_DIR, OBSW_ROOT_DIR

DATE_TODAY = datetime.datetime.now()
DATE_STRING_FULL = DATE_TODAY.strftime("%Y-%m-%d %H:%M:%S")

GENERATE_CPP_H = True
COPY_CPP_FILE = True
COPY_CPP_H_FILE = True
MOVE_CSV_FILE = True

PARSE_HOST_BSP = False

CSV_MOVE_DESTINATION = f'{ROOT_DIR}'

CPP_FILENAME = f'{__package__}/translateEvents.cpp'
CPP_H_FILENAME = f'{__package__}/translateEvents.h'

if PARSE_HOST_BSP:
    BSP_FOLDER = "bsp_hosted"
else:
    BSP_FOLDER = "bsp_sam9g20"

CSV_FILENAME = f"{BSP_FOLDER}_events.csv"
CPP_COPY_DESTINATION = f'{OBSW_ROOT_DIR}/{BSP_FOLDER}/fsfwconfig/events/'

FILE_SEPARATOR = ";"
SUBSYSTEM_DEFINITION_DESTINATIONS = [
    f'{OBSW_ROOT_DIR}/{BSP_FOLDER}/fsfwconfig/events/subsystemIdRanges.h',
    f'{OBSW_ROOT_DIR}/fsfw/src/fsfw/events/fwSubsystemIdRanges.h',
    f'{OBSW_ROOT_DIR}/common/events/commonSubsystemIds.h'
]
HEADER_DEFINITION_DESTINATIONS = [
    f'{OBSW_ROOT_DIR}/mission/', f'{OBSW_ROOT_DIR}/fsfw/src/fsfw',
    f'{OBSW_ROOT_DIR}/{BSP_FOLDER}', f'{OBSW_ROOT_DIR}/test/'
]


def parse_events(generate_csv: bool = True, generate_cpp: bool = True):
    print("EventParser: Parsing events: ")
    event_list = generate_event_list()
    # xml_test()

    if generate_csv:
        handle_csv_export(
            file_name=CSV_FILENAME, event_list=event_list, file_separator=FILE_SEPARATOR
        )
    if generate_cpp:
        handle_cpp_export(
            event_list=event_list, date_string=DATE_STRING_FULL, file_name=CPP_FILENAME,
            generate_header=GENERATE_CPP_H, header_file_name=CPP_H_FILENAME
        )
        if COPY_CPP_FILE:
            print(f"EventParser: Copying file to {CPP_COPY_DESTINATION}")
            copy_file(CPP_FILENAME, CPP_COPY_DESTINATION)
            copy_file(CPP_H_FILENAME, CPP_COPY_DESTINATION)
    print("")


def generate_event_list() -> list:
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
    event_list = sorted(event_table.items())
    print(f"Found {len(event_list)} entries:")
    PrettyPrinter.pprint(event_list)
    return event_list


def xml_test():
    root = ElementTree.Element('a')
    root.append(ElementTree.Comment("Comment Block"))
    b = ElementTree.SubElement(root, 'b', {"hallo": "hallo"})
    b.text = "Hallo"
    c = ElementTree.SubElement(root, 'c')
    c.text = "Hallo"
    d = ElementTree.SubElement(c, 'd')
    d.text = "Hallo2"

    tree = ElementTree.ElementTree(element=root)

    xml_string_rep = ElementTree.tostring(root, method="xml", encoding='utf-8', xml_declaration=True)
    xml_pretty_str = minidom.parseString(xml_string_rep).toprettyxml(indent="    ")
    # with open("New_Database.xml", "w") as f:
    #     f.write(xml_pretty_str)

    tree.write("New_Database.xml", encoding='utf-8', xml_declaration=True)
    print(xml_string_rep)
    print(xml_pretty_str)
    # your XML file, encoded as UTF-8
