"""
@file       mib_subservice_parser.py
@brief      Parses the Subservice definitions for the Mission Information Base.
@details    Used by the MIB Exporter, inherits generic File Parser
@author     R. Mueller
@date       14.11.2019

Example Stringset to scan for:

enum Subservice: uint8_t {
		//!< [EXPORT] : [COMMAND] Perform connection test
	    CONNECTION_TEST = 1,
	    //!< [EXPORT] : [REPLY] Connection test reply
	    CONNECTION_TEST_REPORT = 2,
	    EVENT_TRIGGER_TEST = 128, //!<  [EXPORT] : [COMMAND] Trigger test reply and test event
	    MULTIPLE_EVENT_TRIGGER_TEST = 129, //!< [EXPORT] : [COMMAND] Trigger multiple events (5)
	    MULTIPLE_CONNECTION_TEST = 130 //!< [EXPORT] : [COMMAND] Trigger multiple connection tests
	};

"""
import re
from enum import Enum

from parserbase.mib_file_list_parser import FileListParser
from parserbase.mib_parser import FileParser
from utility.mib_csv_writer import CsvWriter
from utility.mib_printer import Printer

SUBSERVICE_DEFINITION_DESTINATION = "../../mission/pus/"
SUBSERVICE_CSV_NAME = "mib_subservices.csv"
SUBSERVICE_COLUMN_HEADER = ["Service", "Subservice Name", "Subservice Number", "Type", "Comment"]

SQL_DELETE_SUBSVC_CMD = """
    DROP TABLE IF EXISTS Subservice;
    """

SQL_CREATE_SUBSVC_CMD = """
    CREATE TABLE IF NOT EXISTS Subservice(
    id              INTEGER PRIMARY KEY,
    service         INTEGER,
    subsvcName      TEXT,
    subsvcNumber    INTEGER,
    type            TEXT CHECK( type IN ('TC','TM')),
    comment         TEXT
    )
    """

SQL_INSERT_INTO_SUBSVC_CMD = """
INSERT INTO Subservice(service,subsvcName,subsvcNumber,type,comment)
VALUES(?,?,?,?,?)
"""

class SubserviceColumns(Enum):
    """
    Specifies order of MIB columns
    """
    SERVICE = 0
    NAME = 1
    NUMBER = 2
    TYPE = 3
    COMMENT = 4


Clmns = SubserviceColumns


def main():
    """
    If this file is run separately, this main will be run.
    :return:
    """
    header_parser = FileListParser(SUBSERVICE_DEFINITION_DESTINATION)
    header_file_list = header_parser.parse_header_files(False, "Parsing subservice header files: ")
    packet_subservice_parser = SubserviceParser(header_file_list)
    subservice_table = packet_subservice_parser.parse_files()
    Printer.print_content(subservice_table, "Printing subservice table:")
    print("Found " + str(len(subservice_table)) + " subservice entries.")
    subservice_writer = CsvWriter(SUBSERVICE_CSV_NAME, subservice_table, SUBSERVICE_COLUMN_HEADER)
    subservice_writer.write_to_csv()
    subservice_writer.move_csv("..")



# TODO: Not really happy with the multi-line implementation, but this is not trivial..
#       Right not, we are not using the last lines stored, we just store the string
#       of the last line (if its only a comment). It propably would be better to always
#       scan 3 or 4 lines at once. However, this is not easy too..
# pylint: disable=too-few-public-methods
class SubserviceParser(FileParser):
    """
    This parser class can parse the subservice definitions.
    """
    def __init__(self, file_list: list):
        super().__init__(file_list)
        # Column System allows reshuffling of table columns in constructor
        self.clmns_len = SubserviceColumns.__len__()
        # this table includes the current new table entry,
        # which will be updated for target parameter
        self.dict_entry_list = list(range(self.clmns_len))
        self.dict_entry_list[Clmns.COMMENT.value] = ""
        self.subservice_enum_found = False
        # This list will store the last three lines for longer comments.
        self.last_line_list = ["", "", ""]
        # If an export command was found, cache the possibility of a match.
        self.possible_match_on_next_lines = False

    # This is called for every file
    def _handle_file_parsing(self, file_name: str, *args: any, **kwargs):
        self_print_parsing_info = False
        if len(args) == 1 and isinstance(args[0], bool):
            self_print_parsing_info = args[0]

        # Read service from file name
        service_match = re.search('Service[^0-9]*([0-9]{1,3})', file_name)
        if service_match:
            self.dict_entry_list[Clmns.SERVICE.value] = service_match.group(1)
        self.dict_entry_list[Clmns.NAME.value] = " "
        file = open(file_name, "r")
        if self_print_parsing_info:
            print("Parsing " + file_name + " ...")
        # Scans each line for possible variables
        for line in file.readlines():
            self.__handle_line_reading(line)

    def __handle_line_reading(self, line):
        """
        Handles the reading of single lines.
        :param line:
        :return:
        """
        # Case insensitive matching
        enum_match = re.search(r'[\s]*enum[\s]*Subservice([^\n]*)', line, re.IGNORECASE)
        if enum_match:
            self.subservice_enum_found = True
        if self.subservice_enum_found:
            self.__handle_enum_scanning(line)
            self.last_line_list[2] = self.last_line_list[1]
            self.last_line_list[1] = self.last_line_list[0]
            self.last_line_list[0] = line

    def __handle_enum_scanning(self, line: str):
        """
        Two-line reading. First check last line. For export command.
        """
        self.__scan_for_export_command(self.last_line_list[0])
        subservice_match = self.__scan_subservices(line)
        if subservice_match:
            self.index = self.index + 1
            dict_entry_tuple = tuple(self.dict_entry_list[:self.clmns_len])
            self.mib_table.update({self.index: dict_entry_tuple})
            self.__clear_tuple()

    def __clear_tuple(self):
        self.dict_entry_list[Clmns.NAME.value] = ""
        self.dict_entry_list[Clmns.TYPE.value] = ""
        self.dict_entry_list[Clmns.NUMBER.value] = ""
        self.dict_entry_list[Clmns.COMMENT.value] = ""
        self.possible_match_on_next_lines = False


    def __scan_for_export_command(self, line: str) -> bool:
        command_string = re.search(r"([^\[]*)\[export\][: ]*\[([\w]*)\][\s]*([^\n]*)",
                                   line, re.IGNORECASE)
        if command_string:
            # Check whether there is a separated export command
            # (export command is not on same line as subservice definition)
            # ugly solution but has worked so far.
            string = command_string.group(1).lstrip()
            if len(string) <= 8:
                self.possible_match_on_next_lines = True
                if self.__scan_for_type(line):
                    self.__scan_for_comment(line)
            return True
        self.__add_possible_comment_string(line)
        return False

    def __add_possible_comment_string(self, line):
        """
        If no command was found, the line might be a continuation of a comment.
        Strip whitespaces and comment symbols and add to comment buffer.
        """
        possible_multiline_comment = line.lstrip()
        possible_multiline_comment = possible_multiline_comment.lstrip('/')
        possible_multiline_comment = possible_multiline_comment.lstrip('<')
        possible_multiline_comment = possible_multiline_comment.lstrip('!')
        possible_multiline_comment = possible_multiline_comment.rstrip()
        if len(possible_multiline_comment) > 0:
            self.dict_entry_list[Clmns.COMMENT.value] += possible_multiline_comment

    def __scan_subservices(self, line):
        """
        Scan for subservice match.
        :param line:
        :return:
        """
        subservice_match = \
            re.search(r"[\s]*([\w]*)[\s]*=[\s]*([0-9]{1,3})(?:,)?(?:[ /!<>]*([^\n]*))?", line)
        if subservice_match:
            self.dict_entry_list[Clmns.NAME.value] = subservice_match.group(1)
            self.dict_entry_list[Clmns.NUMBER.value] = subservice_match.group(2)
            # I am assuming that an export string is longer than 7 chars.
            if len(subservice_match.group(3)) > 7:
                # Export command on same line overrides old commands. Read for comment.
                if self.__process_comment_string(subservice_match.group(3)):
                    return True
            # Check whether exporting was commanded on last lines
            return bool(self.possible_match_on_next_lines)
        if re.search(r'}[\s]*;', line):
            self.subservice_enum_found = False
        return subservice_match

    def __process_comment_string(self, comment_string) -> bool:
        # look for packet type specifier
        export_command_found = self.__scan_for_type(comment_string)
        # Look for everything after [EXPORT] : [TYPESPECIFIER] as comment
        if export_command_found:
            self.__scan_for_comment(comment_string)
        return export_command_found

    def __scan_for_type(self, string) -> bool:
        type_match = re.search(r'\[reply\]|\[tm\]', string, re.IGNORECASE)
        if type_match:
            self.dict_entry_list[Clmns.TYPE.value] = 'TM'
            return True
        type_match = re.search(r'\[command\]|\[tc\]', string, re.IGNORECASE)
        if type_match:
            self.dict_entry_list[Clmns.TYPE.value] = 'TC'
            return True
        self.dict_entry_list[Clmns.TYPE.value] = 'Unspecified'
        return False

    def __scan_for_comment(self, comment_string):
        comment_match = re.search(r':[\s]*\[[\w]*\][\s]*([^\n]*)', comment_string)
        if comment_match:
            self.dict_entry_list[Clmns.COMMENT.value] = comment_match.group(1)

    def _post_parsing_operation(self):
        pass


if __name__ == "__main__":
    main()
