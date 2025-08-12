#include "plotsetup.h"

PlotHandles setupMainPlot(QCustomPlot* plot){
    PlotHandles handles;

        // Настройка графиков
    handles.graphRef = plot->addGraph();
    handles.graphRef->setPen(QPen(Qt::red));
    handles.graphRef->setName("nrm_ref");

    handles.graphAnl = plot->addGraph();
    handles.graphAnl->setPen(QPen(Qt::blue));
    handles.graphAnl->setName("nrm_anl");

    plot->xAxis->setLabel("Время");
    plot->yAxis->setLabel("Напряжение, В");

    return handles;
}

void plotAdcData(QCustomPlot* plot, const QByteArray& byteArray)
{
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

    plot->clearGraphs();
    plot->addGraph();
    plot->graph(0)->setData(x, y);
    plot->xAxis->setLabel("Номер точки (сигнал лазерного диода)");
    plot->yAxis->setLabel("Напряжение, В");
    plot->xAxis->setRange(0, 99);
    plot->yAxis->setRange(0, 3.3);
    plot->replot();

}
