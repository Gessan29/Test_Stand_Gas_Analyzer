#include "mainwindow.h"

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

void MainWindow::check_current(){
    const float sample = 0.15;

    uint16_t data = (parser.buffer[2] << 8) | parser.buffer[1];
    float tok = (float)data / 1000.0;
    tok = tok / (0.018 * 200);
    if (tok >= sample - 0.1 && tok <= sample + 0.1){
        logHtml(QString("<font color='green'>Измерено: %1 мА — Ток питания платы допустим</font><br>").arg(tok, 0, 'f', 3));
    }
    else {
       logHtml(QString("<font color='red'>Измерено: %1 мА — Ток питания платы не допустим</font><br>").arg(tok, 0, 'f', 3));
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
