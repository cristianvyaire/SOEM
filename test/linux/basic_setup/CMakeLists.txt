
set(SOURCES basic_setup.c)
add_executable(basic_setup ${SOURCES})
target_link_libraries(basic_setup soem)
install(TARGETS basic_setup DESTINATION bin)
