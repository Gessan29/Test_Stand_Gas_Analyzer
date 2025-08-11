#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QVector>
#include <QMessageBox>
#include <QString>
#include <QWidget>

#include "qcustomplot.h"
#include "protocol_parser.h"
#include "customdialog.h"
#include "um_defs.h"
#include "um_protocol_defs.h"
#include "um_sender_udp.h"
#include "um_receiver_udp.h"

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
    void on_port_ready_read();
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
    void updateConnectionStatus(QHostAddress address);
    void check_mode_acm(um_alg_cmd cmd, um_status status);
    void on_um_vector_received(um_vector_id id, std::vector<float> vector);
private:
    Ui::MainWindow *ui;
    QSerialPort *port;
    udp_um_sender* udpSender;
    udp_um_receiver* udpReceiver;
    QTimer* sendTimer; // таймер для отправки следующих пакетов через задержку
    QTimer* responseTimer; // таймер ожидания ответа от МК

    QCPGraph* graphRef = nullptr;
    QCPGraph* graphAnl = nullptr;
    double xCounter = 0;   // счетчик точек

    QTimer* plotTimer = nullptr; // таймер для replot()

    bool ethernetConnected = false; // флаг соединения Ethernet

    struct protocol_parser parser;
    QVector<QVector<uint8_t>> testPackets;
    int currentPacketIndex = 0; //индекс текущего пакета в очереди
    bool isTesting = false; // флаг, идет ли сейчас тестирование
    bool emergencyStopTriggered = false;
    void sendPacket(uint8_t cmd, uint8_t status, uint8_t value);

};

#endif // MAINWINDOW_H
