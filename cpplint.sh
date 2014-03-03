#!/bin/bash

python cpplint.py --filter=-whitespace/tab,-whitespace/braces,-readability/casting \
	--linelength=80 --extensions=c,h *
