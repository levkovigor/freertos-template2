"""Part of the Mission Information Base Exporter for the SOURCE project by KSat.
Object exporter.
To use MySQLdb, run pip install mysqlclient or install in IDE.
On Windows, Build Tools installation might be necessary
"""
import datetime

from fsfwgen.objects.objects import ObjectDefinitionParser, sql_object_exporter, \
    write_translation_file, export_object_file, write_translation_header_file
from fsfwgen.utility.printer import PrettyPrinter
from fsfwgen.utility.file_management import copy_file, move_file
from definitions import DATABASE_NAME, OBSW_ROOT_DIR, ROOT_DIR

DATE_TODAY = datetime.datetime.now()
DATE_STRING_FULL = DATE_TODAY.strftime("%Y-%m-%d %H:%M:%S")

GENERATE_CSV = True
MOVE_CSV = True

GENERATE_CPP = True
COPY_CPP = True

GENERATE_HEADER = True

PARSE_HOST_BSP = False

EXPORT_TO_SQL = True

if PARSE_HOST_BSP:
    BSP_DIR_NAME = "bsp_hosted"
else:
    BSP_DIR_NAME = "bsp_sam9g20"

CPP_COPY_DESTINATION = f"{OBSW_ROOT_DIR}/{BSP_DIR_NAME}/fsfwconfig/{__package__}/"
CSV_MOVE_DESTINATION = f"{ROOT_DIR}"
CPP_FILENAME = f"{__package__}/translateObjects.cpp"
CPP_H_FILENAME = f"{__package__}/translateObjects.h"
CSV_OBJECT_FILENAME = f"{__package__}/{BSP_DIR_NAME}_objects.csv"
FILE_SEPARATOR = ";"

if PARSE_HOST_BSP:
    OBJECTS_PATH = f"{OBSW_ROOT_DIR}/{BSP_DIR_NAME}/fsfwconfig/objects/systemObjectList.h"
else:
    OBJECTS_PATH = f"{OBSW_ROOT_DIR}/{BSP_DIR_NAME}/fsfwconfig/objects/systemObjectList.h"

FRAMEWORK_OBJECTS_PATH = f"{OBSW_ROOT_DIR}/fsfw/src/fsfw/objectmanager/frameworkObjects.h"
COMMON_OBJECTS_PATH = f"{OBSW_ROOT_DIR}/common/objects/commonObjectsList.h"
OBJECTS_DEFINITIONS = [OBJECTS_PATH, FRAMEWORK_OBJECTS_PATH, COMMON_OBJECTS_PATH]

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


def parse_objects():
    print("Parsing objects: ")
    # fetch objects
    object_parser = ObjectDefinitionParser(OBJECTS_DEFINITIONS)
    subsystem_definitions = object_parser.parse_files()
    # id_subsystem_definitions.update(framework_subsystem_definitions)
    list_items = sorted(subsystem_definitions.items())
    PrettyPrinter.pprint(list_items)
    print("ObjectParser: Number of objects: ", len(list_items))

    handle_file_export(list_items)
    if EXPORT_TO_SQL:
        print("ObjectParser: Exporting to SQL")
        sql_object_exporter(
            object_table=list_items, delete_cmd=SQL_DELETE_OBJECTS_CMD,
            insert_cmd=SQL_INSERT_INTO_OBJECTS_CMD,
            create_cmd=SQL_CREATE_OBJECTS_CMD, db_filename=f"../{DATABASE_NAME}"
        )


def handle_file_export(list_items):
    if GENERATE_CPP:
        print("ObjectParser: Generating translation C++ file.")
        write_translation_file(
            filename=CPP_FILENAME, list_of_entries=list_items, date_string_full=DATE_STRING_FULL
        )
        if COPY_CPP:
            print("ObjectParser: Copying object file to " + CPP_COPY_DESTINATION)
            copy_file(CPP_FILENAME, CPP_COPY_DESTINATION)
    if GENERATE_HEADER:
        write_translation_header_file(filename=CPP_H_FILENAME)
        copy_file(filename=CPP_H_FILENAME, destination=CPP_COPY_DESTINATION)
    if GENERATE_CSV:
        print("ObjectParser: Generating text export.")
        export_object_file(
            filename=CSV_OBJECT_FILENAME, object_list=list_items, file_separator=FILE_SEPARATOR
        )
        if MOVE_CSV:
            move_file(file_name=CSV_OBJECT_FILENAME, destination=CSV_MOVE_DESTINATION)
