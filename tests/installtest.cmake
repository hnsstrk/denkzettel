# Checks that installing puts the desktop file where SPEC 2.5 needs it: once as
# the application entry and once as the XDG autostart entry that starts the
# daemon with the session. Whether the session then honours it can only be seen
# by logging out and in (manual M1 checklist, sprint-02 3.3) — that the install
# rules exist at all is checked here. The AppStream metainfo rides along, and it
# is validated as well: an invalid one costs the software centre its text and
# its pictures without anything in the program going wrong (#73).
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

foreach(entry IN ITEMS "${APPLICATION_ENTRY}" "${AUTOSTART_ENTRY}" "${METAINFO_ENTRY}" "${DBUS_SERVICE_ENTRY}")
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

# One group per shortcut, and both of them are checked: SPEC 2.4 holds this per
# shortcut, and a second action forgotten here would register, show up in the
# system settings and do nothing at all.
#
# **What this does not check is that the two names match the code.** The ids
# below are typed out here and read from nowhere; nothing in this file can see
# `actionId()` in globalshortcuts.cpp. Measured 29.08.2026 in the review of
# #21: with `actionId(Recorder)` renamed to "show-recorderX" all fourteen test
# sets stayed green. So this is a guard against a group forgotten in the
# desktop file, and against nothing else — CLAUDE.md, finding 48.
#
# What does catch the drift is not a check but the program: at every start
# Denkzettel asks the shortcut service what it holds and reads the desktop file
# for the action of that name, and a name that matches nothing reaches the user
# as DesktopActionMissing with one executable step (SPEC 2.4, retro B5). That
# is the road this file cannot walk, and it is the road the user is on.
foreach(action IN ITEMS "show-capture" "show-recorder")
    if(NOT application_entry MATCHES "\nActions=[^\n]*${action};")
        message(FATAL_ERROR
            "${APPLICATION_ENTRY} does not list ${action} in Actions= — "
            "kglobalacceld will not find the action.")
    endif()

    if(NOT application_entry MATCHES "\n\\[Desktop Action ${action}\\]")
        message(FATAL_ERROR
            "${APPLICATION_ENTRY} has no [Desktop Action ${action}] group — "
            "the key press then runs into nothing.")
    endif()

    string(FIND "${application_entry}" "[Desktop Action ${action}]" action_group_start)
    string(SUBSTRING "${application_entry}" ${action_group_start} -1 action_group)

    if(NOT action_group MATCHES "\nExec=[^\n]+")
        message(FATAL_ERROR
            "The group [Desktop Action ${action}] in ${APPLICATION_ENTRY} has no "
            "Exec line — without it the service has nothing to start.")
    endif()
endforeach()

# What tells the two actions apart on the key press, and the reason issue #125
# went unnoticed for four weeks: without DBusActivatable, KIO's
# ApplicationLauncherJob runs the Exec line of the action instead of calling
# ActivateAction, both actions run the identical command line, and the running
# service cannot say which key was pressed — every press opens the capture
# window. Nothing about that is visible: the registration reads back correctly,
# the component is active, and Meta+N works, because its target is what happens
# anyway.
if(NOT application_entry MATCHES "\nDBusActivatable=true")
    message(FATAL_ERROR
        "${APPLICATION_ENTRY} has no DBusActivatable=true — the key press then runs "
        "the Exec line of the action, and both actions carry the same one.")
endif()

# And the service file that carries the other half: with DBusActivatable set the
# launcher only ever calls the bus name, so a session in which the daemon is not
# running needs the bus to be able to start it. The name has to be the one the
# launcher calls, which is the desktop file's own name, and the Exec line has to
# be absolute — a relative one is refused by the bus without a word to anybody.
file(READ "${STAGING_DIR}${DBUS_SERVICE_ENTRY}" dbus_service_entry)

# NAME and not NAME_WE: the latter cuts at the *first* dot and would ask for
# `Name=io` — measured, the check went red over a correct file.
get_filename_component(dbus_service_name "${DBUS_SERVICE_ENTRY}" NAME)
string(REGEX REPLACE "\\.service$" "" dbus_service_name "${dbus_service_name}")
if(NOT dbus_service_entry MATCHES "\nName=${dbus_service_name}\n")
    message(FATAL_ERROR
        "${DBUS_SERVICE_ENTRY} does not declare Name=${dbus_service_name} — the bus "
        "would start nothing for the name the launcher calls.")
endif()

if(NOT dbus_service_entry MATCHES "\nExec=/[^\n]+/denkzetteld")
    message(FATAL_ERROR
        "${DBUS_SERVICE_ENTRY} has no absolute Exec line for denkzetteld — the bus "
        "cannot activate the service from it.")
endif()

# The metainfo is validated at its installed location, not in the source tree:
# what a software centre reads is what the installation put down. --no-net keeps
# the run independent of the network — the screenshot URLs point into the
# repository and a check that fetches them would go red on a GitHub outage.
# --explain, because "validation failed" on its own names no line and no rule.
execute_process(
    COMMAND ${APPSTREAMCLI} validate --no-net --explain "${STAGING_DIR}${METAINFO_ENTRY}"
    RESULT_VARIABLE validate_result
    OUTPUT_VARIABLE validate_output
    ERROR_VARIABLE validate_output
)

if(NOT validate_result EQUAL 0)
    message(FATAL_ERROR
        "${METAINFO_ENTRY} is not valid AppStream:\n${validate_output}")
endif()

message(STATUS "${validate_output}")
