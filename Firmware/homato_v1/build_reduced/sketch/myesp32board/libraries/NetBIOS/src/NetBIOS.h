#line 1 "/Users/bhargavveepuri/Homato/v1/homato/Firmware/homato_v1/myesp32board/libraries/NetBIOS/src/NetBIOS.h"
//
#ifndef __ESPNBNS_h__
#define __ESPNBNS_h__

#include "Arduino.h"
#include "AsyncUDP.h"

class NetBIOS {
protected:
  AsyncUDP _udp;
  String _name;
  void _onPacket(AsyncUDPPacket &packet);

public:
  NetBIOS();
  ~NetBIOS();
  bool begin(const char *name);
  void end();
};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_NETBIOS)
extern NetBIOS NBNS;
#endif

#endif
