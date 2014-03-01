#!/bin/bash

python cpplint.py --filter=-whitespace/tab,-whitespace/braces \
	--linelength=80 --extensions=c,h *
