import math
import time

from utility.obsw_logger import get_logger

import datetime
from enum import Enum, auto
from typing import Final, Dict
from crcmod import crcmod

logger = get_logger()


class TmDictionaryKeys(Enum):
    SERVICE = auto()
    SUBSERVICE = auto()
    SUBCOUNTER = auto()
    SSC = auto()
    DATA = auto()
    CRC = auto()
    VALID = auto()
    # Service 1
    TC_PACKET_ID = auto()
    TC_SSC = auto()
    ERROR_CODE = auto()
    STEP_NUMBER = auto()
    # Service 5
    EVENT_ID = auto()
    REPORTER_ID = auto()
    EVENT_PARAM_1 = auto()
    EVENT_PARAM_2 = auto()


PusTmInfoT = Dict[TmDictionaryKeys, any]


class PusTelemetry:
    """
    Generic PUS telemetry class representation.
    It is instantiated by passing the raw pus telemetry packet (bytearray) to the constructor.
    It automatically deserializes the packet, exposing various packet fields via getter functions.
    PUS Telemetry structure according to ECSS-E-70-41A p.46. Also see structure below (bottom).
    """
    PUS_HEADER_SIZE = 6
    CDS_SHORT_SIZE = 7
    PUS_TIMESTAMP_SIZE = CDS_SHORT_SIZE
    def __init__(self, raw_telemetry: bytearray = bytearray()):
        if raw_telemetry == bytearray():
            return
        self._packet_raw: Final = raw_telemetry
        self._pus_header: Final = PusPacketHeader(raw_telemetry)
        self._valid = False
        if self._pus_header.length + PusTelemetry.PUS_HEADER_SIZE + 1 > len(raw_telemetry):
            logger.error("PusTelemetry: Passed packet shorter than specified packet "
                         "length in PUS header")
            return
        self._data_field_header: Final = PusPacketDataFieldHeader(raw_telemetry[6:])
        self._tm_data: Final = raw_telemetry[PusPacketDataFieldHeader.DATA_HEADER_SIZE +
                                             PusTelemetry.PUS_HEADER_SIZE + 1:
                                             len(raw_telemetry) - 2]
        self._crc: Final = raw_telemetry[len(raw_telemetry) - 2] << 8 | \
                           raw_telemetry[len(raw_telemetry) - 1]
        self.print_info = ""
        self.__perform_crc_check(raw_telemetry)

    def get_service(self):
        """
        :return: Service ID
        """
        return self._data_field_header.service_type

    def get_subservice(self):
        """
        :return: Subservice ID
        """
        return self._data_field_header.service_subtype

    def is_valid(self):
        return self._valid

    def get_tm_data(self) -> bytearray:
        """
        :return: TM application data (raw)
        """
        return self._tm_data

    def pack_tm_information(self) -> PusTmInfoT:
        """
        Packs important TM information needed for tests in a convenient dictionary
        :return: TM dictionary
        """
        tm_information = {
            TmDictionaryKeys.SERVICE: self.get_service(),
            TmDictionaryKeys.SUBSERVICE: self.get_subservice(),
            TmDictionaryKeys.SSC: self.get_ssc(),
            TmDictionaryKeys.DATA: self._tm_data,
            TmDictionaryKeys.CRC: self._crc,
            TmDictionaryKeys.VALID: self._valid
        }
        return tm_information

    def __perform_crc_check(self, raw_telemetry: bytearray):
        crc_func = crcmod.mkCrcFun(0x11021, rev=False, initCrc=0xFFFF, xorOut=0x0000)
        if len(raw_telemetry) < self.get_packet_size():
            logger.warning("PusPacketHeader: Invalid packet length")
            return
        data_to_check = raw_telemetry[0:self.get_packet_size()]
        crc = crc_func(data_to_check)
        if crc == 0:
            self._valid = True
        else:
            logger.warning("PusPacketHeader: Invalid CRC detected !")

    def specify_packet_info(self, print_info: str):
        """
        Caches a print information string for later printing
        :param print_info:
        :return:
        """
        self.print_info = print_info

    def append_packet_info(self, print_info: str):
        """
        Similar to the function above, but appends to the existing information string.
        :param print_info:
        :return:
        """
        self.print_info = self.print_info + print_info

    def append_telemetry_content(self, content_list: list):
        """
        Default implementation adds the PUS header content to the list which can then be
        printed with a simple print() command. To add additional content, override this method
        (don't forget to still call this function with super() if the header is required)
        :param content_list: Header content will be appended to this list
        :return:
        """
        self._data_field_header.append_data_field_header(content_list)
        self._pus_header.append_pus_packet_header(content_list)
        if self.is_valid():
            content_list.append("Yes")
        else:
            content_list.append("No")

    def append_telemetry_column_headers(self, header_list: list):
        """
        Default implementation adds the PUS header content header (confusing, I know)
        to the list which can then be  printed with a simple print() command.
        To add additional headers, override this method
        (don't forget to still call this function with super() if the header is required)
        :param header_list: Header content will be appended to this list
        :return:
        """
        self._data_field_header.append_data_field_header_column_header(header_list)
        self._pus_header.append_pus_packet_header_column_headers(header_list)
        header_list.append("Packet valid")

    def get_raw_packet(self) -> bytes:
        """
        Get the whole TM wiretapping_packet as a bytearray (raw)
        :return: TM wiretapping_packet
        """
        return self._packet_raw

    def get_packet_size(self) -> int:
        """
        :return: Size of the TM wiretapping_packet
        """
        # PusHeader Size + _tm_data size
        size = PusTelemetry.PUS_HEADER_SIZE + self._pus_header.length + 1
        return size

    def get_ssc(self) -> int:
        """
        Get the source sequence count
        :return: Source Sequence Count (see below, or PUS documentation)
        """
        return self._pus_header.source_sequence_count

    def return_full_packet_string(self):
        return self.return_data_string(self._packet_raw, len(self._packet_raw))

    def print_full_packet_string(self):
        """
        Print the full TM packet in a clean format.
        """
        logger.info(self.return_data_string(self._packet_raw, len(self._packet_raw)))

    def print_source_data(self):
        """
        Prints the TM source data in a clean format
        :return:
        """
        logger.info(self.return_data_string(self._tm_data, len(self._tm_data)))

    def return_source_data(self):
        """
        Returns the source data string
        """
        return self.return_data_string(self._tm_data, len(self._tm_data))

    @staticmethod
    def return_data_string(byte_array: bytearray, length: int) -> str:
        """
        Returns the TM data in a clean printable string format
        Prints payload data in default mode
        and prints the whole packet if full_packet = True is passed.
        :return:
        """
        str_to_print = "["
        for index in range(length):
            str_to_print += str(hex(byte_array[index])) + " , "
        str_to_print = str_to_print.rstrip()
        str_to_print = str_to_print.rstrip(',')
        str_to_print = str_to_print.rstrip()
        str_to_print += ']'
        return str_to_print


# pylint: disable=too-many-instance-attributes
class PusPacketHeader:
    """
    This class unnpacks the PUS packet header, also see PUS structure below or PUS documentation.
    """

    def __init__(self, pus_packet_raw: bytes):
        if len(pus_packet_raw) < PusTelemetry.PUS_HEADER_SIZE:
            logger.warning("PusPacketHeader: Packet size smaller than PUS header size!")
            self.version = 0
            self.type = 0
            self.data_field_header_flag = 0
            self.apid = 0
            self.segmentation_flag = 0
            self.source_sequence_count = 0
            self.length = 0
            return
        self.version = pus_packet_raw[0] >> 5
        self.type = pus_packet_raw[0] & 0x10
        self.data_field_header_flag = (pus_packet_raw[0] & 0x8) >> 3
        self.apid = ((pus_packet_raw[0] & 0x7) << 8) | pus_packet_raw[1]
        self.segmentation_flag = (pus_packet_raw[2] & 0xC0) >> 6
        self.source_sequence_count = ((pus_packet_raw[2] & 0x3F) << 8) | pus_packet_raw[3]
        self.length = pus_packet_raw[4] << 8 | pus_packet_raw[5]

    def append_pus_packet_header(self, array):
        array.append(str(chr(self.apid)))
        array.append(str(self.source_sequence_count))

    @staticmethod
    def append_pus_packet_header_column_headers(array):
        array.append("APID")
        array.append("SSC")


class PusPacketDataFieldHeader:
    """
    Unpacks the PUS packet data field header.
    """
    DATA_HEADER_SIZE = PusTelemetry.PUS_TIMESTAMP_SIZE + 4
    def __init__(self, bytes_array: bytearray):
        self.pus_version_and_ack_byte = (bytes_array[0] & 0x70) >> 4
        self.service_type = bytes_array[1]
        self.service_subtype = bytes_array[2]
        self.subcounter = bytes_array[3]
        self.time = PusTelemetryTimestamp(bytes_array[4:13])

    def append_data_field_header(self, content_list: list):
        """
        Append important data field header parameters to the passed content list.
        :param content_list:
        :return:
        """
        content_list.append(str(self.service_type))
        content_list.append(str(self.service_subtype))
        content_list.append(str(self.subcounter))
        self.time.print_time(content_list)

    def append_data_field_header_column_header(self, header_list: list):
        """
        Append important data field header column headers to the passed list.
        :param header_list:
        :return:
        """
        header_list.append("Service")
        header_list.append("Subservice")
        header_list.append("Subcounter")
        self.time.print_time_headers(header_list)



class PusTelemetryTimestamp:
    """
    Unpacks the time datafield of the TM packet. Right now, CDS Short timeformat is used,
    and the size of the time stamp is expected to be seven bytes.
    """
    CDS_ID = 4
    SECONDS_PER_DAY = 86400
    EPOCH = datetime.datetime.utcfromtimestamp(0)
    DAYS_CCSDS_TO_UNIX = 4383
    TIMESTAMP_SIZE = PusTelemetry.PUS_TIMESTAMP_SIZE
    def __init__(self, byte_array: bytearray=bytearray([])):
        if len(byte_array) > 0:
            # pField = byte_array[0]
            self.days = ((byte_array[1] << 8) | (byte_array[2])) - \
                    PusTelemetryTimestamp.DAYS_CCSDS_TO_UNIX
            self.seconds = self.days * (24 * 60 * 60)
            s_day = ((byte_array[3] << 24) | (byte_array[4] << 16) |
                 (byte_array[5]) << 8 | byte_array[6]) / 1000
            self.seconds += s_day
            self.time = self.seconds
            self.datetime = str(datetime.datetime.
                            utcfromtimestamp(self.time).strftime("%Y-%m-%d %H:%M:%S.%f"))

    @staticmethod
    def pack_current_time() -> bytearray:
        """
        Returns a seven byte CDS short timestamp
        """
        timestamp = bytearray()
        p_field = (PusTelemetryTimestamp.CDS_ID << 4) + 0
        days = (datetime.datetime.utcnow() - PusTelemetryTimestamp.EPOCH).days + \
               PusTelemetryTimestamp.DAYS_CCSDS_TO_UNIX
        days_h = (days & 0xFF00) >> 8
        days_l = days & 0xFF
        seconds = time.time()
        fraction_ms = seconds - math.floor(seconds)
        days_ms = int((seconds % PusTelemetryTimestamp.SECONDS_PER_DAY) * 1000 + fraction_ms)
        days_ms_hh = (days_ms & 0xFF000000) >> 24
        days_ms_h = (days_ms & 0xFF0000) >> 16
        days_ms_l = (days_ms & 0xFF00) >> 8
        days_ms_ll = (days_ms & 0xFF)
        timestamp.append(p_field)
        timestamp.append(days_h)
        timestamp.append(days_l)
        timestamp.append(days_ms_hh)
        timestamp.append(days_ms_h)
        timestamp.append(days_ms_l)
        timestamp.append(days_ms_ll)
        return timestamp


    def print_time(self, array):
        array.append(self.time)
        array.append(self.datetime)

    @staticmethod
    def print_time_headers(array):
        array.append("OBSWTime (s)")
        array.append("Time")


# pylint: disable=line-too-long
# Structure of a PUS Packet :
# A PUS packet consists of consecutive bits, the allocation and structure is standardised.
# Extended information can be found in ECSS-E-70-41A  on p.46
# The easiest form to send a PUS Packet is in hexadecimal form.
# A two digit hexadecimal number equals one byte, 8 bits or one octet
# o = optional, Srv = Service
#
# The structure is shown as follows for TM[17,2]
# 1. Structure Header
# 2. Structure Subheader
# 3. Component (size in bits)
# 4. Hexadecimal number
# 5. Binary Number
# 6. Decimal Number
#
# -------------------------------------------Packet Header(48)------------------------------------------|   Packet   |
#  ----------------Packet ID(16)----------------------|Packet Sequence Control (16)| Packet Length (16) | Data Field |
# Version       | Type(1) |Data Field    |APID(11)    | SequenceFlags(2) |Sequence |                    | (Variable) |
# Number(3)     |         |Header Flag(1)|            |                  |Count(14)|                    |            |
#           0x18               |    0x73              |       0xc0       | 0x19    |   0x00  |   0x04   |            |
#    000      1      0      000|  01110011            | 11  000000       | 00011001|00000000 | 0000100  |            |
#     0   |   1   |  0     |    115(ASCII s)          | 3 |            25          |   0     |    4     |            |
#
#   - Packet Length is an unsigned integer C = Number of Octets in Packet Data Field - 1
#
# Packet Data Field Structure:
#
# ------------------------------------------------Packet Data Field------------------------------------------------- |
# ---------------------------------Data Field Header ---------------------------|AppData|Spare|    PacketErrCtr      |
# CCSDS(1)|TM PUS Ver.(3)|Ack(4)|SrvType (8)|SrvSubtype(8)|  Time (o)  |Spare(o)|  (var)|(var)|         (16)         |
#        0x11 (0x1F)            |  0x11     |   0x01      |            |        |       |     | Calc.    |    Calc.  |
#    0     001     1111         |00010001   | 00000001    |            |        |       |     |          |           |
#    0      1       1111        |    17     |     2       |            |        |       |     |          |           |
#
#   - The source ID is present as one byte. Is it necessary? For now, ground = 0x00.
