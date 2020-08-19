# -*- coding: utf-8 -*-
"""
@file   obsw_tc_service200.py
@brief  PUS Service 200:  PUS custom service 200: Mode commanding
@author R. Mueller
@date   02.05.2020
"""
from tc.obsw_pus_tc_base import PusTelecommand
from tc.obsw_pus_tc_packer import TcQueueT
import config.obsw_config as g
import struct


def pack_service200_test_into(tc_queue: TcQueueT) -> TcQueueT:
    tc_queue.appendleft(("print", "Testing Service 200"))
    # Object ID: Dummy Device
    object_id = g.DUMMY_DEVICE_ID
    # Set On Mode
    tc_queue.appendleft(("print", "Testing Service 200: Set Mode On"))
    mode_data = pack_mode_data(object_id, 1, 0)
    command = PusTelecommand(service=200, subservice=1, ssc=2000, app_data=mode_data)
    tc_queue.appendleft(command.pack_command_tuple())
    # Set Normal mode
    tc_queue.appendleft(("print", "Testing Service 200: Set Mode Normal"))
    mode_data = pack_mode_data(object_id, 2, 0)
    command = PusTelecommand(service=200, subservice=1, ssc=2010, app_data=mode_data)
    tc_queue.appendleft(command.pack_command_tuple())
    # Set Raw Mode
    tc_queue.appendleft(("print", "Testing Service 200: Set Mode Raw"))
    mode_data = pack_mode_data(object_id, 3, 0)
    command = PusTelecommand(service=200, subservice=1, ssc=2020, app_data=mode_data)
    tc_queue.appendleft(command.pack_command_tuple())
    # Set Off Mode
    tc_queue.appendleft(("print", "Testing Service 200: Set Mode Off"))
    mode_data = pack_mode_data(object_id, 0, 0)
    command = PusTelecommand(service=200, subservice=1, ssc=2030, app_data=mode_data)
    tc_queue.appendleft(command.pack_command_tuple())
    tc_queue.appendleft(("export", "log/tmtc_log_service200.txt"))
    return tc_queue


# Mode 0: Off, Mode 1: Mode On, Mode 2: Mode Normal, Mode 3: Mode Raw
def pack_mode_data(object_id: bytearray, mode_: int, submode_: int) -> bytearray:
    # Normal mode
    mode = struct.pack(">I", mode_)
    # Submode default
    submode = struct.pack('B', submode_)
    mode_data = object_id + mode + submode
    return mode_data
