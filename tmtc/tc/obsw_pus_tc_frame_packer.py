"""
@brief  Helper module to pack telecommand frames.
"""
from typing import List, Tuple
from tc.obsw_pus_tc_base import PusTelecommand, PusTcInfo
from utility.obsw_logger import get_logger

LOGGER = get_logger()

def pack_tc_frame(tc_list: List[PusTelecommand], max_frame_size: int,
                  do_fill_tc_frame: bool = False) -> Tuple[bytearray, int, int]:
    """
    Packs a given list of PusTelecommands into a TC frame.
    :param tc_list: List of PUS telecommands
    :param max_frame_size: Maximum allowed or desired frame size. Used to perform range checks
    :param do_fill_tc_frame: If this is set to true, the frame is filled up to max_frame_size with
    zeros
    :return: A tuples consisting of the TC frame, the current size of the frame and the number
    of packed commands.
    """
    frame = bytearray()
    packed_commands = 0
    for tc in tc_list:
        next_packet_raw = tc.pack()
        if len(frame) + len(next_packet_raw) > max_frame_size:
            LOGGER.warning("Next telecommands would be too large for TC frame, skipping.")
            break
        frame += next_packet_raw
        packed_commands += 1
    if do_fill_tc_frame:
        frame = fill_tc_frame(frame, max_frame_size)
    current_frame_size = len(frame)
    return frame, current_frame_size, packed_commands

def fill_tc_frame(tc_frame: bytearray, max_frame_size: int):
    """
    Fill a given TC frame with 0 until it has the specified frame size.
    """
    current_frame_len = len(tc_frame)
    remaining_size_to_fill = max_frame_size - current_frame_len
    if remaining_size_to_fill > 0:
        tc_frame.extend(bytearray(remaining_size_to_fill))
    return tc_frame

def pack_tc_info(tc_list: List[PusTelecommand]) -> List[PusTcInfo]:
    tc_info_list = list()
    for tc in tc_list:
        tc_info_list.append(tc.pack_information())
    return tc_info_list