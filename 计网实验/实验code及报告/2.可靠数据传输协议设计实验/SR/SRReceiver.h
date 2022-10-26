#ifndef STOP_WAIT_RDT_RECEIVER_H
#define STOP_WAIT_RDT_RECEIVER_H
#include "RdtReceiver.h"
#include<deque>
struct rcvPck {
	bool flag;       //指示该位置是否被占用，ture表示占用
	Packet winPck;   //存放数据包
};

class SRReceiver :public RdtReceiver
{
private:
	int expectSequenceNumberRcvd;	// 期待收到的下一个报文序号
	int base;                       //当前窗口基序号
	int winlen;                     //窗口大小
	int seqlen;                     //序号宽度
	deque<rcvPck> window;               //窗口数组
	Packet lastAckPkt;				//上次发送的确认报文

public:
	SRReceiver();
	virtual ~SRReceiver();

public:
	
	void receive(const Packet &packet);	//接收报文，将被NetworkService调用
};


#endif

