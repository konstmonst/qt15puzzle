#pragma once
#include <QObject>
#include <QThread>
#include <vector>
#include "state.h"

/**
  * This Thread is used to run the solver.
  */
class SolverThread : public QThread
{
    Q_OBJECT
    std::vector<State>* _solution;
    State               _start;
    void                _solveAStar();
    bool                _enabled;

public slots:
    void startSolving(std::vector<State>* solution, State start);
    void setEnabled(bool value) { _enabled = value; }

signals:
    void solvingDone();
    void updateOpenedClosed(unsigned opened, unsigned closed);

protected:
    void run();

public:
    SolverThread(QObject *parent = 0);
    virtual ~SolverThread();
};

