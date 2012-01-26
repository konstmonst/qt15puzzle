#include <QtDebug>
#include <QtGlobal>
#include <QLayoutItem>
#include "puzzlewidget.h"

#include <iostream>
using namespace std;

PuzzleWidget::PuzzleWidget(QWidget* parent)
    : QWidget(parent)
{
    _layout = new QGridLayout(this);
    _layout->setOriginCorner(Qt::TopLeftCorner);
    _solverThread = new SolverThread();

    QObject::connect(this, SIGNAL(startSolving(std::vector<State>*,State)), _solverThread, SLOT(startSolving(std::vector<State>*,State)));
    QObject::connect(_solverThread, SIGNAL(solvingDone()), this, SLOT(solvingDone()));

    _createTilePixmaps();

    for(unsigned i = 0; i < _currState.size(); i++)
    {
        QLabel* label = new QLabel(this);
        label->show();
        //label->setAttribute(Qt::WA_DeleteOnClose);
        _layout->addWidget(label, i % PUZZLE_BOARD_WIDTH, i / PUZZLE_BOARD_HEIGHT);
        //qDebug() << "adding label i:" << i << "at (" << (i % PUZZLE_BOARD_WIDTH) << "," << (i / PUZZLE_BOARD_HEIGHT) << ")";
    }

    _currState.randomize(1);
    _updatePixmaps(_currState);

    setLayout(_layout);
    setAcceptDrops(true);
    resize(minimumSizeHint());

    emit updateStatusBar(stateString());
    emit updateButtons(_history.size(), _solution.size());

    setDisabled(true);
    emit startSolving(&_solution, _currState);
}

/**
  * This method creates Pixmaps of the Tiles.
  */
void PuzzleWidget::_createTilePixmaps()
{
    QFont tileFont("times", 40);
    QRect border_rect(0, 0, 99, 99);

    for(unsigned i = 0; i < _currState.size(); i++)
    {
        _tilePixmaps[i] = QPixmap(border_rect.width() + 1, border_rect.height() + 1);
        _tilePixmaps[i].fill(QColor("transparent"));

        QPainter painter(&_tilePixmaps[i]);
        painter.setFont(tileFont);
        painter.setPen(Qt::black);
        painter.drawRect(border_rect);
        if(i > 0)
        {
            painter.setBrush(QBrush(Qt::black));
            painter.drawText(border_rect, Qt::AlignCenter, QString::number(i));
        }
    }
}

/**
  *
  */
PuzzleWidget::~PuzzleWidget()
{
    /*
    QLayoutItem *child;
    while ((child = layout->takeAt(0)) != 0)
    {
        delete child;
    }
    */
}

void PuzzleWidget::reset()
{
    _currState.reset();
    _history.clear();
    _solution.clear();
    _updatePixmaps(_currState);
    stopProcessing();
}

/**
  * Permutes the current state a bit.
  */
void PuzzleWidget::randomize()
{
    _currState.randomize();
    _history.clear();
    _updatePixmaps(_currState);

    setDisabled(true);
    emit startSolving(&_solution, _currState);
}

/**
  * Switches to the next State.
  */
void PuzzleWidget::next()
{
    if(!_solution.empty())
    {
        //cout << "next: saving " << endl << _currState;
        _history.push_back(_currState);
        _currState = _solution.back();
        _solution.pop_back();
        _updatePixmaps(_currState);
        //cout << "next: using " << endl << _currState;

        emit updateButtons(_history.size(), _solution.size());
        emit updateStatusBar(stateString());
    }
}

/**
  * Switches to the previous State.
  */
void PuzzleWidget::prev()
{
    if(!_history.empty())
    {
        //cout << "prev: saving " << endl << _currState;
        _solution.push_back(_currState);
        _currState = _history.back();
        _history.pop_back();
        _updatePixmaps(_currState);
        //cout << "prev: using " << endl << _currState;
        emit updateButtons(_history.size(), _solution.size());
        emit updateStatusBar(stateString());
    }
}

void PuzzleWidget::startSolverThread()
{
    setDisabled(true);
    emit startSolving(&_solution, _currState);
}

void PuzzleWidget::modeSumManhattan()
{
    valueFunc = &State::valueFuncSumManhattanDist;
    startSolverThread();
}

void PuzzleWidget::modeNumBadTile()
{
    valueFunc = &State::valueFuncNumBadTiles;
    startSolverThread();
}

void PuzzleWidget::modeCombined()
{
    valueFunc = &State::valueFuncCombined;
    startSolverThread();
}


void PuzzleWidget::modeZero()
{
    valueFunc = &State::valueFuncZero;
    startSolverThread();
}

void PuzzleWidget::modeNum1sAfterXor()
{
    valueFunc = &State::valueNum1sAfterXor;
    startSolverThread();
}

void PuzzleWidget::solvingDone()
{
    emit processingDone();
    emit updateButtons(_history.size(), _solution.size());
    emit updateStatusBar(stateString());
    setEnabled(true);
}

void PuzzleWidget::stopProcessing()
{
    if(_solverThread->isRunning())
    {
        _solverThread->terminate();
        _solverThread->wait();
        _solution.clear();
    }
    solvingDone();
}

QString PuzzleWidget::stateString()const
{
    QString result("Val: \"");

    if(valueFunc == &State::valueFuncSumManhattanDist)
        result += "Sum-Manhattan-Dists ";
    else if(valueFunc == &State::valueFuncNumBadTiles)
        result += "Num-Bad-Tiles ";
    else if(valueFunc == &State::valueFuncCombined)
        result += "Combined ";
    else if(valueFunc == &State::valueFuncZero)
        result += "Zero ";
    else
        result += "Unknown ";

    result += "\" solution size: ";
    result += QString::number(_solution.size());
    return result;
}


/**
  * Updates the pixmaps on the Tile Widgets.
  */
void PuzzleWidget::_updatePixmaps(const State& state)
{
    for(unsigned i = 0; i < PUZZLE_BOARD_WIDTH * PUZZLE_BOARD_HEIGHT; i++)
    {
        QLabel* label = (QLabel*)_layout->itemAtPosition(i / PUZZLE_BOARD_WIDTH, i % PUZZLE_BOARD_HEIGHT)->widget();
        label->setPixmap(_tilePixmaps[state.valueAt(i)]);
    }
}


/**
  * Converts position inside PuzzleWidget to an index.
  */
unsigned PuzzleWidget::_pos2stateIndex(QPoint pos)
{
    unsigned y = pos.y() * PUZZLE_BOARD_HEIGHT / height();
    unsigned x = pos.x() * PUZZLE_BOARD_WIDTH / width();
    return y * PUZZLE_BOARD_WIDTH + x;
}

/*
QPoint PuzzleWidget::_pos2gridIndex(QPoint pos)
{
    return QPoint(pos.y() * PUZZLE_BOARD_HEIGHT / height(),  pos.x() * PUZZLE_BOARD_WIDTH / width());
}
*/

void PuzzleWidget::_swapItems(QPoint p1, QPoint p2)
{
    // swap the items in the layout
    QLayoutItem* i1 = _layout->itemAtPosition(p1.x(), p1.y());
    QLayoutItem* i2 = _layout->itemAtPosition(p2.x(), p2.y());

    _layout->removeItem(i1);
    _layout->removeItem(i2);
    _layout->addItem(i1, p2.x(), p2.y());
    _layout->addItem(i2, p1.x(), p1.y());
}


QPixmap buildGrayedPixmap(const QPixmap& pixmap)
{
    QPixmap tempPixmap = pixmap;
    QPainter painter;
    painter.begin(&tempPixmap);
    painter.fillRect(pixmap.rect(), QColor(127, 127, 127, 127));
    //painter.drawText(QPoint(child->width() / 2, child->height() / 2), "1");
    painter.drawText(QPoint(0, 0), "drag");
    painter.end();
    return tempPixmap;
}

void PuzzleWidget::mousePressEvent(QMouseEvent *event)
{
    QLabel *child = static_cast<QLabel*>(childAt(event->pos()));

    if (!child)
        return;

    qDebug() << __FUNCTION__ << " entering: " << event->pos();// QPoint(event->pos() - pos());

    QPixmap pixmap = *child->pixmap();

    QByteArray itemData;
    QDataStream dataStream(&itemData, QIODevice::WriteOnly);

    unsigned srcIndex = _pos2stateIndex(event->pos());
    dataStream << pixmap << QPoint(event->pos() - child->pos()) << srcIndex;

    QMimeData *mimeData = new QMimeData;
    mimeData->setData("application/x-dnditemdata", itemData);

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setPixmap(pixmap);
    //drag->setHotSpot(event->pos() - pos() - child->pos());
    drag->setHotSpot(event->pos() - child->pos());


    child->setPixmap(buildGrayedPixmap(pixmap));

    //drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::CopyAction);
    drag->exec(Qt::CopyAction, Qt::CopyAction);
    /*
    if (drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::CopyAction) == Qt::MoveAction)
        child->close();
    else
    {
        child->show();
        child->setPixmap(pixmap);
    }
    */
        //_layout->getItemPosition(_layout->findChild);

    //qDebug() << __FUNCTION__ << " exiting: " << QPoint(event->pos() - pos());
}

void PuzzleWidget::dropEvent(QDropEvent *event)
{
    qDebug() << __FUNCTION__ << "event pos " << event->pos();
    if (event->mimeData()->hasFormat("application/x-dnditemdata"))
    {
        QByteArray itemData = event->mimeData()->data("application/x-dnditemdata");
        QDataStream dataStream(&itemData, QIODevice::ReadOnly);

        QPixmap pixmap;
        QPoint offset;
        unsigned srcIdx;

        dataStream >> pixmap >> offset >> srcIdx;
        //unsigned destIdx = _pos2stateIndex(event->pos() - offset);
        unsigned destIdx = _pos2stateIndex(event->pos());
        //qDebug() << __FUNCTION__ << " drop index " << _pos2gridIndex(event->pos().x() , event->pos().y());
        //unsigned destIndex = _pos2gridIndex(event->pos().x() , event->pos().y());
        //qDebug() << " source index " << srcIndex << " target index " << destIndex;

        //if(_pos2gridIndex(event->pos() - pos()) has en empty widget

        cout << "src index " << srcIdx << " dest index " << destIdx << endl;
        //QPoint diff = srcIdx - destIdx;
        //if(diff.manhattanLength() == 1)
        if(_currState.validSwap(srcIdx, destIdx))
        {
            _history.push_back(_currState);

            _currState.swap(srcIdx, destIdx);           

            setDisabled(true);
            emit startSolving(&_solution, _currState);
        }
        else
            qDebug() << "bad move!";
        _updatePixmaps(_currState);
        /*
        QLabel *newIcon = new QLabel(this);
        newIcon->setPixmap(pixmap);
        newIcon->move(event->pos() - offset);
        newIcon->show();
        newIcon->setAttribute(Qt::WA_DeleteOnClose);
        */

        if (event->source() == this)
        {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        }
        else
        {
            event->acceptProposedAction();
        }
    }
    else
    {
        event->ignore();
    }
}

void PuzzleWidget::dragMoveEvent(QDragMoveEvent *event)
{
    //qDebug() << __FUNCTION__ << endl;
    if (event->mimeData()->hasFormat("application/x-dnditemdata"))
    {
        if (event->source() == this)
        {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        }
        else
        {
            event->acceptProposedAction();
        }
    }
    else
    {
        event->ignore();
    }
}

void PuzzleWidget::dragEnterEvent(QDragEnterEvent *event)
{
    qDebug() << __FUNCTION__;
    if (event->mimeData()->hasFormat("application/x-dnditemdata"))
    {
        if (event->source() == this)
        {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        }
        else
        {
            event->acceptProposedAction();
        }
    }
    else
    {
        event->ignore();
    }
}


void PuzzleWidget::testCase1()
{
    unsigned char values1[16] = {1, 3,  7,  4,
                               5, 0,  2,  8,
                              10, 6, 13, 12,
                              9, 14, 11, 15};

    _history.clear();

    for(unsigned i = 0; i < sizeof(values1); i++)
        _currState.setValue(i, values1[i]);

    _updatePixmaps(_currState);
    setDisabled(true);
    emit startSolving(&_solution, _currState);

}

