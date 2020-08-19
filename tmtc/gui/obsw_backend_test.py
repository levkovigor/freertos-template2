from multiprocessing.connection import Listener
from multiprocessing import Process
from utility.obsw_logger import get_logger
import logging


LOGGER = get_logger()

class TmTcBackend(Process):
    def __init__(self):
        from obsw_tmtc_client import TmTcHandler
        super(TmTcBackend, self).__init__()
        self.address = ('localhost', 6000)     # family is deduced to be 'AF_INET'
        self.tmtc_backend = TmTcHandler()
        self.listener = Listener(self.address, authkey=None)
        self.conn = 0

    def run(self):
        self.listen()

    def listen(self):
        self.conn = self.listener.accept()
        LOGGER.info("TmTcBackend: Connection accepted from %s",str(self.listener.last_accepted))
        while True:
            msg = self.conn.recv()
            # do something with msg
            # here, the core client could be called to perform operations based on received message
            if msg == 'test':
                LOGGER.info("TmTcBackend: Hallo Welt !")
            elif msg == 'close':
                try:
                    self.conn.close()
                    break
                except OSError:
                    logging.exception("Error: ")
        self.listener.close()
