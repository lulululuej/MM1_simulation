#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "lcgrand.h"

#define BUSY 1
#define IDLE 0
#define Q_LIMIT 1000000
int next_event_type, num_events, server_status, num_in_q;
int num_delays_required, num_cus_delayed;
long double mean_inter_arr_time, mean_service_time, area_num_in_q, area_server_status, sim_time, time_arrival[Q_LIMIT + 1], time_last_event, next_arrival_time, next_departure_time, total_of_delays;
FILE *infile, *outfile;

void initialize(void);
void timing(void);
void update_time_avg_stats(void);
void arrival(void);
void departure(void);
void report(void);
double exponential(double mean);

int main() {
    infile = fopen("mm1.in", "r");
    outfile = fopen("mm1.out", "w");
    num_events = 2; // arrive 1 and depart 2
    
    fscanf(infile, "%d %Lf %Lf", &num_delays_required, &mean_inter_arr_time, &mean_service_time);
    printf("num of delay required = %d\n", num_delays_required);
    printf("mean interval = %Lf\n", mean_inter_arr_time);
    printf("mean service = %Lf\n", mean_service_time);
    initialize();
    
    while(num_cus_delayed < num_delays_required){
        //printf("simulation time = %f\n", sim_time);
        //printf("num cus delayed = %d\n", num_cus_delayed);
        timing(); // Determine the next event
        update_time_avg_stats(); // Update time-average area_num_in_q area_server_status
        switch (next_event_type)
        {
        case 1:
            arrival();
            break;
        
        case 2:
            departure();
            break;
        }
    }
    
    report();
    
    fclose(infile);
    fclose(outfile);
    
    printf("Simulation Done !");
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
    
    next_arrival_time = sim_time + exponential(mean_inter_arr_time);
    next_departure_time = 1.0e+30;
}

void timing(void){
    double min_time_next_event;
    next_event_type = 0;
    //printf("next arrival time = %f next departure time = %f\n",next_arrival_time,next_departure_time);
    //compare and choose the smallest time to occur
    if(next_arrival_time<=next_departure_time){
        min_time_next_event = next_arrival_time;
        next_event_type = 1;
    }
    else{
        min_time_next_event = next_departure_time;
        next_event_type = 2;
    }
    //printf("next event type = %d", next_event_type);

    if(next_event_type == 0){
        fprintf(outfile,"\nEvent list is empty at time %Lf",sim_time);
        exit(1);
    }
    
    sim_time = min_time_next_event;
    //printf("sim time = %f\n" ,sim_time);
}

double exponential(double mean){
    return -mean * log(lcgrand(1));
}

void update_time_avg_stats(void){
    double time_since_last_event;
    time_since_last_event = sim_time - time_last_event;
    //printf("simulation time = %lf\n", sim_time);
    //printf("time last event = %lf\n", time_last_event);
    //printf("time since last event = %lf\n", time_since_last_event);
    time_last_event = sim_time;
    
    area_num_in_q += num_in_q * time_since_last_event;
    area_server_status += server_status * time_since_last_event;
}

void arrival(void){
    double delay;
    //printf("server status = %d\n", server_status);
    
    next_arrival_time = sim_time + exponential(mean_inter_arr_time);
    if(server_status == BUSY){
        ++num_in_q;
        if (num_in_q > Q_LIMIT){
            fprintf(outfile, "\nOverflow of the array time_arrival at");
            fprintf(outfile, " time %Lf", sim_time);
            exit(2);
        }
        time_arrival[num_in_q] = sim_time;
    }
    else{
        delay = 0;
        total_of_delays += delay;
        ++num_cus_delayed;
        server_status = BUSY;
        next_departure_time = sim_time + exponential(mean_service_time);
    }
    //printf("num cus deleay = %d\n", num_cus_delayed);
}

void departure(void){
    //++num_cus_delayed;
    double delay;
    if(num_in_q == 0){
        server_status = IDLE;
        next_departure_time = 1.0e+30;
    }
    else{
        --num_in_q;
        delay = sim_time - time_arrival[1];
        total_of_delays += delay;
        ++num_cus_delayed;
        next_departure_time = sim_time + exponential(mean_service_time);

        for(int i = 1; i<=num_in_q; i++){
            time_arrival[i] = time_arrival[i+1];
        }
    }
}

void report(void){
    long double mean_inter_arr_rate, mean_service_rate, theo_avg_delay, theo_avg_num_in_q, theo_server_util;
    mean_inter_arr_rate = pow(mean_inter_arr_time,-1);
    mean_service_rate = pow(mean_service_time,-1);
    theo_avg_delay = mean_inter_arr_rate/mean_service_rate/(mean_service_rate-mean_inter_arr_rate);
    theo_avg_num_in_q = pow(mean_inter_arr_rate,2)/(mean_service_rate*(mean_service_rate-mean_inter_arr_rate));
    theo_server_util = mean_inter_arr_rate/mean_service_rate;
    fprintf(outfile, "Mean interarrival time%11.3Lf minutes\n", mean_inter_arr_time);
    fprintf(outfile, "Mean service time%16.3Lf minutes\n", mean_service_time);
    fprintf(outfile, "Number of customers%16.3d\n", num_delays_required);
    fprintf(outfile, "-----------------------------------------\n");
    fprintf(outfile, "[Theo] Avg delay in queue%8.3Lf minutes\n",theo_avg_delay);
    fprintf(outfile, "[Expe] Avg delay in queue%8.3Lf minutes\n\n",total_of_delays/num_cus_delayed);
    fprintf(outfile, "[Theo] Avg number in queue%7.3Lf \n",theo_avg_num_in_q);
    fprintf(outfile, "[Expe] Avg number in queue%7.3Lf \n\n",area_num_in_q/sim_time);
    fprintf(outfile, "[Theo] Server utilization%8.3Lf \n",theo_server_util);
    fprintf(outfile, "[Expe] Server utilization%8.3Lf \n",area_server_status/sim_time);
    fprintf(outfile, "-----------------------------------------\n");
    fprintf(outfile, "Time simulation ended %12.3Lf minutes", sim_time);
}