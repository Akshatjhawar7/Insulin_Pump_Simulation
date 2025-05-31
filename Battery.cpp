#include "Battery.h"
#include <QTimer>

Battery::Battery(QObject *parent)
    : QObject(parent), m_value(100)
{
    // Decreases battery by 1 every 15 seconds
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Battery::decrease);
    timer->start(15000);
}

int Battery::value() const
{
    return m_value;
}

// NEW: setValue method (for recharging or forcibly changing battery)
void Battery::setValue(int newValue)
{
    // Keep battery in [0, 100]
    if (newValue < 0)
        newValue = 0;
    else if (newValue > 100)
        newValue = 100;

    m_value = newValue;

    // Emit batteryValueChanged signal
    emit batteryValueChanged(m_value);

    // Check boundary conditions
    if (m_value == 50)
        emit batteryLow50(m_value);
    if (m_value == 10)
        emit batteryCritical10(m_value);
    if (m_value == 0)
        emit batteryEmpty();
}

void Battery::decrease()
{
    if (m_value > 0)
        m_value--;
    else
        m_value = 0;

    emit batteryValueChanged(m_value);

    if (m_value == 50)
        emit batteryLow50(m_value);
    if (m_value == 10)
        emit batteryCritical10(m_value);
    if (m_value == 0)
        emit batteryEmpty();
}
