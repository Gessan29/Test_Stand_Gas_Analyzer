#ifndef PLOTSETUP_H
#define PLOTSETUP_H
#include "qcustomplot.h"
struct PlotHandles {
    QCPGraph* graphRef = nullptr;
    QCPGraph* graphAnl = nullptr;
};

PlotHandles setupMainPlot(QCustomPlot* plot);

void plotAdcData(QCustomPlot* plot2, const QByteArray& byteArray);

#endif // PLOTSETUP_H
