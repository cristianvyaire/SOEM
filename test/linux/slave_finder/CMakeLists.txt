
set(SOURCES slave_finder.c)
add_executable(slave_finder ${SOURCES})
target_link_libraries(slave_finder soem)
install(TARGETS slave_finder DESTINATION bin)
