#include "um_receiver_udp.h"
#include <QNetworkDatagram>
#include <QDataStream>
#include <math.h>

udp_um_receiver::udp_um_receiver(uint16_t listenPort, QObject * parent)
    : um_receiver_base(parent)
    , socket(new QUdpSocket(this))
    , listenPort(listenPort)
{
    socket->bind(listenPort);
    connect( socket, &QUdpSocket::readyRead, this,
             &udp_um_receiver::on_socket_ready_read );
}

udp_um_receiver::~udp_um_receiver()
{}

void udp_um_receiver::set_port(quint16 port) { listenPort = port; }

void udp_um_receiver::on_socket_ready_read()
{
    while(socket->hasPendingDatagrams())
    {
        auto dg = socket->receiveDatagram();

        if(check_for_serach_packet_answer(dg))
        {
            emit raw_packet(dg.data());
            handle_as_search_packet_answer(dg);
            continue;
        }

        decode_packet(dg.data());
    }
}

bool udp_um_receiver::check_for_serach_packet_answer(const QNetworkDatagram & dg)
{
    if(dg.data().size() != 6)
        return false;

    if(static_cast<uint8_t>(dg.data().at(0)) != 0x02)
        return false;
    if(static_cast<uint8_t>(dg.data().at(1)) != 0x00)
        return false;
    if(static_cast<uint8_t>(dg.data().at(2)) != 0x80)
        return false;
    if(static_cast<uint8_t>(dg.data().at(3)) != 0x00)
        return false;
    return true;
}

void udp_um_receiver::handle_as_search_packet_answer(const QNetworkDatagram & dg)
{
    auto devAddr = dg.senderAddress();
    emit device_found(devAddr);
}
