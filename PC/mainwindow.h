#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QVector>
#include <QMessageBox>
#include <QString>
#include <QWidget>
#include <QMutex>

#include "plotsetup.h"
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
    void result();
    void handleCaseCommon(double sample, double ratio, const QString& labelText);
    void check_current();
    void setupPort();
    void peltie();
    void set_param(float temp);
    void setupSettings();
    void onResponseTimeout();
    void closeTest();
    void logHtml(const QString& message);
    void updateConnectionStatus(QHostAddress address);
    void check_mode_acm(um_alg_cmd cmd, um_status status);
    void on_um_vector_received(um_vector_id id, std::vector<float> vector);
    void test_temp(float temp);
private:
    Ui::MainWindow *ui;
    QSerialPort *port;
    udp_um_sender* udpSender;
    udp_um_receiver* udpReceiver;

    QTimer* sendTimer; // таймер для отправки следующих пакетов через задержку
    QTimer* responseTimer; // таймер ожидания ответа от МК
    QTimer* plotTimer; // для отрисовки графиков

    QVector<double> lastRef, lastAnl, lastX;

    QMetaObject::Connection conn_data, conn_rec;

    PlotHandles plots;

    QMutex dataMutex;
    bool dataUpdated = false; // флаг обновления данных для аналит. и реп. каналов

    bool ethernetConnected = false; // флаг соединения Ethernet

    QVector<std::shared_ptr<um_data>> dataBuffer;
    int remainingOutputs = 0;

    struct protocol_parser parser;
    QVector<QVector<uint8_t>> testPackets;
    int currentPacketIndex = 0; //индекс текущего пакета в очереди
    bool isTesting = false; // флаг, идет ли сейчас тестирование
    bool emergencyStopTriggered = false;
    void sendPacket(uint8_t cmd, uint8_t status, uint8_t value);

    bool acmModePrinted = false;

    int measurementCount = 0;
    const int requiredMeasurements = 500;
    double sumTemperature = 0.0;
    double sumPeltierCurrent = 0.0;
    bool averagingInProgress = true;
    QEventLoop averagingloop;

    void startAveragingMeasurements();
    void processAveragedResults();

    bool autoScrollEnabled = true;

};

#endif // MAINWINDOW_H
