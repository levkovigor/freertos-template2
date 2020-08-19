# -*- coding: utf-8 -*-
"""
@file   obsw_tc_service9.py
@brief  PUS Service 9:  Time management.
@author R. Mueller
@date   01.11.2019
"""

from datetime import datetime
from tc.obsw_pus_tc_packer import TcQueueT, PusTelecommand
from utility.obsw_logger import get_logger

LOGGER = get_logger()


def pack_service9_test_into(tc_queue: TcQueueT) -> TcQueueT:
    tc_queue.appendleft(("print", "Testing Service 9"))
    time_test_ascii_code_a = '2019-08-30T20:50:33.892429Z' + '\0'
    time_test_ascii_code_b = '2019-270T05:50:33.002000Z' + '\0'
    time_test_current_time = datetime.now().isoformat() + "Z" + '\0'
    time_test1 = time_test_ascii_code_a.encode('ascii')
    time_test2 = time_test_ascii_code_b.encode('ascii')
    time_test3 = time_test_current_time.encode('ascii')
    LOGGER.info("Time Code 1 :%s", time_test1)
    LOGGER.info("Time Code 2 :%s", time_test2)
    LOGGER.info("Time Code 3 :%s", time_test3)
    # time setting
    tc_queue.appendleft(("print", "Testing Service 9: Testing timecode A"))
    command = PusTelecommand(service=9, subservice=128, ssc=900, app_data=bytearray(time_test1))
    tc_queue.appendleft(command.pack_command_tuple())
    tc_queue.appendleft(("print", "Testing Service 9: Testing timecode B"))
    command = PusTelecommand(service=9, subservice=128, ssc=910, app_data=bytearray(time_test2))
    tc_queue.appendleft(command.pack_command_tuple())
    tc_queue.appendleft(("print", "Testing Service 9: Testing timecode Current Time"))
    command = PusTelecommand(service=9, subservice=128, ssc=920, app_data=bytearray(time_test3))
    tc_queue.appendleft(command.pack_command_tuple())
    # TODO: Add other time formats here
    tc_queue.appendleft(("export", "log/tmtc_log_service9.txt"))
    return tc_queue