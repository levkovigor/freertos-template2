#! /usr/bin/python3
"""
@file   mib_events.py
@brief  Part of the Mission Information Base Exporter for the SOURCE project by KSat.
@details
Event exporter.

To use MySQLdb, run pip install mysqlclient or install in IDE.
On Windows, Build Tools installation might be necessary
@data   21.11.2019
"""
import re
import datetime
from parserbase.mib_file_list_parser import FileListParser
from parserbase.mib_parser import FileParser
from utility.mib_printer import PrettyPrinter
from utility.mib_file_management import copy_file, move_file

DATE_TODAY = datetime.datetime.now()
DATE_STRING_FULL = DATE_TODAY.strftime("%Y-%m-%d %H:%M:%S")

GENERATE_CPP = True
GENERATE_CSV = True
COPY_CPP_FILE = True
MOVE_CSV_FILE = True

CSV_FILENAME = "mib_events.csv"
CSV_MOVE_DESTINATION = "../"

CPP_FILENAME = "translateEvents.cpp"
CPP_COPY_DESTINATION = "../../fsfwconfig/events/"

FILE_SEPARATOR = ";"
SUBSYSTEM_DEFINITION_DESTINATIONS = ["../../fsfwconfig/events/subsystemIdRanges.h",
                                     "../../fsfw/events/fwSubsystemIdRanges.h"]
HEADER_DEFINITION_DESTINATIONS = ["../../mission/", "../../fsfw/"]


def main():
    print("EventParser: Parsing events: ")
    event_list = parse_events()
    if GENERATE_CSV:
        handle_csv_export(CSV_FILENAME, event_list)
        if MOVE_CSV_FILE:
            move_file(CSV_FILENAME, CSV_MOVE_DESTINATION)
    if GENERATE_CPP:
        handle_cpp_export(CPP_FILENAME, event_list)
        if COPY_CPP_FILE:
            print("EventParser: Copying file to " + CPP_COPY_DESTINATION)
            copy_file(CPP_FILENAME, CPP_COPY_DESTINATION)
    print("")


def parse_events():
    subsystem_parser = SubsystemDefinitionParser(SUBSYSTEM_DEFINITION_DESTINATIONS)
    subsystem_table = subsystem_parser.parse_files()
    print("Found " + str(len(subsystem_table)) + " subsystem definitions.")
    PrettyPrinter.pprint(subsystem_table)
    event_header_parser = FileListParser(HEADER_DEFINITION_DESTINATIONS)
    event_headers = event_header_parser.parse_header_files(
        True, "Parsing event header file list:\n", True)
    # g.PP.pprint(event_headers)
    # myEventList = parseHeaderFiles(subsystem_table, event_headers)
    event_parser = EventParser(event_headers, subsystem_table)
    event_table = event_parser.parse_files()
    list_items = sorted(event_table.items())
    print("Found " + str(len(list_items)) + " entries:")
    PrettyPrinter.pprint(list_items)
    return list_items


class SubsystemDefinitionParser(FileParser):
    def __init__(self, file_list):
        super().__init__(file_list)

    def _handle_file_parsing(self, file_name: str, *args, **kwargs):
        file = open(file_name, "r")
        for line in file.readlines():
            match = re.search(r'([A-Z0-9_]*) = ([0-9]{1,2})', line)
            if match:
                self.mib_table.update({match.group(1): [match.group(2)]})

    def _post_parsing_operation(self):
        pass


class EventParser(FileParser):
    def __init__(self, file_list, interface_list):
        super().__init__(file_list)
        self.interfaces = interface_list
        self.count = 0
        self.myId = 0
        self.currentId = 0
        self.last_lines = ["", "", ""]

    def _handle_file_parsing(self, file_name: str, *args: any, **kwargs):
        try:
            file = open(file_name, 'r', encoding='utf-8')
            all_lines = file.readlines()
        except UnicodeDecodeError:
            file = open(file_name, 'r', encoding='cp1252')
            all_lines = file.readlines()
        total_count = 0
        for line in all_lines:
            self.__handle_line_reading(line, file_name)
        if self.count > 0:
            print("File " + file_name + " contained " + str(self.count) + " events.")
            total_count += self.count
            self.count = 0

    def _post_parsing_operation(self):
        pass

    def __handle_line_reading(self, line, file_name):
        if not self.last_lines[0] == '\n':
            twolines = self.last_lines[0] + ' ' + line.strip()
        else:
            twolines = ''
        match1 = re.search('SUBSYSTEM_ID[\s]*=[\s]*SUBSYSTEM_ID::([A-Z_0-9]*);', twolines)
        if match1:
            self.currentId = self.interfaces[match1.group(1)][0]
            # print( "Current ID: " + str(currentId) )
            self.myId = self.return_number_from_string(self.currentId)
        match = re.search(
            '(//)?[\t ]*static const(?:expr)? Event[\s]*([A-Z_0-9]*)[\s]*=[\s]*'
            'MAKE_EVENT\(([0-9]{1,2}),[\s]*SEVERITY::([A-Z]*)\);[\t ]*(//!<)?([^\n]*)', twolines)
        if match:
            if match.group(1):
                self.last_lines[0] = line
                return
            description = " "
            if match.group(6):
                description = self.clean_up_description(match.group(6))
            string_to_add = match.group(2)
            full_id = (self.myId * 100) + self.return_number_from_string(match.group(3))
            severity = match.group(4)
            if full_id in self.mib_table:
                # print("EventParser: Duplicate Event " + hex(full_id) + " from " + file_name +
                #      " was already in " + self.mib_table[full_id][3])
                pass
            self.mib_table.update({full_id: (string_to_add, severity, description, file_name)})
            self.count = self.count + 1
        self.last_lines[0] = line

    def build_checked_string(self, first_part, second_part):
        my_str = first_part + self.convert(second_part)
        if len(my_str) > 16:
            print("EventParser: Entry: " + my_str + " too long. Will truncate.")
            my_str = my_str[0:14]
        # else:
        # print( "Entry: " + myStr + " is all right.")
        return my_str

    @staticmethod
    def return_number_from_string(a_string):
        if a_string.startswith('0x'):
            return int(a_string, 16)
        elif a_string.isdigit():
            return int(a_string)
        else:
            print('EventParser: Illegal number representation: ' + a_string)
            return 0

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
    def clean_up_description(description):
        description = description.lstrip('//!<>')
        description = description.lstrip()
        if description == '':
            description = ' '
        return description


def export_to_file(filename, list_of_entries):
    print("EventParser: Exporting to file: " + filename)
    file = open(filename, "w")
    for entry in list_of_entries:
        file.write(str(entry[0]) + FILE_SEPARATOR + entry[1][0] + FILE_SEPARATOR + entry[1][1]
                   + FILE_SEPARATOR + entry[1][2] + FILE_SEPARATOR + entry[1][3] + '\n')
    file.close()
    return


def write_translation_file(filename, list_of_entries):
    outputfile = open(filename, "w")
    definitions = ""

    function = "const char * translateEvents(Event event){\n\tswitch((event&0xFFFF)){\n"
    for entry in list_of_entries:
        definitions += "const char *" + entry[1][0] + "_STRING = \"" + entry[1][0] + "\";\n"
        function += "\t\tcase " + str(entry[0]) + ":\n\t\t\treturn " + entry[1][0] + "_STRING;\n"
    function += '\t\tdefault:\n\t\t\treturn "UNKNOWN_EVENT";\n'
    outputfile.write("/**\n * @brief    Auto-generated event translation file. "
                     "Contains " + str(len(list_of_entries)) + " translations.\n"
                     " * Generated on: " + DATE_STRING_FULL +
                     " \n */\n")
    outputfile.write("#include \"translateEvents.h\"\n\n")
    outputfile.write(definitions + "\n" + function + "\t}\n\treturn 0;\n}\n")
    outputfile.close()


def handle_csv_export(file_name: str, list_items: list):
    """
    Generates the CSV in the same directory as the .py file and copes the CSV to another
    directory if specified.
    """
    export_to_file(file_name, list_items)

def handle_cpp_export(file_name: str, list_items):
    print("EventParser: Generating translation cpp file.")
    write_translation_file(file_name, list_items)

if __name__ == "__main__":
    main()
