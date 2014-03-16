cmake_minimum_required (VERSION 2.6)
project (octopus)

if (CMAKE_BUILD_TYPE MATCHES DEBUG)
	add_definitions (-DOCTOPUS_DEBUG)
endif (CMAKE_BUILD_TYPE MATCHES DEBUG)

set (SRC_LIST
	octopus.c
	oct_conn.c
	oct_log.c
	oct_proxy.c
	oct_thread.c
	oct_http.c
	oct_socket.c
	)
add_executable (octopus ${SRC_LIST})
target_link_libraries (octopus pthread)
