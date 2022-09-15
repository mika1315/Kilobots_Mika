#include <kilombo.h>
#include <stdbool.h>
#include <math.h>

#ifdef SIMULATOR
#include <limits.h>
#else
#include <avr/io.h>
#include <stdlib.h>
#endif

#include "util.h"
#include "dispersion.h"  // defines the USERDATA structure
#include "simulation.h"
REGISTER_USERDATA(USERDATA)

#ifdef SIMULATOR
#include <jansson.h>
json_t *json_state();
#endif

#define T_DELAY 64

void setup() {
	// Initialize the random generator
    while(get_voltage() == -1);
    rand_seed(rand_hard() + kilo_uid);

	mydata->tumble_time = rand_soft();
	mydata->run_time = rand_soft();
}

void loop() {
	mydata->cycle = kilo_ticks%(mydata->tumble_time + mydata->run_time + T_DELAY);
	
	if(mydata->cycle == 0) {
		 // tumble state
		spinup_motors();
		set_color(RGB(3,0,0)); // red
		if(rand_soft()%2)
			set_motors(kilo_turn_right, 0);
		else
			set_motors(0, kilo_turn_left);
	} else if(mydata->cycle == mydata->tumble_time) {
		// run state
		spinup_motors();
		set_motors(kilo_straight_left, kilo_straight_right);
		set_color(RGB(0,3,0)); // green
	} else if(mydata->cycle == mydata->run_time) {
		mydata->tumble_time = rand_soft();
		mydata->run_time = rand_soft();
	}
}

int main(void) {
    // initialize hardware
	kilo_init();
	
	// register your program
	kilo_start(setup, loop);

	// Simulator settings
	SET_CALLBACK(global_setup, global_setup);
	SET_CALLBACK(botinfo, botinfo);
	SET_CALLBACK(reset, setup);
	SET_CALLBACK(json_state, json_state); // Saving bot state as json. Not for use in the real bot, only in the simulator.
	SET_CALLBACK(obstacles, obstacles_walls);

    return 0;
}