# -*- coding: utf-8 -*-
"""
Program: obsw_tm_service_3.py
Date: 30.12.2019
Description: Deserialize Housekeeping TM
Author: R. Mueller
"""

from tm.obsw_pus_tm_base import PusTelemetry
from typing import Type
import struct


class Service3TM(PusTelemetry):
    def __init__(self, byte_array: bytes):
        super().__init__(byte_array)
        # print("Length of TM data: " + str(len(self._tm_data)))
        self.sid = struct.unpack('>I', self._tm_data[0:4])[0]
        self.hkHeader = []
        self.hkContent = []
        self.hkDefinitionHeader = []
        self.hkDefinition = []
        self.numberOfParameters = 0
        self.validity_buffer = []
        self.specify_packet_info("Housekeeping Packet")
        self.paramLength = 0
        if self.get_subservice() == 10 or self.get_subservice() == 12:
            self.handle_filling_definition_arrays()
        if self.get_subservice() == 25 or self.get_subservice() == 26:
            self.handle_filling_hk_arrays()

    def append_telemetry_content(self, array):
        super().append_telemetry_content(array)
        array.append(hex(self.sid))
        array.append(int(self.paramLength))

    def append_telemetry_column_headers(self, array):
        super().append_telemetry_column_headers(array)
        array.append("SID")
        array.append("HK Data Size")

    def handle_filling_definition_arrays(self):
        self.hkHeader = ["SID", "Report Status", "Collection Interval", "Number Of IDs"]
        reporting_enabled = self._tm_data[4]
        collection_interval = struct.unpack('>f', self._tm_data[5:9])[0]
        number_of_params = self._tm_data[9]
        parameters = []
        index2 = 1
        for index in range(10, (number_of_params * 4) + 10, 4):
            parameter = struct.unpack('>I', self._tm_data[index:index + 4])[0]
            self.hkHeader.append("Pool ID " + str(index2))
            parameters.append(str(hex(parameter)))
            index2 = index2 + 1
        if reporting_enabled == 1:
            status_string = "On"
        else:
            status_string = "Off"
        self.hkContent = [hex(self.sid), status_string, collection_interval, number_of_params]
        self.hkContent.extend(parameters)

    def handle_filling_hk_arrays(self):
        self.paramLength = len(self._tm_data) - 4
        # TODO: This can be automated by using the MIB parser pool names and pool datatypes
        if self.sid == 0x1f00 or self.sid == 0x2f00:
            self.handle_gps_hk_data()
        elif self.sid == 0x4300 or self.sid == 0x4400:
            self.handle_test_hk_data()

    def handle_gps_hk_data(self):
        # TODO: size check. sth not right with gps 0 test
        self.numberOfParameters = 9
        self.hkHeader = ["Fix Mode", "SV in Fix", "GNSS Week", "Time of Week", "Latitude",
                         "Longitude", "Mean Sea Altitude", "Position X", "Position Y", "Position Z",
                         "Velocity X", "Velocity Y", "Velocity Z"]
        fixMode = self._tm_data[4]
        svInFix = self._tm_data[5]
        gnssWeek = struct.unpack('>H', self._tm_data[6:8])[0]
        timeOfWeek = struct.unpack('>I', self._tm_data[8:12])[0]
        latitude = struct.unpack('>I', self._tm_data[12:16])[0]
        longitude = struct.unpack('>I', self._tm_data[16:20])[0]
        msa = struct.unpack('>I', self._tm_data[20:24])[0]
        positionX = struct.unpack('>d', self._tm_data[24:32])[0]
        positionY = struct.unpack('>d', self._tm_data[32:40])[0]
        positionZ = struct.unpack('>d', self._tm_data[40:48])[0]
        vx = struct.unpack('>d', self._tm_data[48:56])[0]
        vy = struct.unpack('>d', self._tm_data[56:64])[0]
        vz = struct.unpack('>d', self._tm_data[64:72])[0]
        self.hkContent = [fixMode, svInFix, gnssWeek, timeOfWeek, latitude, longitude, msa, positionX, positionY,
                          positionZ, vx, vy, vz]
        self.validity_buffer = self._tm_data[72:]
        # print(self.validityBuffer)
        # print(str(format(self.validityBuffer[0], '#010b')))
        # print(str(format(self.validityBuffer[1], '#010b')))
        # print("Validity Buffer Length: " + str(len(self.validityBuffer)))

    def handle_test_hk_data(self):
        self.numberOfParameters = 6
        self.hkHeader = ["Bool", "UINT8", "UINT16", "UINT32", "FLOAT1", "FLOAT2"]
        testBool = self._tm_data[4]
        testUint8 = self._tm_data[5]
        testUint16 = (self._tm_data[6] << 8) | self._tm_data[7]
        testUint32 = struct.unpack('>I', self._tm_data[8:12])[0]
        floatVector1 = struct.unpack('>f', self._tm_data[12:16])[0]
        floatVector2 = struct.unpack('>f', self._tm_data[16:20])[0]
        self.hkContent = [testBool, testUint8, testUint16, testUint32, floatVector1, floatVector2]
        self.validity_buffer = self._tm_data[20:]
        # print(self.validityBuffer)
        # print(str(format(self.validityBuffer[0], '#010b')))
        # print("Validity Buffer Length: " + str(len(self.validityBuffer)))


Service3TM: Type[PusTelemetry]
