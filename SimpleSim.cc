#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <iostream>
#include "./COST/cost.h"
#include <deque>

#include "./Models/definitions.h"
#include "./Models/Network.h"
#include "./Models/AccessPoint.h"
#include "./Models/Station.h"
#include "./Models/CSMACAChannel1.h"
#include "./Models/Sink.h"
#include "./Models/TrafficGeneratorApp.h"

double x_AP[1];
double y_AP[1];
double z_AP[1];

double x_[2];
double y_[2];
double z_[2];

int traces_on = 1;
double RSSI[2];

component SimpleSim : public CostSimEng {
    public:
        void Setup(double STime, int seed, double BW_STA);
        void Start();        
        void Stop();
    
    public:
        AccessPoint [] ap;
        Station [] sta;
        CSMACAChannel1 channel;
        Sink sink;
        Network net;
        TrafficGeneratorApp [] TGApp;  // Added traffic generators
};

void SimpleSim::Setup(double STime, int seed, double BW_STA ) {
    printf("---- Simplified Wi-Fi sim : Setup ----\n");
    
    ap.SetSize(1);
    sta.SetSize(2);
    TGApp.SetSize(4);  // 2 for uplink, 2 for downlink

    // Configure AP
    ap[0].id = 0;
    ap[0].x = 0;
    ap[0].y = 0;
    ap[0].z = 2;
    ap[0].NumberStations = 2;
    ap[0].Pt = 20;
    ap[0].qmin = 1;
    ap[0].QL = 10000;
    ap[0].MAX_AMPDU = 64;
    ap[0].CWmin = 15;
    ap[0].max_BEB_stages = 6;
    ap[0].pe = 0;
    ap[0].channel_width = 80;
    ap[0].SU_spatial_streams = 2;
    ap[0].out_to_wireless.SetSize(2);

    x_AP[0] = ap[0].x;
    y_AP[0] = ap[0].y;
    z_AP[0] = ap[0].z;

    // Configure Traffic Generators
    // Downlink traffic (AP to STAs)
    for(int n = 0; n < 2; n++) {
        TGApp[n].Load = BW_STA;  // 1 Mbps load
        TGApp[n].L_data = 12000; 
        TGApp[n].id = n;
        TGApp[n].node_attached = 0;  // attached to AP
        TGApp[n].destination = n;    // to respective STA
        TGApp[n].mode = 0;
        TGApp[n].source_app = n;
        TGApp[n].destination_app = n;
    }

    // Uplink traffic (STAs to AP)
    for(int n = 0; n < 2; n++) {
        TGApp[n+2].Load = BW_STA;  // 1 Mbps load
        TGApp[n+2].L_data = 12000;
        TGApp[n+2].id = n+2;
        TGApp[n+2].node_attached = n;  // attached to respective STA
        TGApp[n+2].destination = 0;    // to AP
        TGApp[n+2].mode = 0;
        TGApp[n+2].source_app = n;
        TGApp[n+2].destination_app = n;
    }

    // Configure Stations
    double STA_X[2] = {1.0, 30.0};
    
    for(int n = 0; n < 2; n++) {
        sta[n].id = n;
        sta[n].x = STA_X[n];
        sta[n].y = 0;
        sta[n].z = 2;
        sta[n].NumberStations = 1;
        sta[n].Pt = 20;
        sta[n].qmin = 1;
        sta[n].QL = 150;
        sta[n].MAX_AMPDU = 64;
        sta[n].CWmin = 15;
        sta[n].max_BEB_stages = 6;
        sta[n].pe = 0;
        sta[n].channel_width = 80;
        sta[n].SU_spatial_streams = 2;
        sta[n].out_to_wireless.SetSize(1);

        x_[n] = sta[n].x;
        y_[n] = sta[n].y;
        z_[n] = sta[n].z;
    }

    // Configure Network
    net.Rate = 1000E6;
    net.out_to_APs.SetSize(1);
    net.out_to_apps.SetSize(2);  // Added for traffic generators

    // Configure Channel
    channel.NumNodes = 3;  // 1 AP + 2 STAs
    channel.out_slot.SetSize(3);

    // Connect Traffic Generators to Network
    for(int n = 0; n < 2; n++) {
        connect TGApp[n].out, net.in_from_apps;
        connect net.out_to_apps[n], TGApp[n].in;
    }

    // Connect Network to AP
    connect net.out_to_APs[0], ap[0].in_from_network;
    connect ap[0].out_to_network, net.in_from_APs;

    // Connect AP to STAs
    for(int n = 0; n < 2; n++) {
        connect ap[0].out_to_wireless[n], sta[n].in_from_wireless;
        connect sta[n].out_to_wireless[0], ap[0].in_from_wireless;
    }

    // Connect STAs to Traffic Generators (uplink)
    for(int n = 0; n < 2; n++) {
        connect sta[n].out_to_app, TGApp[n+2].in;
        connect TGApp[n+2].out, sta[n].in_from_app;
    }

    // Connect to Channel
    connect ap[0].out_packet, channel.in_frame;
    connect channel.out_slot[0], ap[0].in_slot;

    for(int n = 0; n < 2; n++) {
        connect sta[n].out_packet, channel.in_frame;
        connect channel.out_slot[n+1], sta[n].in_slot;
    }

    // Connect AP to Sink
    connect ap[0].out_to_network, sink.in;

    printf("----- Simplified Wi-Fi Sim Setup completed -----\n");
}

void SimpleSim::Start() {
    printf("Start\n");
}

void SimpleSim::Stop() {
    printf("########################################################################\n");
    printf("------------------------ Simplified Wi-Fi Results -----------------------\n");
    printf("AP: RSSI = %f | Packet AP Delay = %f\n", RSSI[0], ap[0].queueing_service_delay/ap[0].successful);
    printf("Av A-MPDU size = %f | Tx prob = %f | Coll prob = %f\n", 
           ap[0].avAMPDU_size/ap[0].successful,
           ap[0].transmission_attempts/ap[0].slots,
           ap[0].collisions/ap[0].transmission_attempts);
    printf("########################################################################\n");

    // Save results to file
    FILE *results;
    results = fopen("Results/SimpleSim.txt", "at");
    fprintf(results, "%f %f %f %f %f\n",
            ap[0].queueing_service_delay/ap[0].successful,
            ap[0].avAMPDU_size/ap[0].successful,
            ap[0].transmission_attempts/ap[0].slots,
            ap[0].collisions/ap[0].transmission_attempts,
            RSSI[0]);
    fclose(results);
}

int main(int argc, char *argv[]) {
    if(argc < 3) {
        printf("Usage: %s <seed> <simulation_time>\n", argv[0]);
        return 1;
    }

    int seed = atoi(argv[1]);
    double STime = atof(argv[2]);
    double bw_sta = atof(argv[3]); 

    printf("---- Simplified WiFi Simulation ----\n");
    printf("Seed = %d | SimTime = %f\n", seed, STime);
    printf("Station distances: 1m and 30m\n");

    SimpleSim az;
    az.Seed = seed;
    az.StopTime(STime);
    az.Setup(STime, seed, bw_sta);

    printf("Run\n");
    az.Run();

    return 0;
}