# -*- coding: utf-8 -*-
"""
Program: obsw_tm_service_5.py
Date: 30.12.2019
Description: Deserialize PUS Event Report
Author: R. Mueller
"""

from tm.obsw_pus_tm_base import PusTelemetry, TmDictionaryKeys
from tm.obsw_pus_tm_factory import PusTmInfoT
import struct


class Service5TM(PusTelemetry):
    def __init__(self, byte_array):
        super().__init__(byte_array)
        self.specify_packet_info("Event")
        if self.get_subservice() == 1:
            self.append_packet_info(" Info")
        elif self.get_subservice() == 2:
            self.append_packet_info(" Error Low Severity")
        elif self.get_subservice() == 3:
            self.append_packet_info(" Error Med Severity")
        elif self.get_subservice() == 4:
            self.append_packet_info(" Error High Severity")
        self.eventId = struct.unpack('>H', self._tm_data[0:2])[0]
        self.objectId = struct.unpack('>I', self._tm_data[2:6])[0]
        self.param1 = struct.unpack('>I', self._tm_data[6:10])[0]
        self.param2 = struct.unpack('>I', self._tm_data[10:14])[0]

    def append_telemetry_content(self, array):
        super().append_telemetry_content(array)
        array.append(str(self.eventId))
        array.append(hex(self.objectId))
        array.append(str(hex(self.param1)) + ", " + str(self.param1))
        array.append(str(hex(self.param2)) + ", " + str(self.param2))

    def append_telemetry_column_headers(self, array):
        super().append_telemetry_column_headers(array)
        array.append("Event ID")
        array.append("Reporter ID")
        array.append("Parameter 1")
        array.append("Parameter 2")

    def pack_tm_information(self) -> PusTmInfoT:
        tm_information = super().pack_tm_information()
        add_information = {
            TmDictionaryKeys.REPORTER_ID: self.objectId,
            TmDictionaryKeys.EVENT_ID: self.eventId,
            TmDictionaryKeys.EVENT_PARAM_1: self.param1,
            TmDictionaryKeys.EVENT_PARAM_2: self.param2
        }
        tm_information.update(add_information)
        return tm_information
