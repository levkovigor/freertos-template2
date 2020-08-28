#! /usr/bin/python3.8
"""
@file       mib_packet_content_parser.py
@brief      Parses the Service Packet Definition files for all variables
@details    Used by the Mib Exporter, inherits generic File Parser
"""
import re

from parserbase.mib_file_list_parser import FileListParser
from parserbase.mib_parser import FileParser
from utility.mib_csv_writer import CsvWriter
from utility.mib_printer import Printer

PACKET_CONTENT_DEFINITION_DESTINATION = ["../../mission/pus/servicepackets/",
                                         "../../fsfw/pus/servicepackets/"]
PACKET_CONTENT_CSV_NAME = "mib_packet_data_content.csv"
PACKET_CONTENT_HEADER_COLUMN = ["Service", "Subservice", "Packet Name", "Datatype", "Name",
                                "Size [Bytes]", "Comment"]

SQL_DELETE_PACKET_DATA_CONTENT_CMD = """
    DROP TABLE IF EXISTS PacketContent;
"""

SQL_CREATE_PACKET_DATA_CONTENT_CMD = """
    CREATE TABLE IF NOT EXISTS PacketContent (
    id              INTEGER PRIMARY KEY,
    service         INTEGER,
    subsvc          INTEGER,
    packetName      TEXT ,
    dataType        TEXT,
    name            TEXT,
    size            INTEGER,
    comment         TEXT
    )
"""

SQL_INSERT_PACKET_DATA_CMD = """
INSERT INTO PacketContent(service,subsvc,packetName,dataType,name,size,comment)
VALUES(?,?,?,?,?,?,?)
"""


def main():
    print("PacketContentParser: Parsing for header files.")
    header_file_parser = FileListParser(PACKET_CONTENT_DEFINITION_DESTINATION)
    header_file_list = header_file_parser.parse_header_files(False, "Parsing packet data files: ")
    packet_content_parser = PacketContentParser(header_file_list)
    subservice_table = packet_content_parser.parse_files(True)
    Printer.print_content(subservice_table, "PacketContentParser: Printing packet data table:")
    subservice_writer = CsvWriter(PACKET_CONTENT_CSV_NAME,
                                  subservice_table, PACKET_CONTENT_HEADER_COLUMN)
    subservice_writer.write_to_csv()
    subservice_writer.move_csv("..")


# noinspection PyTypeChecker
class PacketContentParser(FileParser):
    # Initialize all needed columns
    def __init__(self, file_list):
        super().__init__(file_list)
        self.serviceColumn = 0
        self.subserviceColumn = 1
        self.classNameColumn = 2
        self.datatypeColumn = 3
        self.nameColumn = 4
        self.sizeColumn = 5
        self.commentColumn = 6
        self.lastEntryColumn = 7
        self.columnListLength = 8
        self.dictEntryList = list(range(self.columnListLength-1))

        self.datatypeMatch = False
        self.ignoreFlag = False

    def _handle_file_parsing(self, file_name: str, *args: any):
        self_print_parsing_info = False
        if len(args) == 1 and isinstance(args[0], bool):
            self_print_parsing_info = args[0]

        # Read service from file name
        self.dictEntryList[self.serviceColumn] = re.search('[0-9]{1,3}', file_name).group(0)
        self.dictEntryList[self.subserviceColumn] = " "
        file = open(file_name, "r")
        if self_print_parsing_info:
            print("Parsing " + file_name + " ...")
        # Scans each line for possible variables
        for line in file.readlines():
            # Looks for class and struct definitions which mark a PUS packet
            self.scan_for_class_and_struct_match_and_handle_it(line)
            # Looks for variables
            self.scan_for_variable_match_and_handle_it(line)

    # Operation taken when file parsing is complete
    # All packet content sizes are set by analysing the datatype
    def _post_parsing_operation(self):
        self.update_packet_content_sizes()

    def scan_for_class_and_struct_match_and_handle_it(self, line):
        class_or_struct_match = re.search('[\s]*class[\s]*([\w]*)[\s]*.*[\s]*{[\s]*([^\n]*)', line)
        if not class_or_struct_match:
            class_or_struct_match = re.search(
                '[\s]*struct[\s]*([\w]*)[\s]*.*[\s]*{[\s]*([^\n]*)', line)
        if class_or_struct_match:
            self.dictEntryList[self.classNameColumn] = class_or_struct_match.group(1)
            if class_or_struct_match.group(2):
                self.dictEntryList[self.subserviceColumn] = \
                    self.check_for_subservice_string(class_or_struct_match.group(2))

    def scan_for_variable_match_and_handle_it(self, line):
        # Look for datatype definitions
        var_match = self.packet_content_matcher(line)
        if var_match:
            # Attempts to find variable definition inside that packet
            self.update_packet_content_table()

    def packet_content_matcher(self, line):
        # First step: Search for possible parameter definitions
        # Generic serialize element or datatypes
        var_match = re.search(
            r'[\w]*(?:<)?[\s]*(uint32_t|uint8_t|uint16_t|ReturnValue_t|Mode_t|Submode_t|'
            r'object_id_t|float|double|bool|ActionId_t|EventId_t|sid_t|ParameterId_t)'
            r'(?:>)?[\s]*([\w]*)[\s]*(?:[= 0-9]*)?[;](?:[\/!< ]*([^\n]*))?', line)
        if var_match:
            # Debug printout
            # print(var_match.group(0))
            self.handle_generic_variable_match(var_match)
        # Serial Fixed Array List with Size Header
        else:
            var_match = re.search(r'[ \w]*<SerialFixedArrayListAdapter<([\w_, ()]*)>>'
                                  r'[\s]*([\w]*)[\s]*[;](?:[/!< ]*([^\n]*))?', line)
            if var_match:
                self.handle_serial_fixed_array_match(var_match)
        # Serial Buffer, No length field
        if not var_match:
            var_match = re.search(r'[ \w]*<SerialBufferAdapter<([\w_,]*)>>'
                                  r'[\s]*([\w]*)[\s]*[;](?:[/!< ]*([^\n]*))?', line)
            if not var_match:
                var_match = re.search(r'[\w ]*(?:<)?(uint32_t|uint8_t|uint16_t)[\s]*\*'
                                      r'(?:>)?[\s]*([\w]*)[\s]*[;](?:[/!< ]*([^\n]*))?', line)
            if var_match:
                self.handle_serial_buffer_match(var_match)
        # exclude size definition in serialize adapter or any definitions which are not parameter initializations
        # or typedefs
        if var_match and re.search("typedef", var_match.group(0)):
            var_match = False
        return var_match

    def update_packet_content_table(self):
        self.index = self.index + 1
        dict_entry_tuple = tuple(self.dictEntryList[:self.columnListLength])
        if not self.ignoreFlag:
            self.mib_table.update({self.index: dict_entry_tuple})
        else:
            self.ignoreFlag = False

    def handle_generic_variable_match(self, var_match):
        self.handle_var_match(var_match)
        self.handle_exporter_string(var_match.group(3))

    def handle_serial_fixed_array_match(self, var_match):
        if self.check_for_ignore_string(var_match.group(0)):
            pass
        else:
            fixed_array_properties = re.search(
                '([\w_]*)[\s]*,[\s]*([\w_()]*)[\s]*,[\s]*([\w_()]*)[\s]*', var_match.group(1))
            if fixed_array_properties:
                type_of_next_buffer_size = fixed_array_properties.group(3)
                self.index = self.index + 1
                self.dictEntryList[self.datatypeColumn] = type_of_next_buffer_size
                self.dictEntryList[self.nameColumn] = "Size of following buffer"
                dict_entry_tuple = tuple(self.dictEntryList[:self.columnListLength])
                self.mib_table.update({self.index: dict_entry_tuple})
                self.handle_var_match(var_match)
                self.dictEntryList[self.datatypeColumn] = fixed_array_properties.group(1) + " *"
                self.handle_exporter_string(var_match.group(3))

    def handle_serial_buffer_match(self, var_match):
        self.handle_var_match(var_match)
        self.dictEntryList[self.datatypeColumn] = var_match.group(1) + " *"
        self.dictEntryList[self.sizeColumn] = "deduced"
        self.handle_exporter_string(var_match.group(3))

    def handle_var_match(self, var_match):
        self.dictEntryList[self.commentColumn] = ""
        self.dictEntryList[self.sizeColumn] = ""
        self.dictEntryList[self.datatypeColumn] = var_match.group(1)
        self.dictEntryList[self.nameColumn] = var_match.group(2)

    def update_packet_content_sizes(self):
        self.dictEntryList[self.sizeColumn] = " "
        for key, content in self.mib_table.items():
            content = self.attempt_uint_match(content)
            if not self.datatypeMatch:
                content = self.attempt_eight_byte_match(content)
            if not self.datatypeMatch:
                content = self.attempt_four_byte_match(content)
            if not self.datatypeMatch:
                content = self.attempt_two_byte_match(content)
            if not self.datatypeMatch:
                content = self.attempt_one_byte_match(content)
            content = self.handle_uint_buffer_type(content)
            self.mib_table.update({key: content})

    def attempt_uint_match(self, content):
        self.datatypeMatch = re.search('uint([\d]{1,2})_t', content[self.datatypeColumn])
        if self.datatypeMatch:
            content = list(content)
            content[self.sizeColumn] = round(int(self.datatypeMatch.group(1)) / 8)
            content = tuple(content)
        return content

    def attempt_four_byte_match(self, content):
        self.datatypeMatch = re.search(
            r'object_id_t|ActionId_t|Mode_t|float|sid_t|ParameterId_t',
            content[self.datatypeColumn])
        if self.datatypeMatch:
            content = list(content)
            content[self.sizeColumn] = 4
            content = tuple(content)
        return content

    def attempt_eight_byte_match(self, content):
        self.datatypeMatch = re.search('double', content[self.datatypeColumn])
        if self.datatypeMatch:
            content = list(content)
            content[self.sizeColumn] = 8
            content = tuple(content)
        return content

    def attempt_two_byte_match(self, content):
        self.datatypeMatch = re.search('ReturnValue_t|EventId_t', content[self.datatypeColumn])
        if self.datatypeMatch:
            content = list(content)
            content[self.sizeColumn] = 2
            content = tuple(content)
        return content

    def attempt_one_byte_match(self, content):
        self.datatypeMatch = re.search('Submode_t|bool', content[self.datatypeColumn])
        if self.datatypeMatch:
            content = list(content)
            content[self.sizeColumn] = 1
            content = tuple(content)
        return content

    def handle_uint_buffer_type(self, content):
        if re.search('\*', content[self.datatypeColumn]):
            content = list(content)
            content[self.sizeColumn] = "deduced"
            content = tuple(content)
        return content

    # Used to scan exporter string for ignore flag or store any comments
    def handle_exporter_string(self, match):
        exporter_string = re.search('[ /!<]*\[EXPORT[\w]*\][\s]*:[\s]*([^\n]*)', match)
        if exporter_string:
            type_string = re.search("\[TYPE|BUFFERTYPE\][\s]*([\w]*)[^\n|\[]*", exporter_string.group(0),
                                    re.IGNORECASE)
            if type_string:
                self.dictEntryList[self.datatypeColumn] = str(type_string.group(1)) + " *"
            comment_string = re.search("\[COMMENT\][\s]*([\w]*)[^\n|\[]*", exporter_string.group(0),
                                       re.IGNORECASE)
            if comment_string:
                self.dictEntryList[self.commentColumn] = comment_string.group(1)
            self.check_for_ignore_string(exporter_string.group(0))
            if not comment_string:
                self.dictEntryList[self.commentColumn] = exporter_string.group(1)

    # Used to transform comma separated subservice numbers into specific subservice numbers
    def check_for_subservice_string(self, full_description):
        subservice_info = re.search(
            r'^.*//[\s]*[!<]*[\s]*\[EXPORT[\w]*\][\s]*:[\s]*\[SUBSERVICE\][\s]*([^\n]*)',
            full_description, re.IGNORECASE)
        description = ' '
        if subservice_info:
            description = self.handle_subservice_string(subservice_info)
        if full_description == '':
            description = ' '
        return description

    def check_for_ignore_string(self, string):
        ignore_string = re.search("IGNORE", string, re.IGNORECASE)
        if ignore_string:
            self.ignoreFlag = True
            return True

    @staticmethod
    def handle_subservice_string(subservice_info):
        description = ' '
        subservice_list = [int(x) for x in subservice_info.group(1).split(',')]
        subservice_number = len(subservice_list)
        for i in range(subservice_number):
            description = description + str(subservice_list[i])
            if i == subservice_number - 2:
                description = description + " and "
            elif i < subservice_number - 1:
                description = description + ", "
        return description


if __name__ == "__main__":
    main()



