cmake_minimum_required(VERSION 3.10)

project(SCVP_Artifacts)

find_library(SYSTEMC_LIBRARY 
    NAMES systemc
    #PATHS /opt/systemc/lib-macosx64/ # Uncomment for macOS
    PATHS $ENV{SYSTEMC_HOME}/lib-$ENV{SYSTEMC_TARGET_ARCH}/
)

find_library(SYSTEMC_AMS_LIBRARY 
    NAMES systemc-ams
    #PATHS /opt/systemc-ams/lib-macosx64/ # Uncomment for macOS
    PATHS $ENV{SYSTEMC_AMS_HOME}/lib-$ENV{SYSTEMC_TARGET_ARCH}/
)

set(SYSTEMC_INCLUDE $ENV{SYSTEMC_HOME}/include)
set(SYSTEMC_AMS_INCLUDE $ENV{SYSTEMC_AMS_HOME}/include)

#set(SYSTEMC_INCLUDE /opt/systemc/include) # Uncomment for macOS
#set(SYSTEMC_AMS_INCLUDE /opt/systemc-ams/include) # Uncomment for macOS

add_subdirectory(callbacks)
add_subdirectory(clock_generator)
add_subdirectory(custom_fifo)
add_subdirectory(custom_signal)
add_subdirectory(custom_tlm)
add_subdirectory(datatypes)
add_subdirectory(delta_delay)
add_subdirectory(dynamic_processes)
add_subdirectory(event_finder)
add_subdirectory(feedback_loop)
add_subdirectory(fifo_example)
add_subdirectory(interfaces_ports)
add_subdirectory(kpn_artificial_deadlock)
add_subdirectory(kpn_example)
add_subdirectory(multiports)
add_subdirectory(mutex_example)
add_subdirectory(nb_fifo_example)
add_subdirectory(not_chain)
add_subdirectory(portarrays)
add_subdirectory(reporting)
add_subdirectory(sc_event_and_queue)
add_subdirectory(swapping_example)
add_subdirectory(thread_example)
add_subdirectory(tlm_at_1)
add_subdirectory(tlm_at_2)
add_subdirectory(tlm_at_3)
add_subdirectory(tlm_at_4)
add_subdirectory(tlm_at_backpressure)
add_subdirectory(tlm_cpu_example)
add_subdirectory(tlm_lt_debug_transport)
add_subdirectory(tlm_lt_dmi)
add_subdirectory(tlm_lt_initiator_interconnect_target)
add_subdirectory(tlm_lt_initiator_target)
add_subdirectory(tlm_memory_manager)
add_subdirectory(tlm_multipasstrough_sockets)
add_subdirectory(tlm_payload_extensions)
add_subdirectory(tlm_protocol_checker)
add_subdirectory(tlm_quantum_keeper)
add_subdirectory(tlm_simple_sockets)
add_subdirectory(ams-eln)
add_subdirectory(ams-eln2)
add_subdirectory(ams-tdf)
add_subdirectory(ams-tdf2)
add_subdirectory(ams-lsf)
add_subdirectory(ams-lsf2)
