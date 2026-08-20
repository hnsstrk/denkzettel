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

# With the application installed, a shortcut does not trigger a D-Bus message but
# the desktop action of the same name: kglobalacceld treats a component whose name
# ends in .desktop as a service action component and starts the action through an
# ApplicationLauncherJob. If the group is missing, the service logs an error and
# the key press fizzles out — exactly the user's finding of 2026-08-01 (SPEC 2.4).
# What gets checked is the application entry, because KService resolves the
# component through it.
file(READ "${STAGING_DIR}${APPLICATION_ENTRY}" application_entry)

if(NOT application_entry MATCHES "\nActions=[^\n]*show-capture;")
    message(FATAL_ERROR
        "${APPLICATION_ENTRY} does not list show-capture in Actions= — "
        "kglobalacceld will not find the action.")
endif()

if(NOT application_entry MATCHES "\n\\[Desktop Action show-capture\\]")
    message(FATAL_ERROR
        "${APPLICATION_ENTRY} has no [Desktop Action show-capture] group — "
        "the key press then runs into nothing.")
endif()

string(FIND "${application_entry}" "[Desktop Action show-capture]" action_group_start)
string(SUBSTRING "${application_entry}" ${action_group_start} -1 action_group)

if(NOT action_group MATCHES "\nExec=[^\n]+")
    message(FATAL_ERROR
        "Die Gruppe [Desktop Action show-capture] in ${APPLICATION_ENTRY} hat keine "
        "Exec-Zeile — ohne sie hat der Dienst nichts zu starten.")
endif()
