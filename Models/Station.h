/*
	Station
*/

#ifndef _STATION_
#define _STATION_

#include "definitions.h"
//#include "FIFO.h"
#include "PHY_model.h"
#include <deque>

component Station : public TypeII
{
	public:
		void Setup();
		void Start();
		void Stop();		
		int BinaryExponentialBackoff(int attempt);	
		void FrameTransmissionDelay(double TotalBitsToBeTransmitted,int NMPDUs, int station_id);
		void update_stats_AMPDU(data_packet &ampdu_packet, int queue_size); 

	public: // Connections
		inport void in_from_app(data_packet &packet);
		outport [] void out_to_wireless(data_packet &packet);	

		inport void in_from_wireless(data_packet &packet);
		outport void out_to_app(data_packet &packet);	

		/*
		inport void in_info_ap(info &info_request);
		outport [] void out_info_ap(info &info_response);

		inport void in_request_ap(info &info_request);
		outport [] void out_response_ap(info &info_response);	
		*/

		// For the DCF
		inport void inline in_slot(SLOT_indicator &slot);
		outport void out_packet(data_packet &frame);

	public:
		int id;		
		double x,y,z;

		int MAX_AMPDU;
		int QL;
		int CWmin;
		int max_BEB_stages;
		double pe;
		double channel_width;
		int SU_spatial_streams;

		double Pt;
		double BitsSymbol[20];
		double CodingRate[20];


		//FIFO test_queue;
		FIFO MAC_queue;
		int qmin; // Minimum number of packets in the buffer to start a tx

		int current_ampdu_size; // Number of packets aggregated in current transmission
		int current_destination;
		int NumberStations;

		AMPDU_packet_t aux_ampdu; 

		struct csv_STA_t {
			std::vector <double> timestamp;
            std::vector <double> L_ampdu; 
            std::vector <int>    destination; 
            std::vector <int>    source; 
            std::vector <double> T_s; 
            std::vector <double> T_q;  
			std::vector <double> queue_size;
			std::vector <double> throughput;  
        }STAcsv; 

		const double MAX_T_AGG = 4.85E-3; // 4.85 ms limit for AMPDU


	private:
		int mode; // 0: idle; 1: in transmission
		int backoff_counter; // Current backoff counter
		int attempts; // Number of attempts per packet
		int device_has_transmitted;
		double T = 0;
		double T_c = 0;

	public: //statistics - results
		double transmission_attempts;
		double collisions;
		double arrived; // number of arrived packets
		double rho; // to compute the prob that the system is 'busy'. Not default meaning.
		double service_time;
		double queueing_service_delay;
		double aux_service_time;
		double slots;
		double EB; // average backoff value
		double blocking_prob;
		double av_MPDUsize; // Average number of Packets Transmitted per av_MPDUsize?
		double successful; // Number of successful transmissions
		double queue_occupation;	
};

void Station :: Setup()
{
	printf("Station Setup()\n");

};

void Station :: Start()
{
	printf("Station Start()\n");

	mode = 0;
	device_has_transmitted=0;
	backoff_counter=(int)Random(CWmin);
	attempts = 0;	
	//printf("%f - Node %d ready\n",SimTime(),id);

	current_ampdu_size=0; 
	collisions = 0;
	transmission_attempts = 0;
	arrived = 0;
	rho = 0;
	service_time = 0;
	aux_service_time = 0;
	slots=0;
	EB=0;
	blocking_prob=0;

	successful=0;
	av_MPDUsize=0;
	queue_occupation=0;

	aux_ampdu.reset(); 

};

void Station :: Stop()
{

//	if(id==1)
//	{
		printf("---------------- Results Station %d----------------\n",id);
		printf("Transmission Probability = %f (%f / %f)|| EB=%f\n",transmission_attempts/slots,transmission_attempts,slots,EB/transmission_attempts);
		printf("Collision Probability = %f (%f / %f)\n",collisions/transmission_attempts,collisions,transmission_attempts);
		printf("Queue Utilization = %f (%f / %f)\n",rho/arrived,rho,arrived);
		printf("Queue Occupation = %f\n",queue_occupation/arrived);
		printf("Service Time = %f | Queueing + Service Delay = %f\n",service_time/successful,queueing_service_delay/successful);
		printf("Load = %f (packets/s) - Througput = %f (packets/s)- Blocking Probability = %f\n",arrived/SimTime(),av_MPDUsize / SimTime(),blocking_prob/arrived);
		printf("Average AMPDU size = %f\n",av_MPDUsize/successful);
//	}

};


void Station :: in_from_app(data_packet &packet)
{
	//printf("%f - New packet (to %d) arrives to Station %d. Packets in the buffer = %d \n",SimTime(),packet.destination,id,(int)MAC_queue.size());

	arrived++;
	int QueueSize = MAC_queue.QueueSize();
	queue_occupation+=QueueSize;
	//printf("%f - New frame. Queue Size = %d \n",SimTime(),QueueSize);
	if(QueueSize >= qmin) 
	{
		rho++;
	}

	if(QueueSize < QL)
	{
		packet.queueing_service_delay = SimTime();
		MAC_queue.PutPacket(packet);
	}
	else
	{
		blocking_prob++;
	}

};


void Station :: in_slot(SLOT_indicator &slot)
{

	//printf("---- AP IN SLOT? Status = %d----\n",slot.status);

	slots++;

	// ######################################################################################################
	// Implications of the last slot: idle (0), successful (1) or collision (>1)
	
	if(slot.status == 0) // Idle
	{
		//printf("%f - Channel Empty - Mode = %d - BO = %d\n",SimTime(),mode,backoff_counter);
	}
	if(slot.status == 1) // Successful transmission
	{
		//printf("%f - Successful Transmission || Mode = %d | BO = %d\n",SimTime(),mode,backoff_counter);
		if(device_has_transmitted == 1)
		{
			if(traces_on) printf("%f - STA (%d) : Successful Transmission | AMPDU size = %d\n",SimTime(),id,current_ampdu_size);
			mode=0; // Move to not transmitting
			device_has_transmitted = 0; 
			av_MPDUsize+=current_ampdu_size;
			// We remove from the buffer the batch of successful received packets
			data_packet frame_test;
			double queueing_service_delay_aux=0; // to calculate the queueing service delay of each packet
			double mpdu_counter = 0; 

			
			for(auto& packet_iter : aux_ampdu.mpdu_packets)
			{
				mpdu_counter += 1; 
				if(Random() > pe){
					queueing_service_delay_aux += (SimTime() - packet_iter.queueing_service_delay - SLOT) ; 
				
					update_stats_AMPDU(packet_iter, MAC_queue.QueueSize() - mpdu_counter); 
					PRINTF_COLOR(RED , "%.6f [STA OUT W]      Packet %.0f from STA %d (%.0f/%d)\n",SimTime(), packet_iter.ID_packet ,packet_iter.destination, mpdu_counter, current_ampdu_size);
					out_to_wireless[packet_iter.destination](frame_test); 				
				}

				else{
					printf("%f - STA - Packet to %d with errors (packet ID = %.0f, PER = %.2f)\n",SimTime(),packet_iter.destination,packet_iter.ID_packet, pe );

				}

			}

			//printf("%f - STA - End TX : Queue Size = %li\n",SimTime(),MAC_queue.size());
			queueing_service_delay_aux = queueing_service_delay_aux / current_ampdu_size;
			queueing_service_delay += queueing_service_delay_aux;
			current_ampdu_size=0; // Quite irrelevant, since we are not transmitting yet
			attempts=0;
			service_time += (SimTime()-aux_service_time-SLOT);
			successful++;
			mode = 0;
		}
	}
	if(slot.status > 1) // Collision
	{
		//printf("%f - Collision || Mode = %d | BO = %d\n",SimTime(),mode,backoff_counter);
		if(device_has_transmitted == 1)
		{
			if(traces_on) printf("%f - STA (%d) : Collision | Attempts = %d !!!\n",SimTime(),id,attempts);
			device_has_transmitted = 0;
			//attempts++;			
			backoff_counter = BinaryExponentialBackoff(attempts);
			EB+=backoff_counter; // To calculate the average backoff value
			// mode is kept to 1
			//printf("%f- STA %d re-starts transmission mode with BO=%d \n",SimTime(),id,backoff_counter);
			collisions++; // stat
			
		}
		else
		{
			// Someone else has suffered a collision
		}

	}

	// ######################################################################################################

	if(mode == 0) // Not in transmission mode
	{
		int QueueSize = MAC_queue.QueueSize();
		//if((QueueSize >= qmin && continuation==0 && QueueSize >= smin)||(QueueSize >= smin && continuation==1))
		if(QueueSize >= qmin)
		{
			mode = 1; // I satisfy the conditions to switch to transmission mode
			attempts=0; // First attempt.
			backoff_counter = BinaryExponentialBackoff(attempts);
			EB+=backoff_counter;
			current_ampdu_size = 1; // To update with queue size < QMAX			
			aux_service_time=SimTime();
			//printf("%f- STA %d switchs to transmission mode with BO=%d, Current av_MPDUsize = %d (QS=%d) \n",SimTime(),id,backoff_counter,current_ampdu_size,QueueSize);
		}
	}

	if(mode == 1) // In transmission mode
	{
		//printf("Mode = 1 and backoff counter = %d\n",backoff_counter);		
		if(backoff_counter==0)
		{
			//printf("%f - @@@@@@@@@@@@@@@@@@@@@@@Station %d is going to start a transmission with buffer size %li-------------------------------------------------------------\n",SimTime(),id,MAC_queue.size());
			//printf("%f-Node %d backoff = 0\n",SimTime(),id);
			// Time to sent a frame

			aux_ampdu.reset(); 
			current_ampdu_size = MIN((int)MAC_queue.QueueSize(),MAX_AMPDU);

			// 1. Pick the first packet in the buffer. Identify the STA.
			data_packet first_packet_in_buffer = MAC_queue.GetFirstPacket();
			
			
			aux_ampdu.dest_ID = first_packet_in_buffer.destination; 	
			current_destination = first_packet_in_buffer.destination; 
			
			// 2. Select up to MAX_AMPDU packets to that STA.			
			int current_ampdu_size_per_station = 0;		
			double TotalBitsToBeTransmitted = 0;
			double queue_delay_per_packet = 0; 

			for(int q = 0 ; q < MAC_queue.QueueSize(); q++)
			{
				data_packet packet_to_check = MAC_queue.GetPacketAt(q); 
				if(current_destination == packet_to_check.destination && current_ampdu_size_per_station < MAX_AMPDU)
				{				
					FrameTransmissionDelay(TotalBitsToBeTransmitted,current_ampdu_size_per_station,current_destination);

					if(T >= MAX_T_AGG){ // making sure that adding an extra packet will not exceed hard limit
						break; 
					}

					queue_delay_per_packet += ( SimTime() - packet_to_check.in_queue_time) ; 
					packet_to_check.T_q = 		SimTime() - packet_to_check.in_queue_time; 

					MAC_queue.DeletePacketIn(q); 
					q -= 1; 

					aux_ampdu.mpdu_packets.push_back(packet_to_check); 
					aux_ampdu.total_length += packet_to_check.L; 
					aux_ampdu.size += 1; 


					//.PutPacketIn(packet_to_check,current_ampdu_size_per_station);		
					//printf("************** Removed from %d, and added to %d\n",q,current_ampdu_size_per_station);					
					TotalBitsToBeTransmitted+=packet_to_check.L;
					current_ampdu_size_per_station++;
				}
			}
			int current_ampdu_size_sta = MIN(current_ampdu_size_per_station,MAX_AMPDU);	


			FrameTransmissionDelay(TotalBitsToBeTransmitted,current_ampdu_size_sta,current_destination);
			data_packet frame;
			frame.AMPDU_size = current_ampdu_size_sta;
			frame.source=id;
			frame.T = T;
			frame.T_c = T_c;
			frame.T_q = queue_delay_per_packet; 

			for (auto& packet: aux_ampdu.mpdu_packets){
				packet.scheduled_time = SimTime() + T; 
			}
			//printf("%f-STA %d | Destination %d | W/O STA = %d | W STA = %d\n",SimTime(),id,current_destination,current_ampdu_size,current_ampdu_size_sta);

			/*
			// Check MCS for the station 
			info request;
			request.station_id = id;
			//printf("%f-AP %d requests info to the STATION %d\n",SimTime(),id,current_destination);
			out_info_ap[current_destination](request); 
			*/

			current_ampdu_size = current_ampdu_size_sta;
			// Calculate duration = f(MCS,ampdu_size, etc.)
			//-------------------------
			// Packets that will be transmitted
			
			PRINTF_COLOR(BG_CYAN, "%.6f [STA%d_TXOP%d] AMPDU_size = %d | Destination %d | T_s = %.3f ms | TotalBits = %.0f\n",SimTime(), id ,attempts, current_ampdu_size_sta, current_destination, T * 1000, TotalBitsToBeTransmitted);
			aux_ampdu.print(); 
			
			attempts++; 
 			device_has_transmitted=1;
			transmission_attempts++; // stat

			out_packet(frame); // To the channel!!!

		}
		else
		{
			backoff_counter--;

		}
	}

};


void Station :: in_from_wireless(data_packet &packet)
{
	//printf("*********************** %d - %d ********************************************\n",packet.destination,id);
	if(packet.destination == id) out_to_app(packet);	
}


int Station :: BinaryExponentialBackoff(int attempt)
{
	int CW = Random(MIN(pow(2,attempt),pow(2,max_BEB_stages))*(CWmin+1));
	//printf("Attempt = %d | BEB --> CW = %d\n",attempt,CW);
	return CW;	
};

void Station :: FrameTransmissionDelay(double TotalBitsToBeTransmitted, int N_MPDUs, int station_id)
{	

	// station = 0 (AP) in all cases. To improve.
	
	// Effective Pt
	double effPt = Pt;	
	if (SU_spatial_streams > 1) effPt = effPt - 3*SU_spatial_streams;
	if (channel_width > 20) effPt = effPt - 3*(channel_width/20);

	double distance = CalculateDistance(x,y,z,x_AP[station_id],y_AP[station_id],z_AP[station_id]);
	double PL = PathLoss(distance);
	double Pr = effPt - PL;
	RSSI[station_id] = Pr;

	//printf("STA to AP %d: I'm at %f,%f,%f and you are at %f,%f,%f | PL = %f | Pr = %f\n",station_id,x,y,z,x_[station_id],y_[station_id],z_[station_id],PL,Pr);

	if (Pr < -82)
	{
		//printf("************************* There is no conectivity ************************* [We assume MCS 1 and pe = 1]\n");
		BitsSymbol[station_id] = 1;
		CodingRate[station_id] = (double) 1/2;
		// pe = 0.9;
	}

	if( Pr >= -82 && Pr < -79)
	{
		BitsSymbol[station_id] = 1;
		CodingRate[station_id] = (double) 1/2;
	}	 
	if( Pr >= -79 && Pr < -77)
	{
		BitsSymbol[station_id] = 2;
		CodingRate[station_id] = (double) 1/2;
	}	 
	if( Pr >= -77 && Pr < -74)
	{
		BitsSymbol[station_id] = 2;
		CodingRate[station_id] = (double) 3/4;
	}	 
	if( Pr >= -74 && Pr < -70)
	{
		BitsSymbol[station_id] = 4;
		CodingRate[station_id] = (double) 1/2;
	}	 
	if( Pr >= -70 && Pr < -66)
	{
		BitsSymbol[station_id] = 4;
		CodingRate[station_id] = (double) 3/4;
	}	 
	if( Pr >= -66 && Pr < -65)
	{
		BitsSymbol[station_id] = 6;
		CodingRate[station_id] = (double) 1/2;
	}	 
	if( Pr >= -65 && Pr < -64)
	{
		BitsSymbol[station_id] = 6;
		CodingRate[station_id] = (double) 2/3;
	}	 
	if( Pr >= -64 && Pr < -59)
	{
		BitsSymbol[station_id] = 6;
		CodingRate[station_id] = (double) 3/4;
	}	 
	if( Pr >= -59 && Pr < -57)
	{
		BitsSymbol[station_id] = 8;
		CodingRate[station_id] = (double) 3/4;
	}	 
	if( Pr >= -57 && Pr < -55)
	{
		BitsSymbol[station_id] = 8;
		CodingRate[station_id] = (double) 5/6;
	}	 
	if( Pr >= -55 && Pr < -53)
	{
		BitsSymbol[station_id] = 10;
		CodingRate[station_id] = (double) 3/4;
	}	 
	if( Pr >= -53)
	{
		BitsSymbol[station_id] = 10;
		CodingRate[station_id] = (double) 5/6;
	}	 
	
	int Subcarriers = 980; // https://www.arubanetworks.com/assets/wp/WP_802.11AX.pdf, page 12
	if(channel_width == 80) Subcarriers = 980;
	if(channel_width == 40) Subcarriers = 468;
	if(channel_width == 20) Subcarriers = 234;
	

	int SU_spatial_streams = 2;
	double ORate = SU_spatial_streams * BitsSymbol[station_id] * CodingRate[station_id] * Subcarriers;
	double OBasicRate = (double) 1/2 * 1 * 48;

	double L = TotalBitsToBeTransmitted/N_MPDUs;
	
	int SF = 16;
	int TB = 18;
	int MD = 32;
	int MAC_H_size = 240;

	double T_RTS = Legacy_PHY_duration + ceil((SF+160+TB)/OBasicRate)*4E-6;
	double T_CTS = Legacy_PHY_duration + ceil((SF+112+TB)/OBasicRate)*4E-6;
	double T_DATA = PHY_duration + ceil((SF+N_MPDUs*(MD + MAC_H_size + L) + TB)/ORate)*16E-6;
	double T_ACK = Legacy_PHY_duration + ceil((SF+240+TB)/OBasicRate)*4E-6;

	//double 
	T = T_RTS + SIFS + T_CTS + SIFS + T_DATA + SIFS + T_ACK + DIFS + SLOT;
	T_c = T_RTS + SIFS + T_CTS + DIFS + SLOT;
	//printf("0000 Data Rate = %f (%f) and The value of T = %f (T_DATA = %f)| NMPDUS = %d\n",ORate,OBasicRate,T,T_DATA,N_MPDUs);

	// return T;
};



void Station ::update_stats_AMPDU(data_packet &ampdu_packet, int queue_size){

    double AMPDU_L = ampdu_packet.L; 
	double now = SimTime(); 
    double T_s = ampdu_packet.T; 
    double T_q = ampdu_packet.T_q; 
	double throughput = AMPDU_L / (T_s + T_q); 

	PRINTF_COLOR(BG_RED, "%.6f [DBG STATS]    Packet %.0f from src %d to dest %d | T_s = %.3f ms, T_q = %.3f ms | L_packet = %.0f\n", SimTime(), ampdu_packet.ID_packet,  ampdu_packet.source, ampdu_packet.destination, T_s * 1000, T_q * 1000, AMPDU_L ); 

	STAcsv.timestamp.push_back(now); 
    STAcsv.L_ampdu.push_back(AMPDU_L);
    STAcsv.T_s.push_back(T_s);
    STAcsv.T_q.push_back(T_q);
    STAcsv.destination.push_back(ampdu_packet.destination);
    STAcsv.source.push_back(ampdu_packet.source);
	STAcsv.queue_size.push_back(queue_size); 
	STAcsv.throughput.push_back(throughput);
} 


#endif
