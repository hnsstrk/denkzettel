#pragma once

#include <QString>

namespace platform
{

/**
 * `~/.local/share/denkzettel/denkzettel.log` — the detailed channel of SPEC 14.
 *
 * It follows the **data** directory and not the configuration: a run with an
 * `XDG_DATA_HOME` of its own gets a log of its own, which is precisely what the
 * journal, filtering by process name, cannot give (PO decision 04.09.2026).
 */
QString logFilePath();

/**
 * Installs the message handler that writes the file (SPEC 14).
 *
 * **A second road for the same message and not a second log**: the handler that
 * was in place stays in place and is called first, so every line still reaches
 * stderr and with it `journalctl --user -t denkzetteld`. What the file leaves
 * out is `qDebug` — SPEC 14 asks for what the program reports, and a debug line
 * is not that.
 *
 * A file that cannot be written is reported on stderr with `fprintf` and never
 * with `qWarning`, which would call this handler again; and it is reported
 * once, because a full disk would otherwise write the same complaint for every
 * line that follows.
 */
void startLogFile();

}
