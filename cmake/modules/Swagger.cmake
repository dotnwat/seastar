find_package(PythonInterp)

macro(SWAGGER_TARGET Input Output)
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${Output}
    COMMAND ${PYTHON_EXECUTABLE}
    ARGS ${PROJECT_SOURCE_DIR}/json/json2code.py -f ${CMAKE_CURRENT_SOURCE_DIR}/${Input} -o ${CMAKE_CURRENT_BINARY_DIR}/${Output}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${Input}
    COMMENT "Swagger ${Input}")
endmacro(SWAGGER_TARGET)
