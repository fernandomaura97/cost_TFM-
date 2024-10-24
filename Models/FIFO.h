
#ifndef _FIFO_QUEUE_
#define _FIFO_QUEUE_

//#include "/home/boris/Programes/Simulation/sense-2003-1229/common/cost.h"

#include "definitions.h"
#include <deque>


/*
	FIFO Class
*/

template <class test>
component FIFO : public TypeII
{	
	private:
		std::deque <data_packet> m_queue;
		
	public:
		data_packet GetFirstPacket();
		data_packet GetPacketAt(int n);
		void DelFirstPacket();		
		void DeletePacketIn(int i);
		void PutPacket(data_packet &packet);	
		void PutPacketFront(data_packet &packet);	
		void PutPacketIn(data_packet &packet, int);	
		int QueueSize();
		void PrintQueueContents(); 
};

data_packet FIFO :: GetFirstPacket()
{
	return(m_queue.front());	
}; 

data_packet FIFO :: GetPacketAt(int n)
{
	return(m_queue.at(n));	
}; 


void FIFO :: DelFirstPacket()
{
	m_queue.pop_front();
}; 

void FIFO :: PutPacket(data_packet &packet)
{	
	m_queue.push_back(packet);
}; 

void FIFO :: PutPacketFront(data_packet &packet)
{	
	m_queue.push_front(packet);
}; 

int FIFO :: QueueSize()
{
	return(m_queue.size());
}; 

void FIFO :: PutPacketIn(data_packet &packet,int i)
{
	m_queue.insert(m_queue.begin()+i,packet);
}; 

void FIFO :: DeletePacketIn(int i)
{
	m_queue.erase(m_queue.begin()+i);
};

void FIFO :: PrintQueueContents()
{
	 std::cout << "Packet IDs in the queue:" << std::endl;
    for (size_t i = 0; i < m_queue.size(); ++i)
    {
        std::cout << "Packet " << i + 1 << ", ID: " << m_queue[i].ID_packet 
				  << ", T_q: " << std::setprecision(3) << m_queue[i].T_q << ", T_s: "<< std::setprecision(3) << m_queue[i].T << std::endl;
    }
}; 



#endif
