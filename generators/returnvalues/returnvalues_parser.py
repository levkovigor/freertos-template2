#! /usr/bin/python3
# -*- coding: utf-8 -*-
"""
@file
    returnvalues_parser.py
@brief
    Part of the Mission Information Base Exporter for the SOURCE project by KSat.
    TODO: Integrate into Parser Structure instead of calling this file (no cpp file generated yet)
@details
    Returnvalue exporter.

    To use MySQLdb, run pip install mysqlclient or install in IDE.
    On Windows, Build Tools installation might be necessary.

@data
    21.11.2019
"""
from modgen.parserbase.file_list_parser import FileListParser
from modgen.returnvalues.returnvalues_parser import InterfaceParser, ReturnValueParser
from modgen.utility.sql_writer import SqlWriter
from modgen.utility.csv_writer import CsvWriter
from modgen.utility.printer import Printer

EXPORT_TO_FILE = True
MOVE_CSV_FILE = True
EXPORT_TO_SQL = True
PRINT_TABLES = True

CSV_RETVAL_FILENAME = "mib_returnvalues.csv"
CSV_MOVE_DESTINATION = "../"
FILE_SEPARATOR = ';'
MAX_STRING_LENGTH = 32
INTERFACE_DEFINITION_FILES = [
    "../../fsfw/returnvalues/FwClassIds.h",
    "../../bsp_sam9g20/fsfwconfig/returnvalues/classIds.h"
]
RETURNVALUE_DESTINATIONS = [
    "../../mission/", "../../fsfw/", "../../bsp_sam9g20/", "../../bsp_hosted/"
]
# RETURNVALUE_DESTINATIONS = ["../../sam9g20/"]

SQL_DELETE_RETURNVALUES_CMD = """
    DROP TABLE IF EXISTS Returnvalues
"""

SQL_CREATE_RETURNVALUES_CMD = """
    CREATE TABLE IF NOT EXISTS Returnvalues (
    id              INTEGER PRIMARY KEY,
    code            TEXT,
    name            TEXT,
    interface       TEXT,
    file            TEXT,
    description     TEXT
    )
"""

SQL_INSERT_RETURNVALUES_CMD = """
INSERT INTO Returnvalues(code,name,interface,file,description)
VALUES(?,?,?,?,?)
"""


def main():
    returnvalue_table = parse_returnvalues()
    if EXPORT_TO_FILE:
        ReturnValueParser.export_to_file(CSV_RETVAL_FILENAME, returnvalue_table, FILE_SEPARATOR)
        if MOVE_CSV_FILE:
            handle_file_move(CSV_MOVE_DESTINATION)
    if EXPORT_TO_SQL:
        pass
        # print("ReturnvalueParser: Exporting to SQL")
        # sql_retval_exporter(returnvalue_table)


def parse_returnvalues():
    """ Core function to parse for the return values """
    interface_parser = InterfaceParser(INTERFACE_DEFINITION_FILES, PRINT_TABLES)
    interfaces = interface_parser.parse_files()
    header_parser = FileListParser(RETURNVALUE_DESTINATIONS)
    header_list = header_parser.parse_header_files(True, "Parsing header file list: ")
    returnvalue_parser = ReturnValueParser(interfaces, header_list, PRINT_TABLES)
    returnvalue_parser.set_moving_window_mode(moving_window_size=7)
    returnvalue_table = returnvalue_parser.parse_files(True)
    if PRINT_TABLES:
        Printer.print_content(returnvalue_table, "Returnvalue Table: ")
    print(
        f"ReturnvalueParser: Found {len(returnvalue_table)} returnvalues.")
    return returnvalue_table


def handle_file_move(destination: str):
    """ Handles moving the CSV file somewhere """
    csv_writer = CsvWriter(CSV_RETVAL_FILENAME)
    if MOVE_CSV_FILE:
        csv_writer.move_csv(destination)


def sql_retval_exporter(returnvalue_table):
    sql_writer = SqlWriter()
    sql_writer.open(SQL_CREATE_RETURNVALUES_CMD)
    for entry in returnvalue_table.items():
        sql_writer.write_entries(
            SQL_INSERT_RETURNVALUES_CMD, (entry[0],
                                          entry[1][2],
                                          entry[1][4],
                                          entry[1][3],
                                          entry[1][1]))
    sql_writer.commit()
    sql_writer.close()


if __name__ == "__main__":
    main()
