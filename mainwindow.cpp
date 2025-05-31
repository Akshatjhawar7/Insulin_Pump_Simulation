#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "TimedSettings.h"
#include "BolusSettings.h"
#include "Calculator.h"
#include "Battery.h"
#include "ControlIQ.h"

#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QLineEdit>
#include <QGridLayout>
#include <QMessageBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QFormLayout>
#include <QApplication>
#include <QTimer>
#include <QDateTime>

#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QValueAxis>

#include <QRandomGenerator>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <sstream>


using namespace std;
using namespace QtCharts;

//////////////////////////////////////////////////////////////////////////
// Helper function: updatePersonalProfilesUI
//////////////////////////////////////////////////////////////////////////
void MainWindow::updatePersonalProfilesUI()
{
    // Clear current layout
    QLayoutItem *child;
    while ((child = profilesListLayout->takeAt(0)) != nullptr) {
        if (child->widget())
            child->widget()->deleteLater();
        delete child;
    }
    // For each profile, add a row with name, View, and Delete buttons.
    for (int i = 0; i < m_profiles.size(); ++i) {
        QWidget *rowWidget = new QWidget();
        rowWidget->setStyleSheet("background-color: #4D4D4D;");
        QHBoxLayout *rowLayout = new QHBoxLayout(rowWidget);

        // Profile name
        QLabel *nameLbl = new QLabel(m_profiles[i].Name);
        nameLbl->setStyleSheet("color: #CC9900; font: bold 16px;");

        // Radio button to mark as active
        QRadioButton *activeRadio = new QRadioButton("Active");
        activeRadio->setChecked(i == m_activeProfileIndex);
        activeRadio->setStyleSheet("color: #CC9900; font: bold 14px;");
        connect(activeRadio, &QRadioButton::toggled, [this, i](bool checked) {
            if (checked)
                m_activeProfileIndex = i;
        });

        // View button – update profile details and switch screen.
        QPushButton *viewButton = new QPushButton("View");
        viewButton->setStyleSheet("background-color: #CC9900; color: black; font: bold 14px; padding: 5px;");
        connect(viewButton, &QPushButton::clicked, [this, i]() {
            m_activeProfileIndex = i;
            if (m_activeProfileIndex >= 0 && m_activeProfileIndex < m_profiles.size()){
                TimedSettings ts = m_profiles[m_activeProfileIndex].timedSettings;
                profileDetailsTimedLabel->setText(
                    "Timed Settings:\nBasal: " + QString::number(ts.basal_rate) +
                    " u/hr\nCorrection: " + QString::number(ts.correction_factor) +
                    " u/mM\nCarb Ratio: " + QString::number(ts.carb_ratio) +
                    " u/g\nTarget BG: " + QString::number(ts.target_bg) + " mM"
                );
                profileDetailsBolusLabel->setText(
                    "Bolus Settings:\nImmediate: " + QString::number(m_profiles[m_activeProfileIndex].bolusSettings.immediateDose) +
                    " u\nExtended: " + QString::number(m_profiles[m_activeProfileIndex].bolusSettings.extendedDose) +
                    " u\nDuration: " + QString::number(m_profiles[m_activeProfileIndex].bolusSettings.duration) + " hrs"
                );
            }
            stackedWidget->setCurrentWidget(profileDetailsScreen);
        });

        // Delete button
        QPushButton *deleteButton = new QPushButton("Delete");
        deleteButton->setStyleSheet("background-color: red; color: white; font: bold 14px; padding: 5px;");
        connect(deleteButton, &QPushButton::clicked, [this, i]() {
            if (QMessageBox::question(this, "Delete Profile",
                                      "Are you sure you want to delete this profile?") == QMessageBox::Yes) {
                m_profiles.removeAt(i);
                if (m_activeProfileIndex >= m_profiles.size())
                    m_activeProfileIndex = m_profiles.size() - 1;
                updatePersonalProfilesUI();
            }
        });

        rowLayout->addWidget(nameLbl);
        rowLayout->addStretch();
        rowLayout->addWidget(activeRadio);
        rowLayout->addSpacing(10);
        rowLayout->addWidget(viewButton);
        rowLayout->addSpacing(10);
        rowLayout->addWidget(deleteButton);

        profilesListLayout->addWidget(rowWidget);
    }
}

//////////////////////////////////////////////////////////////////////////
// Helper function: showUpdateDialog
//////////////////////////////////////////////////////////////////////////
void MainWindow::showUpdateDialog()
{
    if (m_activeProfileIndex < 0 || m_activeProfileIndex >= m_profiles.size())
        return;

    QDialog dialog(this);
    dialog.setWindowTitle("Update Profile Settings");

    QFormLayout *formLayout = new QFormLayout(&dialog);

    QLineEdit *basalEdit = new QLineEdit(&dialog);
    QLineEdit *correctionEdit = new QLineEdit(&dialog);
    QLineEdit *carbRatioEdit = new QLineEdit(&dialog);
    QLineEdit *targetBGEdit = new QLineEdit(&dialog);

    QLineEdit *immediateEdit = new QLineEdit(&dialog);
    QLineEdit *extendedEdit = new QLineEdit(&dialog);
    QLineEdit *durationEdit = new QLineEdit(&dialog);

    // Prepopulate fields
    TimedSettings ts = m_profiles[m_activeProfileIndex].timedSettings;
    basalEdit->setText(QString::number(ts.basal_rate));
    correctionEdit->setText(QString::number(ts.correction_factor));
    carbRatioEdit->setText(QString::number(ts.carb_ratio));
    targetBGEdit->setText(QString::number(ts.target_bg));

    BolusSettings bs = m_profiles[m_activeProfileIndex].bolusSettings;
    immediateEdit->setText(QString::number(bs.immediateDose));
    extendedEdit->setText(QString::number(bs.extendedDose));
    durationEdit->setText(QString::number(bs.duration));

    formLayout->addRow("Basal Rate (u/hr):", basalEdit);
    formLayout->addRow("Correction Factor (u/mM):", correctionEdit);
    formLayout->addRow("Carb Ratio (u/g):", carbRatioEdit);
    formLayout->addRow("Target BG (mM):", targetBGEdit);
    formLayout->addRow("Immediate Dose (u):", immediateEdit);
    formLayout->addRow("Extended Dose (u):", extendedEdit);
    formLayout->addRow("Duration (hrs):", durationEdit);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    formLayout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        double basal = basalEdit->text().toDouble();
        double correction = correctionEdit->text().toDouble();
        double carbRatio = carbRatioEdit->text().toDouble();
        double targetBG = targetBGEdit->text().toDouble();
        double immediate = immediateEdit->text().toDouble();
        double extended = extendedEdit->text().toDouble();
        double duration = durationEdit->text().toDouble();

        m_profiles[m_activeProfileIndex].timedSettings.Update(basal, correction, carbRatio, targetBG);
        m_profiles[m_activeProfileIndex].bolusSettings.Update(immediate, extended, duration);

        profileDetailsTimedLabel->setText(
            "Timed Settings:\nBasal: " + QString::number(basal) +
            " u/hr\nCorrection: " + QString::number(correction) +
            " u/mM\nCarb Ratio: " + QString::number(carbRatio) +
            " u/g\nTarget BG: " + QString::number(targetBG) + " mM"
        );
        profileDetailsBolusLabel->setText(
            "Bolus Settings:\nImmediate: " + QString::number(immediate) +
            " u\nExtended: " + QString::number(extended) +
            " u\nDuration: " + QString::number(duration) + " hrs"
        );
    }
}

//////////////////////////////////////////////////////////////////////////
// Extended Bolus Screen Creation
//////////////////////////////////////////////////////////////////////////
QWidget* MainWindow::createExtendedBolusScreen()
{
    QWidget *extScreen = new QWidget();
    extScreen->setStyleSheet("background-color: black;");
    QVBoxLayout *layout = new QVBoxLayout(extScreen);

    QLabel *title = new QLabel("Extended Bolus");
    title->setStyleSheet("color: #CC9900; font-size: 20px; font-weight: bold;");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    QFormLayout *form = new QFormLayout();
    // "Deliver Now (%)" represents the immediate portion percentage.
    QLineEdit *deliverNowEdit = new QLineEdit("60");
    deliverNowEdit->setStyleSheet("color: white; background-color: #333333; font: bold 16px;");
    deliverNowEdit->setAlignment(Qt::AlignCenter);
    // "Deliver Later (%)" represents the extended portion percentage.
    QLineEdit *deliverLaterEdit = new QLineEdit("40");
    deliverLaterEdit->setStyleSheet("color: white; background-color: #333333; font: bold 16px;");
    deliverLaterEdit->setAlignment(Qt::AlignCenter);
    // "Duration (updates)" is the number of extended update iterations (each update every 20 sec).
    QLineEdit *durationEdit = new QLineEdit("2");
    durationEdit->setStyleSheet("color: white; background-color: #333333; font: bold 16px;");
    durationEdit->setAlignment(Qt::AlignCenter);

    form->addRow("Deliver Now (%):", deliverNowEdit);
    form->addRow("Deliver Later (%):", deliverLaterEdit);
    form->addRow("Duration (updates):", durationEdit);
    layout->addLayout(form);

    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    QPushButton *cancelBtn = new QPushButton("Cancel");
    cancelBtn->setStyleSheet("background-color: #4D4D4D; color: white; font: bold 16px; padding: 10px;");
    QPushButton *okBtn = new QPushButton("OK");
    okBtn->setStyleSheet("background-color: #CC9900; color: black; font: bold 16px; padding: 10px;");
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(cancelBtn);
    buttonsLayout->addSpacing(20);
    buttonsLayout->addWidget(okBtn);
    buttonsLayout->addStretch();
    layout->addLayout(buttonsLayout);

    connect(cancelBtn, &QPushButton::clicked, [this]() {
        stackedWidget->setCurrentIndex(1);
    });
    connect(okBtn, &QPushButton::clicked, [this, deliverNowEdit, deliverLaterEdit, durationEdit]() {
        double nowPercent = deliverNowEdit->text().toDouble();
        double laterPercent = deliverLaterEdit->text().toDouble();
        double dur = durationEdit->text().toDouble(); // number of update iterations
        double finalBolus = bolusUnitsLabel->text().toDouble();

        // Calculate the extended bolus details using the Calculator.
        ExtendedBolusResult res = Calculator::calculateExtendedBolusCustom(finalBolus, nowPercent, laterPercent, dur);
        QString msg = QString("Extended Bolus:\n"
                              "Immediate: %1 u\n"
                              "Extended Total: %2 u\n"
                              "Duration: %3 updates\n"
                              "Per Update: %4 u")
                              .arg(res.immediateBolus, 0, 'f', 2)
                              .arg(res.extendedBolus, 0, 'f', 2)
                              .arg(static_cast<int>(dur))
                              .arg(res.bolusPerHour, 0, 'f', 2);
        QMessageBox::information(this, "Extended Bolus", msg);

        // Deliver the immediate portion.
        m_pumpInsulin -= res.immediateBolus;
        if (m_pumpInsulin < 0)
            m_pumpInsulin = 0;
        m_IOB += res.immediateBolus;
        if (insulinGaugeLabel)
            insulinGaugeLabel->setText(QString::number(m_pumpInsulin, 'f', 1) + " u remaining");
        if (iobLabel)
            iobLabel->setText("IOB: " + QString::number(m_IOB, 'f', 1) + " u");

        // Schedule extended bolus delivery over "dur" update iterations (every 20 sec).
        m_extendedBolusIterations = static_cast<int>(dur);
        m_extendedBolusPerUpdate = res.bolusPerHour;
        if (!m_extendedBolusTimer) {
            m_extendedBolusTimer = new QTimer(this);
            connect(m_extendedBolusTimer, &QTimer::timeout, this, [this]() {
                if (m_extendedBolusIterations > 0) {
                    // Deliver one extended update.
                    m_pumpInsulin -= m_extendedBolusPerUpdate;
                    if (m_pumpInsulin < 0)
                        m_pumpInsulin = 0;
                    m_IOB += m_extendedBolusPerUpdate;
                    if (insulinGaugeLabel)
                        insulinGaugeLabel->setText(QString::number(m_pumpInsulin, 'f', 1) + " u remaining");
                    if (iobLabel)
                        iobLabel->setText("IOB: " + QString::number(m_IOB, 'f', 1) + " u");
                    m_extendedBolusIterations--;
                } else {
                    m_extendedBolusTimer->stop();
                }
            });
        }
        m_extendedBolusTimer->start(20000); // every 20 seconds

        bolusNavButton->setText(QString("Requesting %1 u Bolus").arg(res.bolusPerHour, 0, 'f', 2));
        bolusCancelButton->setVisible(true);
        stackedWidget->setCurrentWidget(0);
    });

    return extScreen;
}

//////////////////////////////////////////////////////////////////////////
// Update UI for Power State
//////////////////////////////////////////////////////////////////////////
void MainWindow::updateUIForPowerState()
{
    if (m_powerOn) {
        powerOnButton->setEnabled(false);
        powerOffButton->setEnabled(true);
        // Enable feature buttons:
        optionsButton->setEnabled(true);
        bolusNavButton->setEnabled(true);
    } else {
        powerOnButton->setEnabled(true);
        powerOffButton->setEnabled(false);
        // Disable feature buttons:
        optionsButton->setEnabled(false);
        bolusNavButton->setEnabled(false);
    }
}

//////////////////////////////////////////////////////////////////////////
// Pump Infusion Update Function
//////////////////////////////////////////////////////////////////////////
void MainWindow::updatePumpInfusion()
{
    // Only run if pump is on and a profile is active.
    if (!m_powerOn
        || m_profiles.isEmpty()
        || m_activeProfileIndex < 0
        || m_activeProfileIndex >= m_profiles.size())
    {
        return;
    }

    // Retrieve the target (we use target BG as a proxy for target IOB)
    double targetIOB = m_profiles[m_activeProfileIndex].timedSettings.target_bg;
    if (targetIOB <= 0) {
        // Invalid profile target
        return;
    }

    // Get basal rate from the active profile
    double basalRate = m_profiles[m_activeProfileIndex].timedSettings.basal_rate;

    // Infusion and absorption logic:
    // If IOB is still high (>50% of target), simulate absorption
    if (m_IOB > targetIOB * 0.50) {
        m_IOB -= 2.0;  // simulate absorption: subtract 2 units per update
    }
    else {
        // If we had previously shown a high‑glucose alert, clear it and notify
        if (m_highGlucoseAlertShown) {
            QMessageBox::information(this, "Resuming Delivery",
                                     "Normal delivery is resuming.");
            m_highGlucoseAlertShown = false;
        }
        // Resume basal delivery
        m_IOB += basalRate;
        m_pumpInsulin -= basalRate;
        if (m_pumpInsulin < 0)
            m_pumpInsulin = 0;
    }

    // Check for sudden low IOB event.
    if (QRandomGenerator::global()->bounded(1000) < 5) {
        m_IOB *= 0.10;  // IOB suddenly drops to 10% of its current value.
        QMessageBox::critical(this, "IOB Drop",
                              "Unexpected IOB drop detected! Please administer a bolus shot immediately.");
    }

    // Update display labels.
    if (insulinGaugeLabel)
        insulinGaugeLabel->setText(QString::number(m_pumpInsulin, 'f', 1) + " u remaining");
    if (iobLabel)
        iobLabel->setText("IOB: " + QString::number(m_IOB, 'f', 1) + " u");

    // Low‑insulin reservoir warning
    if (m_pumpInsulin < 25 && !m_insulinLowAlertShown) {
        QMessageBox::warning(this, "Low Insulin",
                             "Insulin in the reservoir is low. Please refill insulin.");
        m_insulinLowAlertShown = true;
    }
}


//////////////////////////////////////////////////////////////////////////
// MainWindow Constructor
//////////////////////////////////////////////////////////////////////////
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      m_activeProfileIndex(-1),
      m_rangeMonitor()
{
    ui->setupUi(this);
    m_cgmWaitPeriod = false;

    // Create a new ControlIQ instance
    m_controlIQ = new ControlIQ(this);

    m_symbolLabel = new QLabel("Maintains", this);
    m_symbolLabel->setStyleSheet("color: #66CC66; font: bold 16px;");
    m_symbolLabel->setVisible(false);

    // -------------------- Set up the Chart --------------------
    // Use QSplineSeries for smooth curves
    m_chart = new QChart();
    m_chart->setTitle("Control IQ Prediction");
    // Create a QSplineSeries for a smooth curve graph.
    m_series = new QSplineSeries(this);
    m_series->setName("Continuous BG Curve");
    m_chart->addSeries(m_series);
    QValueAxis *axisX = new QValueAxis;
    axisX->setRange(0, 60);
    axisX->setTitleText("Time (s)");
    m_chart->addAxis(axisX, Qt::AlignBottom);
    m_series->attachAxis(axisX);
    QValueAxis *axisY = new QValueAxis;
    axisY->setRange(0, 15);
    axisY->setTitleText("Blood Sugar (mmol/L)");
    m_chart->addAxis(axisY, Qt::AlignLeft);
    m_series->attachAxis(axisY);
    m_chartView = new QChartView(m_chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_currentTime = 0;

    // -------------------- Initialize Pump Reservoir and IOB --------------------
    m_pumpInsulin = 300;
    m_IOB = 5;
    m_insulinLowAlertShown = false;
    m_highGlucoseAlertShown = false; // Initialize the high glucose alert flag

    // -------------------- Create the Stacked Widget and all Screens --------------------
    stackedWidget = new QStackedWidget(this);

    // ---------- HOME SCREEN (index 0) ----------
    QWidget *homeScreen = new QWidget();
    homeScreen->setStyleSheet("background-color: black;");
    QVBoxLayout *homeLayout = new QVBoxLayout(homeScreen);
    homeLayout->setContentsMargins(10, 10, 10, 10);
    // Top Bar
    QHBoxLayout *topBarLayout = new QHBoxLayout();
    QLabel *batteryLabel = new QLabel();
    batteryLabel->setStyleSheet("color: white; font: bold 14px;");
    m_battery = new Battery(this);
    batteryLabel->setText(QString::number(m_battery->value()) + "%");
    connect(m_battery, &Battery::batteryValueChanged, [batteryLabel](int newValue) {
        batteryLabel->setText(QString::number(newValue) + "%");
    });
    topBarLayout->addWidget(batteryLabel);
    // Insulin reservoir gauge label
    insulinGaugeLabel = new QLabel();
    insulinGaugeLabel->setStyleSheet("color: white; font: bold 14px;");
    insulinGaugeLabel->setAlignment(Qt::AlignRight);
    insulinGaugeLabel->setText(QString::number(m_pumpInsulin, 'f', 1) + " u remaining");
    topBarLayout->addWidget(insulinGaugeLabel);
    // IOB label
    iobLabel = new QLabel();
    iobLabel->setStyleSheet("color: white; font: bold 14px;");
    iobLabel->setAlignment(Qt::AlignRight);
    iobLabel->setText("IOB: " + QString::number(m_IOB, 'f', 1) + " u");
    topBarLayout->addWidget(iobLabel);
    topBarLayout->addStretch();
    QVBoxLayout *timeDateLayout = new QVBoxLayout();
    timeDateLayout->setAlignment(Qt::AlignCenter);
    QLabel *timeLabel = new QLabel();
    timeLabel->setStyleSheet("color: white; font: bold 16px;");
    timeLabel->setAlignment(Qt::AlignCenter);
    QLabel *dateLabel = new QLabel();
    dateLabel->setStyleSheet("color: white; font: 14px;");
    dateLabel->setAlignment(Qt::AlignCenter);
    QDateTime current = QDateTime::currentDateTime();
    timeLabel->setText(current.toString("h:mm AP"));
    dateLabel->setText(current.toString("dd MMM"));
    timeDateLayout->addWidget(timeLabel);
    timeDateLayout->addWidget(dateLabel);
    topBarLayout->addLayout(timeDateLayout);
    topBarLayout->addStretch();
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [timeLabel, dateLabel]() {
        QDateTime current = QDateTime::currentDateTime();
        timeLabel->setText(current.toString("h:mm AP"));
        dateLabel->setText(current.toString("dd MMM"));
    });
    timer->start(1000);
    homeLayout->addLayout(topBarLayout);
    // Symbol label layout
    QHBoxLayout *symbolLayout = new QHBoxLayout();
    symbolLayout->addStretch();
    symbolLayout->addWidget(m_symbolLabel);
    symbolLayout->addStretch();
    homeLayout->addLayout(symbolLayout);
    // Main Area: Graph and Info
    QHBoxLayout *mainAreaLayout = new QHBoxLayout();
    QWidget *graphContainer = new QWidget();
    QVBoxLayout *graphLayout = new QVBoxLayout(graphContainer);
    graphLayout->setContentsMargins(0, 0, 0, 0);
    graphLayout->addWidget(m_chartView);
    graphContainer->setMinimumSize(650, 500);
    // The graph is set to be visible.
    graphContainer->setVisible(true);
    mainAreaLayout->addWidget(graphContainer);
    m_graphContainer = graphContainer;
    QVBoxLayout *rightInfoLayout = new QVBoxLayout();
    rightInfoLayout->setAlignment(Qt::AlignCenter);
    mainAreaLayout->addLayout(rightInfoLayout);
    homeLayout->addLayout(mainAreaLayout);
    // Bottom Bar: Feature buttons and power controls.
    QHBoxLayout *bottomBarLayout = new QHBoxLayout();
    bottomBarLayout->setAlignment(Qt::AlignCenter);
    optionsButton = new QPushButton("OPTIONS");
    optionsButton->setStyleSheet("background-color: #CC9900; color: black; font-weight: bold; font-size: 18px; padding: 10px 20px;");
    bolusNavButton = new QPushButton("BOLUS");
    bolusNavButton->setStyleSheet("background-color: #0077CC; color: white; font-weight: bold; font-size: 18px; padding: 10px 20px;");
    bolusCancelButton = new QPushButton("X");
    bolusCancelButton->setStyleSheet("background-color: red; color: white; font-weight: bold; font-size: 18px; padding: 10px 20px;");
    bolusCancelButton->setVisible(false);
    bottomBarLayout->addWidget(optionsButton);
    bottomBarLayout->addSpacing(20);
    bottomBarLayout->addWidget(bolusNavButton);
    bottomBarLayout->addSpacing(10);
    bottomBarLayout->addWidget(bolusCancelButton);
    homeLayout->addLayout(bottomBarLayout);
    // New Buttons Row for power controls.
    QHBoxLayout *newButtonsLayout = new QHBoxLayout();
    newButtonsLayout->setAlignment(Qt::AlignCenter);
    powerOnButton = new QPushButton("Power On");
    powerOnButton->setStyleSheet("background-color: #009900; color: white; font-weight: bold; font-size: 16px; padding: 8px 16px;");
    powerOffButton = new QPushButton("Power Off");
    powerOffButton->setStyleSheet("background-color: #990000; color: white; font-weight: bold; font-size: 16px; padding: 8px 16px;");
    rechargeButton = new QPushButton("Recharge Battery");
    rechargeButton->setStyleSheet("background-color: #CCCC00; color: black; font-weight: bold; font-size: 16px; padding: 8px 16px;");
    refillInsulinButton = new QPushButton("Refill Insulin");
    refillInsulinButton->setStyleSheet("background-color: #CCCC00; color: black; font-weight: bold; font-size: 16px; padding: 8px 16px;");
    newButtonsLayout->addWidget(powerOnButton);
    newButtonsLayout->addSpacing(10);
    newButtonsLayout->addWidget(powerOffButton);
    newButtonsLayout->addSpacing(10);
    newButtonsLayout->addWidget(rechargeButton);
    newButtonsLayout->addSpacing(10);
    newButtonsLayout->addWidget(refillInsulinButton);
    homeLayout->addLayout(newButtonsLayout);
    // Initially disable feature buttons (pump off)
    optionsButton->setEnabled(false);
    bolusNavButton->setEnabled(false);
    powerOffButton->setEnabled(false);

    // Connect Power On, Power Off, Recharge Battery, and Refill Insulin buttons.
    connect(powerOnButton, &QPushButton::clicked, [this]() {
        m_powerOn = true;
        QMessageBox::information(this, "Power On", "Pump is now powered on.");
        Logger::log("Machine has been powered on");
        updateUIForPowerState();
        if (!m_pumpTimer) {
            m_pumpTimer = new QTimer(this);
            connect(m_pumpTimer, &QTimer::timeout, this, &MainWindow::updatePumpInfusion);
            m_pumpTimer->start(15000); // 15 sec simulate 2 hours
        }
    });
    connect(powerOffButton, &QPushButton::clicked, [this]() {
        m_powerOn = false;
        setSimulationVisible(false);
        QMessageBox::information(this, "Power Off", "Pump is now powered off.");
        Logger::log("Machine has been powered off");
        updateUIForPowerState();
    });
    connect(rechargeButton, &QPushButton::clicked, [this]() {
        m_battery->setValue(100);
        QMessageBox::information(this, "Battery Recharged", "Battery level is now 100%.");
    });
    connect(refillInsulinButton, &QPushButton::clicked, [this]() {
        m_pumpInsulin = 300;
        m_insulinLowAlertShown = false;
        if (insulinGaugeLabel)
            insulinGaugeLabel->setText(QString::number(m_pumpInsulin, 'f', 1) + " u remaining");
        QMessageBox::information(this, "Insulin Refilled", "Insulin reservoir refilled to 300 u.");
    });

    stackedWidget->addWidget(homeScreen); // index 0

    // ---------- BOLUS SCREEN (index 1) ----------
    QWidget *bolusScreen = new QWidget();
    bolusScreen->setStyleSheet("background-color: black;");
    QVBoxLayout *bolusLayout = new QVBoxLayout(bolusScreen);

    // Top Bar: Back arrow, bolus units, and check mark.
    QHBoxLayout *bolusTopBar = new QHBoxLayout();
    QPushButton *bolusBackArrow = new QPushButton("<-");
    bolusBackArrow->setStyleSheet("color: white; background-color: black; border: none; font-size: 20px; font-weight: bold;");
    bolusUnitsLabel = new QLabel("0");
    bolusUnitsLabel->setStyleSheet("color: #00FF00; font: bold 24px;");
    bolusUnitsLabel->setAlignment(Qt::AlignCenter);
    bolusCheckMark = new QPushButton("✓");
    bolusCheckMark->setStyleSheet("color: #00FF00; background-color: black; border: none; font-size: 24px; font-weight: bold;");
    bolusTopBar->addWidget(bolusBackArrow);
    bolusTopBar->addStretch();
    bolusTopBar->addWidget(bolusUnitsLabel);
    bolusTopBar->addStretch();
    bolusTopBar->addWidget(bolusCheckMark);
    bolusLayout->addLayout(bolusTopBar);

    // Middle Area: Input for Carbs and BG.
    QHBoxLayout *bolusMiddle = new QHBoxLayout();
    QVBoxLayout *carbsLayout = new QVBoxLayout();
    QLabel *carbsLabel = new QLabel("CARBS");
    carbsLabel->setStyleSheet("color: #CC9900; font: bold 16px;");
    carbsLabel->setAlignment(Qt::AlignCenter);
    bolusCarbsEdit = new QLineEdit();
    bolusCarbsEdit->setText("0");
    bolusCarbsEdit->setStyleSheet("color: white; background-color: #333333; font: bold 16px;");
    bolusCarbsEdit->setAlignment(Qt::AlignCenter);
    carbsLayout->addWidget(carbsLabel);
    carbsLayout->addWidget(bolusCarbsEdit);

    QVBoxLayout *bgLayout = new QVBoxLayout();
    QLabel *bgTitle = new QLabel("GLUCOSE");
    bgTitle->setStyleSheet("color: #CC9900; font: bold 16px;");
    bgTitle->setAlignment(Qt::AlignCenter);
    bolusBGEdit = new QLineEdit();
    bolusBGEdit->setText("0");
    bolusBGEdit->setStyleSheet("color: white; background-color: #333333; font: bold 16px;");
    bolusBGEdit->setAlignment(Qt::AlignCenter);
    bgLayout->addWidget(bgTitle);
    bgLayout->addWidget(bolusBGEdit);

    bolusMiddle->addLayout(carbsLayout);
    bolusMiddle->addSpacing(20);
    bolusMiddle->addLayout(bgLayout);
    bolusLayout->addLayout(bolusMiddle);

    QPushButton *viewCalcButton = new QPushButton("View Calculation");
    viewCalcButton->setStyleSheet("background-color: #4D4D4D; color: #CC9900; font: bold 16px; padding: 10px;");
    bolusLayout->addWidget(viewCalcButton);

    stackedWidget->addWidget(bolusScreen); // index 1

    // -------------------- OPTIONS SCREEN (index 2) --------------------
    QWidget *optionsScreen = new QWidget();
    optionsScreen->setStyleSheet("background-color: black;");
    QVBoxLayout *optionsLayout = new QVBoxLayout(optionsScreen);
    optionsLayout->setContentsMargins(10,10,10,10);
    QHBoxLayout *optionsTopBar = new QHBoxLayout();
    QPushButton *backArrowOptions = new QPushButton("<-");
    backArrowOptions->setStyleSheet("color: white; background-color: black; border: none; font-size: 20px; font-weight: bold;");
    QLabel *optionsTitle = new QLabel("Options");
    optionsTitle->setStyleSheet("color: #CC9900; font-size: 20px; font-weight: bold;");
    optionsTitle->setAlignment(Qt::AlignCenter);
    optionsTopBar->addWidget(backArrowOptions);
    optionsTopBar->addStretch();
    optionsTopBar->addWidget(optionsTitle);
    optionsTopBar->addStretch();
    optionsLayout->addLayout(optionsTopBar);

    QHBoxLayout *optionsMainLayout = new QHBoxLayout();
    QVBoxLayout *optionsListLayout = new QVBoxLayout();
    optionsListLayout->setSpacing(0);
    QPushButton *stopInsulinButton = new QPushButton("STOP INSULIN");
    stopInsulinButton->setStyleSheet("background-color: #4D4D4D; color: red; font: bold 16px; padding: 15px;");
    optionsListLayout->addWidget(stopInsulinButton);
    QPushButton *loadButton = new QPushButton("Load");
    loadButton->setStyleSheet("background-color: #4D4D4D; color: white; font: bold 16px; padding: 15px;");
    optionsListLayout->addWidget(loadButton);
    QPushButton *tempRateButton = new QPushButton("Temp Rate");
    tempRateButton->setStyleSheet("background-color: #4D4D4D; color: white; font: bold 16px; padding: 15px;");
    optionsListLayout->addWidget(tempRateButton);
    QPushButton *myPumpOptButton = new QPushButton("My Pump");
    myPumpOptButton->setStyleSheet("background-color: #4D4D4D; color: #CC9900; font: bold 16px; padding: 15px;");
    optionsListLayout->addWidget(myPumpOptButton);
    optionsMainLayout->addLayout(optionsListLayout);

    QVBoxLayout *optArrowLayout = new QVBoxLayout();
    QPushButton *optArrowUp = new QPushButton("▲");
    optArrowUp->setStyleSheet("background-color: #4D4D4D; color: white; font: bold 18px; padding: 10px;");
    QPushButton *optArrowDown = new QPushButton("▼");
    optArrowDown->setStyleSheet("background-color: #4D4D4D; color: white; font: bold 18px; padding: 10px;");
    optArrowLayout->addWidget(optArrowUp);
    optArrowLayout->addWidget(optArrowDown);
    optArrowLayout->addStretch();
    optionsMainLayout->addSpacing(10);
    optionsMainLayout->addLayout(optArrowLayout);
    optionsLayout->addSpacing(20);
    optionsLayout->addLayout(optionsMainLayout);
    optionsLayout->addStretch();

    stackedWidget->addWidget(optionsScreen); // index 2

    // -------------- MY PUMP SCREEN (index 3) ----------------
    QWidget *myPumpScreen = new QWidget();
    myPumpScreen->setStyleSheet("background-color: black;");
    QVBoxLayout *myPumpLayout = new QVBoxLayout(myPumpScreen);
    myPumpLayout->setContentsMargins(10,10,10,10);

    QHBoxLayout *myPumpTopBar = new QHBoxLayout();
    QPushButton *myPumpBackArrow = new QPushButton("<-");
    myPumpBackArrow->setStyleSheet("color: white; background-color: black; border: none; font-size: 20px; font-weight: bold;");
    connect(myPumpBackArrow, &QPushButton::clicked, [this]() { stackedWidget->setCurrentIndex(2); });
    QLabel *myPumpTitle = new QLabel("My Pump");
    myPumpTitle->setStyleSheet("color: #CC9900; font-size: 20px; font-weight: bold;");
    myPumpTitle->setAlignment(Qt::AlignCenter);
    myPumpTopBar->addWidget(myPumpBackArrow);
    myPumpTopBar->addStretch();
    myPumpTopBar->addWidget(myPumpTitle);
    myPumpTopBar->addStretch();
    myPumpLayout->addLayout(myPumpTopBar);

    QHBoxLayout *myPumpMainLayout = new QHBoxLayout();
    QVBoxLayout *myPumpOptionsLayout = new QVBoxLayout();
    myPumpOptionsLayout->setSpacing(0);
    QPushButton *personalProfilesButton = new QPushButton("Personal Profiles");
    personalProfilesButton->setStyleSheet("background-color: #4D4D4D; color: #CC9900; font: bold 16px; padding: 15px;");
    myPumpOptionsLayout->addWidget(personalProfilesButton);
    QPushButton *basalIQButton = new QPushButton("Basal-IQ");
    basalIQButton->setStyleSheet("background-color: #4D4D4D; color: #CC9900; font: bold 16px; padding: 15px;");
    myPumpOptionsLayout->addWidget(basalIQButton);
    QPushButton *alertsRemindersButton = new QPushButton("Alerts & Reminders");
    alertsRemindersButton->setStyleSheet("background-color: #4D4D4D; color: #CC9900; font: bold 16px; padding: 15px;");
    myPumpOptionsLayout->addWidget(alertsRemindersButton);
    QPushButton *pumpSettingsMPButton = new QPushButton("Pump Settings");
    pumpSettingsMPButton->setStyleSheet("background-color: #4D4D4D; color: #CC9900; font: bold 16px; padding: 15px;");
    myPumpOptionsLayout->addWidget(pumpSettingsMPButton);
    QCheckBox *cgmCheckBox = new QCheckBox("Enable CGM");
    cgmCheckBox->setStyleSheet("color: #CC9900; font: bold 16px;");
    QCheckBox *controlIQCheckBox = new QCheckBox("Enable Control IQ");
    controlIQCheckBox->setStyleSheet("color: #CC9900; font: bold 16px;");
    cgmCheckBox->setChecked(false);
    controlIQCheckBox->setChecked(false);
    controlIQCheckBox->setEnabled(false);
    myPumpOptionsLayout->addWidget(cgmCheckBox);
    myPumpOptionsLayout->addWidget(controlIQCheckBox);
    myPumpMainLayout->addLayout(myPumpOptionsLayout);
    QVBoxLayout *myPumpArrowsLayout = new QVBoxLayout();
    QPushButton *myPumpUpArrow = new QPushButton("▲");
    myPumpUpArrow->setStyleSheet("background-color: #4D4D4D; color: white; font: bold 18px; padding: 10px;");
    QPushButton *myPumpDownArrow = new QPushButton("▼");
    myPumpDownArrow->setStyleSheet("background-color: #4D4D4D; color: white; font: bold 18px; padding: 10px;");
    myPumpArrowsLayout->addWidget(myPumpUpArrow);
    myPumpArrowsLayout->addWidget(myPumpDownArrow);
    myPumpArrowsLayout->addStretch();
    myPumpMainLayout->addSpacing(10);
    myPumpMainLayout->addLayout(myPumpArrowsLayout);
    myPumpLayout->addSpacing(20);
    myPumpLayout->addLayout(myPumpMainLayout);
    myPumpLayout->addStretch();

    stackedWidget->addWidget(myPumpScreen); // index 3

    // -------------------- NEW PROFILE INPUT SCREEN (index 4) --------------------
    QWidget *newProfileScreen = new QWidget();
    newProfileScreen->setStyleSheet("background-color: black;");
    QVBoxLayout *newProfileLayout = new QVBoxLayout(newProfileScreen);
    newProfileLayout->setContentsMargins(10, 10, 10, 10);
    QLabel *newProfileLabel = new QLabel("Enter Profile Name:");
    newProfileLabel->setStyleSheet("color: #CC9900; font: bold 16px;");
    newProfileLabel->setAlignment(Qt::AlignCenter);
    newProfileLayout->addWidget(newProfileLabel);
    QLineEdit *profileNameEdit = new QLineEdit();
    profileNameEdit->setStyleSheet("color: white; background-color: #333333; font: bold 16px; padding: 5px;");
    newProfileLayout->addWidget(profileNameEdit);
    QPushButton *nextFromProfileInput = new QPushButton("Next");
    nextFromProfileInput->setStyleSheet("background-color: #CC9900; color: black; font: bold 16px; padding: 10px;");
    newProfileLayout->addWidget(nextFromProfileInput);
    QPushButton *backFromProfileInput = new QPushButton("Back to My Pump");
    backFromProfileInput->setStyleSheet("background-color: #4D4D4D; color: white; font: bold 16px; padding: 10px;");
    newProfileLayout->addWidget(backFromProfileInput);
    stackedWidget->addWidget(newProfileScreen); // index 4

    // -------------------- PERSONAL PROFILES SCREEN (index 5) --------------------
    personalProfilesScreen = new QWidget();
    personalProfilesScreen->setStyleSheet("background-color: black;");
    QVBoxLayout *ppLayout = new QVBoxLayout(personalProfilesScreen);
    ppLayout->setContentsMargins(10,10,10,10);
    QHBoxLayout *ppTopBar = new QHBoxLayout();
    QPushButton *ppBackArrow = new QPushButton("<-");
    ppBackArrow->setStyleSheet("color: white; background-color: black; border: none; font-size: 20px; font-weight: bold;");
    QLabel *ppTitle = new QLabel("Personal Profiles");
    ppTitle->setStyleSheet("color: #CC9900; font-size: 20px; font-weight: bold;");
    ppTitle->setAlignment(Qt::AlignCenter);
    QPushButton *ppPlusButton = new QPushButton("+");
    ppPlusButton->setStyleSheet("color: white; background-color: black; border: none; font-size: 24px; font-weight: bold;");
    ppTopBar->addWidget(ppBackArrow);
    ppTopBar->addStretch();
    ppTopBar->addWidget(ppTitle);
    ppTopBar->addStretch();
    ppTopBar->addWidget(ppPlusButton);
    ppLayout->addLayout(ppTopBar);
    profilesListLayout = new QVBoxLayout();
    profilesListLayout->setSpacing(0);
    ppLayout->addLayout(profilesListLayout);
    ppLayout->addStretch();
    stackedWidget->addWidget(personalProfilesScreen); // index 5

    // -------------------- TIMED SETTINGS SETUP SCREEN (index 6) --------------------
    QWidget *timedSetupScreen = new QWidget();
    timedSetupScreen->setStyleSheet("background-color: black;");
    QVBoxLayout *tsSetupLayout = new QVBoxLayout(timedSetupScreen);
    tsSetupLayout->setContentsMargins(10,10,10,10);
    QHBoxLayout *tsSetupTopBar = new QHBoxLayout();
    QPushButton *tsSetupBack = new QPushButton("<-");
    tsSetupBack->setStyleSheet("color: white; background-color: black; border: none; font-size: 20px; font-weight: bold;");
    QLabel *tsSetupTimeLabel = new QLabel("12:00 AM");
    tsSetupTimeLabel->setStyleSheet("color: white; font-size: 18px; font-weight: bold;");
    tsSetupTimeLabel->setAlignment(Qt::AlignCenter);
    QPushButton *tsSetupCheck = new QPushButton("✓");
    tsSetupCheck->setStyleSheet("color: #00FF00; background-color: black; border: none; font-size: 24px; font-weight: bold;");
    tsSetupTopBar->addWidget(tsSetupBack);
    tsSetupTopBar->addStretch();
    tsSetupTopBar->addWidget(tsSetupTimeLabel);
    tsSetupTopBar->addStretch();
    tsSetupTopBar->addWidget(tsSetupCheck);
    tsSetupLayout->addLayout(tsSetupTopBar);
    tsSetupLayout->addSpacing(20);
    QGridLayout *tsGrid = new QGridLayout();
    tsGrid->setHorizontalSpacing(15);
    tsGrid->setVerticalSpacing(10);
    QLabel *basalLabel = new QLabel("Basal:");
    basalLabel->setStyleSheet("color: #CC9900; font: bold 16px;");
    QLineEdit *basalEdit = new QLineEdit();
    basalEdit->setPlaceholderText("e.g. 0.715 u/hr");
    basalEdit->setStyleSheet("color: white;");
    QLabel *corrLabel = new QLabel("Correction Factor:");
    corrLabel->setStyleSheet("color: #CC9900; font: bold 16px;");
    QLineEdit *corrEdit = new QLineEdit();
    corrEdit->setPlaceholderText("e.g. 1u:2.8 mmol/L");
    corrEdit->setStyleSheet("color: white;");
    QLabel *carbLabel = new QLabel("Carb Ratio:");
    carbLabel->setStyleSheet("color: #CC9900; font: bold 16px;");
    QLineEdit *carbEdit = new QLineEdit();
    carbEdit->setPlaceholderText("e.g. 1u:10 g");
    carbEdit->setStyleSheet("color: white;");
    QLabel *targetLabel = new QLabel("Target BG:");
    targetLabel->setStyleSheet("color: #CC9900; font: bold 16px;");
    QLineEdit *targetEdit = new QLineEdit();
    targetEdit->setPlaceholderText("e.g. 6.7 mmol/L");
    targetEdit->setStyleSheet("color: white;");
    tsGrid->addWidget(basalLabel, 0, 0);
    tsGrid->addWidget(basalEdit, 0, 1);
    tsGrid->addWidget(corrLabel, 1, 0);
    tsGrid->addWidget(corrEdit, 1, 1);
    tsGrid->addWidget(carbLabel, 2, 0);
    tsGrid->addWidget(carbEdit, 2, 1);
    tsGrid->addWidget(targetLabel, 3, 0);
    tsGrid->addWidget(targetEdit, 3, 1);
    tsSetupLayout->addLayout(tsGrid);
    tsSetupLayout->addStretch();
    stackedWidget->addWidget(timedSetupScreen); // index 6

    // -------------------- CONFIRM SETTINGS SCREEN (index 8) --------------------
    QWidget *confirmScreen = new QWidget();
    confirmScreen->setStyleSheet("background-color: black;");
    QVBoxLayout *confirmLayout = new QVBoxLayout(confirmScreen);
    confirmLayout->setContentsMargins(10,10,10,10);
    QLabel *confirmTitle = new QLabel("Confirm Settings?");
    confirmTitle->setStyleSheet("color: white; font-size: 20px; font-weight: bold;");
    confirmTitle->setAlignment(Qt::AlignCenter);
    confirmLayout->addWidget(confirmTitle);
    QLabel *basalValLabel = new QLabel("Basal: 0.715 u/hr");
    QLabel *corrValLabel2 = new QLabel("Correction Factor: 1u:2.8 mmol/L");
    QLabel *carbValLabel = new QLabel("Carb Ratio: 1u:10 g");
    QLabel *targetValLabel = new QLabel("Target BG: 6.7 mmol/L");
    QString confirmStyle("color: #CC9900; font: bold 16px;");
    basalValLabel->setStyleSheet(confirmStyle);
    corrValLabel2->setStyleSheet(confirmStyle);
    carbValLabel->setStyleSheet(confirmStyle);
    targetValLabel->setStyleSheet(confirmStyle);
    confirmLayout->addSpacing(20);
    confirmLayout->addWidget(basalValLabel);
    confirmLayout->addWidget(corrValLabel2);
    confirmLayout->addWidget(carbValLabel);
    confirmLayout->addWidget(targetValLabel);
    confirmLayout->addSpacing(20);
    QHBoxLayout *confirmButtonsLayout = new QHBoxLayout();
    QPushButton *confirmNoButton = new QPushButton("X");
    confirmNoButton->setStyleSheet("color: white; background-color: #4D4D4D; font-size: 20px; font-weight: bold; padding: 10px 20px;");
    QPushButton *confirmYesButton = new QPushButton("✓");
    confirmYesButton->setStyleSheet("color: #00FF00; background-color: #4D4D4D; font-size: 20px; font-weight: bold; padding: 10px 20px;");
    confirmButtonsLayout->addStretch();
    confirmButtonsLayout->addWidget(confirmNoButton);
    confirmButtonsLayout->addSpacing(20);
    confirmButtonsLayout->addWidget(confirmYesButton);
    confirmButtonsLayout->addStretch();
    confirmLayout->addLayout(confirmButtonsLayout);
    confirmLayout->addStretch();
    stackedWidget->addWidget(confirmScreen); // index 8

    // ---------- BOLUS SETTINGS SCREEN (index 7) ----------
    QWidget *bolusSettingsScreen = new QWidget();
    bolusSettingsScreen->setStyleSheet("background-color: black;");
    QVBoxLayout *bolusSettingsLayout = new QVBoxLayout(bolusSettingsScreen);
    QLabel *bolusSettingsTitle = new QLabel("Bolus Settings");
    bolusSettingsTitle->setStyleSheet("color: #CC9900; font-size: 20px; font-weight: bold;");
    bolusSettingsTitle->setAlignment(Qt::AlignCenter);
    bolusSettingsLayout->addWidget(bolusSettingsTitle);
    bolusImmediateEdit = new QLineEdit();
    bolusImmediateEdit->setPlaceholderText("Immediate Dose (u)");
    bolusImmediateEdit->setStyleSheet("color: white;");
    bolusExtendedEdit = new QLineEdit();
    bolusExtendedEdit->setPlaceholderText("Extended Dose (u)");
    bolusExtendedEdit->setStyleSheet("color: white;");
    bolusDurationEdit = new QLineEdit();
    bolusDurationEdit->setPlaceholderText("Duration (hrs)");
    bolusDurationEdit->setStyleSheet("color: white;");
    bolusSettingsLayout->addWidget(bolusImmediateEdit);
    bolusSettingsLayout->addWidget(bolusExtendedEdit);
    bolusSettingsLayout->addWidget(bolusDurationEdit);
    QPushButton *bolusConfirmButton = new QPushButton("Confirm Bolus Settings");
    bolusConfirmButton->setStyleSheet("background-color: #CC9900; color: black; font: bold 16px; padding: 10px;");
    bolusSettingsLayout->addWidget(bolusConfirmButton);
    stackedWidget->insertWidget(7, bolusSettingsScreen); // index 7

    // Profile Details screen (index 9)
    QWidget *profileDetailsScreenLocal = new QWidget();
    profileDetailsScreenLocal->setStyleSheet("background-color: black;");
    QVBoxLayout *pdLayout = new QVBoxLayout(profileDetailsScreenLocal);
    pdLayout->setContentsMargins(10, 10, 10, 10);
    QHBoxLayout *pdTopBar = new QHBoxLayout();
    QLabel *pdTitle = new QLabel("Profile Details");
    pdTitle->setStyleSheet("color: #CC9900; font-size: 20px; font-weight: bold;");
    pdTitle->setAlignment(Qt::AlignCenter);
    QPushButton *updateButton = new QPushButton("Update");
    updateButton->setStyleSheet("background-color: #CC9900; color: black; font: bold 14px; padding: 5px;");
    pdTopBar->addWidget(pdTitle);
    pdTopBar->addStretch();
    pdTopBar->addWidget(updateButton);
    pdLayout->addLayout(pdTopBar);
    profileDetailsTimedLabel = new QLabel("Timed Settings: N/A");
    profileDetailsTimedLabel->setStyleSheet("color: #CC9900; font: bold 16px;");
    profileDetailsBolusLabel = new QLabel("Bolus Settings: N/A");
    profileDetailsBolusLabel->setStyleSheet("color: #CC9900; font: bold 16px;");
    pdLayout->addWidget(profileDetailsTimedLabel);
    pdLayout->addWidget(profileDetailsBolusLabel);
    QPushButton *pdBackButton = new QPushButton("Back");
    pdBackButton->setStyleSheet("background-color: #4D4D4D; color: white; font: bold 16px; padding: 10px;");
    pdLayout->addWidget(pdBackButton);
    profileDetailsScreen = profileDetailsScreenLocal;
    stackedWidget->addWidget(profileDetailsScreenLocal); // index 9

    setCentralWidget(stackedWidget);

    extendedBolusScreen = createExtendedBolusScreen();
    stackedWidget->addWidget(extendedBolusScreen);

    // -------------------- CONNECT NAVIGATION BUTTONS --------------------
    connect(bolusNavButton, &QPushButton::clicked, [this]() {
        stackedWidget->setCurrentIndex(1);
    });
    connect(optionsButton, &QPushButton::clicked, [this]() {
        stackedWidget->setCurrentIndex(2);
    });
    connect(bolusBackArrow, &QPushButton::clicked, [this]() {
        stackedWidget->setCurrentIndex(0);
    });
    connect(updateButton, &QPushButton::clicked, this, &MainWindow::showUpdateDialog);
    connect(cgmCheckBox, &QCheckBox::toggled, [this, controlIQCheckBox](bool checked){
        m_cgmEnabled = checked;
        if (!m_cgmEnabled) {
            m_controlIQEnabled = false;
            controlIQCheckBox->setChecked(false);
            controlIQCheckBox->setEnabled(false);
            m_controlIQ->stopSimulation();
            if (m_graphContainer) {
                m_graphContainer->setVisible(false);
            }
            m_symbolLabel->setVisible(false);
            m_cgmWaitPeriod = false;
        }
        else {
            controlIQCheckBox->setEnabled(true);
            m_controlIQ->startSimulation();
            m_cgmWaitPeriod = true;
            QMessageBox::information(this, "CGM Activated",
                                     "CGM is warming up.\nPlease wait 15 seconds before bolusing.");
            QTimer::singleShot(15000, this, [this]() {
                m_cgmWaitPeriod = false;
                QMessageBox::information(this, "CGM Ready",
                                         "CGM is now ready. Press BOLUS to auto-populate BG.");
            });
        }
    });
    connect(bolusNavButton, &QPushButton::clicked, [this]() {
        if (m_activeProfileIndex < 0 || m_activeProfileIndex >= m_profiles.size()) {
             QMessageBox::warning(this, "No Active Profile",
                                  "No active profile is activated. Please select a profile first.");
             stackedWidget->setCurrentWidget(0);
             return;
        }
        if (m_cgmEnabled && m_cgmWaitPeriod) {
             QMessageBox::warning(this, "CGM Warming Up",
                                  "Please wait for the CGM to finish its 15-second warm-up.");
             bolusBGEdit->setText("0");
        }
        else if (m_cgmEnabled && !m_cgmWaitPeriod) {
             double predictedBG = m_controlIQ->predictedBloodSugar();
             bolusBGEdit->setText(QString::number(predictedBG, 'f', 1));
        }
        else {
             bolusBGEdit->setText("0");
        }
        stackedWidget->setCurrentIndex(1);
    });
    connect(bolusCancelButton, &QPushButton::clicked, [this]() {
        bolusNavButton->setText("BOLUS");
        bolusCancelButton->setVisible(false);
        QMessageBox::information(this, "Extended Bolus", "Bolus delivery over extended period has been stopped.");
    });
    connect(controlIQCheckBox, &QCheckBox::toggled, [this](bool checked){
        m_controlIQEnabled = checked;
        bool showGraph = (m_cgmEnabled && m_controlIQEnabled);
        if (m_graphContainer) {
            m_graphContainer->setVisible(showGraph);
        }
        m_symbolLabel->setVisible(showGraph);
    });
    connect(bolusCarbsEdit, &QLineEdit::textChanged, [this]() { updateBolusCalculation(); });
    connect(bolusBGEdit, &QLineEdit::textChanged, [this]() { updateBolusCalculation(); });
    connect(viewCalcButton, &QPushButton::clicked, [this]() {
        updateBolusCalculation();
        QMessageBox::information(this, "Calculation",
            "Total Bolus = Food Bolus + Correction Bolus - IOB\n\n"
            "Food Bolus = Carbs / ICR\nCorrection Bolus = (Current BG - Target BG) / CF\n\n"
            "Carbs = " + bolusCarbsEdit->text() + "\n"
            "BG = " + bolusBGEdit->text() + "\n"
            "=> " + bolusUnitsLabel->text() + " units");
    });
    connect(bolusCheckMark, &QPushButton::clicked, [this]() {
        double displayedUnits = bolusUnitsLabel->text().toDouble();
        QString msg = QString("Carbs = %1\nBG = %2\nUnits = %3\n\nProceed?")
                          .arg(bolusCarbsEdit->text())
                          .arg(bolusBGEdit->text())
                          .arg(displayedUnits);
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Confirm Bolus");
        msgBox.setText(msg);
        QPushButton *yesBtn = msgBox.addButton("Yes", QMessageBox::YesRole);
        QPushButton *noBtn = msgBox.addButton("No", QMessageBox::NoRole);
        QPushButton *extendedBtn = msgBox.addButton("Extended", QMessageBox::ActionRole);
        msgBox.exec();
        if (msgBox.clickedButton() == yesBtn) {
            // Immediate bolus: subtract from reservoir and update IOB.
            m_pumpInsulin -= displayedUnits;
            if (m_pumpInsulin < 0)
                m_pumpInsulin = 0;
            m_IOB += displayedUnits;
            if (insulinGaugeLabel)
                insulinGaugeLabel->setText(QString::number(m_pumpInsulin, 'f', 1) + " u remaining");
            if (iobLabel)
                iobLabel->setText("IOB: " + QString::number(m_IOB, 'f', 1) + " u");
            QMessageBox::information(this, "Bolus", "Bolus delivered!");
            stackedWidget->setCurrentWidget(0);
        }
        else if (msgBox.clickedButton() == noBtn) {
            // Do nothing.
        }
        else if (msgBox.clickedButton() == extendedBtn) {
            stackedWidget->setCurrentWidget(extendedBolusScreen);
        }
    });
    connect(backArrowOptions, &QPushButton::clicked, [this]() { stackedWidget->setCurrentIndex(0); });
    connect(myPumpOptButton, &QPushButton::clicked, [this]() { stackedWidget->setCurrentIndex(3); });
    connect(myPumpBackArrow, &QPushButton::clicked, [this]() { stackedWidget->setCurrentIndex(2); });
    connect(personalProfilesButton, &QPushButton::clicked, [this]() {
        updatePersonalProfilesUI();
        stackedWidget->setCurrentWidget(personalProfilesScreen);
    });
    connect(pumpSettingsMPButton, &QPushButton::clicked, [this]() { stackedWidget->setCurrentIndex(7); });
    connect(nextFromProfileInput, &QPushButton::clicked, [this, profileNameEdit]() {
        QString enteredName = profileNameEdit->text();
        if (enteredName.isEmpty())
            enteredName = "Unnamed";
        Person newProfile;
        newProfile.CreateProfile(enteredName);
        m_profiles.append(newProfile);
        m_activeProfileIndex = m_profiles.size() - 1;
        stackedWidget->setCurrentIndex(6);
    });
    connect(backFromProfileInput, &QPushButton::clicked, [this]() { stackedWidget->setCurrentIndex(3); });
    connect(ppBackArrow, &QPushButton::clicked, [this]() { stackedWidget->setCurrentIndex(3); });
    connect(ppPlusButton, &QPushButton::clicked, [this]() { stackedWidget->setCurrentIndex(4); });
    connect(tsSetupBack, &QPushButton::clicked, [this]() { stackedWidget->setCurrentIndex(5); });
    connect(tsSetupCheck, &QPushButton::clicked, [this, basalEdit, corrEdit, carbEdit, targetEdit,
         basalValLabel, corrValLabel2, carbValLabel, targetValLabel]() {
            double basal = basalEdit->text().toDouble();
            double corr = corrEdit->text().toDouble();
            double carb = carbEdit->text().toDouble();
            double target = targetEdit->text().toDouble();
            if (m_activeProfileIndex >= 0 && m_activeProfileIndex < m_profiles.size()) {
                m_profiles[m_activeProfileIndex].timedSettings.Update(basal, corr, carb, target);
            }
            basalValLabel->setText("Basal: " + QString::number(basal) + " u/hr");
            corrValLabel2->setText("Correction Factor: " + QString::number(corr));
            carbValLabel->setText("Carb Ratio: " + QString::number(carb));
            targetValLabel->setText("Target BG: " + QString::number(target));
            stackedWidget->setCurrentIndex(7);
        });
    connect(confirmNoButton, &QPushButton::clicked, [this]() { stackedWidget->setCurrentIndex(6); });
    connect(confirmYesButton, &QPushButton::clicked, [this]() { stackedWidget->setCurrentIndex(7); });
    connect(bolusConfirmButton, &QPushButton::clicked, [this]() {
        if (m_activeProfileIndex >= 0 && m_activeProfileIndex < m_profiles.size()) {
            double imm = bolusImmediateEdit->text().toDouble();
            double ext = bolusExtendedEdit->text().toDouble();
            double dur = bolusDurationEdit->text().toDouble();
            m_profiles[m_activeProfileIndex].bolusSettings.Update(imm, ext, dur);
        }
        updatePersonalProfilesUI();
        stackedWidget->setCurrentIndex(5);
    });
    connect(pdBackButton, &QPushButton::clicked, [this]() { stackedWidget->setCurrentIndex(5); });

    connect(m_battery, &Battery::batteryLow50, this, [this](int value) {
        QMessageBox::warning(this, "Battery Warning", "Battery level is at 50%");
    });
    connect(m_battery, &Battery::batteryCritical10, this, [this](int value) {
        QMessageBox::critical(this, "Battery Critical", "Battery level is at 10%");
    });
    connect(m_battery, &Battery::batteryEmpty, this, [this]() {
        QMessageBox::critical(this, "Battery Empty", "Battery finished. Simulation will stop.");
        m_controlIQ->stopSimulation();
        qApp->quit();
    });

    connect(bolusCarbsEdit, &QLineEdit::textChanged, [this]() { updateBolusCalculation(); });
    connect(bolusBGEdit, &QLineEdit::textChanged, [this]() { updateBolusCalculation(); });

    // -------------------- Control IQ and Chart Update Integration --------------------
    connect(m_controlIQ, &ControlIQ::newCGMReading, [this](double currentBG) {
        if (!m_cgmEnabled || !m_controlIQEnabled)
            return;
        // Append new data points to the smooth spline series.
        m_series->append(m_currentTime, currentBG);
        if (m_symbolLabel) {
            m_rangeMonitor.updateSymbol(currentBG, m_symbolLabel);
        }
        // Use the Calculator for a smoothed predicted BG value.
        double predBG = Calculator::calculatePredictedBG(currentBG);
        m_series->append(m_currentTime + 10, predBG);
        m_currentTime += 10;
        QValueAxis *axisX = qobject_cast<QValueAxis*>(m_chart->axisX());
        if (axisX) {
            axisX->setRange(m_currentTime - 60, m_currentTime + 10);
        }
    });

    connect(m_controlIQ, &ControlIQ::newPrediction, [this](double predictedBG) {
        if (!m_cgmEnabled || !m_controlIQEnabled)
            return;

        // First, update your “Stops/Rises/etc” symbol using the RangeMonitor
        m_rangeMonitor.updateSymbol(predictedBG, m_symbolLabel);

        // Now only warn once if predicted BG climbs above 10 mmol/L
        if (predictedBG > 10.0) {
            if (!m_highGlucoseAlertShown) {
                QMessageBox::warning(this, "High Glucose Warning",
                                     "Predicted blood glucose is above 10 mmol/L — please take action.");
                m_highGlucoseAlertShown = true;
            }
        }
        else {
            // clear the flag once we fall back below threshold
            m_highGlucoseAlertShown = false;
        }
    });

}

MainWindow::~MainWindow()
{
    delete ui;
}

// ========== Bolus Screen Calculation Helpers ==========
void MainWindow::updateBolusCalculation()
{
    double carbs = bolusCarbsEdit->text().toDouble();
    double bg = bolusBGEdit->text().toDouble();

    double targetBG = m_profiles[m_activeProfileIndex].timedSettings.target_bg;
    double correctionFactor = m_profiles[m_activeProfileIndex].timedSettings.correction_factor;
    double carbRatio = m_profiles[m_activeProfileIndex].timedSettings.carb_ratio;
    cout << "[DEBUG] From active profile: Target BG = " << targetBG
         << ", Correction Factor = " << correctionFactor
         << ", Carb Ratio = " << carbRatio << endl;

    double IOB = m_IOB;

    CalculationResult res = Calculator::calculateBolus(carbs, bg, targetBG, correctionFactor, carbRatio, IOB);
    double finalBolus = res.finalBolus;

    if (bg < targetBG && bg > 3.9) {
        maybeOfferReduction(finalBolus);
    }
    bolusUnitsLabel->setText(QString::number(finalBolus, 'f', 1));
}

void MainWindow::maybeOfferReduction(double &units)
{
    double original = units;
    double reduced = units * 0.8;

    QDialog dialog(this);
    dialog.setWindowTitle("BG Below Target");
    QLabel *label = new QLabel(
        QString("Your BG is below the target.\nReduce bolus by 20%%?\n\nOriginal = %1\nReduced = %2")
            .arg(original, 0, 'f', 1)
            .arg(reduced, 0, 'f', 1),
        &dialog
    );
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Yes | QDialogButtonBox::No,
        Qt::Horizontal,
        &dialog
    );
    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    layout->addWidget(label);
    layout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        units = reduced;
    }
}

//////////////////////////////////////////////////////////////////////////
// setSimulationVisible() definition
//////////////////////////////////////////////////////////////////////////
void MainWindow::setSimulationVisible(bool visible)
{
    if (m_graphContainer)
        m_graphContainer->setVisible(visible);
}
