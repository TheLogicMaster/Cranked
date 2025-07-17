#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "pd_api.h"

static PlaydateAPI* pd = NULL;

#ifdef _WINDLL
__declspec(dllexport)
#endif
int eventHandler(PlaydateAPI* playdate, PDSystemEvent event, uint32_t arg) {
    (void)arg;

    if (event == kEventInitLua) {
        pd = playdate;

    }

    return 0;
}
