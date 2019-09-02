
set(ksgrd_network_helper_SRCS
    main.cpp
    Capture.cpp
    Packet.cpp
    ConnectionMapping.cpp
    Accumulator.cpp
)

add_executable(ksgrd_network_helper ${ksgrd_network_helper_SRCS})
target_include_directories(ksgrd_network_helper PUBLIC ${PCAP_INCLUDE_DIR})
target_link_libraries(ksgrd_network_helper ${PCAP_LIBRARY})
kde_target_enable_exceptions(ksgrd_network_helper PUBLIC)
set_target_properties(ksgrd_network_helper PROPERTIES CXX_STANDARD 14 CXX_STANDARD_REQUIRED TRUE)

# Why can't CMake fix this itself?'
target_link_libraries(ksgrd_network_helper pthread)

install(TARGETS ksgrd_network_helper DESTINATION ${KDE_INSTALL_LIBEXECDIR}/ksysguard)

if (Libcap_FOUND)
    install(
        CODE "execute_process(
        COMMAND ${SETCAP_EXECUTABLE}
        CAP_NET_RAW=+ep
        \$ENV{DESTDIR}${KDE_INSTALL_FULL_LIBEXECDIR}/ksysguard/ksgrd_network_helper)"
    )
endif()
