"""
@brief      DLE Encoder Implementation
@details
DLE encoding can be used to provide a simple transport layer for serial data.
A give data stream is encoded by adding a STX char at the beginning and an ETX char at the end.
All STX and ETX occurences in the packet are encoded as well so the receiver can simply look
for STX and ETX occurences to identify packets.
"""

# TODO: Implementation / Translation of C code
