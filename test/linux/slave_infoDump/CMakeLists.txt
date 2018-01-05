
set(SOURCES slave_infoDump.c)
add_executable(slave_infoDump ${SOURCES})
target_link_libraries(slave_infoDump soem)
install(TARGETS slave_infoDump DESTINATION bin)
