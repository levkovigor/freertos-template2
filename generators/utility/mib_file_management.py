#! /usr/bin/python3.8
# -*- coding: utf-8 -*-
import shutil
import os


def copy_file(filename: str, destination: str = "", delete_existing_file: bool = False):
    if os.path.exists(filename):
        try:
            shutil.copy2(filename, destination)
        except FileNotFoundError as error:
            print("copy_file: File not found!")
            print(error)
        except shutil.SameFileError:
            print("copy_file: Source and destination are the same!")


def move_file(file_name: str, destination: str = ""):
    if os.path.exists(file_name):
        try:
            shutil.copy2(file_name, destination)
            os.remove(file_name)
        except FileNotFoundError as error:
            print("File not found!")
            print(error)