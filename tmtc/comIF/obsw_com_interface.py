# -*- coding: utf-8 -*-
"""
Program: obsw_com_interface.py
Date: 01.11.2019
Description: Generic Communication Interface. Defines the syntax of the communication functions.
             Abstract methods must be implemented by child class (e.g. Ethernet Com IF)

@author: R. Mueller
"""
from abc import abstractmethod
from typing import Tuple
from tm.obsw_pus_tm_factory import PusTmListT
from utility.obsw_tmtc_printer import TmTcPrinter
from tc.obsw_pus_tc_base import PusTcInfoT


# pylint: disable=useless-return
# pylint: disable=no-self-use
# pylint: disable=unused-argument
class CommunicationInterface:
    """
    Generic form of a communication interface to separate communication logic from
    the underlying interface.
    """
    def __init__(self, tmtc_printer: TmTcPrinter):
        self.tmtc_printer = tmtc_printer

    @abstractmethod
    def close(self) -> None:
        """
        Closes the ComIF and releases any held resources (for example a Communication Port)
        :return:
        """

    @abstractmethod
    def send_data(self, data: bytearray):
        """
        Send data, for example a frame containing packets.
        """

    @abstractmethod
    def send_telecommand(self, tc_packet: bytearray, tc_packet_info: PusTcInfoT = None) -> None:
        """
        Send telecommands
        :param tc_packet: TC wiretapping_packet to send
        :param tc_packet_info: TC wiretapping_packet information
        :return: None for now
        """

    @abstractmethod
    def receive_telemetry(self, parameters: any = 0) -> PusTmListT:
        """
        Returns a list of packets. Most of the time,
        this will simply call the pollInterface function
        :param parameters:
        :return:
        """
        packet_list = []
        return packet_list

    @abstractmethod
    def poll_interface(self, parameters: any = 0) -> Tuple[bool, PusTmListT]:
        """
        Poll the interface and return a list of received packets
        :param parameters:
        :return: Tuple: boolean which specifies wheather a wiretapping_packet was received,
        and the wiretapping_packet list containing
        Tm packets
        """

    @abstractmethod
    def data_available(self, parameters: any) -> bool:
        """
        Check whether TM data is available
        :param parameters:
        :return:
        """
