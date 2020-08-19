"""
Program: obsw_module_test.py
Date: 01.11.2019
Description: All functions related to TmTc Sending and Receiving, used by UDP client

Manual:
Set up the UDP client as specified in the header comment and use the unit testing mode

A separate thread is used to listen for replies and send a new telecommand
if the first reply has not been received.

@author: R. Mueller
"""
import time

import config.obsw_config as g
from comIF.obsw_com_interface import CommunicationInterface
from utility.obsw_tmtc_printer import TmTcPrinter
from utility.obsw_logger import get_logger

from sendreceive.obsw_tm_listener import TmListener
from tc.obsw_pus_tc_base import TcQueueEntryT
from tm.obsw_pus_tm_factory import PusTmQueueT


logger = get_logger()


# pylint: disable=too-many-instance-attributes
class CommandSenderReceiver:
    """
    This is the generic CommandSenderReceiver object. All TMTC objects inherit this object,
    for example specific implementations (e.g. SingleCommandSenderReceiver)
    """
    def __init__(self, com_interface: CommunicationInterface, tmtc_printer: TmTcPrinter,
                 tm_listener: TmListener):
        """
        :param com_interface: CommunicationInterface object. Instantiate the desired one
        and pass it here
        :param tmtc_printer: TmTcPrinter object. Instantiate it and pass it here.
        """
        self._tm_timeout = g.G_TM_TIMEOUT
        self._tc_send_timeout_factor = g.G_TC_SEND_TIMEOUT_FACTOR

        if isinstance(com_interface, CommunicationInterface):
            self._com_interface = com_interface
        else:
            logger.error("CommandSenderReceiver: Invalid communication interface!")
            raise TypeError("CommandSenderReceiver: Invalid communication interface!")

        if isinstance(tmtc_printer, TmTcPrinter):
            self._tmtc_printer = tmtc_printer
        else:
            logger.error("CommandSenderReceiver: Invalid TMTC printer!")
            raise TypeError("CommandSenderReceiver: Invalid TMTC printer!")

        if isinstance(tm_listener, TmListener):
            self._tm_listener = tm_listener
        else:
            logger.error("CommandSenderReceiver: Invalid TM listener!")
            raise TypeError("Invalid TM Listener!")

        self._start_time = 0
        self._elapsed_time = 0
        self._timeout_counter = 0

        # needed to store last actual TC packet from queue
        self._last_tc = bytearray()
        self._last_tc_info = dict()

        # this flag can be used to notify when the operation is finished
        self._operation_pending = False
        # This flag can be used to notify when a reply was received.
        self._reply_received = False

    def set_tm_timeout(self, tm_timeout: float = g.G_TM_TIMEOUT):
        """
        Set the TM timeout. Usually, the global value set by the args parser is set,
        but the TM timeout can be reset (e.g. for slower architectures)
        :param tm_timeout: New TM timeout value as a float value in seconds
        :return:
        """
        self._tm_timeout = tm_timeout

    def set_tc_send_timeout_factor(self, new_factor: float = g.G_TC_SEND_TIMEOUT_FACTOR):
        """
        Set the TC resend timeout factor. After self._tm_timeout * new_factor seconds,
        a telecommand will be resent again.
        :param new_factor: Factor as a float number
        :return:
        """
        self._tc_send_timeout_factor = new_factor

    def _check_for_first_reply(self) -> None:
        """
        Checks for replies. If no reply is received, send telecommand again in checkForTimeout()
        :return: None
        """
        if self._tm_listener.event_reply_received.is_set():
            self._reply_received = True
            self._operation_pending = False
            self._tm_listener.event_reply_received.clear()
        else:
            self._check_for_timeout()

    def check_queue_entry(self, tc_queue_entry: TcQueueEntryT) -> bool:
        """
        Checks whether the entry in the tc queue is a telecommand.
        The last telecommand and respective information are stored in _last_tc
        and _last_tc_info
        :param tc_queue_entry:
        :return: True if queue entry is telecommand, False if it is not
        """
        queue_entry_first, queue_entry_second = tc_queue_entry
        queue_entry_is_telecommand = False
        if queue_entry_first == "wait":
            wait_time = queue_entry_second
            self._tm_timeout = self._tm_timeout + wait_time
            time.sleep(wait_time)
        # printout optimized for logger and debugging
        elif queue_entry_first == "print":
            print_string = queue_entry_second
            print()
            self._tmtc_printer.print_string(print_string, True)
        elif queue_entry_first == "rawprint":
            print_string = queue_entry_second
            self._tmtc_printer.print_string(print_string, False)
        elif queue_entry_first == "export":
            export_name = queue_entry_second
            self._tmtc_printer.add_print_buffer_to_buffer_list()
            self._tmtc_printer.print_to_file(export_name, True)
        elif queue_entry_first == "timeout":
            self._tm_timeout = queue_entry_second
        else:
            self._last_tc, self._last_tc_info = (queue_entry_first, queue_entry_second)
            return True
        return queue_entry_is_telecommand

    def _check_for_timeout(self):
        """
        Checks whether a timeout after sending a telecommand has occured and sends telecommand
        again. If resending reached certain counter, exit the program.
        :return:
        """
        if self._start_time == 0:
            self._start_time = time.time()
        if self._timeout_counter == 5:
            logger.info("CommandSenderReceiver: No response from command !")
            self._operation_pending = False
        if self._start_time != 0:
            self._elapsed_time = time.time() - self._start_time
        if self._elapsed_time >= self._tm_timeout * self._tc_send_timeout_factor:
            if g.G_RESEND_TC:
                logger.info("CommandSenderReceiver: Timeout, sending TC again !")
                self._com_interface.send_telecommand(self._last_tc, self._last_tc_info)
                self._timeout_counter = self._timeout_counter + 1
                self._start_time = time.time()
            else:
                # todo: we could also stop sending and clear the TC queue
                self._reply_received = True
        time.sleep(0.5)

    def print_tm_queue(self, tm_queue: PusTmQueueT):
        for tm_packet_list in tm_queue:
            try:
                for tm_packet in tm_packet_list:
                    self._tmtc_printer.print_telemetry(tm_packet)
            except AttributeError as e:
                logger.exception(
                    "CommandSenderReceiver Exception: Invalid queue entry. Traceback:", e)
