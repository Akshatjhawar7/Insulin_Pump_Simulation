#ifndef INSULIN_H
#define INSULIN_H

class Insulin{
private:
    double insulinLevel;
    bool isPaused;
public:
    Insulin(int);
    void run(int);
    void pause();
};

#endif // INSULIN_H
