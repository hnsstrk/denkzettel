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

# Bei installierter Anwendung löst ein Kürzel keine D-Bus-Nachricht aus, sondern
# die gleichnamige Desktop-Action: kglobalacceld behandelt eine Komponente, deren
# Name auf .desktop endet, als Service-Action-Komponente und startet die Aktion
# über einen ApplicationLauncherJob. Fehlt die Gruppe, protokolliert der Dienst
# einen Fehler und der Tastendruck verpufft — genau der Kundenbefund vom
# 01.08.2026 (SPEC 2.4). Geprüft wird der Anwendungseintrag, denn über ihn löst
# KService die Komponente auf.
file(READ "${STAGING_DIR}${APPLICATION_ENTRY}" application_entry)

if(NOT application_entry MATCHES "\nActions=[^\n]*show-capture;")
    message(FATAL_ERROR
        "${APPLICATION_ENTRY} führt show-capture nicht in Actions= — "
        "kglobalacceld findet die Aktion dann nicht.")
endif()

if(NOT application_entry MATCHES "\n\\[Desktop Action show-capture\\]")
    message(FATAL_ERROR
        "${APPLICATION_ENTRY} hat keine Gruppe [Desktop Action show-capture] — "
        "der Tastendruck läuft dann ins Leere.")
endif()

string(FIND "${application_entry}" "[Desktop Action show-capture]" action_group_start)
string(SUBSTRING "${application_entry}" ${action_group_start} -1 action_group)

if(NOT action_group MATCHES "\nExec=[^\n]+")
    message(FATAL_ERROR
        "Die Gruppe [Desktop Action show-capture] in ${APPLICATION_ENTRY} hat keine "
        "Exec-Zeile — ohne sie hat der Dienst nichts zu starten.")
endif()
