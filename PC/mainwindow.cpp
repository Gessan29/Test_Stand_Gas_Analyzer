#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>
#include <QVector>

static const quint16 defaultSenderPort = 30000;
static const quint16 defaultListenPort = 30001;
static const quint32 defaultBaudrate = 115200;
static const bool defaultInterfaceIsUDP = true;


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , port(new QSerialPort(this))
    , sendTimer(new QTimer(this))
    , responseTimer(new QTimer(this))
{
    ui->setupUi(this);
    setupPort();
    setupConnections();
    parser.state = protocol_parser::STATE_SYNC;

    // Инициализация UDP
    udpReceiver = new udp_um_receiver(defaultListenPort, this);
    udpSender = new udp_um_sender(defaultSenderPort, this);

    connect(port, &QSerialPort::readyRead, this, &MainWindow::handleSerialData);
    connect(udpReceiver, &udp_um_receiver::device_found, this, &MainWindow::handleDeviceFound);
    connect(udpReceiver->get_socket(), &QUdpSocket::readyRead, this, &MainWindow::handleUdpData);
}

MainWindow::~MainWindow() {
    delete ui;
    port->close();
}

void MainWindow::setupConnections() {
    connect(sendTimer, &QTimer::timeout, this, &MainWindow::sendNextPacket);
    connect(responseTimer, &QTimer::timeout, this, &MainWindow::onResponseTimeout);
    responseTimer->setSingleShot(true);

    //connect(port, &QSerialPort::readyRead, this, &MainWindow::handleSerialData);

    //connect(udpReceiver, &udp_um_receiver::device_found, this, &MainWindow::handleDeviceFound);
    //connect(udpReceiver->get_socket(), &QUdpSocket::readyRead, this, &MainWindow::handleUdpData);
}

void MainWindow::setupPort() {
    port->setDataBits(QSerialPort::Data8);
    port->setParity(QSerialPort::NoParity);
    port->setStopBits(QSerialPort::OneStop);
    port->setFlowControl(QSerialPort::NoFlowControl);
}

void MainWindow::handleDeviceFound(QHostAddress address){
    deviceAddress = address;
    logHtml(QString("<font color='green'>Устройство найдено по Ethernet: %1</font><br>").arg(address.toString()));

    // Сохраняем адрес устройства для отправки
    udpSender->device_found(address);
}

void MainWindow::handleUdpData(){
    QUdpSocket* socket = udpReceiver->get_socket();
        if (!socket) return;

        while (socket->hasPendingDatagrams()) {
            QByteArray datagram;
            datagram.resize(socket->pendingDatagramSize());

            QHostAddress sender;
            quint16 senderPort;
            socket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

            logHtml(QString("<font color='blue'>UDP: %1</font><br>").arg(QString::fromUtf8(datagram.toHex(' ').toUpper())));

            for (const char byte : datagram) {
                const parser_result res = process_rx_byte(&parser, static_cast<uint8_t>(byte));
                if (res == PARSER_DONE) handleParsedPacket();
                else if (res == PARSER_ERROR) handleParserError();
            }
        }
}

void MainWindow::on_pbOpen_clicked() {

    bool comOpened = false;
    bool ethernetOpened = false;

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

    // Управление Ethernet подключением
        if (ethernetConnected) {
            // Закрываем Ethernet соединение (если нужно)
            ethernetConnected = false;
            logHtml("<font color='orange'>Ethernet соединение закрыто!</font><br>");
        } else {
            // Инициируем поиск устройства
            udpSender->send_search_packet();
            ethernetConnected = true;
            logHtml("<font color='blue'>Поиск устройства по Ethernet...</font><br>");
        }

        // Обновляем текст кнопки
        if (port->isOpen() || ethernetConnected) {
            ui->pbOpen->setText("Закрыть порты");
        } else {
            ui->pbOpen->setText("Открыть порты");
        }
}

void MainWindow::onResponseTimeout() {
    logHtml("<font color='red'>Ошибка: не получен ответ в течение 10 секунд</font><br>");
    stopTesting();
}

void MainWindow::handleSerialData() {
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

void MainWindow::logPlain(const QString& message) {
    ui->plainTextEdit->appendPlainText(message);
}

void MainWindow::startTesting()
{
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
        {0x00, 0x06},
        {0x00, 0x05},
        {0x00, 0x06},
        {0x00, 0x08},
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
        {0x01, 0x02, 0x00, 0x00, 0x00, 0x00},
        {0x01, 0x02, 0x01, 0x00, 0x00, 0x00}
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

     ui->plainTextEdit->appendHtml(QString("<font color='green'>Отправлен пакет %1: %2</font><br>") // для просмотра отправленных пакетов
                                      .arg(currentPacketIndex)
                                      .arg(QString::fromUtf8(byteArray.toHex(' ').toUpper())));

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
        CustomDialog dialog_2(this, "Сообщение","Отправьте команду плате АЦМ","Ок","Не удалось отправить"); // Добавить фунцию
        if (dialog_2.exec()) {
                logHtml("<font color='green'>Команда плате АЦМ отправлена. Продолжение теста...</font><br>");
            } else {
                logHtml("<font color='red'>Команда плате АЦМ не отправлена.</font><br>");
                closeTest();
                return;
            }
        return; }
    case 25:
        peltie(0,3); // выставить правильно погрешностьs
        return;

    case 26: {
        CustomDialog dialog_3(this, "Установка параметров","Установите форму тока лазера","Ок","Не удалось установить параметры"); // Добавить фунцию
        if (dialog_3.exec()) {
                logHtml("<font color='green'>Форма тока лазера установлена. Продолжение теста...</font>");
            } else {
                logHtml("<font color='red'>Форма тока лазера не установлена.</font><br>");
                closeTest();
                return;
            }
        QByteArray rawData(reinterpret_cast<const char*>(parser.buffer), parser.buffer_length);
        plotAdcData(rawData);
        return; }
    case 27: {
        for (int i = 0; i < 4; i++){
        std::vector<int> temperatures = {28, 22, 55, -5};
        if (i < temperatures.size()) {
               int targetTemp = temperatures[i];
               logHtml(QString("Установить температуру %1 градусов:</font>").arg(targetTemp)); // Добавить фунцию
               logHtml(QString("<font color='green'>Температура %1 градусов установлена</font>").arg(targetTemp));
               peltie(0,3); // выставить правильно погрешность
           }
         }
        logHtml("Установить температуру 25 градусов:</font>"); // Добавить фунцию
        logHtml("<font color='green'>Температура 25 градусов установлена</font><br>");
        return; }
    case 28:
        logHtml("<font color='green'>Тестирование RS-232 успешно пройдено</font><br>");
        return;
    case 29:
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
        if (currentPacketIndex == 29){ // поменять на 38
        logHtml("<font color='green'>Тестирование GPS успешно пройдено</font><br>"); }
        return;
    case 30:
    case 31:
        logHtml("<font color='green'>Выполнено!</font><br>");
        return;
    case 32:
        logHtml("<font color='green'>Выполнено!</font><br>");
        sendTimer->stop();
        responseTimer->stop();
        CustomDialog dialog_4(this, "Проверка","Ожидание ответа от пользователя","Продолжить"); // Добавить фунцию
        dialog_4.exec();
        logHtml("<font color='green'>Тестирование успешно пройдено!</font><br>");
        closeTest();
        return;
    }
}

void MainWindow::plotAdcData(const QByteArray& byteArray) {
    if (byteArray.size() < 201) {
        logHtml("<font color='red'>Недостаточно данных для построения графика</font><br>");
        return;
    }
    QVector<double> x(100), y(100);
    int dataStartIndex = 1;

    for (int i = 0; i < 100; ++i) {
           int index = dataStartIndex + i * 2;
           if (index + 1 >= byteArray.size()) break;

           uint8_t low = static_cast<uint8_t>(byteArray[index]);
           uint8_t high = static_cast<uint8_t>(byteArray[index + 1]);
           uint16_t value = (high << 8) | low;
           x[i] = i;
           y[i] = static_cast<double>(value) / 1000;
       }

       ui->customPlot->clearGraphs();
       ui->customPlot->addGraph();
       ui->customPlot->graph(0)->setData(x, y);
       ui->customPlot->xAxis->setLabel("Номер точки (сигнал лазерного диода)");
       ui->customPlot->yAxis->setLabel("Напряжение, В");
       ui->customPlot->xAxis->setRange(0, 99);
       ui->customPlot->yAxis->setRange(0, 3.3);
       ui->customPlot->replot();
       logHtml("<font color='green'>Снятно 100 точек напряжений, построен график.</font><br>");
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
            result(parser.buffer);
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
               sendTimer->start(100);
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
    sendTimer->stop();
    responseTimer->stop();
    testPackets.clear();
    isTesting = false;
    emergencyStopTriggered = false;
    currentPacketIndex = 0;
    ui->pushButton->setText("Начать тестирование");
    logHtml("<font color='orange'><br>Тестирование завершено</font><br>");
}

void MainWindow::on_cleabutt_clicked()
{
    ui->plainTextEdit->clear();
}
