# -*- coding: utf-8 -*-
"""
@file   obsw_tc_service3.py
@brief  PUS Service 3:  Housekeeping Service.
@author R. Mueller
@date   02.05.2020
"""
import struct
from typing import Deque
from tc.obsw_pus_tc_base import PusTelecommand
import config.obsw_config as g

# adding custom defintion to hk using test pool variables
sid_test = g.TEST_SID
sid_custom = g.CUSTOM_SID
sid_gps = g.GPS0_SID
collection_interval_hk = struct.pack('>f', 3)
collection_interval_diag = struct.pack('>f',0.8)
number_of_parameters = struct.pack('>B', 5)
p1 = g.TEST_ID_1
p2 = g.TEST_ID_2
p3 = g.TEST_ID_3
p4 = g.TEST_ID_4
p5 = g.TEST_ID_5
hk_definition = sid_test + collection_interval_hk + number_of_parameters + p1 + p2 + p3 + p4 + p5
diag_definition = (sid_custom + collection_interval_diag + number_of_parameters +
                        p1 + p2 + p3 + p4 + p5)


def pack_service3_test_into(tc_queue: Deque) -> Deque:
    tc_queue.appendleft(("print", "Testing Service 3"))
    # Predefined packet testing
    pack_predefined_tests(tc_queue)

    pack_custom_tests(tc_queue)

    tc_queue.appendleft(("export", "log/tmtc_log_service3.txt"))
    return tc_queue


def pack_predefined_tests(tc_queue: Deque):
    # enable gps0
    tc_queue.appendleft(("print", "Testing Service 3: Enable GPS definition"))
    command = PusTelecommand(service=3, subservice=5, ssc=3000, app_data=sid_gps)
    tc_queue.appendleft(command.pack_command_tuple())

    # enable test
    tc_queue.appendleft(("print", "Testing Service 3: Enable test definition"))
    command = PusTelecommand(service=3, subservice=5, ssc=3010, app_data=sid_test)
    tc_queue.appendleft(command.pack_command_tuple())

    # wait a bit to receive at least 2 packets..
    tc_queue.appendleft(("wait", 2))

    # disable gps0
    tc_queue.appendleft(("print", "Testing Service 3: Disable GPS definition"))
    command = PusTelecommand(service=3, subservice=6, ssc=3020, app_data=sid_gps)
    tc_queue.appendleft(command.pack_command_tuple())

    # disable test
    tc_queue.appendleft(("print", "Testing Service 3: Disable test definition"))
    command = PusTelecommand(service=3, subservice=6, ssc=3030, app_data=sid_test)
    tc_queue.appendleft(command.pack_command_tuple())

    # report gps definition
    tc_queue.appendleft(("print", "Testing Service 3: Reporting GPS definition"))
    command = PusTelecommand(service=3, subservice=9, ssc=3040, app_data=sid_gps)
    tc_queue.appendleft(command.pack_command_tuple())

    # report test definition
    tc_queue.appendleft(("print", "Testing Service 3: Reporting test definition"))
    command = PusTelecommand(service=3, subservice=9, ssc=3050, app_data=sid_test)
    tc_queue.appendleft(command.pack_command_tuple())

    # generate one gps 0 definition
    tc_queue.appendleft(("print", "Testing Service 3: Generate one gps 0 defintion"))
    command = PusTelecommand(service=3, subservice=27, ssc=3060, app_data=sid_gps)
    tc_queue.appendleft(command.pack_command_tuple())

    # generate test definition
    tc_queue.appendleft(("print", "Testing Service 3: Generate test defintion"))
    command = PusTelecommand(service=3, subservice=27, ssc=3070, app_data=sid_test)
    tc_queue.appendleft(command.pack_command_tuple())


def pack_custom_tests(tc_queue: Deque):
    # deleting pre-defined test entry
    tc_queue.appendleft(("print", "Testing Service 3: Deleting pre-defined HK definition"))
    command = PusTelecommand(service=3, subservice=3, ssc=3100, app_data=sid_test)
    tc_queue.appendleft(command.pack_command_tuple())

    # adding pre-defined definition to hk using test pool variables
    tc_queue.appendleft(("print", "Testing Service 3: Adding pre-defined HK definition"))
    command = PusTelecommand(service=3, subservice=1, ssc=3110, app_data=hk_definition)
    tc_queue.appendleft(command.pack_command_tuple())

    # adding custom definition to diagnostics using test pool variables
    tc_queue.appendleft(("print", "Testing Service 3: Adding custom diganostics definition"))
    command = PusTelecommand(service=3, subservice=2, ssc=3120, app_data=diag_definition)
    tc_queue.appendleft(command.pack_command_tuple())

    # enable custom hk definition
    tc_queue.appendleft(("print", "Testing Service 3: Enable custom definition"))
    command = PusTelecommand(service=3, subservice=5, ssc=3130, app_data=hk_definition)
    tc_queue.appendleft(command.pack_command_tuple())

    # enable custom diag definition
    tc_queue.appendleft(("print", "Testing Service 3: Enable custom diagnostics definition"))
    command = PusTelecommand(service=3, subservice=7, ssc=3140, app_data=diag_definition)
    tc_queue.appendleft(command.pack_command_tuple())

    # Disable custom diag definition
    tc_queue.appendleft(("print", "Testing Service 3: Disable custom diagnostics definition"))
    command = PusTelecommand(service=3, subservice=8, ssc=3160, app_data=sid_custom)
    tc_queue.appendleft(command.pack_command_tuple())

    # Disable custom hk definition
    tc_queue.appendleft(("print", "Testing Service 3: Disable custom definition"))
    command = PusTelecommand(service=3, subservice=6, ssc=3150, app_data=sid_test)
    tc_queue.appendleft(command.pack_command_tuple())

    # report custom test definition
    tc_queue.appendleft(("print", "Testing Service 3: Reporting hk definition"))
    command = PusTelecommand(service=3, subservice=9, ssc=3170, app_data=sid_test)
    tc_queue.appendleft(command.pack_command_tuple())

    # report custom Diag definition
    tc_queue.appendleft(("print", "Testing Service 3: Reporting diag definition"))
    command = PusTelecommand(service=3, subservice=11, ssc=3180, app_data=sid_custom)
    tc_queue.appendleft(command.pack_command_tuple())

    # generate one custom hk definition
    tc_queue.appendleft(("print", "Testing Service 3: Generate one custom hk definition"))
    command = PusTelecommand(service=3, subservice=27, ssc=3190, app_data=sid_test)
    tc_queue.appendleft(command.pack_command_tuple())

    # generate one custom diag definition
    tc_queue.appendleft(("print", "Testing Service 3: Generate one custom diagnostics definition"))
    command = PusTelecommand(service=3, subservice=28, ssc=3200, app_data=sid_custom)
    tc_queue.appendleft(command.pack_command_tuple())

    # modify custom hk definition interval
    # new_interval = struct.pack('>f', 10.0)
    # new_interval_command = sid1 + new_interval
    # tc_queue.appendleft(("print", "Testing Service 3: Changing pre-defined HK definition interval"))
    # command = PusTelecommand(service=3, subservice=31, ssc=3090, app_data=new_interval_command)
    # tc_queue.appendleft(command.pack_command_tuple())
    # report custom HK definition
    # tc_queue.appendleft(("print", "Testing Service 3: Reporting pre-defined HK definition with changed interval"))
    # command = PusTelecommand(service=3, subservice=9, ssc=3090, app_data=sid1)
    # tc_queue.appendleft(command.pack_command_tuple())
    # modify custom diag definition interval
    # new_interval_command = sid2 + new_interval
    # tc_queue.appendleft(("print", "Testing Service 3: Changing custom diag HK definition interval"))
    # command = PusTelecommand(service=3, subservice=32, ssc=3090, app_data=new_interval_command)
    # tc_queue.appendleft(command.pack_command_tuple())
    # report custom diag definition
    # tc_queue.appendleft(("print", "Testing Service 3: Reporting diag definition"))
    # command = PusTelecommand(service=3, subservice=11, ssc=3100, app_data=sid2)
    # tc_queue.appendleft(command.pack_command_tuple())
    # append parameter to custom hk definiton
    # append parameter to custom diag definition

    # delete custom diag definition
    # tc_queue.appendleft(("print", "Testing Service 3: Deleting custom diagnostics definition"))
    # command = PusTelecommand(service=3, subservice=4, ssc=3120, app_data=sid2)
    # tc_queue.appendleft(command.pack_command_tuple())

