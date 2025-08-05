#ifndef UM_RECEIVER_UDP_H
#define UM_RECEIVER_UDP_H

#include "um_receiver_base.h"
#include <QHostAddress>
#include <QUdpSocket>

class udp_um_receiver : public um_receiver_base
{
    Q_OBJECT
public:
    udp_um_receiver(uint16_t listenPort, QObject * parent = nullptr);
    ~udp_um_receiver();
    QUdpSocket* get_socket() const { return socket; }

signals:
    // Ответ по сети
    void device_found(QHostAddress address);
    void raw_packet(QByteArray arr);

public slots:
    void set_port(quint16 port);

private:
    QUdpSocket * socket;
    uint16_t listenPort;

    void on_socket_ready_read();

    bool check_for_serach_packet_answer(const QNetworkDatagram & dg);
    void handle_as_search_packet_answer(const QNetworkDatagram & dg);
};

#endif // UM_RECEIVER_UDP_H
