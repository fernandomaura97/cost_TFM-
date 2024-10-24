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

// Modified arrays to support 2 stations
double x_AP[1];
double y_AP[1];
double z_AP[1];

double x_[2];  
double y_[2];  
double z_[2];  
double RSSI[2];

struct input_arg_t {
    int seed;
    double STime;
    double BGLoad;
} st_input_args;

bool traces_on = true; 


#define DONWLINK_TRAFFIC    1 
#define UPLINK_TRAFFIC      0

component SimplifiedWiFiSim : public CostSimEng {
    public:
        void Setup(double BGLoad, int LBG, input_arg_t st, double distance);
        void Start();
        void Stop();

    public:
        AccessPoint[] AP;
        Station[] STA;  // Will hold 2 stations now
        CSMACAChannel1 channel1;
        TrafficGeneratorApp[] TGApp;  // Will hold 2 traffic generators
        Network Net;
        Sink sink;

        double BGLoad_ = 0;
        double distance_X = 0; 
};

void SimplifiedWiFiSim::Setup(double BGLoad, int LBG, input_arg_t st, double distance) {
    BGLoad_ = BGLoad;
    distance_X = distance; 

    printf("---- Simplified Wi-Fi sim : Setup ----\n");
    
    // Modified for two traffic sources
    TGApp.SetSize(2);
    for(int i = 0; i < 2; i++) {
        TGApp[i].Load = BGLoad;
        TGApp[i].L_data = LBG;
        TGApp[i].id = i;
        TGApp[i].node_attached = i;
        TGApp[i].destination = i;  // Both go to AP
        TGApp[i].mode = 0;
        TGApp[i].source_app = i;
        TGApp[i].destination_app = i;
    }

    // Single AP setup
    AP.SetSize(1);
    AP[0].id = 0;
    AP[0].x = 0;
    AP[0].y = 0;
    AP[0].z = 2;
    AP[0].NumberStations = 2;  // Changed to 2 stations
    AP[0].Pt = 20;
    AP[0].qmin = 1;
    AP[0].QL = 10000;
    AP[0].MAX_AMPDU = 64; // same as in MG1 sim
    AP[0].CWmin = 15;
    AP[0].max_BEB_stages = 6;
    AP[0].pe = 0;
    AP[0].channel_width = 80;
    AP[0].SU_spatial_streams = 2;
    AP[0].out_to_wireless.SetSize(2);  // Changed to 2 for two stations
    
    x_AP[0] = AP[0].x;
    y_AP[0] = AP[0].y;
    z_AP[0] = AP[0].z;

    // Modified for two stations
    STA.SetSize(2);

    for(int i = 0; i < 2; i++) {
        STA[i].id = i;
        if (i == 0) {
            STA[i].x = 1;  // STA0 at 1 meter
        }
        else{
            STA[i].x = distance_X;  // STA 1 at distance of input arg. 

        }
        STA[i].y = 0;
        STA[i].z = 2;
        STA[i].NumberStations = 2;  // Total number of stations
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
    Net.out_to_apps.SetSize(2);  // Changed to 2
    Net.out_to_APs.SetSize(1);

    // Channel setup
    channel1.NumNodes = 3;  // AP + 2 STAs
    channel1.out_slot.SetSize(3);  // Changed to 3

    // Connections

    // Network to AP
    connect Net.out_to_APs[0], AP[0].in_from_network;
    connect AP[0].out_to_network, Net.in_from_APs;


    // Channel connections
    connect AP[0].out_packet, channel1.in_frame;
    connect channel1.out_slot[0], AP[0].in_slot;

       
    for(int i = 0; i < 2; i++) { // connect STAs to the shared channel
        connect STA[i].out_packet, channel1.in_frame;
        connect channel1.out_slot[i+1], STA[i].in_slot;
    }

    // AP to Stations
    for(int i = 0; i < 2; i++) {
        connect AP[0].out_to_wireless[i], STA[i].in_from_wireless;
        connect STA[i].out_to_wireless[0], AP[0].in_from_wireless;
    }

    #if DOWNLINK_TRAFFIC // AP transmits to both STAs
        // Apps to Network
        for(int i = 0; i < 2; i++) {
            connect TGApp[i].out, Net.in_from_apps;
            connect Net.out_to_apps[i], TGApp[i].in;

            // app to sink
            connect STA[i].out_to_app, sink.in;
        }

    #elif UPLINK_TRAFFIC // both STAs transmit to AP

        for(int i = 0; i<2; i++ ){ // TGApp to STA
            connect TGApp[i].out, STA[i].in_from_app; // Now STAs are the ones transmitting 
            connect STA[i].out_to_app, TGApp[i].in;
            
            // app in AP/nwk, to sink
            connect Net.out_to_apps[i], sink.in;  // also connect the network side of AP corresponding to each STA to the sink 
        }

    #endif



    printf("----- Simplified Wi-FiSim Setup completed -----\n");
}

void SimplifiedWiFiSim::Start() {
    printf("Start\n");
}

void SimplifiedWiFiSim::Stop() {
    printf("########################################################################\n");
    printf("------------------------ Simplified Wi-Fisim Results ----------------------------\n");
    
    // Modified to show results for both stations
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
    // Modified to log results for both stations
    for(int i = 0; i < 2; i++) {
        fprintf(results, "%f %f %f %f %f %f %f %d\n",
                BGLoad_,
                AP[0].queueing_service_delay/AP[0].successful,
                AP[0].avAMPDU_size/AP[0].successful,
                AP[0].transmission_attempts/AP[0].slots,
                AP[0].collisions/AP[0].transmission_attempts,
                AP[0].queue_occupation/AP[0].arrived,
                RSSI[i],
                i);  // Added station ID to results
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

    printf("---- Simplified WiFiSim ----\n");
    printf("Seed = %d | SimTime = %f\n", seed, STime);
    printf("Input Parameters: BGLoad = %f | LBG = %d\n", BGLoad, LBG);

    SimplifiedWiFiSim sim;
    sim.Seed = seed;
    sim.StopTime(STime);
    sim.Setup(BGLoad, LBG, st_input_args, distance_X);

    printf("Run\n");
    sim.Run();

    return 0;
}