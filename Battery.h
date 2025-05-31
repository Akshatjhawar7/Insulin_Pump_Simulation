#ifndef BATTERY_H
#define BATTERY_H
#include "Logger.h"

#include <QObject>

class Battery : public QObject
{
    Q_OBJECT
public:
    explicit Battery(QObject *parent = nullptr);

    // Getter for current battery value
    int value() const;

    // NEW: Setter for battery value, e.g., to "recharge" to 100
    void setValue(int newValue);

signals:
    void batteryValueChanged(int newValue);
    void batteryLow50(int value);
    void batteryCritical10(int value);
    void batteryEmpty();

private slots:
    void decrease();

private:
    int m_value;
};

#endif // BATTERY_H
