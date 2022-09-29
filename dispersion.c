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


uint8_t prob_moving = 5;
uint16_t offset = 0;
uint16_t scaling = 1;
float d_optim = 85.f;
uint8_t const lower_tumble_time = 0;
uint16_t const upper_tumble_time = 4 * 32;
uint32_t const kiloticks_random_walk_choice = 15;
uint32_t const neighbors_age_of_removal = 248;

#ifdef SIMULATOR
#include <jansson.h>
json_t *json_state();

void init_params() {
    init_int(prob_moving);
    init_float(offset);
    init_float(scaling);
    init_float(d_optim);
}
#endif

/****************************************************/
/* Colors {{{1 */
#define NRAINBOWCOLORS (sizeof(colors) / sizeof((colors)[0]))
// Double rainbow colors (all the way !)
uint8_t colors[] = {
    RGB(3,0,0),  // red
    RGB(3,3,0),  // yellow
    RGB(0,3,0),  // green
    RGB(0,3,3),  // green
    RGB(0,0,3),  // blue

    RGB(2,2,2),  // grey
    RGB(0,0,0),  // off
    RGB(2,1,0),  // orange
    RGB(1,1,1),  // grey

    RGB(2,0,0),  // red
    RGB(2,2,0),  // yellow
    RGB(0,2,0),  // green
    RGB(0,2,2),  // green
    RGB(0,0,2),  // blue
};

void set_color_from_nb_neighbours() {
    uint8_t const nb_neighbours = mydata->N_Neighbors;
    if(nb_neighbours == 0) {
 //       printf("set_color_from_nb_neighbours: nb_neighbours: %d\n", nb_neighbours);
        return;
    }
    float color_idx = (NRAINBOWCOLORS - 1) * ((float)(nb_neighbours - 0) / (float)(MAXN - 0));
    set_color(colors[(uint8_t)color_idx]);
//    printf("set_color_from_nb_neighbours: nb_neighbours: %d\n", nb_neighbours);
}


/****************************************************/
/* Messages {{{1 */
void setup_message(void) {
    uint8_t index = 0;
    //mydata->msg.type = NORMAL;
    mydata->msg.type = (int8_t)mydata->data_type;

    mydata->msg.data[index++] = kilo_uid & 0xff;         // 0 low  ID
    mydata->msg.data[index++] = kilo_uid >> 8;           // 1 high ID

    mydata->msg.crc = message_crc(&mydata->msg);
}


/* Process a received message at the front of the ring buffer.
 * Go through the list of neighbors. If the message is from a bot
 * already in the list, update the information, otherwise
 * add a new entry in the list
 */
void process_message() {
    uint8_t index = 0;
    uint8_t i;
    uint16_t ID;

    // Estimate distance of neighbor
    uint8_t d = estimate_distance(&RB_front().dist);
    if (d > CUTOFF)
        return;

    uint8_t *data = RB_front().msg.data;
    ID = data[index] | (data[index+1] << 8);
    index += 2;

    //data_type_t data_type = data[index++];
    data_type_t data_type = RB_front().msg.type;

    // search the neighbor list by ID
    for(i = 0; i < mydata->N_Neighbors; i++)
        if(mydata->neighbors[i].ID == ID) // found it
            break;

    if(i == mydata->N_Neighbors)   // this neighbor is not in list
        if(mydata->N_Neighbors < MAXN-1) // if we have too many neighbors, we overwrite the last entry
            mydata->N_Neighbors++;          // sloppy but better than overflow

    // i now points to where this message should be stored
    mydata->neighbors[i].ID = ID;
    mydata->neighbors[i].timestamp = kilo_ticks;
    mydata->neighbors[i].dist = d;
    mydata->neighbors[i].data_type = data_type;
}


/* Go through the list of neighbors, remove entries older than a threshold,
 * currently 2 seconds.
 */
void purgeNeighbors(void) {
    int8_t i;

    for (i = mydata->N_Neighbors-1; i >= 0; i--)
        if (kilo_ticks - mydata->neighbors[i].timestamp > neighbors_age_of_removal) //31 ticks = 1 s
        { //this one is too old.
            mydata->neighbors[i] = mydata->neighbors[mydata->N_Neighbors-1]; //replace it by the last entry
            mydata->N_Neighbors--;
        }
}


void clearNeighbors(void) {
    mydata->N_Neighbors = 0;
}

float Uniform(void){
    return ((float)rand()+1.0)/((float)RAND_MAX+2.0);
}

float rand_normal(float mu, float sigma) {
    float z = sqrt(-2.0*log(Uniform())) * sin(2.0*M_PI*Uniform());
    return mu + sigma*z;
}

void get_d_min() {
    int8_t i;
    uint8_t candidate_d_min = d_optim;

    for(i = 0; i < mydata->N_Neighbors; i++) {
        if (candidate_d_min > mydata->neighbors[i].dist)
            candidate_d_min = mydata->neighbors[i].dist;
    }
    mydata->d_min = candidate_d_min;
}

void setup() {
    // Initialize the random generator
    while(get_voltage() == -1);
    rand_seed(rand_hard() + kilo_uid);

    for(;;) {
        mydata->tumble_time = fabs(64 + rand_normal(0, 1) * 32); // 2 sec // not too big
        if (mydata->tumble_time < upper_tumble_time && mydata->tumble_time > lower_tumble_time) break;
    }
    mydata->run_time = 64; // 255;
    mydata->direction = rand_soft() % 2;
    mydata->prob = (rand_soft() * 100) / 255;

    // Reinit neighbors list
    clearNeighbors();

    setup_message();
}

void loop() {

    purgeNeighbors();

    // Process messages in the RX ring buffer
    while (!RB_empty()) {
        process_message();
        RB_popfront();
    }

    /*
    run and tumble algorithm
    */
    get_d_min();
    mydata->cycle = kilo_ticks - mydata->last_kiloticks;

    //printf("\ncycle : %d\n", mydata->cycle);
    //printf("tumble time : %d\n", mydata->tumble_time);
    //printf("run time : %d\n", mydata->run_time);
    //printf("prob %d\n", mydata->prob);
    //printf("direction %d\n", mydata->direction);

    if (mydata->flag == 0) {
        for(;;) {
            mydata->tumble_time = fabs(64 + rand_normal(0, 1) * 32); // 2 sec // not too big
            if (mydata->tumble_time < upper_tumble_time && mydata->tumble_time > lower_tumble_time) break;
        }
        mydata->frustration = 1.0f - mydata->d_min / d_optim; //  depends on min dist to closest robot
        printf("d_min  : %f\n", mydata->d_min);
        printf("d_min / d_optim : %f\n", mydata->d_min / d_optim);
        printf("frustration  : %f\n", mydata->frustration);
        mydata->run_time = offset + mydata->frustration * scaling;
        printf("run time : %d\n", mydata->run_time);
        //mydata->run_time = 64;
        mydata->direction = rand_soft() % 2;
        mydata->prob = (rand_soft() * 100) / 255;
        mydata->flag = 1;
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
        } else {
            mydata->last_kiloticks = kilo_ticks;
            mydata->flag = 0;
        }
    } else { // stop
        if (mydata->cycle < mydata->tumble_time + mydata->run_time) {
            set_motors(0, 0);
            set_color(RGB(3,3,3)); // white
        } else {
            mydata->last_kiloticks = kilo_ticks;
            mydata->flag = 0;
        }
    }

    // set_color_from_nb_neighbours();

}

int main(void) {
    // initialize hardware
    kilo_init();

    // initialize ring buffer
    RB_init();

    // register message callbacks
    kilo_message_rx = rxbuffer_push;
    kilo_message_tx = message_tx;
    
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
