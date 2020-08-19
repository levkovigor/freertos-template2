"""
@file   obsw_serial_com_if.py
@brief  Serial Communication Interface
@author R. Mueller
@date   01.11.2019
"""
import time
import logging
from typing import Tuple
from enum import Enum

import serial
from comIF.obsw_com_interface import CommunicationInterface
from utility.obsw_tmtc_printer import TmTcPrinter
from tm.obsw_pus_tm_factory import PusTelemetryFactory, PusTmListT
from tc.obsw_pus_tc_base import PusTcInfoT
from utility.obsw_logger import get_logger


LOGGER = get_logger()
SERIAL_FRAME_LENGTH = 256
HEADER_BYTES_BEFORE_SIZE = 5


class SerialCommunicationType(Enum):
    TIMEOUT_BASED = 0
    FIXED_FRAME_BASED = 1
    DLE_ENCODING = 2


# pylint: disable=arguments-differ
class SerialComIF(CommunicationInterface):
    """
    Communication Interface to use serial communication. This requires the PySerial library.
    """
    def __init__(self, tmtc_printer: TmTcPrinter, com_port: str, baud_rate: int,
                 serial_timeout: float,
                 ser_com_type: SerialCommunicationType = SerialCommunicationType.FIXED_FRAME_BASED,
                 com_type_args: any = 0):
        """
        Initiaze a serial communication handler.
        :param tmtc_printer: TMTC printer object. Can be used for diagnostic purposes, but main
        packet handling should be done by a separate thread.
        :param com_port: Specify COM port.
        :param baud_rate: Specify baud rate
        :param serial_timeout: Specify serial timeout
        :param ser_com_type: Specify how to handle serial reception
        """
        super().__init__(tmtc_printer)
        try:
            self.serial = serial.Serial(port=com_port, baudrate=baud_rate, timeout=serial_timeout)
        except serial.SerialException as error:
            LOGGER.exception("Serial Port could not be closed! Traceback: %s", str(error))
        self.data = bytearray()
        self.ser_com_type = ser_com_type
        if self.ser_com_type == SerialCommunicationType.FIXED_FRAME_BASED:
            self.serial_frame_size = com_type_args
        elif self.ser_com_type == SerialCommunicationType.DLE_ENCODING:
            # todo: We need a separate thread which does nothing but poll the serial interface for
            #       data (with the option to disable listening temporarily) and writes the data
            #       into a ring buffer (Python: deque with maxlen). We also need a lock because 2
            #       threads use the deque
            pass

    def close(self):
        try:
            self.serial.close()
        except serial.SerialException as e:
            logging.exception("Serial Port could not be closed! Traceback: " + str(e))

    def send_data(self, data: bytearray):
        self.serial.write(data)

    def send_telecommand(self, tc_packet: bytearray, tc_packet_info: PusTcInfoT = None) -> None:
        self.serial.write(tc_packet)

    def receive_telemetry(self, parameters: any = 0) -> PusTmListT:
        (packet_received, packet_list) = self.poll_interface()
        if packet_received:
            return packet_list
        return []

    def poll_interface(self, parameters: any = 0) -> Tuple[bool, PusTmListT]:
        if self.ser_com_type == SerialCommunicationType.FIXED_FRAME_BASED:
            if self.data_available():
                self.data = self.serial.read(self.serial_frame_size)
                pus_data_list = self.poll_pus_packets_fixed_frames(bytearray(self.data))
                packet_list = []
                for pus_packet in pus_data_list:
                    packet = PusTelemetryFactory.create(pus_packet)
                    packet_list.append(packet)
                return True, packet_list
            return False, []
        elif self.ser_com_type == SerialCommunicationType.DLE_ENCODING:
            # todo: the listener thread will fill a deque with a maximum length ( = ring buffer).
            #       If the number of bytes contained is larger than 0, we have to analyze the
            #       buffer for STX and ETX chars. We should propably copy all bytes in deque
            #       into a member list and then start scanning for STX and ETX. The deque should
            #       be protected with a lock when performing copy operations.
            LOGGER.warning("This communication type was not implemented yet!")
        else:
            LOGGER.warning("This communication type was not implemented yet!")

    def data_available(self, timeout: float = 0) -> bool:
        if self.serial.in_waiting > 0:
            return True
        if timeout > 0:
            start_time = time.time()
            elapsed_time = 0
            while elapsed_time < timeout:
                if self.serial.in_waiting > 0:
                    return True
                elapsed_time = time.time() - start_time
        return False

    @staticmethod
    def poll_pus_packets_fixed_frames(data: bytearray) -> list:
        pus_data_list = []
        if len(data) == 0:
            return pus_data_list

        payload_length = data[4] << 8 | data[5]
        packet_size = payload_length + 7
        if payload_length == 0:
            return []
        read_size = len(data)
        pus_data = data[0:packet_size]
        pus_data_list.append(pus_data)

        SerialComIF.read_multiple_packets(data, packet_size, read_size, pus_data_list)
        return pus_data_list

    @staticmethod
    def read_multiple_packets(data: bytearray, start_index: int, frame_size: int,
                              pus_data_list: list):
        while start_index < frame_size:
            start_index = SerialComIF.parse_next_packets(data, start_index, frame_size,
                                                         pus_data_list)

    @staticmethod
    def parse_next_packets(data: bytearray, start_index: int, frame_size: int,
                           pus_data_list: list) -> int:
        next_payload_len = data[start_index + 4] << 8 | data[start_index + 5]
        if next_payload_len == 0:
            end_index = frame_size
            return end_index
        next_packet_size = next_payload_len + 7
        # remaining_size = frame_size - start_index

        if next_packet_size > SERIAL_FRAME_LENGTH:
            LOGGER.error("PUS Polling: Very large packet detected, "
                         "packet splitting not implemented yet!")
            LOGGER.error("Detected Size: " + str(next_packet_size))
            end_index = frame_size
            return end_index

        end_index = start_index + next_packet_size
        pus_data = data[start_index:end_index]
        pus_data_list.append(pus_data)
        return end_index


