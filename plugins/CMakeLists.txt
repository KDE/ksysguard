if (libpcap_FOUND)
    add_subdirectory(process/network)
endif()
add_subdirectory(process/nvidia)

add_subdirectory(global/nvidia)
add_subdirectory(global/ksgrd)
