cmake_minimum_required (VERSION 2.6)
project (octopus)
set (SRC_LIST
	octopus.c
	oct_log.c
	)
add_executable (octopus ${SRC_LIST})
