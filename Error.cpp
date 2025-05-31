#include "Error.h"
#include <iostream>

using namespace std;

vector<QString> Error::errorLogs;

void Error::logError(ErrorCode code, const QString &details) {
    QString errorMsg;
    switch (code) {
        case LOW_BATTERY:
            errorMsg = "Low battery detected. Please recharge the pump.";
            break;
        case LOW_INSULIN:
            errorMsg = "Low insulin level detected. Please refill insulin.";
            break;
        case CGM_DISCONNECT:
            errorMsg = "CGM disconnected. Please check the sensor and connection.";
            break;
        case OCCLUSION_ALERT:
            errorMsg = "Occlusion alert. Please check the infusion site for blockages.";
            break;
        case SHUT_DOWN:
            errorMsg = "Pump shutdown initiated. Please restart or contact support.";
            break;
        default:
            errorMsg = "Unknown error.";
            break;
    }

    if (!details.isEmpty()) {
        errorMsg += " Details: " + details;
    }

    errorLogs.push_back(errorMsg);

    cout << "ERROR: " << errorMsg.toStdString() << endl;

    if (isCritical(code)) {
        cout << "Critical error detected. Insulin delivery suspended." << endl;
    }
}

QString Error::getGuidance(ErrorCode code) {
    switch (code) {
        case LOW_BATTERY:
            return "Recharge the pump as soon as possible.";
        case LOW_INSULIN:
            return "Refill the insulin reservoir immediately.";
        case CGM_DISCONNECT:
            return "Reconnect or replace the CGM sensor.";
        case OCCLUSION_ALERT:
            return "Check the infusion site for blockages and reposition if necessary.";
        case SHUT_DOWN:
            return "Restart the pump. If the problem persists, contact support.";
        default:
            return "No guidance available.";
    }
}

bool Error::isCritical(ErrorCode code) {
    if (code == SHUT_DOWN || code == OCCLUSION_ALERT) {
        return true;
    }
    return false;
}

void Error::clearErrorLog() {
    errorLogs.clear();
    cout << "Error logs cleared." << endl;
}

vector<QString> Error::getErrorLogs() {
    return errorLogs;
}
