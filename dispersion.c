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

double Uniform(void){
    return ((double)rand()+1.0)/((double)RAND_MAX+2.0);
}

double rand_normal(double mu, double sigma) {
    double z = sqrt(-2.0*log(Uniform())) * sin(2.0*M_PI*Uniform());
    return mu + sigma*z;
}

void setup() {
    // Initialize the random generator
    while(get_voltage() == -1);
    rand_seed(rand_hard() + kilo_uid);

    mydata->tumble_time = 255 + rand_normal(0, 1) * 32; // 2 secs
    mydata->run_time = 255;
    mydata->direction = rand_soft() % 2;
}

void loop() {
    mydata->cycle = kilo_ticks%(mydata->tumble_time + mydata->run_time);
    
    if (mydata->cycle == 0) {
        kilo_ticks = 0;
        mydata->tumble_time = 255 + rand_normal(0, 1) * 32; // 2 sec
        mydata->run_time = 255;
    } else if (mydata->cycle < mydata->tumble_time) {
        // tumble state
        spinup_motors();
        set_color(RGB(3,0,0)); // red
        if(mydata->direction)
            set_motors(kilo_turn_right, 0);
        else
            set_motors(0, kilo_turn_left);
    } else if (mydata->cycle < mydata->tumble_time + mydata->run_time) {
        // run state
        spinup_motors();
        set_motors(kilo_straight_left, kilo_straight_right);
        set_color(RGB(0,3,0)); // green        

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