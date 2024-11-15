#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <iostream>

#include "./COST/cost.h"
#include <deque>
#include "./Models/definitions.h"
#include "./Models/Network.h"
#include "./Models/TrafficGeneratorApp.h"
#include "./Models/AccessPoint.h"
#include "./Models/Station.h"
#include "./Models/CSMACAChannel1.h"
#include "./Models/Sink.h"

double x_AP[1];
double y_AP[1];
double z_AP[1];

double x_[2];  
double y_[2];  
double z_[2];  
double RSSI[2];

int NBG = 2; 
int NXR = 0; 

struct input_arg_t {
    int seed;
    double STime;
    double BGLoad;
} st_input_args;

#define DL_TRAFFIC_BOOL 0
#define UL_TRAFFIC_BOOL 1

bool traces_on = true; 

component SimplifiedWiFiSim : public CostSimEng {
    public:
        void Setup(double BGLoad_UL, double BGLoad_DL, int LBG, input_arg_t st, double distance);
        void Start();
        void Stop();

    public:
        AccessPoint[] AP;
        Station[] STA;  
        CSMACAChannel1 channel1;
        TrafficGeneratorApp[] TGApp;  
        Network Net;
        Sink sink;

        double BGLoad_ = 0;
        double distance_X = 0; 
};

void SimplifiedWiFiSim::Setup(double BGLoad_UL, double BGLoad_DL, int LBG, input_arg_t st, double distance) {
    BGLoad_ = BGLoad_DL;
    distance_X = distance; 

    printf("---- Simplified Wi-Fi sim : Setup ----\n");

    #define CONST_UL_ID 10
    #define CONST_DL_ID 30
    TGApp.SetSize(2*NBG); 
    for(int i = 0; i < NBG; i++) {
        TGApp[i].Load = BGLoad_DL;
        TGApp[i].L_data = LBG;
        TGApp[i].id = i + CONST_DL_ID;  
        TGApp[i].node_attached = i;
        TGApp[i].destination = i;  
        TGApp[i].mode = 0;
        TGApp[i].source_app = i;
        TGApp[i].destination_app = i;
        TGApp[i].does_transmit = DL_TRAFFIC_BOOL; 
    }

     for(int i = 0; i < NBG; i++) {
        TGApp[NBG + i].Load = BGLoad_UL;
        TGApp[NBG + i].L_data = LBG;
        TGApp[NBG + i].id = i + CONST_UL_ID;  
        TGApp[NBG + i].node_attached = i + NBG;
        TGApp[NBG + i].destination = 0;   
        TGApp[NBG + i].mode = 0;
        TGApp[NBG + i].source_app = i + NBG;
        TGApp[NBG + i].destination_app = i + NBG;
        TGApp[NBG + i].does_transmit = UL_TRAFFIC_BOOL; 
    }

    // Single AP setup
    AP.SetSize(1);
    AP[0].id = 0;
    AP[0].x = 0;
    AP[0].y = 0;
    AP[0].z = 2;
    AP[0].NumberStations = 2;  
    AP[0].Pt = 20;
    AP[0].qmin = 1;
    AP[0].QL = 10000;
    AP[0].MAX_AMPDU = 64; 
    AP[0].CWmin = 15;
    AP[0].max_BEB_stages = 6;
    AP[0].pe = 0;
    AP[0].channel_width = 80;
    AP[0].SU_spatial_streams = 2;
    AP[0].out_to_wireless.SetSize(2);  

    x_AP[0] = AP[0].x;
    y_AP[0] = AP[0].y;
    z_AP[0] = AP[0].z;

    STA.SetSize(NBG);

    for(int i = 0; i < NBG; i++) {
        STA[i].id = i;
        if (i == 0) {
            STA[i].x = 1;  
        }
        else{
            STA[i].x = distance_X;  
        }
        STA[i].y = 0;
        STA[i].z = 2;
        STA[i].NumberStations = 2;  
        STA[i].Pt = 20;
        STA[i].qmin = 1;
        STA[i].QL = 150;
        STA[i].MAX_AMPDU = 64;
        STA[i].CWmin = 15;
        STA[i].max_BEB_stages = 6;
        STA[i].pe = 0;
        STA[i].channel_width = 80;
        STA[i].SU_spatial_streams = 2;
        STA[i].out_to_wireless.SetSize(1);

        x_[i] = STA[i].x;
        y_[i] = STA[i].y;
        z_[i] = STA[i].z;
    }

    // Network setup
    Net.Rate = 1000E6;
    Net.out_to_apps.SetSize(2*NBG);  
    Net.out_to_APs.SetSize(1);

    // Channel setup
    channel1.NumNodes = 3;  
    channel1.out_slot.SetSize(3);  

    // Connections    
    for(int n=0;n<2*NBG;n++)
    {
        connect TGApp[n].out,Net.in_from_apps;
        connect Net.out_to_apps[n],TGApp[n].in;
    }

    connect Net.out_to_APs[0], AP[0].in_from_network;
    connect AP[0].out_to_network, Net.in_from_APs;

    for(int i = 0; i < 2; i++) {
        connect AP[0].out_to_wireless[i], STA[i].in_from_wireless;
        connect STA[i].out_to_wireless[0], AP[0].in_from_wireless;
    }

    for (int n = 0; n<NBG; n++){
        connect STA[n].out_to_app, TGApp[NBG + n].in;  
        connect TGApp[NBG + n].out, STA[n].in_from_app; 
    }

    connect AP[0].out_packet, channel1.in_frame;
    connect channel1.out_slot[0], AP[0].in_slot;

    for(int i = 0; i < NBG; i++) {
        connect STA[i].out_packet, channel1.in_frame;
        connect channel1.out_slot[i+1], STA[i].in_slot;
    }

    printf("----- Simplified Wi-FiSim Setup completed -----\n");
}

void SimplifiedWiFiSim::Start() {
    printf("Start\n");
}

void SimplifiedWiFiSim::Stop() {
    printf("########################################################################\n");
    printf("------------------------ Simplified Wi-Fisim Results ----------------------------\n");
    
    for(int i = 0; i < 2; i++) {
        printf("STA%d: RSSI = %f | Packet AP Delay = %f\n", 
               i, RSSI[i], AP[0].queueing_service_delay/AP[0].successful);
    }
    
    printf("AP Stats:\n");
    printf("Av A-MPDU size = %f | Tx prob = %f | Coll prob = %f | Buffer size = %f\n",
           AP[0].avAMPDU_size/AP[0].successful,
           AP[0].transmission_attempts/AP[0].slots,
           AP[0].collisions/AP[0].transmission_attempts,
           AP[0].queue_occupation/AP[0].arrived);

    FILE *results;
    results = fopen("Results/SimplifiedWiFiSim.txt", "at");
    for(int i = 0; i < 2; i++) {
        fprintf(results, "%f %f %f %f %f %f %f %d\n",
                BGLoad_,
                AP[0].queueing_service_delay/AP[0].successful,
                AP[0].avAMPDU_size/AP[0].successful,
                AP[0].transmission_attempts/AP[0].slots,
                AP[0].collisions/AP[0].transmission_attempts,
                AP[0].queue_occupation/AP[0].arrived,
                RSSI[i],
                i);  
    }
    fclose(results);
}

int main(int argc, char *argv[]) {
    int seed = atoi(argv[1]);
    double STime = atof(argv[2]);
    double BGLoad = atof(argv[3]);
    int LBG = atoi(argv[4]);
    double distance_X = atof(argv[5]); 

    st_input_args.seed = seed;
    st_input_args.STime = STime;
    st_input_args.BGLoad = BGLoad;

    double BGLoad_UL = BGLoad; 
    double BGLoad_DL = BGLoad; 

    printf("---- Simplified WiFiSim ----\n");
    printf("Seed = %d | SimTime = %f\n", seed, STime);
    printf("Input Parameters: BGLoad = %f | LBG = %d\n", BGLoad, LBG);

    SimplifiedWiFiSim sim;
    sim.Seed = seed;
    sim.StopTime(STime);
    sim.Setup(BGLoad_UL, BGLoad_DL, LBG, st_input_args, distance_X);

    printf("Run\n");
    sim.Run();

    return 0;
}