import signal
from comIF.obsw_com_interface import CommunicationInterface
from utility.obsw_logger import get_logger

logger = get_logger()


def keyboard_interrupt_handler(com_interface: CommunicationInterface):
    logger.info("Disconnect registered")
    # Unit Test closes Serial Port at the end
    # We could do some optional stuff here
    if com_interface is not None:
        com_interface.send_telecommand(bytearray([0, 0, 0, 0, 0]))


class GracefulKiller:
    kill_now = False

    def __init__(self):
        signal.signal(signal.SIGINT, self.exit_gracefully)
        signal.signal(signal.SIGTERM, self.exit_gracefully)

    def exit_gracefully(self):
        self.kill_now = True
        print("I was killed")



