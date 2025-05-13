#include "um_sender_rs232.h"
#include "um_receiver_rs232.h"
#include <QDataStream>

um_sender_rs232::um_sender_rs232( quint32 baudrate,
                                  um_receiver_rs232 * receiver,
                                  QObject * parent )
    : um_sender_base(parent)
    , port(new QSerialPort(this))
{
    port->setBaudRate(baudrate);
    connect( this, &um_sender_rs232::port_ready_read,
             receiver, &um_receiver_rs232::on_port_ready_read );
    connect( port, &QSerialPort::readyRead, this, [this]()
    {
        emit port_ready_read(port->readAll());
    });
}

um_sender_rs232::~um_sender_rs232() {}

void um_sender_rs232::set_port_baudrate(quint32 baudrate)
{
    port->setBaudRate(baudrate);
}

void um_sender_rs232::set_port_name(const QString & portName)
{
    if(port->portName() != portName)
    {
        if(port->isOpen())
            port->close();
        port->setPortName(portName);
        port->open(QSerialPort::ReadWrite);
    }
}

void um_sender_rs232::send_packet(const QByteArray & arr)
{
    QByteArray packet(arr.size()+5, Qt::Uninitialized);
    QDataStream ds(&packet, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::LittleEndian);
    ds << um_receiver_rs232::sync_byte;
    ds << static_cast<quint16>(arr.size() + 2);
    ds.writeRawData(arr.data(), arr.size());
    ds << um_receiver_rs232::calc_crc(0xFFFF, (const quint8*)arr.data(), arr.size());
    qDebug() << packet.toHex(' ');
    port->write(packet);
}

