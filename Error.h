#ifndef ERROR_H
#define ERROR_H

#include <QString>
#include <vector>

class Error {
public:
    // Enumeration for error codes.
    enum ErrorCode {
        LOW_BATTERY,
        LOW_INSULIN,
        CGM_DISCONNECT,
        OCCLUSION_ALERT,
        SHUT_DOWN
    };
    static void logError(ErrorCode code, const QString &details = "");
    static QString getGuidance(ErrorCode code);
    static bool isCritical(ErrorCode code);
    static void clearErrorLog();
    static std::vector<QString> getErrorLogs();

private:
    static std::vector<QString> errorLogs;
};

#endif // ERROR_H
