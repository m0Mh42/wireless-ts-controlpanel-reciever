#include "radio.h"

bool _radio_sendpacket(RF24 *_radio, transaction_unit *_transaction_unit, uint64_t *_local_seq)
{
    // TODO Radio Transmition Error Handling
    while (true)
    {
        bool radio_sent = _radio->write(_transaction_unit, sizeof(transaction_unit));
        if (radio_sent)
        {
            *_local_seq += 1;
            break;
        }
    }
    return true;
}

void radio_setup(RF24 *_radio)
{
    _radio->begin();
    _radio->openWritingPipe(addresses[DEVICE_TS_SLAVE]);  // TS 1
    _radio->openReadingPipe(1, addresses[DEVICE_MASTER]); // Master Reciever
    _radio->setPALevel(RF24_PA_MAX);
    _radio->setChannel(CHANNEL);

    // Defected Chip Error
    if (!_radio->isChipConnected())
    {
        while (true)
        {
            digitalWrite(LED_BUILTIN, HIGH);
            delay(250);
            digitalWrite(LED_BUILTIN, LOW);
            delay(250);
        }
    }
}

void radio_transact(RF24 *_radio, transaction_unit *_transaction_unit, uint64_t *_local_seq)
{
    _transaction_unit->seq = *_local_seq;
    _radio_sendpacket(_radio, _transaction_unit, _local_seq);
}