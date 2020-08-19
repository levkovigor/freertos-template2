# -*- coding: utf-8 -*-
"""
Created on Wed Apr  4 11:44:48 2018
Generic PUS packet class to deserialize raw PUS telemetry.
@author: S. Gaisser
"""

import crcmod
import datetime


class ObswPusPacket:
    def __init__(self, byte_array: bytearray):
        self.__packet_raw = byte_array
        self.PUSHeader = PUSPacketHeader(byte_array)
        byte_array = byte_array[6:]
        self.dataFieldHeader = OBSWPUSPacketDataFieldHeader(byte_array)
        byte_array = byte_array[12:]
        self.data = byte_array[:len(byte_array) - 2]
        self.crc = byte_array[len(byte_array) - 2] << 8 | byte_array[len(byte_array) - 1]

    def get_raw_packet(self) -> bytearray:
        return self.__packet_raw

    def append_pus_packet_header(self, array):
        self.dataFieldHeader.printDataFieldHeader(array)
        self.PUSHeader.printPusPacketHeader(array)

    def append_pus_packet_header_column_headers(self, array):
        self.dataFieldHeader.printDataFieldHeaderColumnHeader(array)
        self.PUSHeader.printPusPacketHeaderColumnHeaders(array)

    def getPacketSize(self):
        # PusHeader Size + data size
        size = PUSPacketHeader.headerSize + self.PUSHeader.length + 1
        return size

    def getService(self):
        return self.dataFieldHeader.type

    def getSubservice(self):
        return self.dataFieldHeader.subtype

    def getSSC(self):
        return self.PUSHeader.sourceSequenceCount

    def printData(self):
        print(self.returnDataString())

    def returnDataString(self):
        strToPrint = "["
        for byte in self.data:
            strToPrint += str(hex(byte)) + " , "
        strToPrint = strToPrint.rstrip(' , ')
        strToPrint += ']'
        return strToPrint



