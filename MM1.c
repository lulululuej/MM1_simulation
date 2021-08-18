#include <stdio.h>
#include <math.h>
#include "lcgrand.h"

#define BUSY 1
#define IDLE 0
#define Q_LIMIT 100

int num_delays_required, next_event_type, num_cus_delayed, num_events, num_in_q, server_status;
float mean_interval, mean_service, area_num_in_q, area_server_status, sim_time, time_arrival[Q_LIMIT + 1], time_last_event, next_arrival_time, next_departure_time, total_of_delays;
FILE *infile, *outfile;

void initialize(void);
void timing(void);
void update_time_avg_stats(void);
void arrive(void);
void depart(void);
void exp(float mean);
void repoert(void);


int main(void) {
    infile = fopen("mm1.in", "r");
    outfile = fopen("mm1.out", "w");
    num_events = 2; // arrive 1 and depart 2
    
    fscanf(infile, "%d%f%f", &num_delays_required, &mean_interval, &mean_service);
    
    initialize();
    
    while(num_cus_delayed < num_delays_required){
        timing(); // Determine the next event
        update_time_avg_stats(); // Update time-average area_num_in_q area_server_status
        if (next_event_type == 1){
            arrive();
            break;
        }
        else if (next_event_type == 2){
            depart();
            break;
        }
    }
    
    report();
    
    fclose(infile);
    fclose(outfile);
    
    return 0;
}

void initialize(void){
    sim_time = 0.0;

    next_event_type = 0;
    num_cus_delayed = 0;
    num_in_q = 0;
    server_status = IDLE;
    area_num_in_q = 0.0;
    area_server_status = 0.0;
    
    next_arrival_time = sim_time + exp(mean_interval);
    next_departure_time = 1.0e+30;
}

void timing(void){
    float min_time_next_event;
    next_event_type = 0;
    
    //compare and choose the smallest time to occur
    if(next_arrival_time<=next_departure_time){
        min_time_next_event = next_arrival_time;
        next_event_type = 1;
    }
    else{
        min_time_next_event = next_departure_time;
        next_event_type = 2;
    }
    

    if(next_event_type == 0){
        fprintf(outfile,"\nEvent list is empty at time %f",sim_time);
        exit(1);
    }
    
    sim_time = min_time_next_event;
}

void exp(float mean){
    return -mean * log(lcgrand(1));
}

void update_time_avg_interval(void){

}