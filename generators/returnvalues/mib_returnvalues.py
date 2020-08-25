#! /usr/bin/python3.8
"""
@file
    mib_returnvalues.py
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
import re
# import getpass
# import MySQLdb
from parserbase.mib_parser import FileParser
from parserbase.mib_file_list_parser import FileListParser
from utility.mib_csv_writer import CsvWriter
from utility.mib_printer import Printer, PrettyPrinter
from utility.mib_sql_writer import SqlWriter

EXPORT_TO_FILE = True
MOVE_CSV_FILE = True
EXPORT_TO_SQL = True
PRINT_TABLES = True
CSV_RETVAL_FILENAME = "mib_returnvalues.csv"
CSV_MOVE_DESTINATION = "../"
FILE_SEPARATOR = ';'
MAX_STRING_LENGTH = 32
INTERFACE_DEFINITION_FILES = ["../../fsfw/returnvalues/FwClassIds.h",
                              "../../config/returnvalues/classIds.h"]
RETURNVALUE_DESTINATIONS = ["../../mission/", "../../fsfw/", "../../config/", "../../sam9g20/"]

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
        ReturnValueParser.export_to_file(CSV_RETVAL_FILENAME, returnvalue_table)
        if MOVE_CSV_FILE:
            handle_file_move(CSV_MOVE_DESTINATION)
    if EXPORT_TO_SQL:
        print("ReturnvalueParser: Exporting to SQL")
        sql_retval_exporter(returnvalue_table)


def parse_returnvalues():
    """ Core function to parse for the return values """
    interface_parser = InterfaceParser(INTERFACE_DEFINITION_FILES, PRINT_TABLES)
    interfaces = interface_parser.parse_files()
    header_parser = FileListParser(RETURNVALUE_DESTINATIONS)
    header_list = header_parser.parse_header_files(True, "Parsing header file list: ")
    returnvalue_parser = ReturnValueParser(interfaces, header_list, PRINT_TABLES)
    returnvalue_table = returnvalue_parser.parse_files(True)
    if PRINT_TABLES:
        Printer.print_content(returnvalue_table, "Returnvalue Table: ")
    print("ReturnvalueParser: "
          "Found " + str(len(returnvalue_table)) + " returnvalues.")
    return returnvalue_table


def handle_file_move(destination: str):
    """ Handles moving the CSV file somewhere """
    csv_writer = CsvWriter(CSV_RETVAL_FILENAME)
    if MOVE_CSV_FILE:
        csv_writer.move_csv(destination)


class InterfaceParser(FileParser):
    def __init__(self, file_list, print_table):
        super().__init__(file_list)
        self.count = 0
        self.print_table = print_table

    def _handle_file_parsing(self, file_name: str, *args, **kwargs):
        try:
            file = open(file_name, 'r', encoding='utf-8')
            all_lines = file.readlines()
        except UnicodeDecodeError:
            file = open(file_name, 'r', encoding='cp1252')
            all_lines = file.readlines()
        if "FwClassIds.h" in file_name:
            count_matched = False
            # Parses first entry, which has explicit value 1
            for line in all_lines:
                if not count_matched:
                    match = re.search(r'[\s]*([A-Z_0-9]*) = ([0-9]*),[\s]*//([A-Z]{1,3})', line)
                else:
                    match = re.search(r'[\s]*([A-Z_0-9]*),[\s]*//([A-Z]{1,3})', line)
                if match and not count_matched:
                    self.count = int(match.group(2))
                    self.mib_table.update({match.group(1): [1, match.group(3)]})
                    self.count += 1
                    count_matched = True
                elif match:
                    self.mib_table.update({match.group(1): [self.count, match.group(2)]})
                    self.count += 1
        elif "classIds.h" in file_name:
            file = open(file_name, "r")
            all_lines = file.readlines()
            for line in all_lines:
                match = re.search(r'[\s]*([\w]*) = FW_CLASS_ID_COUNT,[\s]*(//([A-Z]{1,3}))?', line)
                if match:
                    self.mib_table.update({match.group(1): [self.count, match.group(2)]})
                    self.count += 1
            for line in all_lines:
                match = re.search(r'^[\s]*([\w]*)[,]*[\s]*//[!<]*[\s]*([^\n]*)', line)
                if match:
                    self.mib_table.update({match.group(1): [self.count, match.group(2)]})
                    self.count += 1

    def _post_parsing_operation(self):
        if self.print_table:
            PrettyPrinter.pprint(self.mib_table)

class ReturnValueParser(FileParser):
    """
    Generic return value parser.
    """
    def __init__(self, interfaces, file_list, print_tables):
        super().__init__(file_list)
        self.print_tables = print_tables
        self.interfaces = interfaces
        self.return_value_dict = dict()
        self.count = 0
        # Stores last three lines
        self.last_lines = ["", "", ""]
        self.current_interface_id_entries = {
            "Name": "",
            "ID": 0,
            "FullName": ""
        }
        self.return_value_dict.update({0: ('OK', 'System-wide code for ok.', 'RETURN_OK',
                                           'HasReturnvaluesIF.h', 'HasReturnvaluesIF')})
        self.return_value_dict.update({1: ('Failed', 'Unspecified system-wide code for failed.',
                                           'RETURN_FAILED', 'HasReturnvaluesIF.h',
                                           'HasReturnvaluesIF')})

    def _handle_file_parsing(self, file_name: str, *args, **kwargs):
        try:
            file = open(file_name, 'r', encoding='utf-8')
            all_lines = file.readlines()
        except UnicodeDecodeError:
            print("ReturnValueParser: Decoding error with file " + file_name)
            file = open(file_name, 'r', encoding='cp1252')
            all_lines = file.readlines()
        if len(args) == 1:
            print_truncated_entries = args[0]
        else:
            print_truncated_entries = False
        for line in all_lines:
            self.__handle_line_reading(line, file_name, print_truncated_entries)

    def __handle_line_reading(self, line, file_name, print_truncated_entries: bool):
        newline = line
        if self.last_lines[0] != '\n':
            two_lines = self.last_lines[0] + ' ' + newline.strip()
        else:
            two_lines = ''
        interface_id_match = re.search(r'INTERFACE_ID[\s]*=[\s]*CLASS_ID::([a-zA-Z_0-9]*)',
                                       two_lines)
        if interface_id_match:
            self.__handle_interfaceid_match(interface_id_match)

        returnvalue_match = re.search(
            r'^[\s]*static const(?:expr)? ReturnValue_t[\s]*([a-zA-Z_0-9]*)[\s]*=[\s]*'
            r'MAKE_RETURN_CODE[\s]*\([\s]*([x0-9a-fA-F]{1,4})[\s]*\);[\t ]*(//)?([^\n]*)',
            two_lines)
        if returnvalue_match:
            self.__handle_returnvalue_match(returnvalue_match, file_name, print_truncated_entries)
        self.last_lines[1] = self.last_lines[0]
        self.last_lines[0] = newline

    def __handle_interfaceid_match(self, interface_id_match):
        # print("Interface ID" + str(match1.group(1)) + "found in " + fileName)
        self.current_interface_id_entries["ID"] = \
            self.interfaces[interface_id_match.group(1)][0]
        self.current_interface_id_entries["Name"] = \
            self.interfaces[interface_id_match.group(1)][1]
        self.current_interface_id_entries["FullName"] = interface_id_match.group(1)
        # print( "Current ID: " + str(self.current_interface_id_entries["ID"]) )

    def __handle_returnvalue_match(self, returnvalue_match, file_name: str,
                                   print_truc_entries: bool):
        # valueTable.append([])
        description = self.clean_up_description(returnvalue_match.group(4))
        string_to_add = self.build_checked_string(
            self.current_interface_id_entries["Name"], returnvalue_match.group(1),
            print_truc_entries)
        full_id = (self.current_interface_id_entries["ID"] << 8) + return_number_from_string(
            returnvalue_match.group(2))
        if full_id in self.return_value_dict:
            # print('Duplicate returncode ' + hex(full_id) + ' from ' + file_name +
            #      ' was already in ' + self.return_value_dict[full_id][3])
            pass
        self.return_value_dict.update(
            {full_id: (string_to_add, description, returnvalue_match.group(1),
                       file_name, self.current_interface_id_entries["FullName"])})
        #                 valueTable[count].append(fullId)
        #                 valueTable[count].append(stringToAdd)
        self.count = self.count + 1

    def _post_parsing_operation(self):
        if self.print_tables:
            PrettyPrinter.pprint(self.return_value_dict)
        self.mib_table = self.return_value_dict

    @staticmethod
    def export_to_file(filename: str, list_of_entries: dict):
        file = open(filename, "w")
        for entry in list_of_entries.items():
            file.write(hex(entry[0]) + FILE_SEPARATOR + entry[1][0] + FILE_SEPARATOR + entry[1][1] +
                       FILE_SEPARATOR + entry[1][2] + FILE_SEPARATOR
                       + entry[1][3] + FILE_SEPARATOR + entry[1][4] + '\n')
        file.close()

    def build_checked_string(self, first_part, second_part, print_truncated_entries: bool):
        """ Build a checked string """
        my_str = first_part + '_' + self.convert(second_part)
        if len(my_str) > MAX_STRING_LENGTH:
            if print_truncated_entries:
                print("Warning: Entry " + my_str + " too long. Will truncate.")
            my_str = my_str[0:MAX_STRING_LENGTH]
        else:
            # print("Entry: " + myStr + " is all right.")
            pass
        return my_str

    @staticmethod
    def convert(name):
        single_strings = name.split('_')
        new_string = ''
        for one_string in single_strings:
            one_string = one_string.lower()
            one_string = one_string.capitalize()
            new_string = new_string + one_string
        return new_string

    @staticmethod
    def clean_up_description(descr_string):
        description = descr_string.lstrip('!<- ')
        if description == '':
            description = ' '
        return description



def return_number_from_string(a_string):
    if a_string.startswith('0x'):
        return int(a_string, 16)
    if a_string.isdigit():
        return int(a_string)
    print('Error: Illegal number representation: ' + a_string)
    return 0


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


# def writeEntriesToDB(listOfEntries):
#     print("Connecting to database...")
#     user = getpass.getpass("User: ")
#     passwd = getpass.getpass()
#     conn = MySQLdb.connect(host="127.0.0.1", user=user, passwd=passwd, db="flpmib")
#     written = conn.cursor()
#     print("done.")
#     # delete old entries
#     print("Kill old entries.")
#     written.execute("DELETE FROM txp WHERE TXP_NUMBR = 'DSX00000'")
#     print("Insert new ones:")
#     for entry in listOfEntries.items():
#         written.execute("INSERT INTO txp (txp_numbr, txp_from, txp_to, txp_altxt) "
#                         "VALUES ('DSX00000', %s, %s, %s)", [entry[0], entry[0], entry[1][0]])
#     conn.commit()
#     print("Done. That was easy.")
#
#
# def writeEntriesToOtherDB(listOfEntries):
#     print("Connecting to other database...")
#     conn = MySQLdb.connect(host="buggy.irs.uni-stuttgart.de",
#                            user='returncode', passwd='returncode', db="returncode")
#     written = conn.cursor()
#     print("connected.")
#     # delete old entries
#     print("Kill old entries.")
#     written.execute("DELETE FROM returncodes WHERE true")
#     print("Insert new ones:")
#     for entry in listOfEntries.items():
#         written.execute("INSERT INTO returncodes (code,name,interface,file,description) "
#                         "VALUES (%s, %s, %s, %s, %s)",
#                         [entry[0], entry[1][2], entry[1][4], entry[1][3], entry[1][1]])
#     conn.commit()
#     print("Done. That was hard.")


if __name__ == "__main__":
    main()
