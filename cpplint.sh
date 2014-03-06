#!/bin/bash

python cpplint.py --linelength=80 --extensions=c,h \
	--filter=-whitespace/tab,-whitespace/braces,-readability/casting,-legal/copyright,-build/include \
	 *
