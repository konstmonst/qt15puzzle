#pragma once
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QVBoxLayout>
#include <QStatusBar>
#include <QMainWindow>
#include <QCoreApplication>
#include <QApplication>
#include <QFileSystemModel>
#include <QSortFilterProxyModel>
#include <QListView>
#include <QTreeView>
#include <QTableView>
#include <QListView>
#include <QSplitter>
#include <QToolBar>
#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>
#include <QLabel>
#include <QTimer>
#include "puzzlewidget.h"

#define APP_NAME "QT15-Puzzle"

class MainWindow : public QMainWindow
{
	Q_OBJECT

    PuzzleWidget*       _wdgPuzzle;
    QToolBar*           _toolMain;
    QLabel*				_labNextMove;
    QLabel*				_labOpenedClosed;
    QTimer*             _timNextPusher;

    QAction*            _actReset;
    QAction*            _actRandomize;
    QAction*            _actPrev;
    QAction*            _actNext;
    QAction*            _actAutoSolve;
    QAction*			_actExit;
    QAction*            _actModeNumTiles;
    QAction*            _actModeManhSum;
    QAction*            _actModeCombined;
    QAction*            _actModeZero;
    QAction*			_actAboutQt;
    QAction*			_actAboutThisApp;
    QAction*            _actStop;
    QAction*            _actToggleSolver;
    QAction*            _actModeCound1sAfterXor;

    void _createMenus();
    void _createToolbars();
    void _createStatusBar();

private slots:
    void aboutThisApp();  
    void updateButtons(unsigned histSz, unsigned solutionSz);
    void updateOpenedClose(unsigned opened, unsigned closed);
    void solverBusy();
    void solverDone();

public:
    MainWindow();
	virtual ~MainWindow();


};
