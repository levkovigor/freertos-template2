# -*- coding: utf-8 -*-
"""
@file   obsw_tc_packer.py
@brief  Packs the TC queue for specific G_SERVICE or device testing
@details
Contains the sevice packet which can pack specific service queues (hardcoded for now)
@author R. Mueller
@date   01.11.2019
"""
import os

from tc.obsw_pus_tc_base import PusTelecommand, TcQueueT
from tc.obsw_tc_service2 import pack_service2_test_into
from tc.obsw_tc_service3 import pack_service3_test_into
from tc.obsw_tc_service8 import pack_service8_test_into
from tc.obsw_tc_service9 import pack_service9_test_into
from tc.obsw_tc_service20 import pack_service20_test_into
from tc.obsw_tc_service200 import pack_mode_data, pack_service200_test_into
from tc.obsw_tc_service5_17 import pack_service5_test_into, pack_service17_test_into
from tc.obsw_tc_gps import pack_gps_test_into

from utility.obsw_logger import get_logger
import config.obsw_config as g
from collections import deque
from typing import Union

LOGGER = get_logger()

def pack_service_queue(service: Union[int, str], service_queue: TcQueueT):
    if service == 2:
        return pack_service2_test_into(service_queue)
    if service == 3:
        return pack_service3_test_into(service_queue)
    if service == 5:
        return pack_service5_test_into(service_queue)
    if service == 8:
        return pack_service8_test_into(service_queue)
    if service == 9:
        return pack_service9_test_into(service_queue)
    if service == 17:
        return pack_service17_test_into(service_queue)
    if service == 20:
        return pack_service20_test_into(service_queue)
    if service == 200:
        return pack_service200_test_into(service_queue)
    if service == "Dummy":
        return pack_dummy_device_test_into(service_queue)
    if service == "GPS0":
        # Object ID: GPS Device
        object_id = g.GPS0_ObjectId
        return pack_gps_test_into(object_id, service_queue)
    if service == "GPS1":
        # Object ID: GPS Device
        object_id = g.GPS1_ObjectId
        return pack_gps_test_into(object_id, service_queue)
    if service == "Error":
        return pack_error_testing_into(service_queue)
    LOGGER.warning("Invalid Service !")


# TODO: a way to select certain services would be nice (by passing a dict or array maybe)
def create_total_tc_queue() -> TcQueueT:
    if not os.path.exists("log"):
        os.mkdir("log")
    tc_queue = deque()
    tc_queue = pack_service2_test_into(tc_queue)
    tc_queue = pack_service3_test_into(tc_queue)
    tc_queue = pack_service5_test_into(tc_queue)
    tc_queue = pack_service8_test_into(tc_queue)
    tc_queue = pack_service9_test_into(tc_queue)
    tc_queue = pack_service17_test_into(tc_queue)
    tc_queue = pack_service200_test_into(tc_queue)
    tc_queue = pack_dummy_device_test_into(tc_queue)
    object_id = g.GPS0_ObjectId
    tc_queue = pack_gps_test_into(object_id, tc_queue)
    return tc_queue


def pack_dummy_device_test_into(tc_queue: TcQueueT) -> TcQueueT:
    tc_queue.appendleft(("print", "Testing Dummy Device"))
    # Object ID: Dummy Device
    object_id = g.DUMMY_DEVICE_ID
    # Set On Mode
    tc_queue.appendleft(("print", "Testing Service Dummy: Set On"))
    mode_data = pack_mode_data(object_id, 1, 0)
    command = PusTelecommand(service=200, subservice=1, ssc=1, app_data=mode_data)
    tc_queue.appendleft(command.pack_command_tuple())
    # Test Service 2 commands
    tc_queue.appendleft(("print", "Testing Service Dummy: Service 2"))
    pack_service2_test_into(tc_queue, True)
    # Test Service 8
    tc_queue.appendleft(("print", "Testing Service Dummy: Service 8"))
    pack_service8_test_into(tc_queue, True)
    tc_queue.appendleft(("export", "log/tmtc_log_service_dummy.txt"))
    return tc_queue

def pack_error_testing_into(tc_queue: TcQueueT) -> TcQueueT:
    # a lot of events
    command = PusTelecommand(service=17, subservice=129, ssc=2010)
    tc_queue.appendleft(command.pack_command_tuple())
    # a lot of ping testing
    command = PusTelecommand(service=17, subservice=130, ssc=2020)
    tc_queue.appendleft(command.pack_command_tuple())
    return tc_queue
