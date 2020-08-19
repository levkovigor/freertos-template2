#!/usr/bin/python3.8
"""
@file   obsw_sequential_sender_receiver.py
@date   01.11.2019
@brief  Used to send multiple TCs in sequence and listen for replies after each sent TC
"""
import sys
import time

import config.obsw_config as g
from sendreceive.obsw_command_sender_receiver import CommandSenderReceiver
from sendreceive.obsw_tm_listener import TmListener
from comIF.obsw_com_interface import CommunicationInterface
from utility.obsw_tmtc_printer import TmTcPrinter
from utility.obsw_logger import get_logger
from tc.obsw_pus_tc_base import TcQueueT

LOGGER = get_logger()


class SequentialCommandSenderReceiver(CommandSenderReceiver):
    """
    Specific implementation of CommandSenderReceiver to send multiple telecommands in sequence
    """
    def __init__(self, com_interface: CommunicationInterface, tmtc_printer: TmTcPrinter,
                 tm_listener: TmListener, tc_queue: TcQueueT):
        """
        :param com_interface: CommunicationInterface object, passed on to CommandSenderReceiver
        :param tm_listener: TmListener object which runs in the background and receives
                            all Telemetry
        :param tmtc_printer: TmTcPrinter object, passed on to CommandSenderReceiver
        for this time period
        """
        super().__init__(com_interface=com_interface, tmtc_printer=tmtc_printer,
                         tm_listener=tm_listener)
        self._tc_queue = tc_queue
        self.__first_reply_received = False
        self.__all_replies_received = False
        self.__mode_op_finished = False

    def send_queue_tc_and_receive_tm_sequentially(self):
        """
        Primary function which is called for sequential transfer.
        :return:
        """
        self._tm_listener.mode_id = g.ModeList.ServiceTestMode
        self._tm_listener.event_mode_change.set()
        # tiny delay for tm listener
        time.sleep(0.1)
        self.__send_and_receive_first_packet()
        # this flag is set in the separate thread !
        try:
            self.__handle_tc_sending()
        except (KeyboardInterrupt, SystemExit):
            LOGGER.info("Closing TMTC Client")
            sys.exit()

    def __handle_tc_sending(self):
        while not self.__all_replies_received:
            while not self._tc_queue.__len__() == 0:
                self.__perform_next_tc_send()
                if self._tc_queue.__len__() == 0:
                    self._start_time = time.time()
            self._check_for_timeout()
            if not self.__mode_op_finished:
                self._tm_listener.event_mode_op_finished.set()
                self.__mode_op_finished = True
        LOGGER.info("SequentialSenderReceiver: All replies received!")

    def __perform_next_tc_send(self):
        if self._tm_listener.event_reply_received.is_set():
            self._reply_received = True
            self._tm_listener.event_reply_received.clear()
        # this flag is set in the separate receiver thread too
        if self._reply_received:
            self.print_tm_queue(self._tm_listener.retrieve_tm_packet_queue())
            self._tm_listener.clear_tm_packet_queue()
            self.__send_next_telecommand()
            self._reply_received = False
        # just calculate elapsed time if start time has already been set (= command has been sent)
        else:
            self._check_for_timeout()

    def __send_and_receive_first_packet(self):
        tc_queue_tuple = self._tc_queue.pop()
        if self.check_queue_entry(tc_queue_tuple):
            pus_packet, pus_packet_info = tc_queue_tuple
            self._com_interface.send_telecommand(pus_packet, pus_packet_info)
        else:
            self.__send_and_receive_first_packet()

    def __send_next_telecommand(self):
        tc_queue_tuple = self._tc_queue.pop()
        if self.check_queue_entry(tc_queue_tuple):
            self._start_time = time.time()
            pus_packet, pus_packet_info = tc_queue_tuple
            self._com_interface.send_telecommand(pus_packet, pus_packet_info)
        elif self._tc_queue.__len__() == 0:
            # Special case: Last queue entry is not a Telecommand
            self.__all_replies_received = True
        else:
            # If the queue entry was not a telecommand, send next telecommand
            self.__perform_next_tc_send()
