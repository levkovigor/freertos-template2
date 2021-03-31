#!/usr/bin/python3.7
"""
@file       mib_device_command_parser.py
@brief      Parses the device commands which are used for the PUS Service 8 as the primary means
            of satellite commanding.
@details    Used by the MIB Exporter, inherits generic File Parser.
            Also has information parser which parses the possible device handler command values
            from the actual device handlers.
@author     R. Mueller
"""
import re
from enum import Enum
from genmib.parserbase.mib_parser import FileParser
from genmib.parserbase.mib_file_list_parser import FileListParser
from genmib.utility.mib_csv_writer import CsvWriter
from genmib.utility.mib_printer import Printer


DH_COMMAND_PACKET_DEFINITION_DESTINATION = "../../mission/devices/devicedefinitions/"
DH_DEFINITION_DESTINATION = "../../mission/devices/"
DH_COMMANDS_CSV_NAME = "mib_device_commands.csv"
DH_COMMAND_HEADER_COLUMNS = [
    "Device Handler", "Command Name", "Action ID", "Command Field Name", "Command Field Position",
    "Command Field Type", "Command Field Option Name", "Command Field Option Value", "Comment"]

SQL_DELETE_CMDTABLE_CMD = """
    DROP TABLE IF EXISTS DeviceHandlerCommand;
"""

SQL_CREATE_CMDTABLE_CMD = """
    CREATE TABLE IF NOT EXISTS DeviceHandlerCommand(
    id                      INTEGER PRIMARY KEY,
    deviceHandler           TEXT,
    commandName             TEXT,
    actionID                INTEGER,
    cmdFieldName            TEXT,
    cmdFieldPos             INTEGER,
    cmdFieldType            TEXT,
    cmdFieldOptName         TEXT,
    cmdFieldOptVal          INTEGER,
    comment                 COMMENT
    )
    """


SQL_INSERT_INTO_CMDTABLE_CMD = """
INSERT INTO DeviceHandlerCommand(deviceHandler,commandName,actionID,cmdFieldName,cmdFieldPos,
                                 cmdFieldType,cmdFieldOptName,cmdFieldOptVal,comment)
VALUES(?,?,?,?,?,?,?,?,?)
"""


class DeviceCommandColumns(Enum):
    """
    Specifies order of MIB columns
    """
    DH_NAME = 0
    NAME = 1
    ACTION_ID = 2
    COMMAND_FIELD_NAME = 3
    COMMAND_INDEX = 4
    TYPE = 5
    COMMAND_FIELD_OPTION_NAME = 6
    COMMAND_FIELD_OPTION_VALUE = 7
    COMMAND_FIELD_COMMENT = 8


Clmns = DeviceCommandColumns


def main():
    """
    The main routine is run if the device command parser is run separately.
    :return:
    """
    info_header_file_parser = FileListParser(DH_DEFINITION_DESTINATION)
    info_header_file_list = info_header_file_parser.\
        parse_header_files(False, "Parsing device handler informations:")
    dh_information_parser = DeviceHandlerInformationParser(info_header_file_list)
    dh_information_table = dh_information_parser.parse_files()
    Printer.print_content(dh_information_table, "Priting device handler command information table: ")

    header_file_parser = FileListParser(DH_COMMAND_PACKET_DEFINITION_DESTINATION)
    header_file_list = \
        header_file_parser.parse_header_files(False, "Parsing device handler command files:")
    packet_subservice_parser = DeviceHandlerCommandParser(header_file_list, dh_information_table)
    dh_command_table = packet_subservice_parser.parse_files()
    Printer.print_content(dh_command_table, "Printing device handler command table:")
    dh_command_writer = CsvWriter(DH_COMMANDS_CSV_NAME, dh_command_table, DH_COMMAND_HEADER_COLUMNS)
    dh_command_writer.write_to_csv()
    dh_command_writer.copy_csv()
    dh_command_writer.move_csv("..")


# pylint: disable=too-few-public-methods
class DeviceHandlerInformationParser(FileParser):
    """
    This helper class parses device handler informations based on the device handler
    header files. These can be used to map commands to the device handler packets later.
    """

    def __init__(self, fileList):
        super().__init__(fileList)
        self.command_dict = dict()
        self.command_enum_dict = dict()
        self.command_enum_name = ""
        self.command_value_name_list = []
        self.command_value_list = []
        self.command_comment_list = []

        # this table includes the current new table entry, which will be updated
        # for target parameter
        self.command_scanning_pending = False

    # This is called for every file. Fill out info table in this routine
    def _handle_file_parsing(self, file_name, *args):
        self_print_parsing_info = False
        if len(args) == 1 and isinstance(args[0], bool):
            self_print_parsing_info = args[0]

        # Read device name from file name
        handler_match = re.search(r'([\w]*).h', file_name)
        if not handler_match:
            print("Device Command Parser: Configuration error, no handler name match !")
        handler_name = handler_match.group(1)
        file = open(file_name, "r")
        if self_print_parsing_info:
            print("Parsing " + file_name + " ...")
        # Scans each line for possible device handler command enums
        for line in file.readlines():
            self.__handle_line_reading(line)
        handler_tuple = (self.command_dict, self.command_enum_dict)
        handler_dict = dict()
        handler_dict.update({handler_name: handler_tuple})
        self.mib_table.update(handler_dict)

        self.command_dict = dict()
        self.command_enum_dict = dict()

    def __handle_line_reading(self, line):
        """
        Searches for enum command definitions or device command definitions.
        :param line:
        :return:
        """
        # Case insensitive matching of device command enums
        enum_match = re.search(r'[\s]*enum[\s]*([\w]*)[\s]*{[\s][/!<>]*[\s]*'
                               r'\[EXPORT[\w]*\][\s]*:[\s]*\[ENUM\]([^\n]*)', line, re.IGNORECASE)
        if enum_match:
            self.command_enum_name = enum_match.group(1)
            self.command_scanning_pending = True
        else:
            self.__handle_command_definition_scanning(line)

        # while command scanning is pending, each line in enum needs to be parsed
        if self.command_scanning_pending:
            self.__handle_command_enum_scanning(line)

    def __handle_command_definition_scanning(self, line):
        command_match = \
            re.search(r'[\s]*static[\s]*const[\s]*DeviceCommandId_t[\s]*([\w]*)[\s]*=[\s]*'
                      r'([\w]*)[\s]*;[\s]*[/!<>]*[\s]*\[EXPORT\][\s]*:[\s]*\[COMMAND\]', line)
        if command_match:
            command_name = command_match.group(1)
            command_id = command_match.group(2)
            self.command_dict.update({command_name: command_id})

    def __handle_command_enum_scanning(self, line):
        self.__scan_command_entries(line)
        if not self.command_scanning_pending:
            # scanning enum finished
            # stores current command into command dictionary with command name as unique key
            command_tuple = self.command_value_name_list, self.command_value_list, \
                           self.command_comment_list
            self.command_enum_dict.update({self.command_enum_name: command_tuple})
            self.command_enum_name = ""
            self.command_value_name_list = []
            self.command_value_list = []
            self.command_comment_list = []

    def __scan_command_entries(self, line):
        command_match = \
            re.search(r'[\s]*([\w]*)[\s]*=[\s]*([0-9]{1,3})[^/][\s]*[/!<>]*[\s]*([^\n]*)', line)
        if command_match:
            self.command_value_name_list.append(command_match.group(1))
            self.command_value_list.append(command_match.group(2))
            self.command_comment_list.append(command_match.group(3))
        elif re.search(r'}[\s]*;', line):
            self.command_scanning_pending = False

    def _post_parsing_operation(self):
        pass


class PendingScanType(Enum):
    """
    Specifies which scan type is performed in the device command parser.
    """
    NO_SCANNING = 0
    STRUCT_SCAN = 1
    CLASS_SCAN = 2


# pylint: disable=too-many-instance-attributes
class DeviceHandlerCommandParser(FileParser):
    """
    This is the actual device handler command parser. It will parse the device handler
    packet definitions. A device handler info table must be passed which can be acquired
    by running the DH information parser.
    """
    def __init__(self, file_list, dh_information_table):
        super().__init__(file_list)
        # this table includes the current new table entry,
        # which will be updated for target parameter
        self.dict_entry_list = list(range(Clmns.__len__()))

        # This table containts information about respective device handler command options
        self.dh_information_table = dh_information_table
        self.enum_dict = dict()

        self.current_enum_name = ""
        self.comment = ""
        self.command_comment = ""
        self.command_index = 0

        self.scanning_pending = PendingScanType.NO_SCANNING.value

    # This is called for every file, fill out mib_table
    def _handle_file_parsing(self, file_name, *args):
        self_print_parsing_info = False
        if len(args) == 1 and isinstance(args[0], bool):
            self_print_parsing_info = args[0]
        file = open(file_name, "r")

        if self_print_parsing_info:
            print("Parsing " + file_name + " ...")

        # Scans each line for possible device handler command enums
        for line in file.readlines():
            self.__handle_line_reading(line)

    def __handle_line_reading(self, line: str):
        """
        Search for struct command definition
        :param line:
        :return:
        """
        self.__scan_for_commands(line)
        # if self.struct_scanning_pending:

    def __scan_for_commands(self, line):
        # Search for struct command definition
        struct_found = self.__scan_for_structs(line)
        if not struct_found:
            self.__scan_for_class(line)
        if self.scanning_pending is not PendingScanType.NO_SCANNING.value:
            self.__scan_command(line)

    def __scan_for_structs(self, line):
        struct_match = re.search(r'[\s]*struct[\s]*([\w]*)[\s]*{[\s]*[/!<>]*[\s]*'
                                 r'\[EXPORT\][ :]*\[COMMAND\]'
                                 r'[\s]*([\w]*)[ :]*([\w]*)', line)
        if struct_match:
            # Scan a found command struct
            self.__start_class_or_struct_scanning(struct_match)
            self.scanning_pending = PendingScanType.STRUCT_SCAN.value
        return struct_match

    def __scan_for_class(self, line):
        # search for class command definition
        class_match = re.search(r'[\s]*class[\s]*([\w]*)[\s]*[^{]*{[ /!<>]*\[EXPORT\][ :]*'
                                r'\[COMMAND\][\s]*([\w]*)[ :]*([\w]*)', line)
        if class_match:
            self.__start_class_or_struct_scanning(class_match)
            self.scanning_pending = PendingScanType.CLASS_SCAN.value

    def __start_class_or_struct_scanning(self, command_match):
        """
        Stores and assigns values that are the same for each command field option
        :param command_match:
        :return:
        """
        handler_name = command_match.group(2)
        self.dict_entry_list[Clmns.DH_NAME.value] = handler_name
        self.dict_entry_list[Clmns.NAME.value] = command_match.group(1)
        command_name = command_match.group(3)
        if handler_name in self.dh_information_table:
            (command_id_dict, self.enum_dict) = self.dh_information_table[handler_name]
            if command_name in command_id_dict:
                self.dict_entry_list[Clmns.ACTION_ID.value] = command_id_dict[command_name]

    def __scan_command(self, line):
        datatype_match = False
        if self.scanning_pending is PendingScanType.STRUCT_SCAN.value:
            datatype_match = \
                re.search(r'[\s]*(uint[0-9]{1,2}_t|float|double|bool|int|char)[\s]*([\w]*);'
                          r'(?:[\s]*[/!<>]*[\s]*\[EXPORT\][: ]*(.*))?', line)
        elif self.scanning_pending is PendingScanType.CLASS_SCAN.value:
            datatype_match = re.search(
                r'[\s]*SerializeElement[\s]*<(uint[0-9]{1,2}_t|float|double|bool|int|char)[ >]*'
                r'([\w]*);(?:[ /!<>]*\[EXPORT\][: ]*(.*))?', line)
        if datatype_match:
            self.__handle_datatype_match(datatype_match)
        elif re.search(r'}[\s]*;', line):
            self.scanning_pending = PendingScanType.NO_SCANNING.value
            self.command_index = 0

    def __handle_datatype_match(self, datatype_match):
        self.dict_entry_list[Clmns.TYPE.value] = datatype_match.group(1)
        self.dict_entry_list[Clmns.COMMAND_FIELD_NAME.value] = datatype_match.group(2)
        size_of_enum = 0
        if datatype_match.group(3) is not None:
            self.__analyse_exporter_sequence(datatype_match.group(3))
        if self.current_enum_name != "":
            size_of_enum = self.__get_enum_size()
        self.__update_device_command_dict(size_of_enum)

    def __analyse_exporter_sequence(self, exporter_sequence):
        # This matches the exporter sequence pairs e.g. [ENUM] BLA [COMMENT] BLABLA [...] ...
        export_string_matches = re.search(r'(?:\[([\w]*)\][\s]*([^\[]*))?', exporter_sequence)
        if export_string_matches:
            if len(export_string_matches.groups()) % 2 != 0:
                print("Device Command Parser: Error when analysing exporter sequence,"
                      " check exporter string format")
            else:
                count = 0
                while count < len(export_string_matches.groups()):
                    sequence_type = export_string_matches.group(count + 1)
                    sequence_entry = export_string_matches.group(count + 2)
                    count = count + 2
                    self.__handle_sequence_pair(sequence_type, sequence_entry)

    def __handle_sequence_pair(self, sequence_type, sequence_entry):
        if sequence_type.casefold() == "enum":
            self.current_enum_name = sequence_entry
        elif sequence_type.casefold() == "comment":
            self.command_comment = sequence_entry

    def __get_enum_size(self) -> int:
        if self.current_enum_name in self.enum_dict:
            size_of_enum = len(self.enum_dict[self.current_enum_name][1])
            return size_of_enum
        return 0

    def __update_device_command_dict(self, size_of_enum: int = 0):
        if size_of_enum > 0:
            enum_tuple = self.enum_dict[self.current_enum_name]
            for count in range(0, size_of_enum):
                self.__update_table_with_command_options(count, enum_tuple)
            self.command_index = \
                self.command_index + 1
        else:
            self.__update_table_with_no_command_options()
        self.index = self.index + 1
        self.current_enum_name = ""

    def __update_table_with_command_options(self, count, enum_tuple):
        enum_value_name_list, enum_value_list, enum_comment_list = enum_tuple
        self.dict_entry_list[Clmns.COMMAND_FIELD_OPTION_NAME.value] = \
            enum_value_name_list[count]
        self.dict_entry_list[Clmns.COMMAND_FIELD_OPTION_VALUE.value] = enum_value_list[count]
        self.dict_entry_list[Clmns.COMMAND_FIELD_COMMENT.value] = enum_comment_list[count]
        self.dict_entry_list[Clmns.COMMAND_INDEX.value] = \
            self.command_index
        dh_command_tuple = tuple(self.dict_entry_list)
        self.index += 1
        self.mib_table.update({self.index: dh_command_tuple})

    def __update_table_with_no_command_options(self):
        self.dict_entry_list[Clmns.COMMAND_FIELD_OPTION_NAME.value] = ""
        self.dict_entry_list[Clmns.COMMAND_FIELD_OPTION_VALUE.value] = ""
        self.dict_entry_list[Clmns.COMMAND_FIELD_COMMENT.value] = self.command_comment
        self.dict_entry_list[Clmns.COMMAND_INDEX.value] = \
            self.command_index
        dh_command_tuple = tuple(self.dict_entry_list)
        self.mib_table.update({self.index: dh_command_tuple})
        self.command_index += 1

    def _post_parsing_operation(self):
        pass


if __name__ == "__main__":
    main()
