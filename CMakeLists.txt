cmake_minimum_required (VERSION 2.6)
project (octopus)
set (SRC_LIST
	octopus.c
	oct_log.c
	oct_proxy.c
	oct_thread.c
	)
add_executable (octopus ${SRC_LIST})
target_link_libraries (octopus pthread)
