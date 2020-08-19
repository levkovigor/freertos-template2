"""
@file   obsw_ethernet_com_if.py
@date   01.11.2019
@brief  Ethernet Communication Interface

@author R. Mueller
"""
import select
import socket
import sys
from typing import Tuple

from comIF.obsw_com_interface import CommunicationInterface, PusTmListT
from tm.obsw_pus_tm_factory import PusTelemetryFactory
from tc.obsw_pus_tc_base import PusTcInfoT
from utility.obsw_tmtc_printer import TmTcPrinter
import config.obsw_config as g


# pylint: disable=abstract-method
# pylint: disable=arguments-differ
# pylint: disable=too-many-arguments
class EthernetComIF(CommunicationInterface):
    """
    Communication interface for UDP communication.
    """

    def send_data(self, data: bytearray):
        self.udp_socket.sendto(data, self.destination_address)

    def __init__(self, tmtc_printer: TmTcPrinter, tm_timeout: float, tc_timeout_factor: float,
                 receive_address: g.ethernetAddressT, send_address: g.ethernetAddressT):
        super().__init__(tmtc_printer)
        self.tm_timeout = tm_timeout
        self.tc_timeout_factor = tc_timeout_factor
        self.udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.receive_address = receive_address
        self.destination_address = send_address
        self.set_up_socket(self.receive_address)

    def close(self) -> None:
        pass

    def send_telecommand(self, tc_packet: bytearray, tc_packet_info: PusTcInfoT = None) -> None:
        self.udp_socket.sendto(tc_packet, self.destination_address)

    def data_available(self, timeout: float = 0) -> bool:
        ready = select.select([self.udp_socket], [], [], timeout)
        if ready[0]:
            return True
        return False

    def poll_interface(self, poll_timeout: float = 0) -> Tuple[bool, PusTmListT]:
        ready = self.data_available(poll_timeout)
        if ready:
            data = self.udp_socket.recvfrom(1024)[0]
            tm_packet = PusTelemetryFactory.create(bytearray(data))
            self.tmtc_printer.print_telemetry(tm_packet)
            packet_list = [tm_packet]
            return True, packet_list
        return False, []

    def receive_telemetry(self, parameters: any = 0) -> list:
        (packet_received, packet_list) = self.poll_interface()
        if packet_received:
            return packet_list
        return []

    def set_up_socket(self, receive_address):
        """
        Sets up the sockets for the UDP communication.
        :return:
        """
        try:
            self.udp_socket.bind(receive_address)
            self.udp_socket.setblocking(False)
        except OSError:
            print("Socket already set-up.")
        except TypeError:
            print("Invalid Receive Address")
            sys.exit()

    def connect_to_board(self):
        """
        For UDP, this can be used to initiate communication.
        :return:
        """
        ping = bytearray([])
        self.send_telecommand(ping)
