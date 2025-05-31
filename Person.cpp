#include "Person.h"
#include <iostream>
using namespace std;

Person::Person() : Name("") {}

Person::Person(const QString &name) : Name(name) {}

void Person::CreateProfile(const QString &name)
{
    Name = name;
    timedSettings = TimedSettings();
    bolusSettings = BolusSettings();
    Logger::log("Profile '" + Name.toStdString() + "' created.");
}

void Person::UpdateProfile(const QString &name)
{
    Name = name;
    Logger::log("Profile '" + Name.toStdString() + "' updated.");
}

void Person::DisplayProfile() const
{
    Logger::log("Profile Name: " + Name.toStdString());
    timedSettings.Display();
    bolusSettings.Display();
}

void Person::Delete()
{
    Name = "";
    Logger::log("Profile deleted.");
}
