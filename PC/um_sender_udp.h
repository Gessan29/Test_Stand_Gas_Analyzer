#ifndef UM_SENDER_UDP_H
#define UM_SENDER_UDP_H

#include "um_sender_base.h"
#include <QHostAddress>
#include <QUdpSocket>

class udp_um_sender : public um_sender_base
{
    Q_OBJECT
public:
    udp_um_sender(uint16_t senderPort, QObject * parent = nullptr);
    ~udp_um_sender();

    // Внутренние
    void set_port(quint16 port);
    void device_found(QHostAddress address);
    void send_search_packet();
private:
    QUdpSocket * socket;
    QHostAddress deviceIP;
    uint16_t senderPort;
    QByteArray dataToSend;



protected:
    virtual void send_packet(const QByteArray & arr) override;
};

#endif // UM_SENDER_UDP_H
