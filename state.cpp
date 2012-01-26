#include "state.h"
#include <cassert>
#include <cstdlib>
#include <QtGlobal>
using namespace std;
unsigned (State::*valueFunc)()const;


static struct StateStaticInit
{
    StateStaticInit() { valueFunc = &State::valueFuncCombined; }
} init;

unsigned State::value() const
{
    return (this->*valueFunc)();
}

static int bitcount_dense_ones(quint64 n)
{
    int tot = 0;

    n ^= (quint64)-1;

    while(n)
    {
        ++tot;
        n &= n - 1;
    }

    return sizeof(quint64) * 8 - tot;
}

/**
  * Resets the puzzle to a solved state.
  *
void State::reset()
{
    unsigned i = 0;
    for(; i < sizeof(values) - 1; i++)
    {
        values[i] = i + 1;
    }
    values[i] = 0;
}
*/

/**
  * Number of misplaced Tiles. 0 is the Best value and PUZZLE_BOARD_WIDTH * PUZZLE_BOARD_HEIGHT is the worst.
  */
unsigned State::valueFuncNumBadTiles()const
{
    unsigned value = 0;
    for(unsigned i = 0; i < 16; i++)
    {
        if(placeOf(valueAt(i)) != i)
            value++;
    }

    assert(value <= 16);
    return value;
}

/**
  * Sum of manhattan distances 0 of the tiles to their target position.
  */
unsigned State::valueFuncSumManhattanDist()const
{
    unsigned value = 0;
    for(unsigned i = 0; i < 16; i++)
    {
        value += manhattanDistance(placeOf(valueAt(i)), i);
    }

    assert(value <= (4 - 1 + 4 - 1) * 16);
    return value;
}

unsigned State::valueFuncCombined()const
{
    return valueFuncSumManhattanDist() + valueFuncNumBadTiles();
}

unsigned State::valueNum1sAfterXor() const
{
    return bitcount_dense_ones(values ^ winningValues());
}

unsigned State::valueFuncZero()const
{
    return 0;
}


/*
bool State::operator==(const State& other)const
{
    for(unsigned i = 0; i < 16; i++)
    {
        if(values[i] != other.values[i])
            return false;
    }
    return true;
}
*/

/**
  *    Returns next possible states for a current state.
  */
void State::getNextPossibleStates(vector<State>& results)const
{
    results.clear();

    unsigned zeroIndex = getZeroIndex();    
    assert(zeroIndex < PUZZLE_BOARD_NUM_TILES);

    if((zeroIndex % PUZZLE_BOARD_WIDTH) > 0)
    {
        State state(*this);
        Q_ASSERT(state.validSwap(zeroIndex, (zeroIndex - 1)));
        state.swap(zeroIndex, (zeroIndex - 1));
        results.push_back(state);
    }
    if((zeroIndex % PUZZLE_BOARD_WIDTH) < (PUZZLE_BOARD_WIDTH - 1))
    {
        State state(*this);
        Q_ASSERT(state.validSwap(zeroIndex, (zeroIndex + 1)));
        state.swap(zeroIndex, (zeroIndex + 1));
        results.push_back(state);
    }
    if(zeroIndex >= PUZZLE_BOARD_WIDTH)
    {
        State state(*this);
        Q_ASSERT(state.validSwap(zeroIndex, (zeroIndex - PUZZLE_BOARD_WIDTH)));
        state.swap(zeroIndex, (zeroIndex - PUZZLE_BOARD_WIDTH));
        results.push_back(state);
    }
    if((zeroIndex + PUZZLE_BOARD_WIDTH) < PUZZLE_BOARD_NUM_TILES)
    {
        State state(*this);
        Q_ASSERT(state.validSwap(zeroIndex + PUZZLE_BOARD_WIDTH, zeroIndex));
        state.swap(zeroIndex + PUZZLE_BOARD_WIDTH, zeroIndex);
        results.push_back(state);
    }
}

/**
  * Randomly permutes the State.
  */
void State::randomize(unsigned times)
{
    vector<State> next;

    for(unsigned i = 0; i < times; i++)
    {
        getNextPossibleStates(next);
        *this = next[qrand() % next.size()];
    }
}

/**
  * Printer function, usefull for debugging.
  */
std::ostream& operator<<(std::ostream& out, const State& state)
{
    out.flags(ios::right | ios::hex);

    out << "+---";
    for(unsigned i = 0; i < PUZZLE_BOARD_WIDTH - 1; i++)
        out << "|---";
    out << "+" << endl;

    unsigned i = 0;
    for(unsigned y = 0; y < PUZZLE_BOARD_HEIGHT; y++)
    {
        for(unsigned x = 0; x < PUZZLE_BOARD_WIDTH; x++)
        {
            out << "| " << (unsigned)state.valueAt(i++) << ' ';
        }
        out << '|' << endl;

        for(unsigned x = 0; x < PUZZLE_BOARD_WIDTH; x++)
        {
            out << "|---";
        }
        out << '+' << endl;
    }
    return out;
}

bool stateTests()
{
    State state;
    Q_ASSERT(state.getZeroIndex() == 15);
    Q_ASSERT(state.isFinal());
    Q_ASSERT(state.validSwap(14, 15));
    state.swap(14, 15);
    Q_ASSERT(state.validSwap(14, 10));
    state.swap(14, 10);
    State state2(state);
    Q_ASSERT(state.validSwap(10, 11));
    state.swap(10, 11);
    Q_ASSERT(state.validSwap(11, 15));
    state.swap(11, 15);
    Q_ASSERT(!(state == state2));

    std::vector<State> next_moves;
    state2.getNextPossibleStates(next_moves);
    Q_ASSERT(next_moves.size() == 4);
    /*
    for(unsigned i = 0; i < next_moves.size(); i++)
        cout << next_moves[i];
*/
    return true;
}
