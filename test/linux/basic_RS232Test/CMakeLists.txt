
set(SOURCES basic_RS232Test.c)
add_executable(basic_RS232Test ${SOURCES})
target_link_libraries(basic_RS232Test soem)
install(TARGETS basic_RS232Test DESTINATION bin)
