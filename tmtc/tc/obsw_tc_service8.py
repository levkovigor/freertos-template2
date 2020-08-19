# -*- coding: utf-8 -*-
"""
@file   obsw_tc_service8.py
@brief  PUS Service 8:  High-level functional commanding.
@author R. Mueller
@date   01.11.2019
"""
from typing import Deque

import config.obsw_config as g
from tc.obsw_pus_tc_base import PusTelecommand
from tc.obsw_tc_service200 import pack_mode_data


def pack_service8_test_into(tc_queue: Deque, called_externally: bool = False) -> Deque:
    if called_externally is False:
        tc_queue.appendleft(("print", "Testing Service 8"))
    object_id = g.DUMMY_DEVICE_ID

    # set mode on
    tc_queue.appendleft(("print", "Testing Service 8: Set On Mode"))
    mode_data = pack_mode_data(object_id, 1, 0)
    command = PusTelecommand(service=200, subservice=1, ssc=800, app_data=mode_data)
    tc_queue.appendleft(command.pack_command_tuple())

    # set mode normal
    tc_queue.appendleft(("print", "Testing Service 8: Set Normal Mode"))
    mode_data = pack_mode_data(object_id, 2, 0)
    command = PusTelecommand(service=200, subservice=1, ssc=810, app_data=mode_data)
    tc_queue.appendleft(command.pack_command_tuple())

    # Direct command which triggers completion reply
    tc_queue.appendleft(("print", "Testing Service 8: Trigger Completion Reply"))
    action_id = g.DUMMY_COMMAND_1
    direct_command = object_id + action_id
    command = PusTelecommand(service=8, subservice=128, ssc=820, app_data=direct_command)
    tc_queue.appendleft(command.pack_command_tuple())

    # Direct command which triggers _tm_data reply
    tc_queue.appendleft(("print", "Testing Service 8: Trigger Data Reply"))
    action_id = g.DUMMY_COMMAND_2
    command_param1 = g.DUMMY_COMMAND_2_PARAM_1
    command_param2 = g.DUMMY_COMMAND_2_PARAM_2
    direct_command = object_id + action_id + command_param1 + command_param2
    command = PusTelecommand(service=8, subservice=128, ssc=830, app_data=direct_command)
    tc_queue.appendleft(command.pack_command_tuple())

    # Direct command which triggers an additional step reply and one completion reply
    tc_queue.appendleft(("print", "Testing Service 8: Trigger Step and Completion Reply"))
    action_id = g.DUMMY_COMMAND_3
    direct_command = object_id + action_id
    command = PusTelecommand(service=8, subservice=128, ssc=840, app_data=direct_command)
    tc_queue.appendleft(command.pack_command_tuple())

    # set mode off
    tc_queue.appendleft(("print", "Testing Service 8: Set Off Mode"))
    mode_data = pack_mode_data(object_id, 0, 0)
    command = PusTelecommand(service=200, subservice=1, ssc=800, app_data=mode_data)
    tc_queue.appendleft(command.pack_command_tuple())
    tc_queue.appendleft(("wait", 2))

    if called_externally is False:
        tc_queue.appendleft(("export", "log/tmtc_log_service8.txt"))
    return tc_queue
