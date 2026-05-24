#ifndef _H_RADIO
#define _H_RADIO

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "header.h"

void radio_setup(RF24 *_radio);
bool radio_transact(RF24 *_radio, transaction_unit *_transaction_unit);
bool radio_read(RF24 *_radio, transaction_unit *_transition_unit);

#endif
