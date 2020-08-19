"""
@file   obsw_dummy_com_if.py
@date   09.03.2020
@brief  Dummy Communication Interface

@author R. Mueller
"""
from typing import Tuple

from comIF.obsw_com_interface import CommunicationInterface
from tc.obsw_pus_tc_base import PusTelecommand, PusTcInfoT, TcDictionaryKeys
from tm.obsw_pus_tm_factory import PusTelemetryFactory
from tm.obsw_tm_service_1 import Service1TmPacked
from utility.obsw_logger import get_logger

LOGGER = get_logger()


class DummyComIF(CommunicationInterface):
    def __init__(self, tmtc_printer):
        super().__init__(tmtc_printer)
        self.service_sent = 0
        self.reply_pending = False
        self.ssc = 0
        self.tc_ssc = 0
        self.tc_packet_id = 0

    def close(self) -> None:
        pass

    def data_available(self, parameters):
        if self.reply_pending:
            return True
        return False

    def poll_interface(self, parameters: any = 0) -> Tuple[bool, list]:
        pass

    def receive_telemetry(self, parameters: any = 0):
        tm_list = []
        if (self.service_sent == 17 or self.service_sent == 5) and self.reply_pending:
            LOGGER.debug("receive crap called")
            tm_packer = Service1TmPacked(subservice=1, ssc=self.ssc, tc_packet_id=self.tc_packet_id,
                                         tc_ssc=self.tc_ssc)

            tm_packet_raw = tm_packer.pack()
            tm_packet = PusTelemetryFactory.create(tm_packet_raw)
            tm_list.append(tm_packet)
            tm_packer = Service1TmPacked(subservice=7, ssc=self.ssc, tc_packet_id=self.tc_packet_id,
                                         tc_ssc=self.tc_ssc)
            tm_packet_raw = tm_packer.pack()
            tm_packet = PusTelemetryFactory.create(tm_packet_raw)
            tm_list.append(tm_packet)
            self.reply_pending = False
            self.ssc += 1
        return tm_list

    def send_telecommand(self, tc_packet: PusTelecommand, tc_packet_info: PusTcInfoT = None) -> None:
        if isinstance(tc_packet_info, dict) and tc_packet_info.__len__() > 0:
            self.service_sent = tc_packet_info[TcDictionaryKeys.SERVICE]
            self.tc_packet_id = tc_packet_info[TcDictionaryKeys.PACKET_ID]
            self.tc_ssc = tc_packet_info[TcDictionaryKeys.SSC]
            self.reply_pending = True
