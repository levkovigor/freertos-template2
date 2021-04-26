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
import re
import datetime
from modgen.utility.mib_csv_writer import CsvWriter
from modgen.utility.mib_printer import PrettyPrinter
from utility.mib_file_management import copy_file
from modgen.parserbase.parser import FileParser
from modgen.utility.mib_sql_writer import SqlWriter, SQL_DATABASE_NAME

DATE_TODAY = datetime.datetime.now()
DATE_STRING_FULL = DATE_TODAY.strftime("%Y-%m-%d %H:%M:%S")

GENERATE_CSV = True
MOVE_CSV = True

GENERATE_CPP = True
COPY_CPP = True

PARSE_HOST_BSP = False

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
        sql_object_exporter(list_items, "../" + SQL_DATABASE_NAME)


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
        write_translation_file(CPP_FILENAME, list_items)
        if COPY_CPP:
            print("ObjectParser: Copying object file to " + CPP_COPY_DESTINATION)
            copy_file(CPP_FILENAME, CPP_COPY_DESTINATION)
    if GENERATE_CSV:
        print("ObjectParser: Generating text export.")
        export_object_file(CSV_OBJECT_FILENAME, list_items)
        if MOVE_CSV:
            csv_writer.move_csv(CSV_MOVE_DESTINATION)


class ObjectDefinitionParser(FileParser):
    def __init__(self, file_list: list):
        super().__init__(file_list)

    def _handle_file_parsing(self, file_name: str, *args, **kwargs):
        file = open(file_name, "r", encoding="utf-8")
        for line in file.readlines():
            match = re.search('([\w]*)[\s]*=[\s]*(0[xX][0-9a-fA-F]+)', line)
            if match:
                self.mib_table.update({match.group(2): [match.group(1)]})

    def _post_parsing_operation(self):
        pass


def export_object_file(filename, object_list):
    file = open(filename, "w")
    for entry in object_list:
        file.write(str(entry[0]) + FILE_SEPARATOR + entry[1][0] + '\n')
    file.close()


def write_translation_file(filename, list_of_entries):
    outputfile = open(filename, "w")
    print('ObjectParser: Writing translation file ' + filename)
    definitions = ""
    function = "const char* translateObject(object_id_t object){\n\tswitch((object&0xFFFFFFFF)){\n"
    for entry in list_of_entries:
        # first part of translate file
        definitions += "const char *" + entry[1][0] + "_STRING = \"" + entry[1][0] + "\";\n"
        # second part of translate file. entry[i] contains 32 bit hexadecimal numbers
        function += "\t\tcase " + str(entry[0]) + ":\n\t\t\treturn " + entry[1][0] + "_STRING;\n"
    function += '\t\tdefault:\n\t\t\treturn "UNKNOWN_OBJECT";\n'
    outputfile.write("/** \n * @brief\tAuto-generated object translation file. Contains "
                     + str(len(list_of_entries)) + " translations. \n"
                    " * Generated on: " + DATE_STRING_FULL + "\n **/ \n")
    outputfile.write("#include \"translateObjects.h\"\n\n")
    outputfile.write(definitions + "\n" + function + "\t}\n\treturn 0;\n}\n")
    outputfile.close()


def sql_object_exporter(object_table: list, sql_table: str = SQL_DATABASE_NAME):
    sql_writer = SqlWriter(sql_table)
    sql_writer.delete(SQL_DELETE_OBJECTS_CMD)
    sql_writer.open(SQL_CREATE_OBJECTS_CMD)
    for entry in object_table:
        sql_writer.write_entries(
            SQL_INSERT_INTO_OBJECTS_CMD, (entry[0], entry[1][0]))
    sql_writer.commit()
    sql_writer.close()


if __name__ == "__main__":
    main()





