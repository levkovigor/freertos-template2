#!/usr/bin/python3.8
"""
@file
    mib_datapool_parser.py
@brief
    Parses the global datapools and generates the corresponding source file optionally.
    Python 3.8 required.
@details
    Used by the MIB Exporter, inherits generic File Parser.
@author
    R. Mueller
@date
    03.01.2020
"""
import re
from enum import Enum
from datetime import date

from genmib.parserbase import FileParser
from genmib.utility.mib_csv_writer import CsvWriter
from genmib.utility.mib_printer import Printer
from modgen.utility.file_management import copy_file


DATE_TODAY = date.today()
DATAPOOL_FILE = "../../config/dataPool/dataPoolInit.h"
DATAPOOL_CSV_NAME = "mib_datapool.csv"
CPP_FILE_NAME = "dataPoolInit.cpp"

WRITE_CSV_FILE = True
COPY_CSV_FILE = True

WRITE_CPP_FILE = True
COPY_CPP_FILE = False

CPP_COPY_DESTINATION = "../../config/dataPool/"
DATAPOOL_HEADER_COLUMNS = ["Pool ID", "Group", "Name", "Code Name", "Size", "Type", "Unit"]


class DatapoolColumns(Enum):
    """
    Specifies order of MIB columns
    """
    POOL_ID = 0
    GROUP = 1
    NAME = 2
    CODE_NAME = 3
    SIZE = 4
    TYPE = 5
    UNIT = 6


Clmns = DatapoolColumns


def main():
    """
    This main is run if the datapool parser is to be run separately.
    :return:
    """
    file_list = [DATAPOOL_FILE]
    print("DatapoolParser: Parsing datapool header file:")
    print(file_list[0])
    datapool_parser = DatapoolParser(file_list)
    datapool_table = datapool_parser.parse_files()
    Printer.print_content(datapool_table, "DatapoolParser: Printing datapool variable table:")
    dh_command_writer = CsvWriter(DATAPOOL_CSV_NAME, datapool_table, DATAPOOL_HEADER_COLUMNS)
    if WRITE_CSV_FILE:
        dh_command_writer.write_to_csv()
        if COPY_CSV_FILE:
            dh_command_writer.move_csv("..")
    if WRITE_CPP_FILE:
        datapool_parser.write_data_pool_init_cpp()
        print("DatapoolParser: C++ File was created.")
        if COPY_CPP_FILE:
            copy_file(CPP_FILE_NAME, CPP_COPY_DESTINATION)
            print("DatapoolParser: Generated C++ file was copied to " + CPP_COPY_DESTINATION)


class DatapoolParser(FileParser):
    """
    This parser reads the central datapool header file.
    It can optionally generate the corresponding source file (C++11 needed).

    Instantiate the class by supplying a number of files to parse and call the parse_files() method
    :return:
    """
    def __init__(self, file_list):
        super().__init__(file_list)

        # this table includes the current new table entry,
        # which will be updated for target parameter
        self.dict_entry_list = list(range(Clmns.__len__()))

        self.cpp_file_information_list = []
        self.parse_success = False
        self.parse_for_datapool_entries = False
        self.prev_group = ""

    def _handle_file_parsing(self, file_name: str, *args, **kwargs):
        file = open(file_name, "r")
        print("Parsing " + file_name + " ...")
        linecount = 1
        for line in file.readlines():
            self.__handle_line_reading(line)
            linecount = linecount + 1

    def _post_parsing_operation(self):
        if len(self.mib_table) > 0:
            self.parse_success = True

    def write_data_pool_init_cpp(self):
        """
        Writes the data pool CPP source file if the parsing was successfull
        :param copy_cpp:
        :param cpp_file_name:
        :param cpp_copy_destination:
        :return:
        """
        if not self.parse_success:
            print("Datapool Parser: MIB Table is empty, no data to write CPP file")
            return
        cpp_file = open(CPP_FILE_NAME, "w")
        current_date = DATE_TODAY.strftime("%d.%m.%Y")
        header = "/**\n * @file\tdataPoolInit.cpp\n *\n * @brief\tAuto-Generated datapool " \
                 "initialization\n * @date\t" + current_date +\
                 "\n */\n#include <config/dataPool/dataPoolInit.h> " \
                 "\n\nvoid datapool::dataPoolInit(poolMap * poolMap) {\n"
        cpp_file.write(header)
        entries_len = self.index + 1
        for index in range(1, entries_len):
            self.__handle_entry_write(cpp_file, index)
        tail = "\n}\n"
        cpp_file.write(tail)
        cpp_file.close()

    def __handle_line_reading(self, line):
        if self.parse_for_datapool_entries:
            if not self.__scan_for_pool_variable(line):
                self.__scan_for_export_string(line)
        if not self.parse_for_datapool_entries:
            datapool_start = re.search(r'[\s]*enum[\w_ ]*[\s]*{', line)
            if datapool_start:
                self.parse_for_datapool_entries = True
        else:
            self.__scan_for_datapool_end(line)

    def __scan_for_export_string(self, line):
        export_string_match = re.search(r'[/*!>< ]*\[EXPORT\][ :]*([^\n]*)', line, re.IGNORECASE)
        if export_string_match:
            self.__handle_export_string_match(export_string_match.group(1))
        return export_string_match

    def __handle_export_string_match(self, string):
        group_match = re.search(r'\[GROUP\][\s]*([^\n\*]*)', string, re.IGNORECASE)
        if group_match:
            self.dict_entry_list[Clmns.GROUP.value] = group_match.group(1).rstrip()

    def __scan_for_pool_variable(self, line):
        pool_var_match = re.search(r'[\s]*([\w]*)[ =]*([\w]*)(?:,)?[\s]*([^\n]*)', line)
        if pool_var_match:
            if pool_var_match.group(1) == "":
                return False
            self.__handle_pool_var_match(pool_var_match)
        return pool_var_match

    def __handle_pool_var_match(self, pool_var_match):
        if re.search(r'NO_PARAMETER', pool_var_match.group(0)):
            return
        self.dict_entry_list[Clmns.CODE_NAME.value] = pool_var_match.group(1)
        self.dict_entry_list[Clmns.POOL_ID.value] = pool_var_match.group(2)
        export_string_match = re.search(r'[/!< ]*\[EXPORT\][: ]*([^\n]*)',
                                        pool_var_match.group(3), re.IGNORECASE)
        if export_string_match:
            self.__handle_pool_var_export_string(export_string_match.group(1))
        datapool_tuple = tuple(self.dict_entry_list)
        self.index = self.index + 1
        self.mib_table.update({self.index: datapool_tuple})
        self.dict_entry_list[Clmns.SIZE.value] = ""
        self.dict_entry_list[Clmns.TYPE.value] = ""
        self.dict_entry_list[Clmns.NAME.value] = ""
        self.dict_entry_list[Clmns.UNIT.value] = ""

    def __handle_pool_var_export_string(self, string):
        extracted_entries = re.findall(r'(?:\[([\w]*)\][\s]*([^\[]*))?', string)
        if extracted_entries:
            extraced_entries_len = len(extracted_entries) - 1
            for group_index in range(extraced_entries_len):
                (group_name, group_content) = extracted_entries[group_index]
                group_content = group_content.rstrip()
                if group_name.casefold() == "name":
                    self.dict_entry_list[Clmns.NAME.value] = group_content
                elif group_name.casefold() == "size":
                    self.dict_entry_list[Clmns.SIZE.value] = group_content
                elif group_name.casefold() == "type":
                    self.dict_entry_list[Clmns.TYPE.value] = group_content
                elif group_name.casefold() == "unit":
                    self.dict_entry_list[Clmns.UNIT.value] = group_content

    def __scan_for_datapool_end(self, line):
        datapool_end = re.search(r'}[\s]*;', line)
        if datapool_end:
            self.parse_for_datapool_entries = False

    def __handle_entry_write(self, cpp_file, index):
        current_mib_entry = self.mib_table.get(index)
        (_, current_group, _, current_code_name,
         current_size, current_type, _) = current_mib_entry
        if current_group != self.prev_group:
            cpp_file.write("\n\t/* " + current_group + " */\n")
            self.prev_group = current_group
        current_pool_entry_init_value = "{"
        if current_size == "":
            print("Size is unknown for a pool entry. Please specify in header file !")
            return
        current_size = int(current_size)
        for count in range(current_size):
            current_pool_entry_init_value = current_pool_entry_init_value + "0"
            if count != current_size - 1:
                current_pool_entry_init_value = current_pool_entry_init_value + ", "
        current_pool_entry_init_value = current_pool_entry_init_value + "}"
        entry_string = "\tpoolMap->emplace(datapool::" + current_code_name + \
                       ",\n\t\t\tnew PoolEntry<" + current_type + ">(" + \
                       current_pool_entry_init_value + "," + str(current_size) + "));\n"
        cpp_file.write(entry_string)


if __name__ == "__main__":
    main()
