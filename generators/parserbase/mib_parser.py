#! /usr/bin/python3.7
"""
@file
    mib_packet_content_parser.py
@brief
    Generic File Parser class
@details
    Used by the MIB Exporter. There are two functions which can be implemented by child class.
    Default implementations are empty

        1. handleFileParsing(...) which handles the parsing of one file
        2. updateTableEntries(...) After all files have been parsed, the table can
           be updated by imlementing this function

    A file list to parse must be supplied.
    Child classes fill out the MIB table (self.mib_table)
@author
    R. Mueller
@date
    14.11.2019
"""
from abc import abstractmethod
from typing import Dict


class FileParser:
    """
    This parent class gathers common file parser operations into a super class.
    Everything else needs to be implemented by child classes
    (e.g. which strings to parse for or operations to take after file parsing has finished)
    """
    def __init__(self, file_list):
        if len(file_list) == 0:
            print("File list is empty !")
            self.file_list_empty = True
        else:
            self.file_list_empty = False
        self.file_list = file_list
        # Can be used to have unique key in MIB tables
        self.index = 0
        # Initialize empty MIB table which will be filled by specific parser implementation
        self.mib_table = dict()

    def parse_files(self, *args: any, **kwargs) -> Dict:
        """
        Core method which is called to parse the files
        :param args: Optional positional arguments. Passed on the file parser
        :param kwargs: Optional keyword arguments. Passed on to file parser
        :return:
        """
        if not self.file_list_empty:
            for file_name in self.file_list:
                # Implemented by child class ! Fill out info table (self.mib_table) in this routine
                self._handle_file_parsing(file_name, *args, **kwargs)
            # Can be implemented by child class to edit the table after it is finished.
            # default implementation is empty
            self._post_parsing_operation()
        return self.mib_table

    # Implemented by child class ! Fill out info table (self.mib_table) in this routine
    @abstractmethod
    def _handle_file_parsing(self, file_name: str, *args, **kwargs):
        pass

    @abstractmethod
    def _post_parsing_operation(self):
        """
        # Can be implemented by child class to perform post parsing operations (e.g. setting a
        flag or editting MIB table entries)
        :return:
        """
