#ifndef __PONNY_MANAGER__
#define __PONNY_MANAGER__

#include "kucyki.h"

void handleUnlockPonny();
bool canTakePonny();
void takePonny();
void returnPonny();
void ponnyHandleMessage(packet_send_t mesg);
void ponnyInit(int rank, int size, int count);
void ponnyStateChange();
#endif