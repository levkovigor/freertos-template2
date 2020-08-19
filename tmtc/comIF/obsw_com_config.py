"""
Set-up function. Initiates the communication interface.
"""
import sys

from comIF.obsw_com_interface import CommunicationInterface
from comIF.obsw_dummy_com_if import DummyComIF
from comIF.obsw_ethernet_com_if import EthernetComIF
from comIF.obsw_serial_com_if import SerialComIF, SerialCommunicationType
from comIF.obsw_qemu_com_if import QEMUComIF

from utility.obsw_logger import get_logger
from utility.obsw_tmtc_printer import TmTcPrinter

import config.obsw_config as g

LOGGER = get_logger()


def set_communication_interface(tmtc_printer: TmTcPrinter) -> CommunicationInterface:
    """
    Return the desired communication interface object
    :param tmtc_printer: TmTcPrinter object.
    :return: CommunicationInterface object
    """
    try:
        if g.G_COM_IF == g.ComIF.Ethernet:
            communication_interface = EthernetComIF(
                tmtc_printer=tmtc_printer, tm_timeout=g.G_TM_TIMEOUT,
                tc_timeout_factor=g.G_TC_SEND_TIMEOUT_FACTOR, send_address=g.G_SEND_ADDRESS,
                receive_address=g.G_REC_ADDRESS)
        elif g.G_COM_IF == g.ComIF.Serial:
            serial_baudrate = g.G_SERIAL_BAUDRATE
            serial_timeout = g.G_SERIAL_TIMEOUT
            serial_frame_size = g.G_SERIAL_FRAME_SIZE
            communication_interface = SerialComIF(
                tmtc_printer=tmtc_printer, com_port=g.G_COM_PORT, baud_rate=serial_baudrate,
                serial_timeout=serial_timeout,
                ser_com_type=SerialCommunicationType.FIXED_FRAME_BASED,
                com_type_args=serial_frame_size)
        elif g.G_COM_IF == g.ComIF.QEMU:
            communication_interface = QEMUComIF(
                tmtc_printer=tmtc_printer, tm_timeout=g.G_TM_TIMEOUT,
                tc_timeout_factor=g.G_TC_SEND_TIMEOUT_FACTOR)
        else:
            communication_interface = DummyComIF(tmtc_printer=tmtc_printer)
        return communication_interface
    except (IOError, OSError):
        LOGGER.error("Error setting up communication interface")
        LOGGER.exception("Error")
        sys.exit()
