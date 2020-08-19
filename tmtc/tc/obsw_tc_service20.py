# -*- coding: utf-8 -*-
"""
@file   obsw_tc_service20.py
@brief  PUS Service 20:  Parameter management.
@author J. Gerhards
@date   30.06.2020
"""
import struct
from typing import Deque

import config.obsw_config as g
from tc.obsw_pus_tc_base import PusTelecommand
from tc.obsw_tc_service200 import pack_mode_data


def pack_service20_test_into(tc_queue: Deque, called_externally: bool = False) -> Deque:
    #parameter IDs
    parameterID0 = 0
    parameterID1 = 1
    parameterID2 = 2

    if called_externally is False:
        tc_queue.appendleft(("print", "Testing Service 20"))
    object_id = g.DUMMY_DEVICE_ID

    # set mode normal
    tc_queue.appendleft(("print", "Testing Service 20: Set Normal Mode"))
    mode_data = pack_mode_data(object_id, 2, 0)
    command = PusTelecommand(service=200, subservice=1, ssc=2000, app_data=mode_data)
    tc_queue.appendleft(command.pack_command_tuple())

    #test invalid subservice
    #use subservice 130 for invalid subservice check, as this is in use for dump reply
    #(and therefore will never be a valid subservice)
    #tc_queue.appendleft(("print", "Testing Service 20: Invalid subservice"))
    #mode_data = pack_mode_data(object_id, 2, 0)
    #command = PusTelecommand(service=20, subservice=130, ssc=810, app_data=mode_data)
    #tc_queue.appendleft(command.pack_command_tuple())

    #test invalid objectid //TODO: do we have an objectid known to be empty (even in future)?
    #tc_queue.appendleft(("print", "Testing Service 20: Invalid object ID"))
    #mode_data = pack_mode_data(object_id, 2, 0)
    #command = PusTelecommand(service=20, subservice=128, ssc=810, app_data=mode_data)
    #tc_queue.appendleft(command.pack_command_tuple())

    #test invalid parameterID for load
    #tc_queue.appendleft(("print", "Testing Service 20: Invalid parameter ID for load"))
    #mode_data = pack_mode_data(object_id, 2, 0)
    #command = PusTelecommand(service=20, subservice=128, ssc=810, app_data=mode_data)
    #tc_queue.appendleft(command.pack_command_tuple())

    #test invalid parameterID for dump
    #tc_queue.appendleft(("print", "Testing Service 20: Invalid parameter ID for dump"))
    #mode_data = pack_mode_data(object_id, 2, 0)
    #command = PusTelecommand(service=20, subservice=129, ssc=810, app_data=mode_data)
    #tc_queue.appendleft(command.pack_command_tuple())

    #test checking Load for uint32_t
    tc_queue.appendleft(("print", "Testing Service 20: Load uint32_t"))
    parameter_id = struct.pack(">I", parameterID0)
    parameter_data = struct.pack(">I", 42)
    payload = object_id + parameter_id + parameter_data
    command = PusTelecommand(service=20, subservice=128, ssc=2001, app_data=payload)
    tc_queue.appendleft(command.pack_command_tuple())

    #test checking Dump for uint32_t
    tc_queue.appendleft(("print", "Testing Service 20: Dump uint32_t"))
    parameter_id = struct.pack(">I", parameterID0)
    payload = object_id + parameter_id
    command = PusTelecommand(service=20, subservice=129, ssc=2001, app_data=payload)
    tc_queue.appendleft(command.pack_command_tuple())

    if called_externally is False:
        tc_queue.appendleft(("export", "log/tmtc_log_service20.txt"))
    return tc_queue


"""
    #test checking Load for int32_t
    tc_queue.appendleft(("print", "Testing Service 20: Load int32_t"))
    mode_data = pack_mode_data(object_id, 2, 0)
    command = PusTelecommand(service=20, subservice=128, ssc=2003, app_data=mode_data)
    tc_queue.appendleft(command.pack_command_tuple())

    #test checking Dump for int32_t
    tc_queue.appendleft(("print", "Testing Service 20: Dump int32_t"))
    mode_data = pack_mode_data(object_id, 2, 0)
    command = PusTelecommand(service=20, subservice=129, ssc=2004, app_data=mode_data)
    tc_queue.appendleft(command.pack_command_tuple())

    #test checking Load for float
    tc_queue.appendleft(("print", "Testing Service 20: Load float"))
    mode_data = pack_mode_data(object_id, 2, 0)
    command = PusTelecommand(service=20, subservice=128, ssc=2005, app_data=mode_data)
    tc_queue.appendleft(command.pack_command_tuple())

    #test checking Dump for float
    tc_queue.appendleft(("print", "Testing Service 20: Dump float"))
    mode_data = pack_mode_data(object_id, 2, 0)
    command = PusTelecommand(service=20, subservice=129, ssc=2006, app_data=mode_data)
    tc_queue.appendleft(command.pack_command_tuple())
"""




"""
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
"""
