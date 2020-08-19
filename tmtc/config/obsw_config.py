"""
@file
    obsw_config.py
@date
    01.11.2019
@brief
    Global settings for UDP client
"""
import enum
import struct
import pprint
from typing import Tuple
import logging

"""
Global service_type definitions
"""
ethernetAddressT = Tuple[str, int]


# Mode options, set by args parser
class ModeList(enum.Enum):
    GUIMode = 0
    ListenerMode = 1
    SingleCommandMode = 2
    ServiceTestMode = 3
    SoftwareTestMode = 4
    BinaryUploadMode = 5
    UnitTest = 6


class ComIF(enum.Enum):
    Dummy = 0
    Serial = 1
    QEMU = 2
    Ethernet = 3



"""
Mission/Device specific information.
"""

# TODO: Automate / Autofill this file with the MIB parser

# Object IDs
GPS0_ObjectId = bytearray([0x44, 0x10, 0x1F, 0x00])
GPS1_ObjectId = bytearray([0x44, 0x20, 0x20, 0x00])
DUMMY_DEVICE_ID = bytearray([0x44, 0x00, 0xAF, 0xFE])

# Commands
DUMMY_COMMAND_1 = struct.pack(">I", 666)
DUMMY_COMMAND_2 = bytearray([0xC0, 0xC0, 0xBA, 0xBE])
DUMMY_COMMAND_2_PARAM_1 = bytearray([0xBA, 0xB0])
DUMMY_COMMAND_2_PARAM_2 = bytearray([0x00, 0x00, 0x00, 0x52, 0x4F, 0x42, 0x49, 0x4E])
DUMMY_COMMAND_3 = bytearray([0xBA, 0xDE, 0xAF, 0xFE])

# SIDs
GPS0_SID = bytearray([0x00, 0x00, 0x1f, 0x00])
GPS1_SID = bytearray([0x00, 0x00, 0x2f, 0x00])
TEST_SID = bytearray([0x00, 0x00, 0x43, 0x00])
CUSTOM_SID = bytearray([0x00, 0x00, 0x44, 0x00])
# Pool IDs
TEST_ID_1 = bytearray([0x01, 0x01, 0x01, 0x02])
TEST_ID_2 = bytearray([0x02, 0x02, 0x02, 0x04])
TEST_ID_3 = bytearray([0x03, 0x03, 0x03, 0x06])
TEST_ID_4 = bytearray([0x04, 0x04, 0x04, 0x08])
TEST_ID_5 = bytearray([0x05, 0x05, 0x05, 0x10])

"""
All global variables, set in main program with arg parser
"""

# TMTC Client
G_TMTC_LOGGER_NAME = "TMTC Logger"
G_ERROR_LOG_FILE_NAME = "tmtc_error.log"
G_PP = pprint.PrettyPrinter()
LOGGER = logging.getLogger(G_TMTC_LOGGER_NAME)
# General Settings
G_SCRIPT_MODE = 1
G_MODE_ID = 0
G_SERVICE = 17
G_DISPLAY_MODE = "long"


G_COM_IF = 2
# COM Port for serial communication
G_COM_PORT = 'COM0'
G_SERIAL_TIMEOUT = 0.5
G_SERIAL_BAUDRATE = 230400
G_SERIAL_FRAME_SIZE = 256
# Time related
G_TM_TIMEOUT = 6
G_TC_SEND_TIMEOUT_FACTOR = 2.0

# Ethernet connection settings
G_REC_ADDRESS = 0
G_SEND_ADDRESS = (0, 0)

# Print Settings
G_PRINT_TO_FILE = True
G_PRINT_HK_DATA = False
G_PRINT_RAW_TM = False
G_PRINT_TM = True
G_RESEND_TC = False

"""
These objects are set for the Unit Test, no better solution found yet
"""
G_TM_LISTENER = None
G_COM_INTERFACE = None
G_TMTC_PRINTER = None


# noinspection PyUnusedLocal
def set_globals(args):
    global G_REC_ADDRESS, G_SEND_ADDRESS, G_SCRIPT_MODE, G_MODE_ID, G_SERVICE, G_DISPLAY_MODE,\
        G_COM_IF, G_COM_PORT, G_SERIAL_TIMEOUT, G_TM_TIMEOUT, G_TC_SEND_TIMEOUT_FACTOR, \
        G_PRINT_TO_FILE, G_PRINT_HK_DATA, G_PRINT_RAW_TM, G_PRINT_TM
    if args.mode == 0:
        LOGGER.info("GUI mode not implemented yet !")
    if args.shortDisplayMode:
        G_DISPLAY_MODE = "short"
    else:
        G_DISPLAY_MODE = "long"
    # Board IP address and ethernet port IP address can be passed optionally
    # by passing line parameter In PyCharm: Set parameters in run configuration
    # Add IP address of Ethernet port. Use command ipconfig in windows console or ifconfig in Linux.
    port_receive = 2008
    rec_address_to_set = (args.clientIP, port_receive)
    # Static IP of board
    port_send = 7
    send_address_to_set = (args.boardIP, port_send)
    if 0 <= args.mode <= 5:
        if args.mode == 0:
            G_MODE_ID = ModeList.GUIMode
        elif args.mode == 1:
            G_MODE_ID = ModeList.ListenerMode
        elif args.mode == 2:
            G_MODE_ID = ModeList.SingleCommandMode
        elif args.mode == 3:
            G_MODE_ID = ModeList.ServiceTestMode
        elif args.mode == 4:
            G_MODE_ID = ModeList.SoftwareTestMode
        elif args.mode == 5:
            G_MODE_ID = ModeList.UnitTest
    else:
        G_MODE_ID = ModeList[1]
    if args.com_if == ComIF.Ethernet.value:
        G_COM_IF = ComIF.Ethernet
    elif args.com_if == ComIF.Serial.value:
        G_COM_IF = ComIF.Serial
    elif args.com_if == ComIF.QEMU.value:
        G_COM_IF = ComIF.QEMU
    else:
        G_COM_IF = ComIF.Dummy

    G_SERVICE = str(args.service)
    if G_SERVICE.isdigit():
        G_SERVICE = int(args.service)
    else:
        G_SERVICE = args.service
    G_REC_ADDRESS = rec_address_to_set
    G_SEND_ADDRESS = send_address_to_set
    G_MODE_ID = G_MODE_ID
    G_PRINT_HK_DATA = args.print_hk
    G_PRINT_TM = args.print_tm
    G_PRINT_TO_FILE = args.print_log
    G_PRINT_RAW_TM = args.rawDataPrint
    G_COM_PORT = args.com_port
    G_TM_TIMEOUT = args.tm_timeout
    G_RESEND_TC = args.resend_tc
    from obsw_user_code import global_setup_hook
    global_setup_hook()

