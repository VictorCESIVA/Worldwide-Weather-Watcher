#include "station.h"
Mode mode_apres_appui(Mode m, Mode precedent, bool rouge)
{
    if (m == CONFIGURATION)
        return m;
    if (rouge)
    {
        if (m == MAINTENANCE)
            return precedent;
        if (m == ECONOMIQUE && ROUGE_ECO_VERS_STANDARD)
            return STANDARD;
        return MAINTENANCE;
    }
    return m == STANDARD ? ECONOMIQUE : m;
}
