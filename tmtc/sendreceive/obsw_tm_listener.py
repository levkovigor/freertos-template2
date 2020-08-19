"""
@file
    obsw_tm_listener.py
@date
    01.11.2019
@brief
    Separate class to listen to telecommands.
    This will enable to run the listener in a separate thread later.
    This Listener will propably have some kind of mode. In default configuration,
    it will just listen for packets
    TODO: Have a look at the new asyncio library. Pyserial has an asyncio extension.
          See: https://pyserial-asyncio.readthedocs.io/en/latest/index.html
          Maybe this would simplify things here.
"""
import sys
import time
import threading
from collections import deque
from typing import TypeVar
from utility.obsw_logger import get_logger
from comIF.obsw_com_interface import CommunicationInterface
from tm.obsw_pus_tm_factory import PusTmQueueT, PusTmInfoQueueT
import config.obsw_config as g

LOGGER = get_logger()
TmListenerT = TypeVar('TmListenerT', bound='TmListener')


class TmListener:
    MODE_OPERATION_TIMEOUT = 300
    """
    Performs all TM listening operations.
    This listener can be used by setting the modeChangeEvent Event with the set() function
    and changing the mode to do special mode operations. The mode operation ends as soon
    the modeOpFinished Event is set() !
    """
    def __init__(self, com_interface: CommunicationInterface, tm_timeout: float,
                 tc_timeout_factor: float):
        self.tm_timeout = tm_timeout
        self.tc_timeout_factor = tc_timeout_factor
        self.com_interface = com_interface
        # this will be the default mode (listener mode)
        self.mode_id = g.ModeList.ListenerMode
        # TM Listener operations can be suspended by setting this flag
        self.event_listener_active = threading.Event()
        self.event_listener_active.set()
        # I don't think a listener is useful without the main program,
        # so we might just declare it daemonic.
        # UPDATE: Right now, the main program is not in a permanent loop and setting the
        # thread daemonic will cancel the program.
        # Solved for now by setting a permanent loop at the end of the main program
        self.listener_thread = threading.Thread(target=self.perform_operation, daemon=True)
        self.lock_listener = threading.Lock()
        # This Event is set by sender objects to perform mode operations
        self.event_mode_change = threading.Event()
        # This Event is set by sender objects if all necessary operations are done
        # to transition back to listener mode
        self.event_mode_op_finished = threading.Event()
        # maybe we will just make the thread daemonic...
        # self.terminationEvent = threading.Event()
        # This Event is set and cleared by the listener to inform the sender objects
        # if a reply has been received
        self.event_reply_received = threading.Event()
        # Will be filled for the Unit Test
        # TODO: Maybe its better to use a normal queue, which (by-design) offers more thread-safety
        #       Then we also don't need a lock. But I think its okay to treat the tm packet queue
        #       as a regular data structure for now.
        #       Then we propably have to pop the queue entries and put them into a list
        #       in the main program
        self.__tm_packet_queue = deque()

    def start(self):
        self.listener_thread.start()

    def perform_operation(self):
        while True:
            if self.event_listener_active.is_set():
                self.default_operation()
            else:
                time.sleep(1)

    def default_operation(self):
        """
        Core function. Normally, polls all packets
        """
        self.perform_core_operation()
        if self.event_mode_change.is_set():
            self.event_mode_change.clear()
            start_time = time.time()
            while not self.event_mode_op_finished.is_set():
                elapsed_time = time.time() - start_time
                if elapsed_time < TmListener.MODE_OPERATION_TIMEOUT:
                    self.perform_mode_operation()
                else:
                    LOGGER.warning("TmListener: Mode operation timeout occured!")
                    break
            self.event_mode_op_finished.clear()
            LOGGER.info("TmListener: Transitioning to listener mode.")
            self.mode_id = g.ModeList.ListenerMode

    def perform_core_operation(self):
        packet_list = self.com_interface.receive_telemetry()
        if len(packet_list) > 0:
            if self.lock_listener.acquire(True, timeout=1):
                self.__tm_packet_queue.append(packet_list)
            else:
                LOGGER.error("TmListener: Blocked on lock acquisition!")
            self.lock_listener.release()

    def perform_mode_operation(self):
        """
        By setting the modeChangeEvent with set() and specifying the mode variable,
        the TmListener is instructed to perform certain operations.
        :return:
        """
        # Listener Mode
        if self.mode_id == g.ModeList.ListenerMode:
            self.event_mode_op_finished.set()
        # Single Command Mode
        elif self.mode_id == g.ModeList.SingleCommandMode:
            # Listen for one reply sequence.
            if self.check_for_one_telemetry_sequence():
                # Set reply event, will be cleared by checkForFirstReply()
                self.event_reply_received.set()
        # Sequential Command Mode
        elif self.mode_id == g.ModeList.ServiceTestMode or \
                self.mode_id == g.ModeList.SoftwareTestMode:
            if self.check_for_one_telemetry_sequence():
                LOGGER.info("TmListener: Reply sequence received!")
                self.event_reply_received.set()
        elif self.mode_id == g.ModeList.UnitTest:
            # TODO: needs to be reworked. Info queue is stupid. just pop the normal queue and
            #       pack the information manually
            self.perform_core_operation()

    def check_for_one_telemetry_sequence(self) -> bool:
        """
        Receive all telemetry for a specified time period.
        This function prints the telemetry sequence but does not return it.
        :return:
        """
        tm_ready = self.com_interface.data_available(self.tm_timeout * self.tc_timeout_factor)
        if tm_ready is False:
            return False
        elif tm_ready is True:
            return self.__read_telemetry_sequence()
        else:
            LOGGER.error("TmListener: Configuration error in communication interface!")
            sys.exit()

    def __read_telemetry_sequence(self):
        """
        Thread-safe implementation for reading a telemetry sequence.
        """
        if self.lock_listener.acquire(True, timeout=1):
            self.__tm_packet_queue.append(self.com_interface.receive_telemetry())
        else:
            LOGGER.error("TmListener: Blocked on lock acquisition for longer than 1 second!")
        self.lock_listener.release()
        start_time = time.time()
        elapsed_time = 0
        LOGGER.info("TmListener: Listening for " + str(self.tm_timeout) + " seconds")
        while elapsed_time < self.tm_timeout:
            tm_ready = self.com_interface.data_available(1.0)
            if tm_ready:
                if self.lock_listener.acquire(True, timeout=1):
                    tm_list = self.com_interface.receive_telemetry()
                    self.__tm_packet_queue.append(tm_list)
                else:
                    LOGGER.critical("TmListener: Blocked on lock acquisition for longer than 1 second!")
                self.lock_listener.release()
            elapsed_time = time.time() - start_time
        # the timeout value can be set by special TC queue entries if wiretapping_packet handling
        # takes longer, but it is reset here to the global value
        if self.tm_timeout is not g.G_TM_TIMEOUT:
            self.tm_timeout = g.G_TM_TIMEOUT
        return True

    def retrieve_tm_packet_queue(self) -> PusTmQueueT:
        if self.lock_listener.acquire(True, timeout=1):
            tm_queue_copy = self.__tm_packet_queue.copy()
        else:
            tm_queue_copy = self.__tm_packet_queue.copy()
            LOGGER.critical("TmListener: Blocked on lock acquisition for longer than 1 second!")
        self.lock_listener.release()
        return tm_queue_copy

    @staticmethod
    def retrieve_info_queue_from_packet_queue(
            tm_queue: PusTmQueueT, pus_info_queue_to_fill: PusTmInfoQueueT):
        tm_queue_copy = tm_queue.copy()
        while tm_queue_copy.__len__() != 0:
            pus_packet_list = tm_queue_copy.pop()
            for pus_packet in pus_packet_list:
                pus_info_queue_to_fill.append(pus_packet.pack_tm_information())

    def clear_tm_packet_queue(self):
        self.__tm_packet_queue.clear()

