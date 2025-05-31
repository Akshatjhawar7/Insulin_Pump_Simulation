#ifndef PERSON_H
#define PERSON_H

#include <QString>
#include "Logger.h"
#include "TimedSettings.h"
#include "BolusSettings.h"
#include "mainwindow.h"

class Person
{
public:
    Person();
    Person(const QString &name);

    void CreateProfile(const QString &name);
    void UpdateProfile(const QString &name);
    void DisplayProfile() const;
    void Delete();

    TimedSettings timedSettings;
    BolusSettings bolusSettings;

    QString Name;
};

#endif // PERSON_H
