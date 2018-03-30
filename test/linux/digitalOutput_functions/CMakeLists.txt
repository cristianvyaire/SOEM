
set(SOURCES digitalOutput_functions.c)
add_executable(digitalOutput_functions ${SOURCES})
target_link_libraries(digitalOutput_functions soem)
install(TARGETS digitalOutput_functions DESTINATION bin)
