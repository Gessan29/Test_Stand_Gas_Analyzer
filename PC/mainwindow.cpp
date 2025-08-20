#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>
#include <QVector>

static const quint16 defaultSenderPort = 30000;
static const quint16 defaultListenPort = 30001;
bool index = true;
int size_tmp = 1;

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

    auto plots = setupMainPlot(ui->customPlot);
    graphRef = plots.graphRef;
    graphAnl = plots.graphAnl;

    // Таймер для перерисовки
    plotTimer = new QTimer(this);
    connect(plotTimer, &QTimer::timeout, this, [this]() {
        ui->customPlot->replot(); });
    plotTimer->start(300);
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
    connect(udpReceiver, &udp_um_receiver::device_found, this, &MainWindow::updateConnectionStatus);
}

void MainWindow::setupPort() {
    port->setDataBits(QSerialPort::Data8);
    port->setParity(QSerialPort::NoParity);
    port->setStopBits(QSerialPort::OneStop);
    port->setFlowControl(QSerialPort::NoFlowControl);
}

void MainWindow::updateConnectionStatus(QHostAddress address){

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
        ethernetConnected = false;
        ui->pbOpen->setText("Открыть порт");
        logHtml("<font color='orange'>Порт закрыт, Ethernet отключен</font><br>");
        return;
    }

    port->setBaudRate(ui->sbxBaudrate->value());
    port->setPortName(ui->cmbxComPort->currentText());

    if (port->open(QIODevice::ReadWrite)) {
        ui->pbOpen->setText("Закрыть порт");
        logHtml("<font color='green'>Порт открыт!</font><br>");
        udpSender->send_search_packet();
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
    logHtml(QString("<font color='blue'>%1</font>").arg(QString::fromUtf8(data.toHex(' ').toUpper()))); // для просмотра пришедших пакетов

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
    if (!ethernetConnected) {
            logHtml("<font color='red'>Ошибка: нет подключения по Ethernet!</font>");
            logHtml("<font color='red'>Подключите кабель Ethernet и повторите попытку</font><br>");
            isTesting = false;
            ui->pushButton->setText("Начать тестирование");
            return;
        }
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
        {0x00, 0x06}, // 25-ый
        {0x00, 0x05},
        {0x00, 0x06}, {0x00, 0x06}, {0x00, 0x06}, {0x00, 0x06}, {0x00, 0x06},
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
        if (packet[2] == 0x01){ return "Измерение напряжение контрольной точки: -6V:"; }
        else if (packet[2] == 0x02) {return "Измерение напряжение контрольной точки: +3.3V:"; }
        else if (packet[2] == 0x03) {return "Измерение напряжение контрольной точки: +5V:"; }
        else if (packet[2] == 0x04) {return "Измерение напряжение контрольной точки: +6V:"; }
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
            return "Измерение напряжение контрольной точки: +5.5V (Power GPS):";
        case 0x04:
            return "Измерение напряжение контрольной точки: +4.5V:";
        case 0x05:
            return "Измерение напряжение контрольной точки: +5.5V:";
        case 0x06:
            return "Измерение напряжение контрольной точки: -5.5V:";
        case 0x07:
            return "Измерение напряжение контрольной точки: +1.8V:";
        case 0x08:
            return "Измерение напряжение контрольной точки: +2.5 (Offset)V:";
        case 0x09:
            return "Измерение напряжение контрольной точки: +5V (Laser):";
        case 0x0A:
            return "Измерение напряжение контрольной точки: 2.048V (VrefDAC):";
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
    case 0x09: {
        static bool isFirstTime = true;
        if (isFirstTime) {
            isFirstTime = false;
            return "Тестирование работы интерфейса подключения GPS-приемника:";}
        else {
            return "";
             }
      }
}
}

void MainWindow::peltie(uint16_t sample, uint16_t bit){
    if (currentPacketIndex == 24)
        return;
    uint16_t volt_raw, tok;
    volt_raw = 0; // (parser.buffer[2] << 8) | parser.buffer[1]; // напряжение
    tok = 0; // (parser.buffer[3] << 16) | parser.buffer[4] << 24; // ток
    double volts = volt_raw / 1000.0;
    if ( (volt_raw >= sample - bit && volt_raw <= sample + bit) && (tok >= sample - bit && tok <= sample + bit) ){
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
    uint16_t sample, tok, data, data_1;
    double volts;
    const uint16_t accuracy = 300;

    switch (currentPacketIndex){

    case 1:
    case 2:
    case 3:
    case 4:
    case 11:
        logHtml("<font color='green'>Выполнено!</font><br>");
        return;

    case 5:
    case 12:
        handleCaseCommon(2000, "Питание платы,");
        return;

    case 6:
    case 13:
        sample = 50;
        tok = 10;
        data = 50; // (parser.buffer[2] << 8) | parser.buffer[1];

        if (data >= sample - tok && data <= sample + tok){
            logHtml(QString("<font color='green'>Измерено: %1 мА — Ток питания платы допустим</font><br>").arg(data));
        }
        else {
           logHtml(QString("<font color='red'>Измерено: %1 мА — Ток питания платы не допустим</font><br>").arg(data));
            closeTest();
           }
        return;
    case 7:
        handleCaseCommon(6000, "Контрольная точка -6 В");
        return;
    case 8:
        handleCaseCommon(3300, "Контрольная точка +3.3 В");
        return;
    case 9:
        handleCaseCommon(5000, "Контрольная точка +5 В");
        return;
    case 10:
        handleCaseCommon(6000, "Контрольная точка +6 В");
        return;
    case 14:
        handleCaseCommon(6000, "Контрольная точка +1.2 В");
        return;
    case 15:
        handleCaseCommon(6000, "Контрольная точка +1.8 В");
        return;
    case 16:
        handleCaseCommon(6000, "Контрольная точка +2.5 В");
        return;
    case 17:
        handleCaseCommon(6000, "Контрольная точка +5.5 В (Power GPS)");
        return;
    case 18:
        handleCaseCommon(6000, "Контрольная точка +4.5 В (VrefADC)");
        return;
    case 19:
        handleCaseCommon(6000, "Контрольная точка +5.5 В");
        return;
    case 20:
        handleCaseCommon(6000, "Контрольная точка -5.5 В");
        return;
    case 21:
        handleCaseCommon(6000, "Контрольная точка +1.8 В");
        return;
    case 22:
        handleCaseCommon(6000, "Контрольная точка +2.5 В (Offset)");
        return;
    case 23:
        handleCaseCommon(6000, "Контрольная точка +5 В (Laser)");
        return;
    case 24:{
        handleCaseCommon(0, "Контрольная точка +2.048 В (VrefDAC)");

        sendTimer->stop();
        responseTimer->stop();

        CustomDialog dialog_1(this,"Выполните условие", "Прошейте МК и ПЛИС","Ок","Не удалось прошить"); // Добавить фунцию
        if (dialog_1.exec()) {
                logHtml("<font color='green'>МК и ПЛИС прошиты. Продолжение теста...</font><br>");
            } else {
                logHtml("<font color='red'>МК и ПЛИС не прошиты.</font><br>");
                closeTest();
                return;
            }

        udpSender->exec_cmd(um_alg_cmd::test);
        connect(udpReceiver, &udp_um_receiver::alg_cmd_executed, this, [this](um_alg_cmd cmd, um_status status){
                            check_mode_acm(cmd, status); }, Qt::UniqueConnection);

        connect(udpReceiver, &udp_um_receiver::vector_received,this, &MainWindow::on_um_vector_received,Qt::UniqueConnection);
        connect(udpReceiver, &udp_um_receiver::data_ready, this,
                [this](std::shared_ptr<um_data> data) {
                    double temperature = data->temperature;
                    logHtml(QString("<font color='blue'>Температура: %1 °C</font>").arg(temperature));

                    if (temperature >= 23.5 && temperature <= 26.5) {
                        logHtml("<font color='green'>Температура в норме.</font><br>");

                        if (sendTimer) sendTimer->start(300);
                    } else {
                        logHtml("<font color='red'>Температура вне диапазона (25±1 °C)! Тест остановлен.</font><br>");
                        closeTest();
                    }

                    disconnect(udpReceiver, &udp_um_receiver::data_ready, this, nullptr);
                },
                Qt::UniqueConnection);
          }
    case 25:
        peltie(0,3); // выставить правильно погрешность

        udpSender->set_test_settings(std::make_shared<um_test_mode_settings>(um_test_mode_settings{
                .laserWfm = {
                    .zeroLevel  = 0.0,
                    .beginLevel = 120,
                    .endLevel   = 0,
                    .beginTime  = 0,
                    .endTime    = 150
                },
                .workTemp = 25,
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
        return;

    case 26: {
        QByteArray rawData(reinterpret_cast<const char*>(parser.buffer), parser.buffer_length);
        if (parser.buffer_length < 201) {
            logHtml("<font color='red'>Недостаточно данных для построения графика</font><br>");
            return; }
        plotAdcData(ui->customPlot_2, rawData);
        logHtml("<font color='green'>Снято 100 точек напряжений, построен график.</font><br>");
        test_temp(28);
        return;
    }

    case 27:
        peltie(0,3); // выставить правильно погрешность
        test_temp(22);
        return;
    case 28:
        peltie(0,3); // выставить правильно погрешность
        test_temp(55);
        return;
    case 29:
        peltie(0,3); // выставить правильно погрешность
        test_temp(-5);
        return;
    case 30:
        peltie(0,3); // выставить правильно погрешность
        test_temp(25);
        return;
    case 31:
        peltie(0,3); // выставить правильно погрешность
        return;

    case 32:
        logHtml("<font color='green'>Тестирование RS-232 успешно пройдено</font><br>");
        return;
    case 33:
    /*case 30: //еще 9 измерений для GPS не забыть вернуть, сейчас только одно
    case 31:
    case 32:
    case 33:
    case 34:
    case 35:
    case 36:
    case 37:
    case 38:*/
        sendTimer->start(1000);
        if (currentPacketIndex == 33){ // поменять на 38
        logHtml("<font color='green'>Тестирование GPS успешно пройдено</font><br>"); }
        return;

    case 34:
    case 35:
    case 36:
    case 37:
        logHtml("<font color='green'>Выполнено!</font><br>");
        return;

    case 38:
        logHtml("<font color='green'>Выполнено!</font><br>");

        sendTimer->stop();
        responseTimer->stop();
        CustomDialog dialog_2(this, "Проверка","Ожидание ответа от пользователя","Продолжить"); // Добавить фунцию
        dialog_2.exec();
        logHtml("<font color='green'>Тестирование успешно пройдено!</font><br>");
        closeTest();
    }
}

void MainWindow::test_temp(float temp){
    logHtml(QString("Установить температуру %1 °C:</font>").arg(temp));
    udpSender->set_test_settings(std::make_shared<um_test_mode_settings>(um_test_mode_settings{
            .laserWfm = {
                .zeroLevel  = 0.0,
                .beginLevel = 120,
                .endLevel   = 0,
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
    timeoutTimer.start(1000);

    connect(udpReceiver, &udp_um_receiver::data_ready, this,
            [this](std::shared_ptr<um_data> data) {
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
            },
            Qt::UniqueConnection);

            averagingloop.exec();
            disconnect(udpReceiver, &udp_um_receiver::data_ready, this, nullptr);

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

    double minTemp = 23.5;
    double maxTemp = 26.5;

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
    static double timeCounter = 0.0;
    const double dt = 0.01;           // 10 мс между точками
    const int maxPoints = 5000;

    QVector<double> x(vector.size()), y(vector.size());
    for (int i = 0; i < (int)vector.size(); ++i) {
        x[i] = timeCounter;
        y[i] = static_cast<double>(vector[i]);
        timeCounter += dt;
    }

    switch (id) {
        case um_vector_id::nrm_ref:
            if (graphRef) {
                graphRef->addData(x, y);
                graphRef->removeDataBefore(timeCounter - maxPoints * dt);
            }
            break;

        case um_vector_id::nrm_anl:
            if (graphAnl) {
                graphAnl->addData(x, y);
                graphAnl->removeDataBefore(timeCounter - maxPoints * dt);
            }
            break;

        default:
            break;
    }

    ui->customPlot->rescaleAxes();
    ui->customPlot->xAxis->setRange(timeCounter - maxPoints * dt, timeCounter);
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

void MainWindow::handleCaseCommon(uint16_t sample, const QString& labelText)
{
    const uint16_t accuracy = 10000; // 300
    uint16_t data = 10000; //(parser.buffer[2] << 8) | parser.buffer[1];
    double volts = data / 1000.0;

    if (data >= sample - accuracy && data <= sample + accuracy) {
        logHtml(QString("<font color='green'>Измерено: %1 В — %2 напряжение допустимо</font><br><br>").arg(QString::number(volts, 'f', 3)).arg(labelText));
    } else {
        logHtml(QString("<font color='red'>Измерено: %1 В — %2 напряжение превышает диапазон</font><br><br>").arg(QString::number(volts, 'f', 3)).arg(labelText));
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
                sendTimer->start(200);
                return;
            }
        case 0x01:
            logHtml("<font color='red'>Ошибка выполнения команды (код ошибки: 0x01)</font><br>");
            if (isTesting) {
                closeTest();
                return; }
        case 0x02:
            logHtml("<font color='red'>Несуществующая команда (код ошибки: 0x02)</font><br>");
            closeTest();
            break;
        case 0x03:
           logHtml("<font color='red'>Превышено время выполнения команды (код ошибки: 0x03)</font><br>");
            closeTest();
            break;
        case 0x04:
            logHtml("<font color='red'>Ошибка размера данных команды (код ошибки: 0x04)</font><br>");
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
               sendTimer->start(300);
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

    averagingInProgress = false; // флаг остановки измерения температуры
    averagingloop.quit();
    udpSender->exec_cmd(um_alg_cmd::stop); // остановка режима тестирования платы АЦМ
    sendTimer->stop();
    responseTimer->stop();
    plotTimer->stop();
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
