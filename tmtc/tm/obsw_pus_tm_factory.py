# -*- coding: utf-8 -*-
from typing import Deque, List, Tuple
from tm.obsw_pus_tm_base import PusTelemetry, PusTmInfoT
from tm.obsw_tm_service_1 import Service1TM
from tm.obsw_tm_service_3 import Service3TM
from tm.obsw_tm_service_5 import Service5TM
from utility.obsw_logger import get_logger
import struct

logger = get_logger()
PusRawTmList = List[bytearray]
PusRawTmQueue = Deque[bytearray]
PusTmTupleT = Tuple[PusTmInfoT, PusTelemetry]

PusTmListT = List[PusTelemetry]
PusTmQueueT = Deque[PusTmListT]

PusTmInfoQueueT = Deque[PusTmInfoT]
PusTmTupleQueueT = Deque[PusTmTupleT]


class PusTelemetryFactory(object):
    """
    Deserialize TM bytearrays into PUS TM Classes
    """
    @staticmethod
    def create(raw_tm_packet: bytearray) -> PusTelemetry:
        service_type = raw_tm_packet[7]
        if service_type == 1:
            return Service1TM(raw_tm_packet)
        if service_type == 2:
            return Service2TM(raw_tm_packet)
        if service_type == 3:
            return Service3TM(raw_tm_packet)
        if service_type == 5:
            return Service5TM(raw_tm_packet)
        if service_type == 8:
            return Service8TM(raw_tm_packet)
        if service_type == 17:
            return Service17TM(raw_tm_packet)
        if service_type == 20:
            return Service20TM(raw_tm_packet)
        if service_type == 200:
            return Service200TM(raw_tm_packet)
        print("The service " + str(service_type) + " is not implemented in Telemetry Factory")
        return PusTelemetry(raw_tm_packet)



class Service2TM(PusTelemetry):
    def __init__(self, byte_array: bytearray):
        super().__init__(byte_array)
        self.specify_packet_info("Raw Commanding Reply")

    def append_telemetry_content(self, array):
        super().append_telemetry_content(array)
        return

    def append_telemetry_column_headers(self, array):
        super().append_telemetry_column_headers(array)
        return


class Service8TM(PusTelemetry):
    def __init__(self, byte_array):
        super().__init__(byte_array)
        self.specify_packet_info("Functional Commanding Reply")

    def append_telemetry_content(self, array):
        super().append_telemetry_content(array)
        return

    def append_telemetry_column_headers(self, array):
        super().append_telemetry_column_headers(array)
        return
    
    
class Service9TM(PusTelemetry):
    def __init__(self, byte_array):
        super().__init__(byte_array)
        self.specify_packet_info("Time Service Reply")

    def append_telemetry_content(self, array: list):
        super().append_telemetry_content(array)
        return

    def append_telemetry_column_headers(self, column_header: list):
        super().append_telemetry_column_headers(column_header)
        return
    
    
class Service17TM(PusTelemetry):
    def __init__(self, byte_array):
        super().__init__(byte_array)
        self.specify_packet_info("Test Reply")

    def append_telemetry_content(self, array):
        super().append_telemetry_content(array)
        return

    def append_telemetry_column_headers(self, array):
        super().append_telemetry_column_headers(array)
        return

class Service20TM(PusTelemetry):
    def __init__(self, byte_array):
        super().__init__(byte_array)
        self.parameter_id = struct.unpack('>I', self._tm_data[0:4])[0]
        self.specify_packet_info("Functional Commanding Reply")

    def append_telemetry_content(self, array):
        super().append_telemetry_content(array)
        array.append(self.parameter_id)
        return

    def append_telemetry_column_headers(self, array):
        super().append_telemetry_column_headers(array)
        array.append("param0_dump_repl")
        return

class Service200TM(PusTelemetry):
    def __init__(self, byte_array):
        super().__init__(byte_array)
        self.isCantReachModeReply = False
        self.isModeReply = False
        self.specify_packet_info("Mode Reply")
        self.objectId = struct.unpack('>I', self._tm_data[0:4])[0]
        if self.get_subservice() == 7:
            self.append_packet_info(": Can't reach mode")
            self.isCantReachModeReply = True
            self.returnValue = self._tm_data[4] << 8 | self._tm_data[5]
        elif self.get_subservice() == 6 or self.get_subservice() == 8:
            self.isModeReply = True
            if self.get_subservice() == 8:
                self.append_packet_info(": Wrong Mode")
            elif self.get_subservice() == 6:
                self.append_packet_info(": Mode reached")
            self.mode = struct.unpack('>I', self._tm_data[4:8])[0]
            self.submode = self._tm_data[8]

    def append_telemetry_content(self, array):
        super().append_telemetry_content(array)
        array.append(hex(self.objectId))
        if self.isCantReachModeReply:
            array.append(hex(self.returnValue))
        elif self.isModeReply:
            array.append(str(self.mode))
            array.append(str(self.submode))
        return

    def append_telemetry_column_headers(self, array):
        super().append_telemetry_column_headers(array)
        array.append("Object ID")
        if self.isCantReachModeReply:
            array.append("Return Value")
        elif self.isModeReply:
            array.append("Mode")
            array.append("Submode")
        return
