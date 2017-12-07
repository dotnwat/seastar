find_package(PythonInterp)

macro(SWAGGER_TARGET Input Output)
  add_custom_command(
    OUTPUT ${Output}
    COMMAND ${PYTHON_EXECUTABLE}
    ARGS ${PROJECT_SOURCE_DIR}/json/json2code.py -f ${Input} -o ${Output}
    DEPENDS ${Input}
    COMMENT "Swagger ${Input}")
endmacro(SWAGGER_TARGET)
