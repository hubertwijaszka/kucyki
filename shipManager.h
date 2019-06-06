#ifndef __SHIPMANAGER__
#define __SHIPMANAGER__
#include <vector>
#include "kucyki.h"

bool canTakeShip();
bool takeRandomShip();
void takeBackShip();
void shipHandleMessage(packet_send_t mesg);
void shipInit(std::vector<int> capacity);
void shipStateChange();
#endif