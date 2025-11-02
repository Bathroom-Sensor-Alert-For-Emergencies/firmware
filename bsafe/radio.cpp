#include "radio.hpp"
#include "packet.hpp"

Radio::Radio(std::uint32_t cs, std::uint32_t gdo0, std::uint32_t gdo2)
    : Communicator(),
      radio{new Module(cs, gdo0, RADIOLIB_NC, gdo2)} {}

bool Radio::begin() {
    return this->radio.begin() == RADIOLIB_ERR_NONE;
}

bool Radio::send(std::uint8_t* data, std::size_t len) {
    return this->radio.transmit(data, len) == RADIOLIB_ERR_NONE;
}

bool Radio::recv(std::uint8_t* data, std::size_t len) {
    return this->radio.receive(data, len) == RADIOLIB_ERR_NONE;
}

void Radio::listen(void (*on_recv)(Packet)) {
    this->radio.startReceive();
    Radio::current = this;
    Radio::on_recv = on_recv;
    this->radio.setPacketReceivedAction([] {
        Packet packet;
        Radio::current->radio.readData((std::uint8_t*)&packet, sizeof(packet));
        Radio::current->radio.finishReceive();
        Radio::on_recv(packet);
        Radio::current->radio.startReceive();
    });
    // TODO: Low power listening
    // this->radio.sleep();
}
