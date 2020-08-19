#! /usr/bin/python3.7
"""
@file
    mib_packet_content_parser.py
@brief
    CSV Writer
@details
   This class writes tables to a csv.
@author
    R. Mueller
@date
    14.11.2019
"""
from utility import mib_globals as g
from utility.mib_file_management import copy_file, move_file


# TODO: Export to SQL
class CsvWriter:
    def __init__(self, filename, table_to_print=None, header_array=None):
        if header_array is None:
            header_array = []
        if table_to_print is None:
            table_to_print = dict()
        self.filename = filename
        self.tableToPrint = table_to_print
        self.headerArray = header_array
        if self.headerArray != 0:
            self.columnNumbers = len(self.headerArray)
        self.fileSeparator = g.fileSeparator

    def write_to_csv(self):
        file = open(self.filename, "w")
        file.write("Index" + self.fileSeparator)
        for index in range(self.columnNumbers):
            # noinspection PyTypeChecker
            if index < len(self.headerArray)-1:
                file.write(self.headerArray[index] + self.fileSeparator)
            else:
                file.write(self.headerArray[index] + "\n")
        for index, entry in self.tableToPrint.items():
            file.write(str(index) + self.fileSeparator)
            for columnIndex in range(self.columnNumbers):
                # noinspection PyTypeChecker
                if columnIndex < len(self.headerArray) - 1:
                    file.write(str(entry[columnIndex]) + self.fileSeparator)
                else:
                    file.write(str(entry[columnIndex]) + "\n")
        file.close()

    def copy_csv(self, copy_destination: str = g.copyDestination):
            copy_file(self.filename, copy_destination)
            print("CSV file was copied to " + copy_destination)

    def move_csv(self, move_destination):
        move_file(self.filename, move_destination)
        if move_destination == ".." or move_destination == "../":
            print("CSV Writer: CSV file was moved to parser root directory")
        else:
            print("CSV Writer: CSV file was moved to " + move_destination)


