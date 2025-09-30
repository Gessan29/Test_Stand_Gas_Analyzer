#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>
#include <QVector>

static const quint16 defaultSenderPort = 30000;
static const quint16 defaultListenPort = 30001;

const double ratio = 0.3589;
QMetaObject::Connection conn_data, conn_rec;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , port(new QSerialPort(this))
    , udpSender(new udp_um_sender(defaultSenderPort, this))
    , udpReceiver(new udp_um_receiver(defaultListenPort, this))
    , sendTimer(new QTimer(this))
    , responseTimer(new QTimer(this))
    , outputTimer(new QTimer(this))
{
    ui->setupUi(this);
    setupPort();
    setupConnections();
    parser.state = protocol_parser::STATE_SYNC; 

    auto plots = setupMainPlot(ui->customPlot, ui->customPlot_2);
    graphRef = plots.graphRef;
    graphAnl = plots.graphAnl;

    plotTimer = new QTimer(this);
    connect(plotTimer, &QTimer::timeout, this, [this]() {
        QMutexLocker locker(&dataMutex);

        if (!dataUpdated) return;

        if (graphRef) graphRef->setData(lastX, lastRef);
        if (graphAnl) graphAnl->setData(lastX, lastAnl);

        ui->customPlot->replot();

        dataUpdated = false;

        });

}

MainWindow::~MainWindow() {
    delete ui;
    port->close();
}

void MainWindow::setupConnections() {
    connect(sendTimer, &QTimer::timeout, this, &MainWindow::sendNextPacket);
    connect(responseTimer, &QTimer::timeout, this, &MainWindow::onResponseTimeout);
    responseTimer->setSingleShot(true);
    connect(port, &QSerialPort::readyRead, this, &MainWindow::on_port_ready_read);
}

void MainWindow::setupPort() {
    port->setDataBits(QSerialPort::Data8);
    port->setParity(QSerialPort::NoParity);
    port->setStopBits(QSerialPort::OneStop);
    port->setFlowControl(QSerialPort::NoFlowControl);
}

void MainWindow::updateConnectionStatus(QHostAddress address){
    if (ethernetConnected){
        return;
    }

    udpSender->device_found(address);
    ethernetConnected = true;
    logHtml("<font color ='green'>Тестируемая плата АЦМ подключена:</font>");
    logHtml("<font color ='green'>IP устройства: </font>" + address.toString());
    logHtml(QString("<font color ='green'>Порт отправки: %1</font>").arg(defaultSenderPort));
    logHtml(QString("<font color ='green'>Порт приема: %1</font><br>").arg(defaultListenPort));
}

void MainWindow::on_pbOpen_clicked() {
    if (port->isOpen()) {
        port->close();
        ui->pbOpen->setText("Открыть порт");
        return;
    }

    port->setBaudRate(ui->sbxBaudrate->value());
    port->setPortName(ui->cmbxComPort->currentText());

    if (port->open(QIODevice::ReadWrite)) {
        ui->pbOpen->setText("Закрыть порт");
        logHtml("<font color='green'>Порт открыт!</font><br>");
    } else {
        logHtml("<font color='red'>Ошибка открытия порта!</font><br>");
    }
}

void MainWindow::onResponseTimeout() {
    logHtml("<font color='red'>Ошибка: не получен ответ в течение 10 секунд</font><br>");
    stopTesting();
}

void MainWindow::on_port_ready_read() {
    const QByteArray data = port->readAll();
    //logHtml(QString("<font color='blue'>%1</font>").arg(QString::fromUtf8(data.toHex(' ').toUpper()))); // для просмотра пришедших пакетов

    for (const char byte : data) {
        const parser_result res = process_rx_byte(&parser, static_cast<uint8_t>(byte));
        if (res == PARSER_DONE) handleParsedPacket();
        else if (res == PARSER_ERROR) handleParserError();
    }
}

void MainWindow::on_pushButton_clicked() {
    if (!port->isOpen()) {
        logHtml("<font color='red'>Порт закрыт!</font><br>");
        return;
    }

    isTesting = !isTesting;
    ui->pushButton->setText(isTesting ? "Остановить" : "Начать");
    isTesting ? startTesting() : closeTest();
}

void MainWindow::logHtml(const QString& message) {
    ui->plainTextEdit->appendHtml(message);
}

void MainWindow::startTesting()
{
    if (graphRef) graphRef->data()->clear();
    if (graphAnl) graphAnl->data()->clear();
    ui->customPlot->replot();

    ui->customPlot_2->clearGraphs();
    ui->customPlot_2->replot();

    acmModePrinted = false;

    currentPacketIndex = 0;
    testPackets = {
        {0x01, 0x00, 0x00, 0x00, 0x00, 0x00}, {0x01, 0x07, 0x00, 0x00, 0x00, 0x00}, {0x01, 0x03, 0x00, 0x00, 0x00, 0x00},
        {0x01, 0x00, 0x01, 0x00, 0x00, 0x00},
        {0x01, 0x02, 0x00, 0x00, 0x00, 0x00}, {0x01, 0x02, 0x01, 0x00, 0x00, 0x00},
        {0x01, 0x01, 0x01, 0x00, 0x00, 0x00}, {0x01, 0x01, 0x02, 0x00, 0x00, 0x00}, {0x01, 0x01, 0x03, 0x00, 0x00, 0x00}, {0x01, 0x01, 0x04, 0x00, 0x00, 0x00},
        {0x01, 0x03, 0x01, 0x00, 0x00, 0x00},
        {0x01, 0x02, 0x00, 0x00, 0x00, 0x00}, {0x01, 0x02, 0x01, 0x00, 0x00, 0x00},

        {0x01, 0x04, 0x00, 0x00, 0x00, 0x00}, {0x01, 0x04, 0x01, 0x00, 0x00, 0x00}, {0x01, 0x04, 0x02, 0x00, 0x00, 0x00}, {0x01, 0x04, 0x03, 0x00, 0x00, 0x00},
        {0x01, 0x04, 0x04, 0x00, 0x00, 0x00}, {0x01, 0x04, 0x05, 0x00, 0x00, 0x00}, {0x01, 0x04, 0x06, 0x00, 0x00, 0x00}, {0x01, 0x04, 0x07, 0x00, 0x00, 0x00},
        {0x01, 0x04, 0x08, 0x00, 0x00, 0x00}, {0x01, 0x04, 0x09, 0x00, 0x00, 0x00}, {0x01, 0x04, 0x0A, 0x00, 0x00, 0x00},
        //
        {0x00, 0x06}, // 25-ый {0x00, 0x06}
        {0x00, 0x05},
        {0x00, 0x06}, {0x00, 0x06},
        {0x00, 0x08}, //32-ой
        {0x01, 0x09, 0x0A, 0x00, 0x00, 0x00},
        /*{0x01, 0x09, 0x0B, 0x00, 0x00, 0x00}
        {0x01, 0x09, 0x0C, 0x00, 0x00, 0x00},
        {0x01, 0x09, 0x0D, 0x00, 0x00, 0x00},
        {0x01, 0x09, 0x0E, 0x00, 0x00, 0x00},
        {0x01, 0x09, 0x0F, 0x00, 0x00, 0x00},
        {0x01, 0x09, 0x10, 0x00, 0x00, 0x00},
        {0x01, 0x09, 0x11, 0x00, 0x00, 0x00},
        {0x01, 0x09, 0x12, 0x00, 0x00, 0x00},
        {0x01, 0x09, 0x13, 0x00, 0x00, 0x00}*/
        // 26 пункт и далее:
        {0x01, 0x00, 0x00, 0x00, 0x00, 0x00},
        {0x01, 0x07, 0x01, 0x00, 0x00, 0x00},
        {0x01, 0x00, 0x01, 0x00, 0x00, 0x00},
        {0x01, 0x02, 0x00, 0x00, 0x00, 0x00}, {0x01, 0x02, 0x01, 0x00, 0x00, 0x00}
    };
    logHtml("<font color='orange'>Тестирование запущено</font><br>");
    sendNextPacket();
}

void MainWindow::sendNextPacket()
{
    if ((!isTesting && !emergencyStopTriggered) || currentPacketIndex >= testPackets.size()) {
        stopTesting();
        return;
    }

     const QVector<uint8_t>& packetData = testPackets[currentPacketIndex++];
     ui->plainTextEdit->appendPlainText(description(packetData));
     transfer packet(packetData);
     serialize_reply(&packet);
     int packetSize = 0;
         if (packet.getCmd() == 0) {
             packetSize = 7;
         } else {
             packetSize = 11;
         }

         QByteArray byteArray(reinterpret_cast<const char*>(packet.buf), packetSize);
     port->write(byteArray);

//     ui->plainTextEdit->appendHtml(QString("<font color='green'>Отправлен пакет %1: %2</font><br>") // для просмотра отправленных пакетов
//                                      .arg(currentPacketIndex)
//                                      .arg(QString::fromUtf8(byteArray.toHex(' ').toUpper())));
    plotTimer->start(50);
    responseTimer->start(10000);
    sendTimer->stop();
}

QString description (const QVector<uint8_t>& packet){
    uint8_t status = packet[1];

    switch (status){
    case 0x00:
        if (packet[2] == 0x01){ return "Подача напряжение питания 12В на плату:"; }
        else {return "Снять напряжение питания 12В с платы:"; }
    case 0x01:
        if (packet[2] == 0x01){ return "Измерение напряжение контрольной точки: 5V (PW Peltier):"; }
        else if (packet[2] == 0x02) {return "Измерение напряжение контрольной точки: +5.3V:"; }
        else if (packet[2] == 0x03) {return "Измерение напряжение контрольной точки: +3.3V:"; }
        else if (packet[2] == 0x04) {return "Измерение напряжение контрольной точки: +4V (PW laser):"; }
    case 0x02:
        if (packet[2] == 0x00){ return "Измерение напряжения питания платы:"; }
        else {return "Измерение тока питания платы:"; }
    case 0x03:
        if (packet[2] == 0x01){ return "Подключение резисторов имитации:"; }
        else {return "Отключение резисторов имитации:"; }
    case 0x04:
        switch (packet[2]){
        case 0x00:
            return "Измерение напряжение контрольной точки: +1.2V:";
        case 0x01:
            return "Измерение напряжение контрольной точки: +1.8V:";
        case 0x02:
            return "Измерение напряжение контрольной точки: +2.5V:";
        case 0x03:
            return "Измерение напряжение контрольной точки: +5V (Power GPS):";
        case 0x04:
            return "Измерение напряжение контрольной точки: +5V REFP:";
        case 0x05:
            return "Измерение напряжение контрольной точки: +5VAA sensor:";
        case 0x06:
            return "Измерение напряжение контрольной точки: -5VAA:";
        case 0x07:
            return "Измерение напряжение контрольной точки: +1.8VA:";
        case 0x08:
            return "Измерение напряжение контрольной точки: +5VAA (Amq_A):";
        case 0x09:
            return "Измерение напряжение контрольной точки: -2.048V:";
        case 0x0A:
            return "Измерение напряжение контрольной точки: 5V (Amq_R):";
        }
    case 0x05:
        return "Измерение формы тока лазерного диода:";
    case 0x06:
        return "Измерение напряжения и тока на эквиваленте элемента Пельтье:";
    case 0x07:
        if (packet[2] == 0x01){ return "Переключить тип входных цепей на внешний оптический блок:"; }
        else {return "Переключить тип входных цепей на эквивалентные схемы:"; }
    case 0x08:
        return "Тестирование работы интерфейса RS232:";
    case 0x09:
        return "Тестирование работы интерфейса подключения GPS-приемника:";
//    {
//        static bool isFirstTime = true;
//        if (isFirstTime) {
//            isFirstTime = false;
//            return "Тестирование работы интерфейса подключения GPS-приемника:";}
//        else {
//            return "";
//             }
//    }
}
}

void MainWindow::peltie(){
    if (currentPacketIndex == 24)
        return;
    int16_t volt_raw, tok_raw;
    float sample_v = 2, bit_v = 2, sample_a = 500, bit_a = 500;
    uint8_t znak = parser.buffer[5];

    volt_raw = (((parser.buffer[2] & 0xFF) << 8) | (parser.buffer[1] & 0xFF)); // напряжение 0
    tok_raw = (parser.buffer[3] & 0xFF) | ((parser.buffer[4] & 0xFF) << 8); // ток 0

    float volts = (float)volt_raw / 1000.0;
    int tok = (int)tok_raw;
    if (znak == 1){
        volts = -volts;
        tok = -tok;
        sample_v = -sample_v;
        sample_a = -sample_a;
    }

    if ( (volts >= sample_v - bit_v && volts <= sample_v + bit_v) && (tok >= sample_a - bit_a && tok <= sample_a + bit_a) ){ // поменять диапазон
        logHtml(QString("<font color='green'>Измерено: %1 мА — Ток эквивалента элемента Пельтье допустим</font>").arg(tok));
        logHtml(QString("<font color='green'>Измерено: %1 В — Напряжение эквивалента элемента Пельтье допустимо</font><br><br>").arg(QString::number(volts, 'f', 3)));
        return;
    }
    else {
       logHtml(QString("<font color='red'>Измерено: %1 мА — Ток эквивалента элемента Пельтье не допустим</font><br>").arg(tok));
       logHtml(QString("<font color='red'>Измерено: %1 В — Напряжение эквивалента элемента Пельтье не допустимо</font><br><br>").arg(QString::number(volts, 'f', 3)));
        closeTest();
    }
}

void MainWindow::result(uint8_t* packet){
    uint16_t data;
    float tok, sample;

    switch (currentPacketIndex){

    case 1:
    case 2:
    case 3:
        logHtml("<font color='green'>Выполнено!</font><br>");
        return;

    case 4:
    case 11:{
        QEventLoop waitPW;
        QTimer waitTimerPW;
        waitTimerPW.setSingleShot(true);
        connect(&waitTimerPW, &QTimer::timeout, &waitPW, &QEventLoop::quit);
        waitTimerPW.start(2000);
        waitPW.exec();
        logHtml("<font color='green'>Выполнено!</font><br>");
        return;
    }

    case 5:
    case 12:
        handleCaseCommon(11.5, 0.1666, "Питание платы,");
        return;

    case 6:
    case 13:
        sample = 0.15;
        data = (parser.buffer[2] << 8) | parser.buffer[1];
        tok = (float)data / 1000.0;
        tok = tok / (0.018 * 200);
        if (tok >= sample - 0.1 && tok <= sample + 0.1){
            logHtml(QString("<font color='green'>Измерено: %1 мА — Ток питания платы допустим</font><br>").arg(tok, 0, 'f', 3));
        }
        else {
           logHtml(QString("<font color='red'>Измерено: %1 мА — Ток питания платы не допустим</font><br>").arg(tok, 0, 'f', 3));
           closeTest();
           }
        return;
    case 7:
        handleCaseCommon(5, ratio, "Контрольная точка 5V (PW Peltier)");
        return;
    case 8:
        //handleCaseCommon(5.3, ratio, "Контрольная точка +5.3V");
        return;
    case 9:
        //handleCaseCommon(3.3, ratio, "Контрольная точка +3.3V");
        return;
    case 10:
        //handleCaseCommon(4.1, ratio, "Контрольная точка +4.1V (PW laser)");
        return;
    case 14:
        //handleCaseCommon(1.2, 1, "Контрольная точка +1.2V");
        return;
    case 15:
        //handleCaseCommon(1.8, 1, "Контрольная точка +1.8V");
        return;
    case 16:
        //handleCaseCommon(2.5, 1, "Контрольная точка +2.5V");
        return;
    case 17:
        handleCaseCommon(5, ratio, "Контрольная точка +5V (Power GPS)");
        return;
    case 18:
        handleCaseCommon(5, ratio, "Контрольная точка +5V (REFP)"); // нет контакта
        return;
    case 19:
        handleCaseCommon(5, ratio, "Контрольная точка +5VAA (sensor)");
        return;
    case 20:
        handleCaseCommon(5, 0.66, "Контрольная точка -5VAA"); // кф. деления????
        return;
    case 21:
        handleCaseCommon(1.8, 1, "Контрольная точка +1.8VA");
        return;
    case 22:
        handleCaseCommon(5, ratio, "Контрольная точка +5VAA (Amq_A)");
        return;
    case 23:
        handleCaseCommon(2.048, 1, "Контрольная точка -2.048V"); // не правильное напряжение
        return;
    case 24:{
        handleCaseCommon(5, ratio, "Контрольная точка +5VAA (Amq_R)");

        sendTimer->stop();
        responseTimer->stop();

        CustomDialog dialog_1(this,"Выполните условие", "Прошейте МК и ПЛИС","Ок","Не удалось прошить"); // Добавить фунцию
        bool ok = dialog_1.execDialog();
        if (ok) {
            logHtml("<font color='green'>МК и ПЛИС прошиты. Продолжение теста...</font><br>");
        } else {
            logHtml("<font color='red'>МК и ПЛИС не прошиты.</font><br>");
            closeTest();
            return;
        }

        logHtml("<font color='blue'>Поиск устройства по Ethernet...</font><br>");

            QEventLoop waitLoop;
            QTimer waitTimer;
            waitTimer.setSingleShot(true);
            connect(&waitTimer, &QTimer::timeout, &waitLoop, &QEventLoop::quit);
            waitTimer.start(2000);
            waitLoop.exec();

            connect(udpReceiver, &udp_um_receiver::device_found,this, &MainWindow::updateConnectionStatus, Qt::UniqueConnection);
            udpSender->send_search_packet();

            QEventLoop connectionLoop;
            QTimer connectionTimer;
            connectionTimer.setSingleShot(true);
            connect(&connectionTimer, &QTimer::timeout, &connectionLoop, &QEventLoop::quit);
            connect(udpReceiver, &udp_um_receiver::device_found, &connectionLoop, &QEventLoop::quit, Qt::UniqueConnection);

                connectionTimer.start(5000);
                connectionLoop.exec();

                disconnect(udpReceiver, &udp_um_receiver::device_found, &connectionLoop, &QEventLoop::quit);

        if (!ethernetConnected){
            logHtml("<font color='red'>Соединение с платой не установлено!</font><br>");
            closeTest();
        }

        udpSender->exec_cmd(um_alg_cmd::test);     
        conn_data = connect(udpReceiver, &udp_um_receiver::alg_cmd_executed, this, [this](um_alg_cmd cmd, um_status status){check_mode_acm(cmd, status); }, Qt::UniqueConnection);
        set_param(25);
        disconnect(conn_data);

        conn_rec = connect(udpReceiver, &udp_um_receiver::vector_received,this, &MainWindow::on_um_vector_received, Qt::UniqueConnection);

        conn_data = connect(udpReceiver, &udp_um_receiver::data_ready, this, [this](std::shared_ptr<um_data> data) {

                    float temperature = data->temperature;
                    logHtml(QString("<font color='blue'>Температура: %1 °C</font>").arg(temperature));

                    if (temperature >= 23 && temperature <= 27) { // вернуть правильную погрешность +-1.5
                        logHtml("<font color='green'>Температура в норме.</font><br>");

                        if (sendTimer) sendTimer->start(300);
                    } else {
                        logHtml("<font color='red'>Температура вне диапазона (25±1 °C)! Тест остановлен.</font><br>");
                        closeTest();
                    }
                disconnect(conn_data);
                });
          }
        return;

    case 25:
        peltie();
        set_param(25);
        return;

    case 26: {
        QByteArray rawData(reinterpret_cast<const char*>(parser.buffer), parser.buffer_length);
        if (parser.buffer_length < 201) {
            logHtml("<font color='red'>Недостаточно данных для построения графика</font><br>");
            return; }
        plotAdcData(rawData, ui->customPlot_2);

        logHtml("<font color='green'>Снято 100 точек напряжений, построен график.</font><br>");

        sendTimer->stop();
        responseTimer->stop();
        CustomDialog dialog_2(this,"Ожидание", "Пауза","Ок"); // Добавить фунцию
        bool ok = dialog_2.execDialog();
        if (ok) {      logHtml("<font color='green'>Продолжение теста...</font><br>");        }      

        test_temp(30);
        return;
    }

    case 27:
        peltie();
        test_temp(-20);
        return;
    case 28:
        peltie();
        udpSender->exec_cmd(um_alg_cmd::stop);
        return;

    case 29:
        if (parser.buffer[5] == 0x01 && parser.buffer[6] == 0x02 && parser.buffer[7] == 0x02 && parser.buffer[8] == 0x00){
            logHtml("<font color='green'>Тестирование RS-232 пройдено, сообщение получено.</font><br>");
            return;
        }
        logHtml("<font color='red'>Тестирование RS-232 не пройдено!</font><br>");
        return;

    case 30: {
    /*case 30: //еще 9 измерений для GPS не забыть вернуть, сейчас только одно
    case 31:
    case 32:
    case 33:
    case 34:
    case 35:
    case 36:
    case 37:
    case 38:*/
        udpSender->exec_cmd(um_alg_cmd::test);
        set_param(25);

        bool gpsSuccess = false;
        int gpsCheckCount = 0;

        QMetaObject::Connection gpsConnection;
        bool shouldCheckGps = true;

        gpsConnection = connect(udpReceiver, &udp_um_receiver::data_ready, this, [this, &gpsSuccess, &gpsCheckCount, &shouldCheckGps](std::shared_ptr<um_data> data) {
                if (!shouldCheckGps) return;

                gpsCheckCount++;
                bool status = data->gpsGeoData.dataValid;
                uint8_t sec = data->gpsGeoData.sec;

                if (status && sec == 0x04) {
                    gpsSuccess = true;
                    shouldCheckGps = false;
                }
            });

        QEventLoop waitLoop;
        QTimer::singleShot(500, &waitLoop, &QEventLoop::quit);
        waitLoop.exec();

        disconnect(gpsConnection);

        if (gpsSuccess) {
            logHtml("<font color='green'>Тестирование GPS успешно пройдено</font><br>");
        } else {
            logHtml("<font color='red'>Тестирование GPS не пройдено!</font><br>");
        }

        udpSender->exec_cmd(um_alg_cmd::stop);

        sendTimer->start(300);
        return;
    }

    case 31:
    case 32:
        logHtml("<font color='green'>Выполнено!</font><br>");
        return;

    case 33: {
        QEventLoop waitPW;
        QTimer waitTimerPW;
        waitTimerPW.setSingleShot(true);
        connect(&waitTimerPW, &QTimer::timeout, &waitPW, &QEventLoop::quit);
        waitTimerPW.start(2000);
        waitPW.exec();
        logHtml("<font color='green'>Выполнено!</font><br>");
        return;
        }
    case 34:
        handleCaseCommon(11.5, 0.1666, "Питание платы,");
        return;

    case 35:
        sample = 0.15;
        data = (parser.buffer[2] << 8) | parser.buffer[1];
        tok = (float)data / 1000.0;
        tok = tok / (0.018 * 200);
        if (tok >= sample - 0.1 && tok <= sample + 0.1){
            logHtml(QString("<font color='green'>Измерено: %1 мА — Ток питания платы допустим</font><br>").arg(tok, 0, 'f', 3));
        }
        else {
           logHtml(QString("<font color='red'>Измерено: %1 мА — Ток питания платы не допустим</font><br>").arg(tok, 0, 'f', 3));
           closeTest();
           }

        sendTimer->stop();
        responseTimer->stop();
        CustomDialog dialog_3(this, "Проверка","Ожидание ответа от пользователя","Продолжить"); // Добавить фунцию
        if (dialog_3.execDialog()){
            logHtml("<font color='green'>Тестирование успешно пройдено!</font><br>");
        }
        closeTest();
    }
}

void MainWindow::set_param(float temp){

    udpSender->set_test_settings(std::make_shared<um_test_mode_settings>(um_test_mode_settings{
            .laserWfm = {
                .zeroLevel  = 0.0,
                .beginLevel = 120,
                .endLevel   = 240,
                .beginTime  = 0,
                .endTime    = 150
            },
            .workTemp = temp,
            .workLine = 90,
            .regParam = {
                .kp               = 0.1,
                .ki               = 0,
                .maxSetDiff       = 1,
                .lineToTempCoef   = 20,
                .switchToLineThr  = 15,
                .lineStableThr    = 0.1
            },
            .control = um_test_mode_control::only_temperature
        })
    );
}

void MainWindow::test_temp(float temp){
    logHtml(QString("Установить температуру %1 °C:</font>").arg(temp));
    set_param(temp);
    startAveragingMeasurements();
}

void MainWindow::startAveragingMeasurements()
{
    measurementCount = 0;
    sumTemperature = 0.0;
    sumPeltierCurrent = 0.0;
    averagingInProgress = true;

    if (sendTimer && sendTimer->isActive()) sendTimer->stop();

    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);
    connect(&timeoutTimer, &QTimer::timeout, &averagingloop, &QEventLoop::quit);
    timeoutTimer.start(2000);

    conn_data = connect(udpReceiver, &udp_um_receiver::data_ready, this,[this](std::shared_ptr<um_data> data) {
                if (!averagingInProgress) {
                    averagingloop.quit();
                    return;
                 }

                sumTemperature += data->temperature;
                sumPeltierCurrent += data->peltierCurrent;
                measurementCount++;

                if (measurementCount >= requiredMeasurements) {
                    averagingInProgress = false;
                    averagingloop.quit();
                }              

                disconnect(conn_data);

            });

            averagingloop.exec();

            if (measurementCount > 0) {
                    processAveragedResults();
                } else {
                    logHtml("<font color='red'>Нет данных для усреднения температуры!</font><br>");
                    closeTest();
                }
}

void MainWindow::processAveragedResults()
{
    if (measurementCount == 0) return;

    double avgTemp = sumTemperature / measurementCount;
    double avgCurrent = sumPeltierCurrent / measurementCount;

    logHtml(QString("<font color='blue'>Температура: %1 °C</font>").arg(avgTemp));
    logHtml(QString("<font color='blue'>Ток: %1 мА</font>").arg(avgCurrent));

    float minTemp = 23.0;
    float maxTemp = 27.5;

    if (avgTemp >= minTemp && avgTemp <= maxTemp) {
        logHtml("<font color='green'>Температура в допустимом диапазоне.</font><br>");
        if (sendTimer) sendTimer->start(200);
    } else {
        logHtml("<font color='red'>Температура вне допустимого диапазона! Тест остановлен.</font><br>");
        closeTest();
    }
}

void MainWindow::on_um_vector_received(um_vector_id id, std::vector<float> vector)
{
        QMutexLocker locker(&dataMutex);

        int n = vector.size();
        if ( n < 200 ){
            return;
        }
        QVector<double> y(n), x(n);

        for (int i = 0; i < n; ++i) {
            x[i] = i;
            y[i] = static_cast<double>(vector[i+1]);
        }

        lastX = x;

        switch (id) {
        case um_vector_id::nrm_ref:
                    lastRef = y;
                    break;
                case um_vector_id::nrm_anl:
                    lastAnl = y;
                    break;
        }

        dataUpdated = true;
}

void MainWindow::check_mode_acm(um_alg_cmd cmd, um_status status){
        switch((int)status){
        case 0:
            if((int)cmd == 2 && !acmModePrinted){
            logHtml("<font color='green'>На плате АЦМ установлен режим тестирования.</font><br>");
            acmModePrinted = true;
            }
            return;

        case 1:
            logHtml("<font color='red'>Некорректное состояние тестируемой платы.</font><br>");
            closeTest();

        case 3:
            logHtml("<font color='red'>При выполнении возникла внутренняя ошибка тестируемой платы!</font><br>");
            closeTest();

        case 4:
            logHtml("<font color='red'>Bad request, формат запроса на плату АЦМ некорректен.</font><br>");
            closeTest();

        default:
            logHtml("<font color='red'>Неизвестная ошибка!</font><br>");
            closeTest();
        }
}

void MainWindow::handleCaseCommon(double sample, double ratio, const QString& labelText)
{
    const double accuracy = 2; // 0.5
    uint16_t data = (parser.buffer[2] << 8) | parser.buffer[1]; ; // 10000
    double volts = (double)data / 1000.0;
    volts = volts / ratio;

    if (volts >= sample - accuracy && volts <= sample + accuracy) {
        logHtml(QString("<font color='green'>Измерено: %1 В — %2 напряжение допустимо</font><br><br>").arg(QString::number(volts, 'f', 3)).arg(labelText));
    } else {
        logHtml(QString("<font color='red'>Измерено: %1 В — %2 напряжение вне допустимого диапазона</font><br><br>").arg(QString::number(volts, 'f', 3)).arg(labelText));
        closeTest();
    }
}

void MainWindow::handleParsedPacket()
{
    responseTimer->stop();

        switch (parser.buffer[0]){
        case 0x00:
            result(parser.buffer);
            if (isTesting || emergencyStopTriggered) {
                sendTimer->start(300);
                return;
            }
        case 0x01:
            logHtml("<font color='red'>Ошибка выполнения команды (код ошибки: 01)</font><br>");
            if (isTesting) {
                closeTest();
                return; }
        case 0x02:
            logHtml("<font color='red'>Несуществующая команда (код ошибки: 02)</font><br>");
            closeTest();
            break;
        case 0x03:
           logHtml("<font color='red'>Превышено время выполнения команды (код ошибки: 03)</font><br>");
            closeTest();
            break;
        case 0x04:
            logHtml("<font color='red'>Ошибка размера данных команды (код ошибки: 04)</font><br>");
            closeTest();
            break;
        default:
            logHtml("<font color='red'>Неизвестная ошибка!</font><br>");
            closeTest();
            break;
        }
}

void MainWindow::closeTest(){
    logHtml("<font color='red'>Завершение тестирования...</font>");
    if (!emergencyStopTriggered) {
               emergencyStopTriggered = true;

               testPackets = {
                   {0x01, 0x00, 0x00, 0x00, 0x00, 0x00},
                   {0x01, 0x07, 0x00, 0x00, 0x00, 0x00},
                   {0x01, 0x03, 0x00, 0x00, 0x00, 0x00}
               };

               currentPacketIndex = 0;
               logHtml("<font color='orange'>Отключение питания...</font><br>");
               sendTimer->start(200);
               return;
           }
    stopTesting();
}

void MainWindow::handleParserError()
{
    logHtml("<font color='red'>Ошибка разбора пакета</font><br>");
    parser.state = protocol_parser::STATE_SYNC;
}

void MainWindow::stopTesting()
{
    if (sendTimer && sendTimer->isActive()) sendTimer->stop();
    if (responseTimer && responseTimer->isActive()) responseTimer->stop();
    if (plotTimer && plotTimer->isActive()) plotTimer->stop();


    disconnect(udpReceiver, &udp_um_receiver::device_found, this, &MainWindow::updateConnectionStatus);
    disconnect(udpReceiver, &udp_um_receiver::alg_cmd_executed, this, nullptr);
    disconnect(udpReceiver, &udp_um_receiver::data_ready, this, nullptr);
    disconnect(conn_data);
    disconnect(conn_rec);

    ethernetConnected = false;
    averagingInProgress = false; // флаг остановки измерения температуры

    averagingloop.quit();

    udpSender->exec_cmd(um_alg_cmd::stop); // остановка режима тестирования платы АЦМ

    // Очищаем данные
    testPackets.clear();
    isTesting = false;
    emergencyStopTriggered = false;
    currentPacketIndex = 0;

    ui->pushButton->setText("Начать тестирование");
    logHtml("<font color='green'>Режим тестирования платы АЦМ отключен.</font><br>");
    logHtml("<font color='orange'><br>Тестирование завершено</font><br>");
}

void MainWindow::on_cleabutt_clicked()
{
    ui->plainTextEdit->clear();
}
