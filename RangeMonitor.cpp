// RangeMonitor.cpp

#include "RangeMonitor.h"
#include <QDebug>

RangeMonitor::RangeMonitor()
{}

void RangeMonitor::updateSymbol(double bg, QLabel *label)
{
    //  10.0 => "Delivers"  (blue color)
    //   8.9 => "Increases" (blue color)
    //   6.25 => "Maintains"
    //   3.9 => "Decreases"
    //   <3.9 => "Stops"

    QString text;
    QString style;

    if (bg >= 10.0) {
        text = "Delivers";
        style = "color: #00CCFF; font: bold 16px;";
    }
    else if (bg >= 8.9) {
        text = "Increases";
        style = "color: #00CCFF; font: bold 16px;";
    }
    else if (bg >= 6.25) {
        text = "Maintains";
        style = "color: #66CC66; font: bold 16px;";
    }
    else if (bg >= 3.9) {
        text = "Decreases";
        style = "color: #FFCC00; font: bold 16px;";
    }
    else {
        text = "Stops";
        style = "color: #FF0000; font: bold 16px;";
    }

    label->setText(text);
    label->setStyleSheet(style);

    qDebug() << "[RangeMonitor] BG=" << bg << " => " << text;
}
