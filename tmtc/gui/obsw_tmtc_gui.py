#!/usr/bin/python3.7
# -*- coding: utf-8 -*-
"""
@file
    obsw_tmtc_gui.py
@date
    01.11.2019
@brief
    This is part of the TMTC client developed by the SOURCE project by KSat
@description
    GUI Testing for TMTC client
@manual
@author:
    R. Mueller
"""
import tkinter as tk
from multiprocessing.connection import Client
from multiprocessing import Process
from utility.obsw_logger import get_logger
import time

LOGGER = get_logger()

# A first simple version has drop down menus to chose all necessary options
# which are normally handled by the args parser.
# when pressing save, all chosen values get passed to the globals file (OBSW_Config)
# To start the program, another button with start needs to be set up.
# A third button to perform a keyboard interrupt should be implemented
# include a really nice source badge and make it large !
# plan this on paper first...
# Step 1: Huge Mission Badge in Tkinter window because that is cool.
# Step 2: Simple buttons to run servce tests around the badge.
class TmTcGUI(Process):
    def __init__(self):
        super(TmTcGUI, self).__init__()
        self.root = None
        self.address = ('localhost', 6000)
        self.conn = Client(self.address, authkey=None)
        self.counter = 0

    def run(self):
        self.open()

    def open(self):
        self.root = tk.Tk()
        self.root.title("Hallo Welt")
        while True:
            LOGGER.info("TmTcGUI: Sending test message")
            self.conn.send("test")
            self.root.update()
            time.sleep(0.1)
            self.counter = self.counter + 1
            if self.counter == 50:
                self.conn.send("close")
                self.conn.close()
                break
        # self.window.mainloop()

    def close(self):
        pass
        # self.window.quit()
