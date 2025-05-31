#include "Helper.h"
#include "mainwindow.h"
#include "Person.h"
#include "TimedSettings.h"
#include "BolusSettings.h"
#include "Calculator.h"
#include "Battery.h"
#include "ControlIQ.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QRadioButton>
#include <QPushButton>
#include <QMessageBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QTimer>
#include <QDateTime>
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QValueAxis>
#include <QRandomGenerator>
#include <QDebug>

// All helper functions work on a MainWindow pointer and access its members.
// Because MainWindow declared Helper as a friend, these functions can access private members.

namespace Helper {

void updatePersonalProfilesUI(MainWindow *mw)
{
    // Clear the layout.
    QLayoutItem *child;
    while ((child = mw->profilesListLayout->takeAt(0)) != nullptr) {
        if (child->widget())
            child->widget()->deleteLater();
        delete child;
    }
    // For each profile, add a row.
    for (int i = 0; i < mw->m_profiles.size(); ++i) {
        QWidget *rowWidget = new QWidget();
        rowWidget->setStyleSheet("background-color: #4D4D4D;");
        QHBoxLayout *rowLayout = new QHBoxLayout(rowWidget);

        QLabel *nameLbl = new QLabel(mw->m_profiles[i].Name);
        nameLbl->setStyleSheet("color: #CC9900; font: bold 16px;");

        QRadioButton *activeRadio = new QRadioButton("Active");
        activeRadio->setChecked(i == mw->m_activeProfileIndex);
        activeRadio->setStyleSheet("color: #CC9900; font: bold 14px;");
        QObject::connect(activeRadio, &QRadioButton::toggled, [mw, i](bool checked) {
            if (checked)
                mw->m_activeProfileIndex = i;
        });

        QPushButton *viewButton = new QPushButton("View");
        viewButton->setStyleSheet("background-color: #CC9900; color: black; font: bold 14px; padding: 5px;");
        QObject::connect(viewButton, &QPushButton::clicked, [mw, i]() {
            mw->m_activeProfileIndex = i;
            if (mw->m_activeProfileIndex >= 0 && mw->m_activeProfileIndex < mw->m_profiles.size()) {
                TimedSettings ts = mw->m_profiles[mw->m_activeProfileIndex].timedSettings;
                mw->profileDetailsTimedLabel->setText(
                    "Timed Settings:\nBasal: " + QString::number(ts.basal_rate) +
                    " u/hr\nCorrection: " + QString::number(ts.correction_factor) +
                    " u/mM\nCarb Ratio: " + QString::number(ts.carb_ratio) +
                    " u/g\nTarget BG: " + QString::number(ts.target_bg) + " mM"
                );
                mw->profileDetailsBolusLabel->setText(
                    "Bolus Settings:\nImmediate: " +
                    QString::number(mw->m_profiles[mw->m_activeProfileIndex].bolusSettings.immediateDose) +
                    " u\nExtended: " +
                    QString::number(mw->m_profiles[mw->m_activeProfileIndex].bolusSettings.extendedDose) +
                    " u\nDuration: " +
                    QString::number(mw->m_profiles[mw->m_activeProfileIndex].bolusSettings.duration) + " hrs"
                );
            }
            mw->stackedWidget->setCurrentWidget(mw->profileDetailsScreen);
        });

        QPushButton *deleteButton = new QPushButton("Delete");
        deleteButton->setStyleSheet("background-color: red; color: white; font: bold 14px; padding: 5px;");
        QObject::connect(deleteButton, &QPushButton::clicked, [mw, i]() {
            if (QMessageBox::question(mw, "Delete Profile", "Are you sure you want to delete this profile?") == QMessageBox::Yes) {
                mw->m_profiles.removeAt(i);
                if (mw->m_activeProfileIndex >= mw->m_profiles.size())
                    mw->m_activeProfileIndex = mw->m_profiles.size() - 1;
                updatePersonalProfilesUI(mw);
            }
        });

        rowLayout->addWidget(nameLbl);
        rowLayout->addStretch();
        rowLayout->addWidget(activeRadio);
        rowLayout->addSpacing(10);
        rowLayout->addWidget(viewButton);
        rowLayout->addSpacing(10);
        rowLayout->addWidget(deleteButton);

        mw->profilesListLayout->addWidget(rowWidget);
    }
}

void showUpdateDialog(MainWindow *mw)
{
    if (mw->m_activeProfileIndex < 0 || mw->m_activeProfileIndex >= mw->m_profiles.size())
        return;

    QDialog dialog(mw);
    dialog.setWindowTitle("Update Profile Settings");
    QFormLayout *formLayout = new QFormLayout(&dialog);

    QLineEdit *basalEdit = new QLineEdit(&dialog);
    QLineEdit *correctionEdit = new QLineEdit(&dialog);
    QLineEdit *carbRatioEdit = new QLineEdit(&dialog);
    QLineEdit *targetBGEdit = new QLineEdit(&dialog);
    QLineEdit *immediateEdit = new QLineEdit(&dialog);
    QLineEdit *extendedEdit = new QLineEdit(&dialog);
    QLineEdit *durationEdit = new QLineEdit(&dialog);

    TimedSettings ts = mw->m_profiles[mw->m_activeProfileIndex].timedSettings;
    basalEdit->setText(QString::number(ts.basal_rate));
    correctionEdit->setText(QString::number(ts.correction_factor));
    carbRatioEdit->setText(QString::number(ts.carb_ratio));
    targetBGEdit->setText(QString::number(ts.target_bg));

    BolusSettings bs = mw->m_profiles[mw->m_activeProfileIndex].bolusSettings;
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

    QObject::connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        double basal = basalEdit->text().toDouble();
        double correction = correctionEdit->text().toDouble();
        double carbRatio = carbRatioEdit->text().toDouble();
        double targetBG = targetBGEdit->text().toDouble();
        double immediate = immediateEdit->text().toDouble();
        double extended = extendedEdit->text().toDouble();
        double duration = durationEdit->text().toDouble();

        mw->m_profiles[mw->m_activeProfileIndex].timedSettings.Update(basal, correction, carbRatio, targetBG);
        mw->m_profiles[mw->m_activeProfileIndex].bolusSettings.Update(immediate, extended, duration);

        mw->profileDetailsTimedLabel->setText(
            "Timed Settings:\nBasal: " + QString::number(basal) +
            " u/hr\nCorrection: " + QString::number(correction) +
            " u/mM\nCarb Ratio: " + QString::number(carbRatio) +
            " u/g\nTarget BG: " + QString::number(targetBG) + " mM"
        );
        mw->profileDetailsBolusLabel->setText(
            "Bolus Settings:\nImmediate: " + QString::number(immediate) +
            " u\nExtended: " + QString::number(extended) +
            " u\nDuration: " + QString::number(duration) + " hrs"
        );
    }
}

QWidget* createExtendedBolusScreen(MainWindow *mw)
{
    QWidget *extScreen = new QWidget();
    extScreen->setStyleSheet("background-color: black;");
    QVBoxLayout *layout = new QVBoxLayout(extScreen);

    QLabel *title = new QLabel("Extended Bolus");
    title->setStyleSheet("color: #CC9900; font-size: 20px; font-weight: bold;");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    QFormLayout *form = new QFormLayout();
    QLineEdit *deliverNowEdit = new QLineEdit("60");
    deliverNowEdit->setStyleSheet("color: white; background-color: #333333; font: bold 16px;");
    deliverNowEdit->setAlignment(Qt::AlignCenter);
    QLineEdit *deliverLaterEdit = new QLineEdit("40");
    deliverLaterEdit->setStyleSheet("color: white; background-color: #333333; font: bold 16px;");
    deliverLaterEdit->setAlignment(Qt::AlignCenter);
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

    QObject::connect(cancelBtn, &QPushButton::clicked, [mw]() {
        mw->stackedWidget->setCurrentIndex(1);
    });
    QObject::connect(okBtn, &QPushButton::clicked, [mw, deliverNowEdit, deliverLaterEdit, durationEdit]() {
        double nowPercent = deliverNowEdit->text().toDouble();
        double laterPercent = deliverLaterEdit->text().toDouble();
        double dur = durationEdit->text().toDouble();
        double finalBolus = mw->bolusUnitsLabel->text().toDouble();

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
        QMessageBox::information(mw, "Extended Bolus", msg);

        mw->m_pumpInsulin -= res.immediateBolus;
        if (mw->m_pumpInsulin < 0)
            mw->m_pumpInsulin = 0;
        mw->m_IOB += res.immediateBolus;
        if (mw->insulinGaugeLabel)
            mw->insulinGaugeLabel->setText(QString::number(mw->m_pumpInsulin, 'f', 1) + " u remaining");
        if (mw->iobLabel)
            mw->iobLabel->setText("IOB: " + QString::number(mw->m_IOB, 'f', 1) + " u");

        mw->m_extendedBolusIterations = static_cast<int>(dur);
        mw->m_extendedBolusPerUpdate = res.bolusPerHour;
        if (!mw->m_extendedBolusTimer) {
            mw->m_extendedBolusTimer = new QTimer(mw);
            QObject::connect(mw->m_extendedBolusTimer, &QTimer::timeout, mw, [mw]() {
                if (mw->m_extendedBolusIterations > 0) {
                    mw->m_pumpInsulin -= mw->m_extendedBolusPerUpdate;
                    if (mw->m_pumpInsulin < 0)
                        mw->m_pumpInsulin = 0;
                    mw->m_IOB += mw->m_extendedBolusPerUpdate;
                    if (mw->insulinGaugeLabel)
                        mw->insulinGaugeLabel->setText(QString::number(mw->m_pumpInsulin, 'f', 1) + " u remaining");
                    if (mw->iobLabel)
                        mw->iobLabel->setText("IOB: " + QString::number(mw->m_IOB, 'f', 1) + " u");
                    mw->m_extendedBolusIterations--;
                } else {
                    mw->m_extendedBolusTimer->stop();
                }
            });
        }
        mw->m_extendedBolusTimer->start(20000);
        mw->bolusNavButton->setText(QString("Requesting %1 u Bolus").arg(res.bolusPerHour, 0, 'f', 2));
        mw->bolusCancelButton->setVisible(true);
        mw->stackedWidget->setCurrentWidget(0);
    });

    return extScreen;
}

void updateUIForPowerState(MainWindow *mw)
{
    if (mw->m_powerOn) {
        mw->powerOnButton->setEnabled(false);
        mw->powerOffButton->setEnabled(true);
        mw->optionsButton->setEnabled(true);
        mw->bolusNavButton->setEnabled(true);
    } else {
        mw->powerOnButton->setEnabled(true);
        mw->powerOffButton->setEnabled(false);
        mw->optionsButton->setEnabled(false);
        mw->bolusNavButton->setEnabled(false);
    }
}

void updatePumpInfusion(MainWindow *mw)
{
    if (!mw->m_powerOn || mw->m_profiles.isEmpty() || mw->m_activeProfileIndex < 0 ||
        mw->m_activeProfileIndex >= mw->m_profiles.size())
        return;

    double targetIOB = mw->m_profiles[mw->m_activeProfileIndex].timedSettings.target_bg;
    if(targetIOB <= 0)
        return;

    double upperThreshold = targetIOB * 1.10;

    // Infusion and absorption logic:
    if (mw->m_IOB >= targetIOB) {
        if (mw->m_IOB > upperThreshold) {
            if (!mw->m_highGlucoseAlertShown) {
                QMessageBox::warning(mw, "High Glucose Level", "High Glucose level detected! Please check your sensor.");
                mw->m_highGlucoseAlertShown = true;
            }
        }
        if (mw->m_IOB > (targetIOB * 0.50)) {
            mw->m_IOB -= 2.0;
        } else {
            if (mw->m_highGlucoseAlertShown) {
                QMessageBox::information(mw, "Resuming Delivery", "Normal delivery is resuming.");
                mw->m_highGlucoseAlertShown = false;
            }
            double basalRate = mw->m_profiles[mw->m_activeProfileIndex].timedSettings.basal_rate;
            mw->m_IOB += basalRate;
            mw->m_pumpInsulin -= basalRate;
            if (mw->m_pumpInsulin < 0)
                mw->m_pumpInsulin = 0;
        }
    } else {
        double basalRate = mw->m_profiles[mw->m_activeProfileIndex].timedSettings.basal_rate;
        mw->m_IOB += basalRate;
        mw->m_pumpInsulin -= basalRate;
        if (mw->m_pumpInsulin < 0)
            mw->m_pumpInsulin = 0;
    }

    if (QRandomGenerator::global()->bounded(1000) < 5) {
        mw->m_IOB *= 0.10;
        QMessageBox::critical(mw, "IOB Drop",
            "Unexpected IOB drop detected! Please administer a bolus shot immediately.");
    }

    if (mw->insulinGaugeLabel)
        mw->insulinGaugeLabel->setText(QString::number(mw->m_pumpInsulin, 'f', 1) + " u remaining");
    if (mw->iobLabel)
        mw->iobLabel->setText("IOB: " + QString::number(mw->m_IOB, 'f', 1) + " u");

    if (mw->m_pumpInsulin < 25 && !mw->m_insulinLowAlertShown) {
        QMessageBox::warning(mw, "Low Insulin", "Insulin in the reservoir is low. Please refill insulin.");
        mw->m_insulinLowAlertShown = true;
    }
}

void updateBolusCalculation(MainWindow *mw)
{
    double carbs = mw->bolusCarbsEdit->text().toDouble();
    double bg = mw->bolusBGEdit->text().toDouble();

    double targetBG = mw->m_profiles[mw->m_activeProfileIndex].timedSettings.target_bg;
    double correctionFactor = mw->m_profiles[mw->m_activeProfileIndex].timedSettings.correction_factor;
    double carbRatio = mw->m_profiles[mw->m_activeProfileIndex].timedSettings.carb_ratio;
    qDebug() << "[DEBUG] From active profile: Target BG =" << targetBG
             << ", Correction Factor =" << correctionFactor
             << ", Carb Ratio =" << carbRatio;

    double IOB = 5.0; // example value
    CalculationResult res = Calculator::calculateBolus(carbs, bg, targetBG, correctionFactor, carbRatio, IOB);
    double finalBolus = res.finalBolus;

    if (bg < targetBG && bg > 3.9) {
        maybeOfferReduction(mw, finalBolus);
    }
    mw->bolusUnitsLabel->setText(QString::number(finalBolus, 'f', 1));
}

void maybeOfferReduction(MainWindow *mw, double &units)
{
    double original = units;
    double reduced = units * 0.8;

    QDialog dialog(mw);
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

    QObject::connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        units = reduced;
    }
}

void setSimulationVisible(MainWindow *mw, bool visible)
{
    if (mw->m_graphContainer)
        mw->m_graphContainer->setVisible(visible);
}

} // namespace Helper
