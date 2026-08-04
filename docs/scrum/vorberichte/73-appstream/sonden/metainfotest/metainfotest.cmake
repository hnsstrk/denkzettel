file(REMOVE_RECURSE "${STAGING_DIR}")

execute_process(
    COMMAND ${CMAKE_COMMAND} -E env DESTDIR=${STAGING_DIR}
            ${CMAKE_COMMAND} --install ${BINARY_DIR}
    RESULT_VARIABLE install_result
    OUTPUT_VARIABLE install_output
    ERROR_VARIABLE install_output
)

if(NOT install_result EQUAL 0)
    message(FATAL_ERROR "Die Installation schlug fehl:\n${install_output}")
endif()

if(NOT EXISTS "${STAGING_DIR}${METAINFO_ENTRY}")
    message(FATAL_ERROR "Die Installation legte ${METAINFO_ENTRY} nicht an.")
endif()

execute_process(
    COMMAND ${APPSTREAMCLI} validate --no-net "${STAGING_DIR}${METAINFO_ENTRY}"
    RESULT_VARIABLE validate_result
    OUTPUT_VARIABLE validate_output
    ERROR_VARIABLE validate_output
)

if(NOT validate_result EQUAL 0)
    message(FATAL_ERROR "appstreamcli validate schlug fehl:\n${validate_output}")
endif()

message(STATUS "${validate_output}")
