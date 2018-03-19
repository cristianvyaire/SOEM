
set(SOURCES basic_digitalTest.c)
add_executable(basic_digitalTest ${SOURCES})
target_link_libraries(basic_digitalTest soem)
install(TARGETS basic_digitalTest DESTINATION bin)
