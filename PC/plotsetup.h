#ifndef PLOTSETUP_H
#define PLOTSETUP_H
#include "qcustomplot.h"
struct PlotHandles {
    QCPGraph* graphRef = nullptr;
    QCPGraph* graphAnl = nullptr;
};

PlotHandles setupMainPlot(QCustomPlot* plot, QCustomPlot* plot_2);

void plotAdcData(const QByteArray& byteArray);

#endif // PLOTSETUP_H
