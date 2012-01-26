#ifndef PUZZLEWIDGET_H
#define PUZZLEWIDGET_H
#include <QPainter>
#include <QWidget>
#include <QLabel>
#include <QFrame>
#include <QGridLayout>
#include <QMouseEvent>
#include <QMimeData>
#include "fifteenpuzzle.h"


class PuzzleWidget : public QWidget
{
    Q_OBJECT
    QGridLayout*        _layout;
    State               _currState;
    std::vector<State>  _solution, _history;
    QPixmap             _tilePixmaps[PUZZLE_BOARD_WIDTH * PUZZLE_BOARD_HEIGHT];
    SolverThread*       _solverThread;

    void        _createTilePixmaps();
    unsigned    _pos2stateIndex(QPoint pos);
    void        _swapItems(QPoint p1, QPoint p2);
    void        _updatePixmaps(const State& state);

public slots:

    void reset();
    void randomize();
    void next();
    void prev();
    void modeSumManhattan();
    void modeNumBadTile();
    void modeCombined();
    void modeZero();
    void modeNum1sAfterXor();
    void solvingDone();
    void stopProcessing();
    void startSolverThread();
    void testCase1();

signals:

    void updateButtons(unsigned histSz, unsigned solSz);
    void updateStatusBar(QString q);
    void startSolving(std::vector<State>* solution, State start);
    void processingDone();

public:

    PuzzleWidget(QWidget* parent = 0);
    virtual ~PuzzleWidget();

    void mousePressEvent(QMouseEvent *event);
    void dropEvent(QDropEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    QString stateString()const;

    SolverThread* getSolverThread()const { return _solverThread; }
};

#endif // PUZZLEWIDGET_H
