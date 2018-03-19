
set(SOURCES basic_analogTest.c)
add_executable(basic_analogTest ${SOURCES})
target_link_libraries(basic_analogTest soem)
install(TARGETS basic_analogTest DESTINATION bin)
