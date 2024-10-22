#ifndef _DEFINITIONS_
#define _DEFINITIONS_


#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))


//#define CWmin 7
//#define max_BEB_stages 6
#define SLOT 9E-6 // microseconds

#define Legacy_PHY_duration 20E-6
#define PHY_duration 100E-6
#define DIFS 31E-6
#define SIFS 16E-6

// // Define macros for colors
// #define RED     "\033[31m"
// #define GREEN   "\033[32m"
// #define YELLOW  "\033[33m"
// #define BLUE    "\033[34m"
// #define MAGENTA "\033[35m"
// #define CYAN    "\033[36m"

#define RESET   "\033[0m"
#define BLUE    "\033[34m"    // Blue
#define CYAN    "\033[36m"    // Cyan (between blue and green)
#define LIGHT_MAGENTA "\033[95m" // Light Magenta (purple hue, mix of red and blue)
#define MAGENTA "\033[35m"    // Magenta (closer to red)
#define RED     "\033[31m"    // Red




#define DEBUG_PRINTS 1 // to set up fancy output packet per packet
#if DEBUG_PRINTS
    #define PRINTF_COLOR(color, format, ...) printf(color format RESET, ##__VA_ARGS__)
#else
    #define PRINTF_COLOR(color, format, ...) (void)0 // do nothing
#endif


struct data_packet
{
	double L_data; // Packet (data) length
	double L_header; // Header Length
	double L; // Total length

	double T_q; //time in queue, new

	int AMPDU_size; // Number of aggregated packets

	// Statistics
	double sent_time; // Time at which the packet is generated from the source
	double scheduled_time; // Time at which the packet is selected for transmission	
	double queueing_service_delay;


	double in_queue_time; 

	double ID_packet; 
	double num_seq;	
	int destination; // id of the destination station
	int source; // id of the traffic source

	int source_app;
	int destination_app;
	
	// To be used by the channel
	double T; // Transmission time, include DIFS, SIFS, etc.
	double T_c; //

	// For the XR server and client communication
	int first_video_frame_packet;
	int last_video_frame_packet;
	int num_packet_in_the_frame;
	double frame_generation_time;
	int NumPacketsPerFrame;
	double TimeSentAtTheServer;
	double TimeReceivedAtTheClient;
	double video_frame_seq;
	double frames_received;



	bool feedback;
	bool rtt;

	double m_owdg;
	double threshold_gamma; 


	bool sliding_rx_frame_loss; 
	double frame_numseq; 
	
	struct Kalman_t{
				double OW_Delay;
				double K_gain;
				double m_current; 
				double m_prev; 
				double residual_z;
			}Kalman_p; 

	
	double packets_received;
}; 

struct sliding_window_t {

			data_packet Packet;
			double      Timestamp; 
			double 		RTT; 
			double 		num_seq;
			// double 		num_seq_pl; //to use some day  
		}; 

struct SLOT_indicator
{
	int status;
};

struct info
{
	int x;
	int y;
	int z;
	int ap_id;
	int station_id;
};


#endif
