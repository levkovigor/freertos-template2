# -*- coding: utf-8 -*-
"""
@file   obsw_tc_service2.py
@brief  PUS Service 2: Device Access, Native low-level commanding
@author R. Mueller
@date   01.11.2019
"""
import struct

import config.obsw_config as g
from tc.obsw_pus_tc_base import PusTelecommand, Deque
from tc.obsw_tc_service200 import pack_mode_data


def pack_service2_test_into(tc_queue: Deque, called_externally: bool = False) -> Deque:
    if called_externally is False:
        tc_queue.appendleft(("print", "Testing Service 2"))
    object_id = g.DUMMY_DEVICE_ID  # dummy device
    # don't forget to set object mode raw (G_SERVICE 200) before calling this !
    # Set Raw Mode
    tc_queue.appendleft(("print", "Testing Service 2: Setting Raw Mode"))
    mode_data = pack_mode_data(object_id, 3, 0)
    command = PusTelecommand(service=200, subservice=1, ssc=2020, app_data=mode_data)
    tc_queue.appendleft(command.pack_command_tuple())
    # toggle wiretapping raw
    tc_queue.appendleft(("print", "Testing Service 2: Toggling Wiretapping Raw"))
    wiretapping_toggle_data = pack_wiretapping_mode(object_id, 1)
    toggle_wiretapping_on_command = PusTelecommand(service=2, subservice=129, ssc=200,
                                                   app_data=wiretapping_toggle_data)
    tc_queue.appendleft(toggle_wiretapping_on_command.pack_command_tuple())
    # send raw command, wiretapping should be returned via TM[2,130] and TC[2,131]
    tc_queue.appendleft(("print", "Testing Service 2: Sending Raw Command"))
    raw_command = g.DUMMY_COMMAND_1
    raw_data = object_id + raw_command
    raw_command = PusTelecommand(service=2, subservice=128, ssc=201, app_data=raw_data)
    tc_queue.appendleft(raw_command.pack_command_tuple())
    # toggle wiretapping off
    tc_queue.appendleft(("print", "Testing Service 2: Toggle Wiretapping Off"))
    wiretapping_toggle_data = pack_wiretapping_mode(object_id, 0)
    toggle_wiretapping_off_command = PusTelecommand(service=2, subservice=129, ssc=204,
                                                    app_data=wiretapping_toggle_data)
    tc_queue.appendleft(toggle_wiretapping_off_command.pack_command_tuple())
    # send raw command which should be returned via TM[2,130]
    tc_queue.appendleft(("print", "Testing Service 2: Send second raw command"))
    command = PusTelecommand(service=2, subservice=128, ssc=205, app_data=raw_data)
    tc_queue.appendleft(command.pack_command_tuple())

    # Set mode off
    tc_queue.appendleft(("print", "Testing Service 2: Setting Off Mode"))
    mode_data = pack_mode_data(object_id, 0, 0)
    command = PusTelecommand(service=200, subservice=1, ssc=2020, app_data=mode_data)
    tc_queue.appendleft(command.pack_command_tuple())
    if called_externally is False:
        tc_queue.appendleft(("export", "log/tmtc_log_service2.txt"))
    return tc_queue


# wiretappingMode = 0: MODE_OFF, wiretappingMode = 1: MODE_RAW
def pack_wiretapping_mode(object_id, wiretapping_mode_):
    wiretapping_mode = struct.pack(">B", wiretapping_mode_)  # MODE_OFF : 0x00, MODE_RAW: 0x01
    wiretapping_toggle_data = object_id + wiretapping_mode
    return wiretapping_toggle_data
