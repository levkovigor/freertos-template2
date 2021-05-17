#! /usr/bin/env python3
"""
@file   objects.py
@brief  Part of the Mission Information Base Exporter for the SOURCE project by KSat.
@details
Object exporter.
To use MySQLdb, run pip install mysqlclient or install in IDE.
On Windows, Build Tools installation might be necessary
@data   21.11.2019
"""
import datetime

from modgen.objects.objects import ObjectDefinitionParser, sql_object_exporter, write_translation_file, \
    export_object_file
from modgen.utility.csv_writer import CsvWriter
from modgen.utility.printer import PrettyPrinter
from modgen.utility.file_management import copy_file
from modgen.utility.sql_writer import SQL_DATABASE_NAME

DATE_TODAY = datetime.datetime.now()
DATE_STRING_FULL = DATE_TODAY.strftime("%Y-%m-%d %H:%M:%S")

GENERATE_CSV = True
MOVE_CSV = True

GENERATE_CPP = True
COPY_CPP = True

PARSE_HOST_BSP = True

EXPORT_TO_SQL = True

if PARSE_HOST_BSP:
    BSP_DIR_NAME = "bsp_hosted"
else:
    BSP_DIR_NAME = "bsp_sam9g20"

CPP_COPY_DESTINATION = f"../../{BSP_DIR_NAME}/fsfwconfig/objects/"
CSV_MOVE_DESTINATION = "../"
CPP_FILENAME = "translateObjects.cpp"
CSV_OBJECT_FILENAME = "mib_objects.csv"
FILE_SEPARATOR = ";"

if PARSE_HOST_BSP:
    SUBSYSTEM_DEFINITION_DESTINATION = f"../../{BSP_DIR_NAME}/fsfwconfig/objects/systemObjectList.h"
else:
    SUBSYSTEM_DEFINITION_DESTINATION = f"../../{BSP_DIR_NAME}/fsfwconfig/objects/systemObjectList.h"

FRAMEWORK_SUBSYSTEM_DEFINITION_DESTINATION = "../../fsfw/objectmanager/frameworkObjects.h"
OBJECTS_DEFINITIONS = [SUBSYSTEM_DEFINITION_DESTINATION, FRAMEWORK_SUBSYSTEM_DEFINITION_DESTINATION]

SQL_DELETE_OBJECTS_CMD = """
    DROP TABLE IF EXISTS Objects
    """

SQL_CREATE_OBJECTS_CMD = """
    CREATE TABLE IF NOT EXISTS Objects(
    id              INTEGER PRIMARY KEY,
    objectid        TEXT,
    name            TEXT
    )
    """

SQL_INSERT_INTO_OBJECTS_CMD = """
INSERT INTO Objects(objectid, name)
VALUES(?,?)
"""


def main():
    print("Parsing objects: ")
    list_items = parse_objects()
    handle_file_export(list_items)
    if EXPORT_TO_SQL:
        print("ObjectParser: Exporting to SQL")
        sql_object_exporter(
            object_table=list_items, delete_cmd=SQL_DELETE_OBJECTS_CMD, insert_cmd=SQL_INSERT_INTO_OBJECTS_CMD,
            create_cmd=SQL_CREATE_OBJECTS_CMD, sql_table="../" + SQL_DATABASE_NAME
        )


def parse_objects():
    # fetch objects
    object_parser = ObjectDefinitionParser(OBJECTS_DEFINITIONS)
    subsystem_definitions = object_parser.parse_files()
    # id_subsystem_definitions.update(framework_subsystem_definitions)
    list_items = sorted(subsystem_definitions.items())
    PrettyPrinter.pprint(list_items)
    print("ObjectParser: Number of objects: ", len(list_items))
    return list_items


def handle_file_export(list_items):
    csv_writer = CsvWriter(CSV_OBJECT_FILENAME)
    if GENERATE_CPP:
        print("ObjectParser: Generating translation C++ file.")
        write_translation_file(filename=CPP_FILENAME, list_of_entries=list_items, date_string_full=DATE_STRING_FULL)
        if COPY_CPP:
            print("ObjectParser: Copying object file to " + CPP_COPY_DESTINATION)
            copy_file(CPP_FILENAME, CPP_COPY_DESTINATION)
    if GENERATE_CSV:
        print("ObjectParser: Generating text export.")
        export_object_file(filename=CSV_OBJECT_FILENAME, object_list=list_items, file_separator=FILE_SEPARATOR)
        if MOVE_CSV:
            csv_writer.move_csv(CSV_MOVE_DESTINATION)


if __name__ == "__main__":
    main()





