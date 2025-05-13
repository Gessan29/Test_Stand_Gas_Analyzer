#ifndef UM_RECEIVER_RS232_H
#define UM_RECEIVER_RS232_H

#include "um_receiver_base.h"
#include <QTimer>

class um_receiver_rs232 : public um_receiver_base
{
    Q_OBJECT
public:
    um_receiver_rs232(QObject * parent = nullptr);
    ~um_receiver_rs232();

public slots:
    void on_port_ready_read(QByteArray arr);

private:
    QByteArray packet;
    QTimer * timer;
    friend class um_sender_rs232;
    static constexpr quint8 sync_byte = 0x55;
    int okCount;
    int badCount;

    static quint16 calc_crc(quint16 initValue, const quint8 * buf, quint32 len);

private slots:
    void on_timer_timeout();
};

#endif // UM_RECEIVER_RS232_H
