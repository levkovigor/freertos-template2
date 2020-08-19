#!/bin/bash
#
# OPUS makefile
#
#  Created on: Mar 04, 2010
#      Author: ziemke
#      Author: Claas Ziemke
#      Copyright 2010, Claas Ziemke <claas.ziemke@gmx.net>
#

BASEDIR=../
include $(BASEDIR)/options.mk

.PHONY: clean doc

all: doc

doc:
	cd ./doxy; doxygen OPUS.doxyfile;
