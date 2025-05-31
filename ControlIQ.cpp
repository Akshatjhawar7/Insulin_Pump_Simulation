#include "ControlIQ.h"
#include "Calculator.h"
#include <QRandomGenerator>
#include <QTimer>
#include <iostream>

using namespace std;

ControlIQ::ControlIQ(QObject *parent)
    : QObject(parent), m_currentBG(5.0) // starting value, e.g., 5 mmol/L
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &ControlIQ::updateCGM);
    // No need to explicitly seed QRandomGenerator::global() as it is auto-seeded.
}

double ControlIQ::currentBloodSugar() const
{
    return m_currentBG;
}

double ControlIQ::predictedBloodSugar() const
{
    // Use the Calculator function to compute the predicted blood sugar.
    return Calculator::calculatePredictedBG(m_currentBG);
}

void ControlIQ::startSimulation()
{
    m_timer->start(10000); // update every 10 seconds
}

void ControlIQ::stopSimulation()
{
    m_timer->stop();
}

void ControlIQ::updateCGM()
{
    // Instead of an abrupt change, smoothly adjust the current BG value
    double newReading = QRandomGenerator::global()->bounded(1, 11);
    // Use a smoothing factor (e.g., 0.3) so that changes take about 30-35 seconds to complete
    m_currentBG += (newReading - m_currentBG) * 0.3;

    emit newCGMReading(m_currentBG);

    double pred = predictedBloodSugar();
    emit newPrediction(pred);

    Logger::log("[ControlIQ] New CGM reading: " + std::to_string(m_currentBG) + ", Predicted BG: " + std::to_string(pred));
}
