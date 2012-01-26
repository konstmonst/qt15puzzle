#include "mainwindow.h"
#include <cstdlib>
#include <iostream>
using namespace std;

/**
  * MainWindow Constructor
  */
MainWindow::MainWindow()
        : QMainWindow()
{
    setWindowTitle(tr(APP_NAME));
    _wdgPuzzle = new PuzzleWidget(this);
    _wdgPuzzle->show();
    setCentralWidget(_wdgPuzzle);

    _createToolbars();
    _createStatusBar();

    _timNextPusher = new QTimer(this);
    QObject::connect(_timNextPusher, SIGNAL(timeout()), _actNext, SLOT(trigger()));
    _timNextPusher->setInterval(100);
    QObject::connect(_actAutoSolve, SIGNAL(triggered()), _timNextPusher, SLOT(start()));
    QObject::connect(_actAutoSolve, SIGNAL(triggered()), this, SLOT(solverBusy()));
    QObject::connect(_actStop, SIGNAL(triggered()), _timNextPusher, SLOT(stop()));

    _createMenus();

    connect(_wdgPuzzle, SIGNAL(startSolving(std::vector<State>*,State)), this, SLOT(solverBusy()));
    connect(_wdgPuzzle, SIGNAL(processingDone()), this, SLOT(solverDone()));

    setGeometry(200, 200, 100, 100);
    _actToggleSolver->toggle();
}

/**
  *	MainWindow Destructor
  */
MainWindow::~MainWindow()
{
}

/**
  *	Creates Menus
  */
void MainWindow::_createMenus()
{
    //------------[File]------------------------
    QMenu* menu = new QMenu(tr("&File"));
    menu->insertAction(0, _actReset);
    menu->insertAction(0, _actRandomize);
    menu->insertAction(0, _actPrev);
    menu->insertAction(0, _actNext);
    menu->addSeparator();

    _actExit = menu->addAction(tr("&Exit"));
    QObject::connect(_actExit, SIGNAL(triggered()), qApp, SLOT(quit()));          
    menuBar()->addMenu(menu);

    //-------------[Mode]-----------------------
    menu = new QMenu(tr("&Mode"));
    _actModeNumTiles = menu->addAction(tr("&Number of bad placed tiles"));
    QObject::connect(_actModeNumTiles, SIGNAL(triggered()), _wdgPuzzle, SLOT(modeNumBadTile()));

    _actModeManhSum = menu->addAction(tr("&Sum of Manhattan distances"));
    QObject::connect(_actModeManhSum, SIGNAL(triggered()), _wdgPuzzle, SLOT(modeSumManhattan()));

    _actModeCombined = menu->addAction(tr("&Combined"));
    QObject::connect(_actModeCombined, SIGNAL(triggered()), _wdgPuzzle, SLOT(modeCombined()));

    _actModeCound1sAfterXor = menu->addAction(tr("&Count 1's after xor"));
    QObject::connect(_actModeCound1sAfterXor, SIGNAL(triggered()), _wdgPuzzle, SLOT(modeNum1sAfterXor()));

    menu->addSeparator(); //-------------------------------------

    _actToggleSolver = menu->addAction(tr("&Use solver"));
    _actToggleSolver->setCheckable(true);

    QObject::connect(_actToggleSolver, SIGNAL(triggered(bool)), _wdgPuzzle->getSolverThread(), SLOT(setEnabled(bool)));
    QObject::connect(_actToggleSolver, SIGNAL(triggered()), _wdgPuzzle, SLOT(startSolverThread()));

    menu->addSeparator(); //-------------------------------------
    QAction* act = menu->addAction(tr("Test Case 1"));
    QObject::connect(act, SIGNAL(triggered()), _wdgPuzzle, SLOT(testCase1()));

    menuBar()->addMenu(menu);

    //--------------[Help]----------------------
    menu = new QMenu(tr("Help"));

    _actAboutQt = menu->addAction(tr("About QT"));
    QObject::connect(_actAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    menuBar()->addMenu(menu);

    _actAboutThisApp = menu->addAction(tr("About "APP_NAME));
    QObject::connect(_actAboutThisApp, SIGNAL(triggered()), this, SLOT(aboutThisApp()));
    menuBar()->addMenu(menu);
}

/**
  * Creates main Toolbar.
  */
void MainWindow::_createToolbars()
{
    _toolMain = addToolBar(tr("Main"));

    _actReset = _toolMain->addAction(tr("&Reset"));
    _actReset->setIcon(QIcon(":/trolltech/styles/commonstyle/images/refresh-32.png"));
    QObject::connect(_actReset, SIGNAL(triggered()), _wdgPuzzle, SLOT(reset()));

    _actRandomize = _toolMain->addAction(tr("Randomi&ze"));
    _actRandomize->setIcon(QIcon(":images/images/reddice.png"));
    QObject::connect(_actRandomize, SIGNAL(triggered()), _wdgPuzzle, SLOT(randomize()));

    _actPrev = _toolMain->addAction(tr("&Previous"));
    _actPrev->setIcon(QIcon(":/trolltech/styles/commonstyle/images/left-32.png"));
    _actPrev->setDisabled(true);
    QObject::connect(_actPrev, SIGNAL(triggered()), _wdgPuzzle, SLOT(prev()));

    _actNext = _toolMain->addAction(tr("&Next"));
    _actNext->setIcon(QIcon(":/trolltech/styles/commonstyle/images/right-32.png"));
    QObject::connect(_actNext, SIGNAL(triggered()), _wdgPuzzle, SLOT(next()));

    QObject::connect(_wdgPuzzle, SIGNAL(updateButtons(unsigned, unsigned)), this, SLOT(updateButtons(unsigned, unsigned)));

    _actAutoSolve = _toolMain->addAction(tr("&AutoStep"));
    _actAutoSolve->setIcon(QIcon(":/trolltech/styles/commonstyle/images/media-play-32.png"));

    _actStop = _toolMain->addAction(tr("&Stop"));
    _actStop->setIcon(QIcon(":/trolltech/styles/commonstyle/images/stop-32.png"));
    QObject::connect(_actStop, SIGNAL(triggered()), _wdgPuzzle, SLOT(stopProcessing()));
}

void MainWindow::updateOpenedClose(unsigned opened, unsigned closed)
{
    _labOpenedClosed->setText(QString("opened: ") + QString::number(opened) + " closed: " + QString::number(closed));
}

/**
  * Creates a Status Bar.
  */
void MainWindow::_createStatusBar()
{
    _labNextMove = new QLabel(_wdgPuzzle->stateString());
    statusBar()->addWidget(_labNextMove, 0);
    QObject::connect(_wdgPuzzle, SIGNAL(updateStatusBar(QString)), _labNextMove, SLOT(setText(QString)));

    _labOpenedClosed = new QLabel("Welcome");
    statusBar()->addWidget(_labOpenedClosed, 1);
    _labOpenedClosed->setHidden(true);
    //_toolMain->addWidget(_labOpenedClosed);
    QObject::connect(_wdgPuzzle->getSolverThread(), SIGNAL(updateOpenedClosed(unsigned,unsigned)), this, SLOT(updateOpenedClose(unsigned,unsigned)));
}

/**
  * This slot is called if history or solution of PuzzleWidget is changed.
  */
void MainWindow::updateButtons(unsigned histSz, unsigned solutionSz)
{
    if(histSz == 0)
        _actPrev->setDisabled(true);
    else
        _actPrev->setDisabled(false);

    if(solutionSz == 0)
        _actNext->setDisabled(true);
    else
        _actNext->setDisabled(false);
}


/**
  * Shows about info for this APplication
  */
void MainWindow::aboutThisApp()
{
    QMessageBox::information(this, tr("About &"APP_NAME),
                             tr("This is "APP_NAME" 2012 by Konstantin Schlese nulleight@gmail.com. "
                                "This program is Distributed under MIT Licesne, you can get the source at "
                                "http://code.google.com/p/qt15puzzle/. The Images used in this program are "
                                "distributed under the Creative Commons."));
}

/**
  *
  */
void MainWindow::solverBusy()
{
    _actReset->setDisabled(true);
    _actRandomize->setDisabled(true);
    _actPrev->setDisabled(true);
    _actNext->setDisabled(true);
    _actModeNumTiles->setDisabled(true);
    _actModeManhSum->setDisabled(true);
    _actModeCombined->setDisabled(true);
    _actAutoSolve->setDisabled(true);

    _labOpenedClosed->setText("solving...");
    _labOpenedClosed->setHidden(false);
    _labOpenedClosed->show();
}

void MainWindow::solverDone()
{
    _actReset->setDisabled(false);
    _actRandomize->setDisabled(false);
    //_actPrev->setDisabled(false); // enableing those 2 is handled by the other signal
    //_actNext->setDisabled(false);
    _actModeNumTiles->setDisabled(false);
    _actModeManhSum->setDisabled(false);
    _actModeCombined->setDisabled(false);
    _actAutoSolve->setDisabled(false);
    _labOpenedClosed->setHidden(true);
}

