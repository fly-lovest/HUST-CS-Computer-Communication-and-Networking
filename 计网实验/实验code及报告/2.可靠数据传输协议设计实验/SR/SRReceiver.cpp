#include "stdafx.h"
#include "Global.h"
#include "SRReceiver.h"


SRReceiver::SRReceiver():expectSequenceNumberRcvd(0),seqlen(8),base(0), winlen(4)
{
	lastAckPkt.acknum = -1; //初始状态下，上次发送的确认包的确认序号为-1，使得当第一个接受的数据包出错时该确认报文的确认号为-1
	lastAckPkt.checksum = 0;
	lastAckPkt.seqnum = -1;	//忽略该字段
	for(int i = 0; i < Configuration::PAYLOAD_SIZE;i++){
		lastAckPkt.payload[i] = '.';
	}
	lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
	for (int i = 0; i < winlen; i++) {
		rcvPck blank;
		blank.flag = false;
		blank.winPck.seqnum = -1;
		window.push_back(blank);
	}                                //将窗口填满，但都为空包，flag置为false，方便后续操作

}


SRReceiver::~SRReceiver()
{
}

void SRReceiver::receive(const Packet &packet) {
	//检查校验和是否正确
	int checkSum = pUtils->calculateCheckSum(packet);
	int offseqnum = (packet.seqnum - this->base + this->seqlen) % this->seqlen;

	//如果校验和正确，同时收到报文的序号在接收窗口范围内
	if (checkSum == packet.checksum && offseqnum < this->winlen && window.at(offseqnum).flag == false) {
		window.at(offseqnum).flag = true;
		window.at(offseqnum).winPck = packet;

		//将该窗口状态设置为ture，占用
		printf("接收方窗口:[ ");
		for (int i = 0; i < window.size(); i++) {
			if(window.at(i).flag == true)
				printf("%dY ", (this->base + i) % seqlen);
			else printf("%dN ", (this->base + i) % seqlen);
		}
		printf("]\n");  //接收到报文前的窗口序列

		pUtils->printPacket("接收方正确收到发送方的报文", packet);

		while (window.front().flag == true) {
			Message msg;
			memcpy(msg.data, window.front().winPck.payload, sizeof(window.front().winPck.payload));
			pns->delivertoAppLayer(RECEIVER, msg);
			this->base = (this->base + 1) % seqlen;
			rcvPck blank;
			blank.flag = false;
			blank.winPck.seqnum = -1;
			window.pop_front();
			window.push_back(blank); //窗口向右滑动一格
		}

		printf("接收方滑动后窗口:[ ");
		for (int i = 0; i < window.size(); i++) {
			if (window.at(i).flag == true)
				printf("%dY ", (this->base + i) % seqlen);
			else printf("%dN ", (this->base + i) % seqlen);
		}
		printf("]\n");  //接收报文后的窗口序列

		lastAckPkt.acknum = packet.seqnum; //确认序号等于收到的报文序号
		lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
		pUtils->printPacket("接收方发送确认报文", lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送确认报文到对方


	}
	else {
		if (checkSum != packet.checksum) {
			pUtils->printPacket("接收方没有正确收到发送方的报文,数据校验错误", packet);
		}
		else {
			pUtils->printPacket("接收方没有正确收到发送方的报文,报文已确认", packet);
			lastAckPkt.acknum = packet.seqnum; //确认序号等于收到的报文序号
			lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
			pUtils->printPacket("接收方重新发送上次的确认报文", lastAckPkt);
			pns->sendToNetworkLayer(SENDER, lastAckPkt);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送已确认报文的ACK
		}
	}
}