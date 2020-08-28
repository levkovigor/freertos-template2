"""
@file   mib_file_list_parser.py
@brief  Generic File Parser class
@details
Used by parse header files. Implemented as class in case header parser becomes more complex
@author R. Mueller
@date   22.11.2019
"""
import os
import re
from typing import Union


# pylint: disable=too-few-public-methods
class FileListParser:
    """
    Generic header parser which takes a directory name or directory name list
    and parses all included header files recursively.
    """
    def __init__(self, directory_list_or_name: Union[str, list]):
        if isinstance(directory_list_or_name, str):
            self.directory_list = [directory_list_or_name]
        elif isinstance(directory_list_or_name, list):
            self.directory_list = directory_list_or_name
        else:
            print("Header Parser: Passed directory list is not a header name or list of "
                  "header names")
        self.header_files = []

    def parse_header_files(self, search_recursively: bool = False,
                           printout_string: str = "Parsing header files: ",
                           print_current_dir: bool = False):
        """
        This function is called to get a list of header files
        :param search_recursively:
        :param printout_string:
        :param print_current_dir:
        :return:
        """
        print(printout_string, end="")
        for directory in self.directory_list:
            self.__get_header_file_list(directory, search_recursively, print_current_dir)
        print(str(len(self.header_files)) + " header files were found.")
        # g.PP.pprint(self.header_files)
        return self.header_files

    def __get_header_file_list(self, base_directory: str, seach_recursively: bool = False,
                               print_current_dir: bool = False):
        if base_directory[-1] != '/':
            base_directory += '/'
        local_header_files = []
        if print_current_dir:
            print("Parsing header files in: " + base_directory)
        base_list = os.listdir(base_directory)
        # g.PP.pprint(base_list)
        for entry in base_list:
            header_file_match = re.match(r"[_.]*.*\.h", entry)
            if header_file_match:
                if os.path.isfile(base_directory + entry):
                    match_string = header_file_match.group(0)
                    if match_string[0] == '.' or match_string[0] == '_':
                        pass
                    else:
                        local_header_files.append(base_directory + entry)
            if seach_recursively:
                next_path = base_directory + entry
                if os.path.isdir(next_path):
                    self.__get_header_file_list(next_path, seach_recursively)
        # print("Files found in: " + base_directory)
        # g.PP.pprint(local_header_files)
        self.header_files.extend(local_header_files)
