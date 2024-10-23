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

double x_[1];
double y_[1];
double z_[1];

double RSSI[1];

struct input_arg_t {
    int seed;
    double STime;
    double BGLoad;
} st_input_args;

bool traces_on = true; 

component SimplifiedWiFiSim : public CostSimEng {
    public:
        void Setup(double BGLoad, int LBG, input_arg_t st);
        void Start();
        void Stop();

    public:
        AccessPoint[] AP;
        Station[] STA;
        CSMACAChannel1 channel1;
        TrafficGeneratorApp[] TGApp;
        Network Net;
        Sink sink;

        // Input parameters
        double BGLoad_ = 0;
};

void SimplifiedWiFiSim::Setup(double BGLoad, int LBG, input_arg_t st) {
    BGLoad_ = BGLoad;
    printf("---- Simplified Wi-Fi sim : Setup ----\n");

    
    // Single background traffic source
    TGApp.SetSize(1);
    TGApp[0].Load = BGLoad;
    TGApp[0].L_data = LBG;
    TGApp[0].id = 0;
    TGApp[0].node_attached = 0;
    TGApp[0].destination = 0;  // To AP
    TGApp[0].mode = 0;
    TGApp[0].source_app = 0;
    TGApp[0].destination_app = 0;

    // Single AP setup
    AP.SetSize(1);
    AP[0].id = 0;
    AP[0].x = 0;
    AP[0].y = 0;
    AP[0].z = 2;
    AP[0].NumberStations = 1;
    AP[0].Pt = 20;  // dBm
    AP[0].qmin = 1;
    AP[0].QL = 10000;
    AP[0].MAX_AMPDU = 128;
    AP[0].CWmin = 15;
    AP[0].max_BEB_stages = 6;
    AP[0].pe = 0;
    AP[0].channel_width = 80;  // MHz
    AP[0].SU_spatial_streams = 2;
    AP[0].out_to_wireless.SetSize(1);
    
    x_AP[0] = AP[0].x;
    y_AP[0] = AP[0].y;
    z_AP[0] = AP[0].z;

    // Single Station setup
    STA.SetSize(1);
    STA[0].id = 0;
    STA[0].x = 30;  // 30m distance from AP
    STA[0].y = 0;
    STA[0].z = 2;
    STA[0].NumberStations = 1;
    STA[0].Pt = 20;  // dBm
    STA[0].qmin = 1;
    STA[0].QL = 150;
    STA[0].MAX_AMPDU = 64;
    STA[0].CWmin = 15;
    STA[0].max_BEB_stages = 6;
    STA[0].pe = 0;
    STA[0].channel_width = 80;  // MHz
    STA[0].SU_spatial_streams = 2;
    STA[0].out_to_wireless.SetSize(1);

    x_[0] = STA[0].x;
    y_[0] = STA[0].y;
    z_[0] = STA[0].z;

    // Network setup
    Net.Rate = 1000E6;
    Net.out_to_apps.SetSize(1);
    Net.out_to_APs.SetSize(1);

    // Channel setup
    channel1.NumNodes = 2;  // AP + 1 STA
    channel1.out_slot.SetSize(2);

    // Connections
    
    // App to Network
    connect TGApp[0].out, Net.in_from_apps;
    connect Net.out_to_apps[0], TGApp[0].in;

    // Network to AP
    connect Net.out_to_APs[0], AP[0].in_from_network;
    connect AP[0].out_to_network, Net.in_from_APs;

    // AP to Station
    connect AP[0].out_to_wireless[0], STA[0].in_from_wireless;
    connect STA[0].out_to_wireless[0], AP[0].in_from_wireless;

    // Station to App
    connect STA[0].out_to_app, sink.in;

    // Channel connections
    connect AP[0].out_packet, channel1.in_frame;
    connect channel1.out_slot[0], AP[0].in_slot;
    connect STA[0].out_packet, channel1.in_frame;
    connect channel1.out_slot[1], STA[0].in_slot;

    printf("----- Simplified Wi-FiSim Setup completed -----\n");
}

void SimplifiedWiFiSim::Start() {
    printf("Start\n");
}

void SimplifiedWiFiSim::Stop() {
    printf("########################################################################\n");
    printf("------------------------ Simplified Wi-Fisim Results ----------------------------\n");
    printf("AP: RSSI = %f | Packet AP Delay = %f\n", RSSI[0], AP[0].queueing_service_delay/AP[0].successful);
    printf("Av A-MPDU size = %f | Tx prob = %f | Coll prob = %f | Buffer size = %f\n",
           AP[0].avAMPDU_size/AP[0].successful,
           AP[0].transmission_attempts/AP[0].slots,
           AP[0].collisions/AP[0].transmission_attempts,
           AP[0].queue_occupation/AP[0].arrived);

    FILE *results;
    results = fopen("Results/SimplifiedWiFiSim.txt", "at");
    fprintf(results, "%f %f %f %f %f %f %f\n",
            BGLoad_,
            AP[0].queueing_service_delay/AP[0].successful,
            AP[0].avAMPDU_size/AP[0].successful,
            AP[0].transmission_attempts/AP[0].slots,
            AP[0].collisions/AP[0].transmission_attempts,
            AP[0].queue_occupation/AP[0].arrived,
            RSSI[0]);
    fclose(results);
}

int main(int argc, char *argv[]) {
    int seed = atoi(argv[1]);
    double STime = atof(argv[2]);
    double BGLoad = atof(argv[3]);
    int LBG = atoi(argv[4]);

    st_input_args.seed = seed;
    st_input_args.STime = STime;
    st_input_args.BGLoad = BGLoad;

    printf("---- Simplified WiFiSim ----\n");
    printf("Seed = %d | SimTime = %f\n", seed, STime);
    printf("Input Parameters: BGLoad = %f | LBG = %d\n", BGLoad, LBG);

    SimplifiedWiFiSim sim;
    sim.Seed = seed;
    sim.StopTime(STime);
    sim.Setup(BGLoad, LBG, st_input_args);

    printf("Run\n");
    sim.Run();

    return 0;
}