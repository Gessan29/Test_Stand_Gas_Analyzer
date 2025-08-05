#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QVector>
#include "um_defs.h"
#include "um_protocol_defs.h"
#include "um_sender_udp.h"
#include "um_receiver_udp.h"
#include <QMessageBox>
#include <QString>
#include <QWidget>
#include <QNetworkDatagram>
#include <QUdpSocket>
#include "protocol_parser.h"
#include "customdialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

QString description (const QVector<uint8_t>& packet);
QString result (const QVector<uint8_t>& packet);

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void handleSerialData();  // Было on_port_ready_read
    void handleDeviceFound(QHostAddress address);
    void handleUdpData();
    void on_pbOpen_clicked();

    void sendNextPacket();
    void on_cleabutt_clicked();
    void stopTesting();
    void on_pushButton_clicked();
    void handleParserError();
    void handleParsedPacket();
    void startTesting();
    void result(uint8_t* packet);
    void handleCaseCommon(uint16_t sample, const QString& labelText);
    void plotAdcData(const QByteArray& byteArray);
    void setupPort();
    void peltie(uint16_t sample, uint16_t bit);
    void setupConnections();
    void onResponseTimeout();
    void closeTest();
    void logHtml(const QString& message);
    void logPlain(const QString& message);

private:
    Ui::MainWindow *ui;
    QSerialPort *port;
    QTimer* sendTimer; // таймер для отправки следующих пакетов через задержку
    QTimer* responseTimer; // таймер ожидания ответа от МК
    struct protocol_parser parser;
    QVector<QVector<uint8_t>> testPackets;
    int currentPacketIndex = 0; //индекс текущего пакета в очереди
    bool isTesting = false; // флаг, идет ли сейчас тестирование
    bool emergencyStopTriggered = false;
    void sendPacket(uint8_t cmd, uint8_t status, uint8_t value);

    udp_um_sender* udpSender;
    udp_um_receiver* udpReceiver;
    QHostAddress deviceAddress; // Адрес найденного устройства
    bool ethernetConnected = false;

};

#endif // MAINWINDOW_H
