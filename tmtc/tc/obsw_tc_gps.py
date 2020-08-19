# -*- coding: utf-8 -*-
"""
@file   obsw_tc_gps.py
@brief  GPS Device: GPS device testing
@author R. Mueller
@date   02.05.2020
"""

from tc.obsw_pus_tc_packer import TcQueueT, PusTelecommand
from tc.obsw_tc_service2 import pack_mode_data

import config.obsw_config as g

def pack_gps_test_into(object_id: bytearray, tc_queue: TcQueueT) -> TcQueueT:
    if object_id == g.GPS0_ObjectId:
        gps_string = "GPS0"
    elif object_id == g.GPS1_ObjectId:
        gps_string = "GPS1"
    else:
        gps_string = "unknown"
    tc_queue.appendleft(("print", "Testing " + gps_string + " Device"))
    # Set Mode Off
    tc_queue.appendleft(("print", "Testing  " + gps_string + ": Set Off"))
    mode_data = pack_mode_data(object_id, 0, 0)
    command = PusTelecommand(service=200, subservice=1, ssc=11, app_data=mode_data)
    tc_queue.appendleft(command.pack_command_tuple())
    # Set Mode On
    tc_queue.appendleft(("print", "Testing " + gps_string + ": Set On"))
    mode_data = pack_mode_data(object_id, 1, 0)
    command = PusTelecommand(service=200, subservice=1, ssc=12, app_data=mode_data)
    tc_queue.appendleft(command.pack_command_tuple())
    # Enable HK report
    sid_gps = 0
    if object_id == g.GPS0_ObjectId:
        sid_gps = g.GPS0_SID
        tc_queue.appendleft(("print", "Testing " + gps_string + ": Enable HK Reporting"))
        command = PusTelecommand(service=3, subservice=5, ssc=13, app_data=sid_gps)
        tc_queue.appendleft(command.pack_command_tuple())
    elif object_id == g.GPS1_ObjectId:
        sid_gps = g.GPS1_SID
        tc_queue.appendleft(("print", "Testing " + gps_string + ": Enable HK Reporting"))
        command = PusTelecommand(service=3, subservice=5, ssc=14, app_data=sid_gps)
        tc_queue.appendleft(command.pack_command_tuple())
    # pack wait interval until mode is on and a few gps replies have been received
    tc_queue.appendleft(("wait", 5))
    # Disable HK reporting
    tc_queue.appendleft(("print", "Testing Service 3: Disable " + gps_string + " definition"))
    command = PusTelecommand(service=3, subservice=6, ssc=15, app_data=sid_gps)
    tc_queue.appendleft(command.pack_command_tuple())
    # Set Mode Off
    tc_queue.appendleft(("print", "Testing " + gps_string + ": Set Off"))
    mode_data = pack_mode_data(object_id, 0, 0)
    command = PusTelecommand(service=200, subservice=1, ssc=13, app_data=mode_data)
    tc_queue.appendleft(command.pack_command_tuple())
    tc_queue.appendleft(("export", "log/tmtc_log_service_" + gps_string + ".txt"))
    return tc_queue
