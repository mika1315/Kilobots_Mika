/* Saving bot state as json. Not for use in the real bot, only in the simulator. */
#include <kilombo.h>

#ifdef SIMULATOR

#include <jansson.h>
#include <stdio.h>
#include <string.h>
#include "dispersion.h"

json_t *json_state()
{
    // Create the state object we return
    json_t* state = json_object();

    // Store all relevant values
    // Note: make sure there are no NaN values, as they will not be saved at all by jansson !
    json_object_set_new(state, "t", json_real(kilo_ticks));
    json_object_set_new(state, "d_min", json_real(mydata->d_min));
    json_object_set_new(state, "d_max", json_real(mydata->d_max));
    if (mydata->d_max > 0)
        json_object_set_new(state, "d_min/d_max", json_real(mydata->d_min/mydata->d_max));
    else
        json_object_set_new(state, "d_min/d_max", json_real(0));

    return state;
}

#endif
// MODELINE "{{{1
// vim:expandtab:softtabstop=4:shiftwidth=4:fileencoding=utf-8
// vim:foldmethod=marker
