#ifndef UM_SENDER_RS232_H
#define UM_SENDER_RS232_H

#include "um_sender_base.h"
#include <QSerialPort>

class um_receiver_rs232;

class um_sender_rs232: public um_sender_base
{
    Q_OBJECT
public:
    um_sender_rs232( quint32 baudrate,
                     um_receiver_rs232 * receiver,
                     QObject * parent = nullptr );
    ~um_sender_rs232();

signals:
    void port_ready_read(QByteArray arr);

public slots:
    void set_port_baudrate(quint32 baudrate);
    void set_port_name(const QString & portName);

private:
    QSerialPort * port;

protected:
    virtual void send_packet(const QByteArray & arr) override;
};

#endif // UM_SENDER_RS232_H
