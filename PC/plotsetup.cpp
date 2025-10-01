#include "plotsetup.h"
#include "mainwindow.h"

PlotHandles setupMainPlot(QCustomPlot* plot, QCustomPlot* plot_2){
    PlotHandles handles;

        // Настройка графиков
    handles.graphRef = plot->addGraph();
    handles.graphRef->setPen(QPen(Qt::red));
    handles.graphRef->setName("nrm_ref");

    handles.graphAnl = plot->addGraph();
    handles.graphAnl->setPen(QPen(Qt::blue));
    handles.graphAnl->setName("nrm_anl");

    plot->xAxis->setLabel("Время, мкс");
    plot->yAxis->setLabel("Напряжение, В");

    plot->yAxis->setRange(0, 2);
    plot->xAxis->setRange(0, 180);

    // масштабирование и перетаскивание
    plot->setInteraction(QCP::iRangeDrag, true);
    plot->setInteraction(QCP::iRangeZoom, true);
    plot->axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);
    plot->axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);

    plot->replot();


    plot_2->xAxis->setLabel("Время, мс");
    plot_2->yAxis->setLabel("Напряжение, В");
    plot_2->xAxis->setRange(0, 99);
    plot_2->yAxis->setRange(0, 3.3);

    // масштабирование и перетаскивание
    plot_2->setInteraction(QCP::iRangeDrag, true);
    plot_2->setInteraction(QCP::iRangeZoom, true);
    plot_2->axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);
    plot_2->axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);

    return handles;
}

void plotAdcData(const QByteArray& byteArray, QCustomPlot* plot)
{
    QVector<double> x(100), y(100);
    int dataStartIndex = 1;

    for (int i = 0; i < 100; ++i) {
        int index = dataStartIndex + i * 2;
        if (index + 1 >= byteArray.size()) break;

        uint8_t low = static_cast<uint8_t>(byteArray[index]);
        uint8_t high = static_cast<uint8_t>(byteArray[index + 1]);
        uint16_t value = (high << 8) | low;

        y[i] = static_cast<double>(value) / 1000.0;
        x[i] = i;

    }

    if (plot->graphCount() == 0)
           plot->addGraph();

    plot->graph(0)->setData(x, y);
    plot->yAxis->rescale();
    plot->replot();
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
