#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QDateTime>
#include <QTimer>
#include "Person.h"
#include "RangeMonitor.h"
#include "Logger.h"

// Forward declarations
class QStackedWidget;
class QWidget;
class Battery;
class ControlIQ;
class Person;

namespace Ui {
class MainWindow;
}

// QtCharts forward declarations
namespace QtCharts {
class QChart;
class QLineSeries;
class QChartView;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    int   m_logSeconds = 0;

    QWidget* createExtendedBolusScreen();
    QStackedWidget *stackedWidget = nullptr;
    QVector<Person> m_profiles;
    int m_activeProfileIndex;
    QLabel *m_symbolLabel = nullptr;
    RangeMonitor m_rangeMonitor;

    // Layout for listing profiles
    QVBoxLayout *profilesListLayout = nullptr;
    void showUpdateDialog();

    // -------------------------------------------------------------------
    // NEW: Power On / Power Off and Battery Charge fields
    // -------------------------------------------------------------------
    QPushButton *powerOnButton  = nullptr;  // "Power On" button
    QPushButton *powerOffButton = nullptr;  // "Power Off" button
    QPushButton *rechargeButton = nullptr;  // "Recharge Battery" button
    QPushButton *refillInsulinButton = nullptr; // "Refill Insulin" button
    bool m_powerOn = false;
    void updateUIForPowerState();

    // -------------------------------------------------------------------
    // The OPTIONS button (referenced in mainwindow.cpp)
    // -------------------------------------------------------------------
    QPushButton *optionsButton = nullptr;

    // -------------------------------------------------------------------
    // For the Bolus screen (index 1)
    // -------------------------------------------------------------------
    QLineEdit   *bolusCarbsEdit   = nullptr;  // Enter Carbs
    QLineEdit   *bolusBGEdit      = nullptr;  // Enter BG
    QLabel      *bolusUnitsLabel  = nullptr;  // Display calculated units
    QPushButton *bolusCheckMark   = nullptr;  // Confirm final bolus
    QPushButton *bolusCancelButton = nullptr; // Cancel extended bolus
    QPushButton *bolusNavButton    = nullptr; // "BOLUS" button on Home screen

    // For the Personal Profiles screen (index 5)
    QWidget *personalProfilesScreen = nullptr;

    // For the Profile Details screen (index 9)
    QWidget *profileDetailsScreen      = nullptr;
    QLabel  *profileDetailsTimedLabel  = nullptr;
    QLabel  *profileDetailsBolusLabel  = nullptr;

    // For Bolus Settings screen (index 7)
    QLineEdit *bolusImmediateEdit  = nullptr;
    QLineEdit *bolusExtendedEdit   = nullptr;
    QLineEdit *bolusDurationEdit   = nullptr;
    QWidget   *extendedBolusScreen = nullptr;
    bool m_highGlucoseAlertShown;

    // -------------------------------------------------------------------
    // Battery, Pump, CGM references
    // -------------------------------------------------------------------
    Battery    *m_battery       = nullptr;
    ControlIQ  *m_controlIQ     = nullptr;
    // Chart-related members:
    QtCharts::QChart      *m_chart      = nullptr;
    QtCharts::QLineSeries *m_series     = nullptr;
    QtCharts::QChartView  *m_chartView  = nullptr;
    QWidget *m_graphContainer  = nullptr;
    bool m_cgmWaitPeriod       = false;
    bool m_cgmEnabled          = false;
    bool m_controlIQEnabled    = false;
    double m_currentTime       = 0.0;

    // -------------------------------------------------------------------
    // NEW: Pump simulation fields
    // -------------------------------------------------------------------
    double m_pumpInsulin = 300;  // Initial insulin in pump reservoir
    double m_IOB = 0;            // Initial Insulin On Board (IOB)
    QTimer *m_pumpTimer = nullptr; // Timer for pump infusion updates
    QLabel *insulinGaugeLabel = nullptr;
    QLabel *iobLabel = nullptr;
    bool m_insulinLowAlertShown = false; // Flag to avoid spamming "Low Insulin" alerts

    // -------------------------------------------------------------------
    // setSimulationVisible() used in mainwindow.cpp
    // -------------------------------------------------------------------
    void setSimulationVisible(bool visible);

    // -------------------------------------------------------------------
    // Helpers for the Personal Profiles UI and Bolus Screen
    // -------------------------------------------------------------------
    void updatePersonalProfilesUI();
    void updateBolusCalculation();
    void maybeOfferReduction(double &units);

    // Extended Bolus delivery fields (new additions)
    QTimer *m_extendedBolusTimer = nullptr; // Timer for extended bolus delivery
    int m_extendedBolusIterations = 0;       // Number of remaining iterations (updates)
    double m_extendedBolusPerUpdate = 0.0;     // Insulin to deliver each update (u)

    // -------------------------------------------------------------------
    // NEW: Pump infusion update function
    // -------------------------------------------------------------------
    void updatePumpInfusion();
};

#endif // MAINWINDOW_H
