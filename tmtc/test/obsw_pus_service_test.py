"""
@file   obsw_pus_service_test.py
@date   01.11.2019
@brief  Contains specific PUS service tests.
@author: R. Mueller
"""
import struct
import unittest
from typing import Deque

from test.obsw_module_test import TestService, PusTmInfoQueueT, TmDictionaryKeys, AssertionDictKeys
from tc.obsw_pus_tc_base import PusTcInfoQueueT
from tc.obsw_pus_tc_packer import pack_service17_test_into, pack_service5_test_into, \
    pack_service2_test_into, pack_service8_test_into, pack_service200_test_into
import config.obsw_config as g
from utility.obsw_logger import get_logger

LOGGER = get_logger()


class TestService2(TestService):
    """
    Test raw commanding service.
    """
    @classmethod
    def setUpClass(cls: TestService):
        super().setUpClass()
        print("Testing Service 2")
        # all commands must be sent sequentially, not as a burst
        cls.wait_intervals = [1, 2, 3, 4, 5]
        cls.wait_time = [2.0, 2.0, 2.0, 2.0, 2.0]
        pack_service2_test_into(cls.test_queue)

    def test_service2(self):
        """
        Tests the raw commanding service.
        """
        assertion_dict = self.perform_testing_and_generate_assertion_dict()
        self.event_expected = 3
        self.misc_expected = 5
        super()._perform_generic_assertion_test(assertion_dict)

    def analyse_tm_info(self, tm_info_queue: PusTmInfoQueueT, assertion_dict: dict):
        while not tm_info_queue.__len__() == 0:
            current_tm_info = tm_info_queue.pop()
            # Tc verification scanning is generic and has been moved to the superclass
            if current_tm_info[TmDictionaryKeys.SERVICE] == 1:
                super().scan_for_respective_tc(current_tm_info)
            # Here, the desired event Id or RID can be specified
            if current_tm_info[TmDictionaryKeys.SERVICE] == 5:
                # mode change
                if (current_tm_info[TmDictionaryKeys.REPORTER_ID] ==
                        struct.unpack('>I', g.DUMMY_DEVICE_ID)[0]) \
                        and (current_tm_info[TmDictionaryKeys.EVENT_ID] == 7401 or
                             current_tm_info[TmDictionaryKeys.EVENT_ID] == 7400):
                    self.event_counter = self.event_counter + 1
            if current_tm_info[TmDictionaryKeys.SERVICE] == 200 and \
                    current_tm_info[TmDictionaryKeys.SUBSERVICE] == 6:
                # mode change confirmation
                self.misc_counter += 1
            # wiretapping messages
            elif current_tm_info[TmDictionaryKeys.SERVICE] == 2 and \
                    (current_tm_info[TmDictionaryKeys.SUBSERVICE] == 130 or
                     current_tm_info[TmDictionaryKeys.SUBSERVICE] == 131):
                self.misc_counter += 1
            if current_tm_info[TmDictionaryKeys.VALID] == 0:
                self.valid = False
        assertion_dict.update({"MiscCount": self.misc_counter})


class TestService5(TestService):
    @classmethod
    def setUpClass(cls: TestService):
        super().setUpClass()
        LOGGER.info("Testing Service 5")
        # Wait intervals after TC 1,2 and 3 with specified wait times
        # This is required because the OBSW tasks runs with fixed sequences
        cls.wait_intervals = [1, 2, 3, 4]
        cls.wait_time = [2.0, 2.0, 2.0, 1.5]
        pack_service5_test_into(cls.test_queue)

    def test_Service5(self):
        # analyseTmInfo() and analyseTcInfo are called here
        assertion_dict = self.perform_testing_and_generate_assertion_dict()
        self.event_expected = 1
        self.misc_expected = 2
        self.fail_expected = 1
        self.tc_complete_counter -= 1
        self._perform_generic_assertion_test(assertion_dict)

    def analyse_tc_info(self, tc_info_queue: PusTcInfoQueueT):
        super().analyse_tc_info(tc_info_queue)

    def analyse_tm_info(self, tm_info_queue: PusTmInfoQueueT, assertion_dict: dict):
        super()._analyse_service5or17_tm(tm_info_queue, assertion_dict)

    @classmethod
    def tearDownClass(cls):
        super().tearDownClass()


class TestService6(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        print("Hallo Test 6!")


class TestService8(TestService):

    @classmethod
    def setUpClass(cls: TestService):
        super().setUpClass()
        LOGGER.info("Testing Service 8")
        cls.wait_intervals = [1, 2, 3, 4, 5]
        cls.wait_time = [1.5, 1.5, 2.2, 2.2, 2.0]
        cls.data_reply_count = 0
        pack_service8_test_into(cls.test_queue)

    def test_Service8(self):

        assertion_dict = self.perform_testing_and_generate_assertion_dict()
        # 3 x Mode changes
        self.misc_expected = 3
        # 2 x Event per mode change
        self.event_expected = 6
        self.data_reply_expected = 1
        # One reply generates an additional step.
        self.tc_verify_step_counter += 1
        self.assertEqual(assertion_dict[AssertionDictKeys.TC_STEP_COUNT],
                         self.tc_verify_step_counter)

        self.assertEqual(self.data_reply_count, self.data_reply_expected)
        super()._perform_generic_assertion_test(assertion_dict)

    def analyse_tm_info(self, tm_info_queue: Deque, assertion_dict: dict):
        while not tm_info_queue.__len__() == 0:
            current_tm_info = tm_info_queue.pop()
            if current_tm_info[TmDictionaryKeys.SERVICE] == 1:
                self.scan_for_respective_tc(current_tm_info)
            if (current_tm_info[TmDictionaryKeys.SERVICE] == 8 and
                    current_tm_info[TmDictionaryKeys.SUBSERVICE] == 130):
                self.data_reply_count += 1
            self._generic_mode_tm_check(current_tm_info, g.DUMMY_DEVICE_ID, [7401, 7400])


class TestService9(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        print("Hallo Test 9!")


class TestService17(TestService):
    @classmethod
    def setUpClass(cls: TestService):
        super().setUpClass()
        LOGGER.info("Testing Service 17")
        cls.wait_intervals = [1, 2, 3, 4]
        cls.wait_time = [1, 1, 1, 1]
        cls.tm_timeout = g.G_TM_TIMEOUT
        pack_service17_test_into(cls.test_queue)

    def test_Service17(self):
        assertion_dict = self.perform_testing_and_generate_assertion_dict()
        self.event_expected = 1
        self.misc_expected = 2
        self.fail_expected = 1
        self.tc_complete_counter -= 1
        self._perform_generic_assertion_test(assertion_dict)

    def _analyse_tm_tc_info(self, tm_info_queue: PusTmInfoQueueT, tc_info_queue: PusTcInfoQueueT):
        assertion_dict = super()._analyse_tm_tc_info(tm_info_queue=tm_info_queue,
                                                     tc_info_queue=tc_info_queue)
        # add anything elsee other than tc verification counter
        # and ssc that is needed for tm analysis
        return assertion_dict

    def analyse_tc_info(self, tc_info_queue):
        super().analyse_tc_info(tc_info_queue)

    def analyse_tm_info(self, tm_info_queue: PusTmInfoQueueT, assertion_dict: dict):
        super()._analyse_service5or17_tm(tm_info_queue, assertion_dict)

    @classmethod
    def tearDownClass(cls):
        print("Testing Service 17 finished")
        super().tearDownClass()

# class TestService20(TestService): # TODO: implement correctly
#
#     @classmethod
#     def setUpClass(cls: TestService):
#         super().setUpClass()
#         LOGGER.info("Testing Service 20")
#         cls.wait_intervals = [1, 2, 3, 4, 5]
#         cls.wait_time = [1.5, 1.5, 2.2, 2.2, 2.0]
#         cls.data_reply_count = 0
#         pack_service20_test_into(cls.test_queue)
#
#     def test_Service20(self):
#
#         assertion_dict = self.perform_testing_and_generate_assertion_dict()
#         # 3 x Mode changes
#         self.misc_expected = 3
#         # 2 x Event per mode change
#         self.event_expected = 6
#         self.data_reply_expected = 1
#         # One reply generates an additional step.
#         self.tc_verify_step_counter += 1
#         self.assertEqual(assertion_dict[AssertionDictKeys.TC_STEP_COUNT],
#                          self.tc_verify_step_counter)
#
#         self.assertEqual(self.data_reply_count, self.data_reply_expected)
#         super()._perform_generic_assertion_test(assertion_dict)
#
#     def analyse_tm_info(self, tm_info_queue: Deque, assertion_dict: dict):
#         while not tm_info_queue.__len__() == 0:
#             current_tm_info = tm_info_queue.pop()
#             if current_tm_info[TmDictionaryKeys.SERVICE] == 1:
#                 self.scan_for_respective_tc(current_tm_info)
#             if (current_tm_info[TmDictionaryKeys.SERVICE] == 8 and
#                     current_tm_info[TmDictionaryKeys.SUBSERVICE] == 130):
#                 self.data_reply_count += 1
#             self._generic_mode_tm_check(current_tm_info, g.DUMMY_DEVICE_ID, [7401, 7400])


class TestService200(TestService):
    @classmethod
    def setUpClass(cls: TestService):
        super().setUpClass()
        LOGGER.info("Testing Service 200")
        cls.wait_intervals = [1, 2, 3]
        cls.wait_time = [2, 2, 2]
        cls.tm_timeout = g.G_TM_TIMEOUT
        pack_service200_test_into(cls.test_queue)

    def test_Service200(self):
        assertion_dict = self.perform_testing_and_generate_assertion_dict()
        # 4 x Mode change with 2 events for each
        self.event_expected = 8
        self.misc_expected = 4
        self._perform_generic_assertion_test(assertion_dict)

    def _analyse_tm_tc_info(self, tm_info_queue: PusTmInfoQueueT, tc_info_queue: PusTcInfoQueueT):
        assertion_dict = super()._analyse_tm_tc_info(tm_info_queue=tm_info_queue,
                                                     tc_info_queue=tc_info_queue)
        # add anything else other than tc verification counter
        # and ssc that is needed for tm analysis
        return assertion_dict

    def analyse_tc_info(self, tc_info_queue):
        super().analyse_tc_info(tc_info_queue)

    def analyse_tm_info(self, tm_info_queue: Deque, assertion_dict: dict):
        while not tm_info_queue.__len__() == 0:
            current_tm_info = tm_info_queue.pop()
            # Tc verification scanning is generic and has been moved to the superclass
            if current_tm_info[TmDictionaryKeys.SERVICE] == 1:
                self.scan_for_respective_tc(current_tm_info)
            self._generic_mode_tm_check(current_tm_info, g.DUMMY_DEVICE_ID, [7401, 7400])

