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


// #define T_DELAY 64
// uint16_t const t_t = 64;
// uint16_t const t_r = ;
uint8_t flag = 1;
uint8_t lower_tumble_time = 0;
uint8_t upper_tumble_time = 4 * 32;
uint8_t check_lastcycle = 0;
uint8_t prob_moving = 5;


#ifdef SIMULATOR
#include <jansson.h>
json_t *json_state();

void init_params() {
    init_int(prob_moving);
}
#endif


float Uniform(void){
    return ((float)rand()+1.0)/((float)RAND_MAX+2.0);
}

float rand_normal(float mu, float sigma) {
    float z = sqrt(-2.0*log(Uniform())) * sin(2.0*M_PI*Uniform());
    return mu + sigma*z;
}

void setup() {
    // Initialize the random generator
    while(get_voltage() == -1);
    rand_seed(rand_hard() + kilo_uid);

    for(;;) {
        mydata->tumble_time = 64 + fabs(rand_normal(0, 1)) * 32; // 2 sec // not too big
        if (mydata->tumble_time < upper_tumble_time && mydata->tumble_time > lower_tumble_time) break;
    }
    mydata->run_time = 64; // 255;
    mydata->direction = rand_soft() % 2;
    mydata->prob = rand_soft() % 100;
}

void loop() {
    mydata->cycle = kilo_ticks%(mydata->tumble_time + mydata->run_time);
    // printf("\ncycle : %d\n", mydata->cycle);
    // printf("run : %d\n", mydata->run_time);
    // printf("tumble %d\n", mydata->tumble_time);
    // printf("prob %d\n", mydata->prob);
    // printf("direction %d\n", mydata->direction);

    if (flag == 0) {
        for(;;) {
            mydata->tumble_time = 64 + fabs(rand_normal(0, 1)) * 32; // 2 sec // not too big
            if (mydata->tumble_time < upper_tumble_time && mydata->tumble_time > lower_tumble_time) break;
        }
        mydata->run_time = 64;
        mydata->direction = rand_soft() % 2;
        mydata->prob = rand_soft() % 100;
        flag = 1;
    } else if (mydata->prob < prob_moving) { // move
        if (mydata->cycle < mydata->tumble_time) {
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
            // flag = 0;
            check_lastcycle = mydata->cycle;
            if (check_lastcycle >= mydata->tumble_time + mydata->run_time - 1) {
                check_lastcycle = 0;
                flag = 0;
            }            
        }
    } else { // stop
        if (mydata->cycle < mydata->tumble_time + mydata->run_time) {
            set_motors(0, 0);
            set_color(RGB(3,3,3)); // white
            // flag = 0;
            check_lastcycle = mydata->cycle;
            if (check_lastcycle >= mydata->tumble_time + mydata->run_time - 1) {
                check_lastcycle = 0;
                flag = 0;
            }
        }
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
