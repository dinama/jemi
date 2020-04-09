aux_source_directory(. SRC)
foreach(FILO ${SRC})
  add_mutest(FILE ${FILO} DEPENDS jemi rt pthread stdc++)
endforeach(FILO)
