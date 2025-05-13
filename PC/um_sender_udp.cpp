#include "um_sender_udp.h"
#include "um_protocol_defs.h"
#include <QNetworkInterface>
#include <QDebug>
#include <QDataStream>

udp_um_sender::udp_um_sender(uint16_t senderPort, QObject * parent)
    : um_sender_base(parent)
    , socket(new QUdpSocket(this))
    , deviceIP()
    , senderPort(senderPort)
    , dataToSend()
{
    socket->bind();
    dataToSend.reserve(1500);
}

udp_um_sender::~udp_um_sender() {}


void udp_um_sender::set_port(quint16 port) { senderPort = port; }

void udp_um_sender::device_found(QHostAddress address)
{
    deviceIP = address;
    socket->writeDatagram(dataToSend, deviceIP, senderPort);
}

void udp_um_sender::send_search_packet()
{
    QByteArray arr(6, 0);
    arr.data()[0] = 0x02;
    arr.data()[2] = 0x80;

    auto ifaces = QNetworkInterface::allInterfaces();
    for(auto & it : ifaces)
    {
        auto flags = it.flags();
        auto addrs = it.addressEntries();

        if( (flags & it.IsUp) && (flags & it.IsRunning) )
        {
            for(auto & a : addrs)
            {
                if(a.ip().protocol() == QAbstractSocket::IPv4Protocol)
                {
                    if(a.broadcast().toString() != "")
                    {
//                        qDebug() << a.broadcast().toString();
                        socket->writeDatagram(arr, a.broadcast(), senderPort);
                    }
                }
            }
        }
    }
}

void udp_um_sender::send_packet(const QByteArray & arr)
{
    dataToSend = arr;
    if(deviceIP.isNull())
        send_search_packet();
    else
        socket->writeDatagram(dataToSend, deviceIP, senderPort);
}
