#! /usr/bin/python3.8
# -*- coding: utf-8 -*-
"""
@file   mib_exporter.py
@brief  Mission Information Base Exporter for the SOURCE project by KSat.
@details
Parses OBSW which is based on FSFW developed by the Institute of Space Systems (IRS) Stuttgart.
Python 3.8 required
This exporter generates the MIB from the SOURCE On-Board Software directly
by using file parser implementations
This exporter has the following capabilities :
    1. Export MIB tables CSV files
    2. Export MIB tables into a SQL database

This exporter currently has parser for following data:
    1. Objects
    2. Returnvalues
    3. Packet content (Telemetry/Telecommands)
    4. Events
    5. Subservices
    6. Device Commands
    7. Global datapool

@developers
Basic Instructions to implement new parserbase:
This parser uses a generic parser class. A specific parser implementation
can be built by implementing the generic parser class.
The parser generally takes a list with all files to parse and a dictionary
with the structure of the MiB table.
This website can be used to experiment with regular expressions: https://regex101.com/

TODO:
    1. Maybe make this file object oriented too.
"""
import os
import pprint

from modgen.utility.mib_csv_writer import CsvWriter
from modgen.utility.mib_printer import Printer
from modgen.utility.mib_sql_writer import SqlWriter
from utility import mib_globals as g
from modgen.parserbase.file_list_parser import FileListParser


from packetcontent.mib_packet_content_parser import (
    PacketContentParser,
    PACKET_CONTENT_DEFINITION_DESTINATION,
    PACKET_CONTENT_CSV_NAME,
    PACKET_CONTENT_HEADER_COLUMN,
    SQL_CREATE_PACKET_DATA_CONTENT_CMD,
    SQL_INSERT_PACKET_DATA_CMD,
    SQL_DELETE_PACKET_DATA_CONTENT_CMD
)
from subservice.mib_subservice_parser import (
    SubserviceParser,
    SUBSERVICE_DEFINITION_DESTINATION,
    SUBSERVICE_CSV_NAME,
    SUBSERVICE_COLUMN_HEADER,
    SQL_CREATE_SUBSVC_CMD,
    SQL_DELETE_SUBSVC_CMD,
    SQL_INSERT_INTO_SUBSVC_CMD,
)
from devicecommands.mib_device_command_parser import (
    DeviceHandlerInformationParser,
    DeviceHandlerCommandParser,
    DH_COMMAND_PACKET_DEFINITION_DESTINATION,
    DH_DEFINITION_DESTINATION,
    DH_COMMANDS_CSV_NAME,
    DH_COMMAND_HEADER_COLUMNS,
    SQL_CREATE_CMDTABLE_CMD,
    SQL_INSERT_INTO_CMDTABLE_CMD,
    SQL_DELETE_CMDTABLE_CMD
)
from returnvalues.returnvalues_parser import (
    InterfaceParser,
    ReturnValueParser,
    INTERFACE_DEFINITION_FILES,
    RETURNVALUE_DESTINATIONS,
    sql_retval_exporter,
    CSV_RETVAL_FILENAME
)
from objects.objects import (
    ObjectDefinitionParser,
    OBJECTS_DEFINITIONS,
    export_object_file,
    CSV_OBJECT_FILENAME,
    sql_object_exporter
)
DO_EXPORT_MIB = True
PRINT_TABLES_TO_CONSOLE = False
EXPORT_TO_CSV = True
EXPORT_TO_SQL = True
COPY_FILE = False
COPY_DESTINATION = "."
FILE_SEPARATOR = ";"
EXECUTE_SQL_COMMANDS = False


def main():
    """
    Performs MIB generation.
    """
    parse_mib()


def parse_mib():
    """
    This is the core function. It builds parses all files,
    builds all tables and returns them in a tuple.
    The structure of respective tables is generated in a
    separate functions and is easily modifiable:
    :return:
    """
    handle_subservices_generation()
    print()
    handle_packet_content_generation()
    print()
    handle_device_handler_command_generation()
    print()
    handle_returnvalue_generation()
    print()
    handle_objects_generation()
    print()
    handle_events_generation()
    print()


def handle_subservices_generation():
    print("MIB Exporter: Parsing subservices")
    subservice_table = generate_subservice_table()
    print("MIB Exporter: Found " + str(len(subservice_table)) + " subservice entries.")
    if PRINT_TABLES_TO_CONSOLE:
        print("MIB Exporter: Printing subservice table: ")
        Printer.print_content(subservice_table)
    if EXPORT_TO_CSV:
        subservice_writer = CsvWriter(
            SUBSERVICE_CSV_NAME, subservice_table, SUBSERVICE_COLUMN_HEADER
        )
        print("MIB Exporter: Exporting to file: " + SUBSERVICE_CSV_NAME)
        subservice_writer.write_to_csv()
    if EXPORT_TO_SQL:
        print("MIB Exporter: Exporting subservices to SQL")
        sql_writer = SqlWriter()
        sql_writer.delete(SQL_DELETE_SUBSVC_CMD)
        sql_writer.sql_writing_helper(
            SQL_CREATE_SUBSVC_CMD, SQL_INSERT_INTO_SUBSVC_CMD, subservice_table
        )


def generate_subservice_table():
    """ Generate the subservice table. """
    subservice_header_parser = FileListParser(
        destination_corrected(SUBSERVICE_DEFINITION_DESTINATION)
    )
    subservice_header_list = subservice_header_parser.parse_header_files(
        False, "MIB Exporter: Parsing subservice files: "
    )
    subservice_file_parser = SubserviceParser(subservice_header_list)
    subservice_table = subservice_file_parser.parse_files()
    return subservice_table


def handle_packet_content_generation():
    print("MIB Exporter: Parsing packing content")
    packet_content_table = generate_packet_content_table()
    print("MIB Exporter: Found " + str(len(packet_content_table)) + " packet content entries.")
    if PRINT_TABLES_TO_CONSOLE:
        print("MIB Exporter: Print packet content table: ")
        Printer.print_content(packet_content_table)
    if EXPORT_TO_CSV:
        packet_content_writer = CsvWriter(
            PACKET_CONTENT_CSV_NAME, packet_content_table, PACKET_CONTENT_HEADER_COLUMN
        )
        print("MIB Exporter: Exporting to file " + PACKET_CONTENT_CSV_NAME)
        packet_content_writer.write_to_csv()
    if EXPORT_TO_SQL:
        print("MIB Exporter: Exporting packet content to SQL")
        sql_writer = SqlWriter()

        sql_writer.sql_writing_helper(
            SQL_CREATE_PACKET_DATA_CONTENT_CMD,
            SQL_INSERT_PACKET_DATA_CMD,
            packet_content_table,
            SQL_DELETE_PACKET_DATA_CONTENT_CMD
        )


def generate_packet_content_table():
    """ Generate packet content table """
    packet_data_header_parser = FileListParser(
        destination_corrected(PACKET_CONTENT_DEFINITION_DESTINATION)
    )
    packet_data_header_list = packet_data_header_parser.parse_header_files(
        False, "MIB Exporter: Parsing packet data files: "
    )
    packet_content_file_parser = PacketContentParser(packet_data_header_list)
    packet_content_table = packet_content_file_parser.parse_files()
    return packet_content_table


def handle_device_handler_command_generation():
    print("MIB Exporter: Parsing device handler commands.")
    dh_command_table = generate_device_command_table()
    print("MIB Exporter: Found " + str(len(dh_command_table)) + " device handler command entries")
    if PRINT_TABLES_TO_CONSOLE:
        print("MIB Exporter: Printing device handler command table: ")
        Printer.print_content(dh_command_table)
    if EXPORT_TO_CSV:
        device_command_writer = CsvWriter(
            DH_COMMANDS_CSV_NAME, dh_command_table, DH_COMMAND_HEADER_COLUMNS
        )
        print("MIB Exporter: Exporting device handler commands to " + DH_COMMANDS_CSV_NAME)
        device_command_writer.write_to_csv()
    if EXPORT_TO_SQL:
        print("MIB Exporter: Exporting device handler commands to SQL")
        sql_writer = SqlWriter()
        sql_writer.sql_writing_helper(
            SQL_CREATE_CMDTABLE_CMD, SQL_INSERT_INTO_CMDTABLE_CMD, dh_command_table,
            SQL_DELETE_CMDTABLE_CMD
        )


def generate_device_command_table(print_info_table: bool = False):
    """ Generate device command table """
    info_header_file_parser = FileListParser(
        destination_corrected(DH_DEFINITION_DESTINATION)
    )
    info_header_file_list = info_header_file_parser.parse_header_files(
        False, "MIB Exporter: Parsing device handler informations: "
    )
    dh_information_parser = DeviceHandlerInformationParser(info_header_file_list)
    dh_information_table = dh_information_parser.parse_files()
    print("MIB Exporter: Found " + str(len(dh_information_table)) +
          " device handler information entries.")
    if print_info_table:
        Printer.print_content(
            dh_information_table, "MIB Exporter: Priting device handler command information table: "
        )

    header_file_parser = FileListParser(
        destination_corrected(DH_COMMAND_PACKET_DEFINITION_DESTINATION)
    )
    header_file_list = header_file_parser.parse_header_files(
        False, "MIB Exporter: Parsing device handler command files: "
    )
    packet_subservice_parser = DeviceHandlerCommandParser(
        header_file_list, dh_information_table
    )
    dh_command_table = packet_subservice_parser.parse_files()
    return dh_command_table


def handle_returnvalue_generation():
    print("MIB Exporter: Parsing returnvalues")
    returnvalue_table = generate_returnvalue_table()
    print("MIB Exporter: Found " + str(len(returnvalue_table)) + " returnvalues.")
    if PRINT_TABLES_TO_CONSOLE:
        print("MIB Exporter: Printing returnvalue table: ")
        Printer.print_content(returnvalue_table)
    if EXPORT_TO_CSV:
        print("MIB Exporter: Exporting returnvalues to " + CSV_RETVAL_FILENAME)
        ReturnValueParser.export_to_file(CSV_RETVAL_FILENAME, returnvalue_table, file_separator=",")
    if EXPORT_TO_SQL:
        print("MIB Exporter: Export returnvalues to SQL: ")
        sql_retval_exporter(returnvalue_table)


def generate_returnvalue_table():
    interface_parser = InterfaceParser(
        destination_corrected(INTERFACE_DEFINITION_FILES), False
    )
    interfaces = interface_parser.parse_files()
    print("MIB Exporter: Found interfaces : " + str(len(interfaces)))
    header_parser = FileListParser(destination_corrected(RETURNVALUE_DESTINATIONS))
    header_list = header_parser.parse_header_files(True, "MIB Exporter: Parsing header file list: ")
    returnvalue_parser = ReturnValueParser(interfaces, header_list, False)
    returnvalue_table = returnvalue_parser.parse_files(False)
    if PRINT_TABLES_TO_CONSOLE:
        Printer.print_content(returnvalue_table, "Returnvalue Table: ")
    return returnvalue_table


def handle_objects_generation():
    print("MIB Exporter: Parsing Objects")
    object_parser = ObjectDefinitionParser(destination_corrected(OBJECTS_DEFINITIONS))
    object_table = object_parser.parse_files()
    object_list_sorted = sorted(object_table.items())
    print("MIB Exporter: Found " + str(len(object_table)) + " entries")
    if EXPORT_TO_CSV:
        print("MIB Exporter: Exporting to file: " + CSV_OBJECT_FILENAME)
        export_object_file(CSV_OBJECT_FILENAME, object_list_sorted)
    if EXPORT_TO_SQL:
        print("MIB Exporter: Exporting objects into SQL table")
        sql_object_exporter(object_list_sorted)


def handle_events_generation():
    pass

def destination_corrected(destination_string):
    """
    If headers are parsed here instead of the respective subdirectories,
    the destination files are located in a different relative destination
    """
    if isinstance(destination_string, list):
        destination_list = []
        for destination in destination_string:
            destination_list.append(destination[3:])
        return destination_list

    return destination_string[3:]


def handle_external_file_running():
    """
    Generates the MIB parser from external files
    TODO: Make this stuff OOP too. Retvals and objects were already refactored
    """
    os.chdir("events")
    os.system("python event_parser.py")
    os.chdir("..")
    print_string = "Exported to file: MIB_Events.csv\r\n"
    return print_string


def update_globals():
    """ Updates the global variables """
    g.PP = pprint.PrettyPrinter(indent=0, width=250)
    g.doExportMIB = DO_EXPORT_MIB
    g.executeSQLcommands = False
    g.printToConsole = PRINT_TABLES_TO_CONSOLE
    g.exportToCSV = EXPORT_TO_CSV
    g.copyFile = COPY_FILE
    g.copyDestination = COPY_DESTINATION
    g.fileSeparator = FILE_SEPARATOR


if __name__ == "__main__":
    main()
