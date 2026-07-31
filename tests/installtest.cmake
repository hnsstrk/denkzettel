# Checks that installing puts the desktop file where SPEC 2.5 needs it: once as
# the application entry and once as the XDG autostart entry that starts the
# daemon with the session. Whether the session then honours it can only be seen
# by logging out and in (manual M1 checklist, sprint-02 3.3) — that the install
# rules exist at all is checked here.
#
# The installation goes into a staging directory via DESTDIR, so the test never
# writes outside the build tree.

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

foreach(entry IN ITEMS "${APPLICATION_ENTRY}" "${AUTOSTART_ENTRY}")
    if(NOT EXISTS "${STAGING_DIR}${entry}")
        message(FATAL_ERROR "Die Installation legte ${entry} nicht an:\n${install_output}")
    endif()
endforeach()
