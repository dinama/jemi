aux_source_directory(. SRC)
add_library(jemi STATIC ${SRC})
install(TARGETS jemi ARCHIVE DESTINATION lib)
install(FILES ../include/jemi.h DESTINATION include)
