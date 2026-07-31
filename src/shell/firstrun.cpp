#include "shell/firstrun.h"

#include <KConfigGroup>

bool runFirstStart(KConfigGroup &configuration)
{
    if (configuration.readEntry("FirstRunDone", false)) {
        return false;
    }

    // The marker is the whole default configuration for now. Values for the
    // settings of SPEC 13 belong at their readEntry() call sites instead:
    // defaults frozen into the file on the first start would never reach
    // existing users once a default changes.
    configuration.writeEntry("FirstRunDone", true);
    configuration.sync();
    return true;
}
