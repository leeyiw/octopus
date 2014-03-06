cmake_minimum_required (VERSION 2.6)
project (octopus)
set (CMAKE_C_FLAGS "-g -O0 -Wall")
set (SRC_LIST
	octopus.c
	oct_log.c
	oct_proxy.c
	oct_thread.c
	oct_http.c
	oct_socket.c
	)
add_executable (octopus ${SRC_LIST})
target_link_libraries (octopus pthread)
