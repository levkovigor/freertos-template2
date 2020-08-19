#!/usr/bin/python3
# -*- coding: utf-8 -*-
"""
@brief  This client was developed by KSat for the SOURCE project to test the on-board software.
@details
This client features multiple sender/receiver modes and has been designed
to be extensible and easy to use. This clien is based on the PUS standard for the format
of telecommands and telemetry.

@manual
Manual installation of crcmod and pyserial for serial communication might be needed
    1. Install pip if it is not installed yet
    2. Install crcmod and all other required packages:
        Command: python3.8 -m pip install crcmod
        or use IDE (interpreter settings -> pip in PyCharm)

The script can be used by specifying command line parameters.
Please run this script with the -h flag or without any command line parameters to display options.
GUI is work-in-progress
It might be necessary to set board or PC IP address if using ethernet communication.
Default values should work normally though. Use higher timeout value (-t Parameter) for STM32

Example command to test PUS Service 17 with serial communication
    obsw_tmtc_client.py -m 3 -s 17 -c 1
Example to run Unit Test with QEMU:
    obsw_tmtc_client.py -m 5 -c 2
Example to test service 3 with HK output and serial communication:
    obsw_tmtc_client.py -m 3 -s 3 --hk -c 1
Get command line help:
    obsw_tmtc_client.py -h

There are four different Modes:
    0. GUI Mode: Experimental mode, also called if no input parameter are specified
    1. Listener Mode: Only Listen for incoming TM packets
    2. SingleCommandMode: Send Single Command repeatedly until answer is received,
       only listen after that
    3. ServiceTestMode: Send all Telecommands belonging to a certain G_SERVICE
       and scan for replies for each telecommand. Listen after that
    4. SoftwareTestMode: Send all services and perform reply scanning like mode 3.
        Listen after that
    5. Unit Test Mode: Performs a unit test which returns a simple OK or NOT OK. This mode
        has the capability to send TCs in bursts, where applicable

There are following communication interfaces, specified by supplying -c <Interface ID>:
    0: Dummy Interface: Not fully developed yet.
    1: Serial Interface: Serial communication. Specify COM port with --COM <Com Port Name>
       or type it in manually. Serial settings are hardcoded for now in obsw_com_config.py .
    2: QEMU Interface: Start a QEMU session and then command the session with the QEMU interface.
    3. Ethernet: Not used anymore. Can be used to send packets to sockets by specifying the
       port and IP address.

If there are problems receiving packets with Ethernet Communication, check the firewall
settings and ensure that python.exe UDP packets are not blocked in advanced firewall settings.
"""
import atexit
import time
import unittest
import logging
import sys
from collections import deque
from typing import Tuple, Union

from test import obsw_pus_service_test
from config import obsw_config as g
from config.obsw_config import set_globals
from tc.obsw_pus_tc_packer import create_total_tc_queue, pack_service_queue
from tc.obsw_pus_tc_base import PusTcInfo
from obsw_user_code import command_preparation_hook

from sendreceive.obsw_single_command_sender_receiver import SingleCommandSenderReceiver
from sendreceive.obsw_sequential_sender_receiver import SequentialCommandSenderReceiver
from sendreceive.obsw_tm_listener import TmListener

from utility.obsw_args_parser import parse_input_arguments
from utility.obsw_tmtc_printer import TmTcPrinter
from utility.obsw_exit_handler import keyboard_interrupt_handler
from utility.obsw_logger import set_tmtc_logger, get_logger

from comIF.obsw_com_config import set_communication_interface

from gui.obsw_tmtc_gui import TmTcGUI
from gui.obsw_backend_test import TmTcBackend


LOGGER = get_logger()


def main():
    """
    Main method, reads input arguments, sets global variables and start TMTC handler.
    """
    set_tmtc_logger()
    LOGGER.info("Starting TMTC Client")

    LOGGER.info("Parsing input arguments")
    args = parse_input_arguments()

    LOGGER.info("Setting global variables")
    set_globals(args)

    LOGGER.info("Starting TMTC Handler")

    if g.G_MODE_ID == g.ModeList.GUIMode:
        # Experimental
        backend = TmTcBackend()
        backend.start()
        gui = TmTcGUI()
        gui.start()
        backend.join()
        gui.join()
        LOGGER.info("Both processes have closed")
        sys.exit()
    else:
        tmtc_handler = TmTcHandler()
        tmtc_handler.perform_operation()

    # At some later point, the program will run permanently and be able to take commands.
    # For now we put a permanent loop here so the program
    # doesn't exit automatically (TM Listener is daemonic)
    while True:
        pass


def command_preparation() -> Tuple[bytearray, Union[None, PusTcInfo]]:
    """
    Prepare command for single command testing
    :return:
    """
    return command_preparation_hook()


class TmTcHandler:
    """
    This is the primary class which handles TMTC reception. This can be seen as the backend
    in case a GUI or front-end is implemented.
    """
    def __init__(self):
        self.mode = g.G_MODE_ID
        self.com_if = g.G_COM_IF
        # This flag could be used later to command the TMTC Client with a front-end
        self.command_received = True

    def perform_operation(self):
        """
        Periodic operation
        """
        while True:
            try:
                if self.command_received:
                    self.command_received = False
                    self.handle_action()
                LOGGER.info("TMTC Client in idle mode")
                time.sleep(5)
            except (IOError, KeyboardInterrupt):
                LOGGER.info("Closing TMTC client.")
                sys.exit()

    def handle_action(self):
        """
        Command handling.
        """
        tmtc_printer = TmTcPrinter(g.G_DISPLAY_MODE, g.G_PRINT_TO_FILE, True)
        communication_interface = set_communication_interface(tmtc_printer)
        atexit.register(keyboard_interrupt_handler, com_interface=communication_interface)

        tm_listener = TmListener(
            com_interface=communication_interface, tm_timeout=g.G_TM_TIMEOUT,
            tc_timeout_factor=g.G_TC_SEND_TIMEOUT_FACTOR
        )
        tm_listener.start()

        if self.mode == g.ModeList.ListenerMode:
            if tm_listener.event_reply_received:
                # TODO: Test this.
                LOGGER.info("TmTcHandler: Packets received.")
                tmtc_printer.print_telemetry_queue(tm_listener.retrieve_tm_packet_queue())
                self.command_received = True
        elif self.mode == g.ModeList.SingleCommandMode:
            pus_packet_tuple = command_preparation()
            sender_and_receiver = SingleCommandSenderReceiver(
                com_interface=communication_interface, tmtc_printer=tmtc_printer,
                tm_listener=tm_listener)
            LOGGER.info("Performing single command operation")
            sender_and_receiver.send_single_tc_and_receive_tm(pus_packet_tuple=pus_packet_tuple)

        elif self.mode == g.ModeList.ServiceTestMode:
            service_queue = deque()
            pack_service_queue(g.G_SERVICE, service_queue)
            LOGGER.info("Performing service command operation")
            sender_and_receiver = SequentialCommandSenderReceiver(
                com_interface=communication_interface, tmtc_printer=tmtc_printer,
                tm_listener=tm_listener, tc_queue=service_queue)
            sender_and_receiver.send_queue_tc_and_receive_tm_sequentially()

        elif self.mode == g.ModeList.SoftwareTestMode:
            all_tc_queue = create_total_tc_queue()
            LOGGER.info("Performing multiple service commands operation")
            sender_and_receiver = SequentialCommandSenderReceiver(
                com_interface=communication_interface, tmtc_printer=tmtc_printer,
                tc_queue=all_tc_queue, tm_listener=tm_listener)
            sender_and_receiver.send_queue_tc_and_receive_tm_sequentially()
            LOGGER.info("SequentialSenderReceiver: Exporting output to log file.")
            tmtc_printer.print_file_buffer_list_to_file("log/tmtc_log.txt", True)

        elif self.mode == g.ModeList.UnitTest:
            # Set up test suite and run it with runner. Verbosity specifies detail level
            g.G_TM_LISTENER = tm_listener
            g.G_COM_INTERFACE = communication_interface
            g.G_TMTC_PRINTER = tmtc_printer
            LOGGER.info("Performing module tests")
            # noinspection PyTypeChecker
            suite = unittest.TestLoader().loadTestsFromModule(obsw_pus_service_test)
            unittest.TextTestRunner(verbosity=2).run(suite)

        elif self.mode == g.ModeList.BinaryUploadMode:
            # Upload binary, prompt user for input, in the end prompt for new mode and enter that
            # mode

            self.command_received = True
        else:
            logging.error("Unknown Mode, Configuration error !")
            sys.exit()


if __name__ == "__main__":
    main()
