#ifndef CONTROLIQ_H
#define CONTROLIQ_H
#include "Logger.h"

#include <QObject>
#include <QTimer>

class ControlIQ : public QObject
{
    Q_OBJECT
public:
    explicit ControlIQ(QObject *parent = nullptr);
    double currentBloodSugar() const;
    double predictedBloodSugar() const; // Now uses Calculator for prediction
    void startSimulation();
    void stopSimulation();
signals:
    // Emitted every 10 seconds with a new simulated CGM value.
    void newCGMReading(double currentBG);
    // Emitted with the new prediction using the Calculator function.
    void newPrediction(double predictedBG);
private slots:
    void updateCGM();
private:
    double m_currentBG;
    QTimer *m_timer;
};

#endif // CONTROLIQ_H
