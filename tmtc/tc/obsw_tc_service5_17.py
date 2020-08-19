# -*- coding: utf-8 -*-
"""
@file   obsw_tc_service5_17.py
@brief  PUS Service 5: Event Service
        PUS Service 17: Test Service
@author R. Mueller
@date   02.05.2020
"""

from tc.obsw_pus_tc_packer import TcQueueT, PusTelecommand


def pack_service5_test_into(tc_queue: TcQueueT) -> TcQueueT:
    tc_queue.appendleft(("print", "Testing Service 5"))
    # invalid subservice
    tc_queue.appendleft(("print", "Testing Service 5: Invalid subservice"))
    command = PusTelecommand(service=5, subservice=1, ssc=500)
    tc_queue.appendleft(command.pack_command_tuple())
    # disable events
    tc_queue.appendleft(("print", "Testing Service 5: Disable event"))
    command = PusTelecommand(service=5, subservice=6, ssc=500)
    tc_queue.appendleft(command.pack_command_tuple())
    # trigger event
    tc_queue.appendleft(("print", "Testing Service 5: Trigger event"))
    command = PusTelecommand(service=17, subservice=128, ssc=510)
    tc_queue.appendleft(command.pack_command_tuple())
    # enable event
    tc_queue.appendleft(("print", "Testing Service 5: Enable event"))
    command = PusTelecommand(service=5, subservice=5, ssc=520)
    tc_queue.appendleft(command.pack_command_tuple())
    # trigger event
    tc_queue.appendleft(("print", "Testing Service 5: Trigger another event"))
    command = PusTelecommand(service=17, subservice=128, ssc=530)
    tc_queue.appendleft(command.pack_command_tuple())
    tc_queue.appendleft(("export", "log/tmtc_log_service5.txt"))
    return tc_queue


def pack_service17_test_into(tc_queue: TcQueueT) -> TcQueueT:
    tc_queue.appendleft(("print", "Testing Service 17"))
    # ping test
    tc_queue.appendleft(("print", "Testing Service 17: Ping Test"))
    command = PusTelecommand(service=17, subservice=1, ssc=1700)
    tc_queue.appendleft(command.pack_command_tuple())
    # enable event
    tc_queue.appendleft(("print", "Testing Service 17: Enable Event"))
    command = PusTelecommand(service=5, subservice=5, ssc=52)
    tc_queue.appendleft(command.pack_command_tuple())
    # test event
    tc_queue.appendleft(("print", "Testing Service 17: Trigger event"))
    command = PusTelecommand(service=17, subservice=128, ssc=1701)
    tc_queue.appendleft(command.pack_command_tuple())
    # invalid subservice
    tc_queue.appendleft(("print", "Testing Service 17: Invalid subservice"))
    command = PusTelecommand(service=17, subservice=243, ssc=1702)
    tc_queue.appendleft(command.pack_command_tuple())
    tc_queue.appendleft(("export", "log/tmtc_log_service17.txt"))
    return tc_queue
