#ifndef STATE_H
#define STATE_H
/*
Commonly used heuristics for this problem include counting the number of misplaced tiles and finding the
 sum of the Manhattan distances between each block and its position in the goal configuration.
 */
#include <vector>
#include <iostream>
#include <cstdlib>
#define PUZZLE_BOARD_WIDTH 4
#define PUZZLE_BOARD_HEIGHT 4
#define PUZZLE_BOARD_NUM_TILES (PUZZLE_BOARD_WIDTH * PUZZLE_BOARD_HEIGHT)
#define PUZZLE_NUM_TILES (PUZZLE_BOARD_WIDTH * PUZZLE_BOARD_HEIGHT - 1)
#include <QDataStream>
#include <QtEndian>

struct State;
std::ostream& operator<<(std::ostream& out, const State& state);

struct State
{
    quint64 values;

    inline void reset()
    {
        for(int i = 1; i < (4 * 4); i++)
        {
            values = (values << 4) | i;
        }
        values <<= 4;
    }

    State() { reset(); }

    static quint64 winningValues()
    {
        return 0x123456789ABCDEF0ULL;
     }

    bool isFinal()const { return winningValues() == values; }

    unsigned value() const;

    inline unsigned valueAt(unsigned i)const
    {
        return (values >> ((15 - i) * 4)) & 0xF;
    }

    inline void setValue(unsigned i, unsigned val)
    {
        quint64 mask = 0xFULL << ((15 - i) * 4);
        values &= ~mask;
        values |= ((quint64)val << ((15 - i) * 4)) & mask;
    }

    inline unsigned size()const { return 4 * 4; }

    unsigned getZeroIndex()const
    {
        unsigned i = 15;
        for(quint64 m = values; m & 0xF; m >>= 4)
            i--;
        return i;
    }

    inline unsigned placeOf(unsigned char value)const
    {
        if(value)
            return value - 1;
        return // value == 0
            (PUZZLE_BOARD_NUM_TILES - 1);
    }

    inline bool operator<(const State& other)const
    {
        return values < other.values;
    }

    inline bool operator==(const State& other)const
    {
        return values == other.values;
    }

    inline static unsigned manhattanDistance(unsigned idx1, unsigned idx2)
    {
        int x1 = (idx1 % PUZZLE_BOARD_WIDTH);
        int y1 = (idx1 / PUZZLE_BOARD_WIDTH);
        int x2 = (idx2 % PUZZLE_BOARD_WIDTH);
        int y2 = (idx2 / PUZZLE_BOARD_WIDTH);
        return abs(x1 - x2) + abs(y1 - y2);
    }

    inline bool validSwap(unsigned idx1, unsigned idx2)const
    {
        return idx1 < 16 &&
               idx2 < 16 &&
               (valueAt(idx1) == 0 || valueAt(idx2) == 0) &&
               (
                    abs(idx2 - idx1) == 4 ||
                    (idx2 - idx1 == 1 && ((idx1 & 3) != 3)) ||
                    (idx1 - idx2 == 1 && ((idx2 & 3) != 3))
               );
    }


    inline void swap(unsigned idx1, unsigned idx2)
    {
        Q_ASSERT(isValid());
        unsigned c = valueAt(idx1);
        setValue(idx1, valueAt(idx2));
        setValue(idx2, c);
        Q_ASSERT(isValid());
    }

    inline bool isValid()
    {
        for(unsigned i = 0; i < 16; i++)
            for(unsigned j = i + 1; j < 16; j++)
                if(valueAt(i) == valueAt(j))
                {
                    //std::cout << "bad state: " << std::endl << *this;
                    return false;
                }
        return true;
    }


    unsigned valueFuncNumBadTiles()const;
    unsigned valueFuncSumManhattanDist()const;
    unsigned valueFuncCombined()const;
    unsigned valueNum1sAfterXor()const;
    unsigned valueFuncZero()const;


    void randomize(unsigned times = 20);
    void getNextPossibleStates(std::vector<State>& results)const;
};

/*
struct StateT
{
    unsigned char values[PUZZLE_BOARD_NUM_TILES];

    StateT();

    inline unsigned size()const { return PUZZLE_BOARD_NUM_TILES; }

    unsigned getZeroIndex()const
    {
        for(unsigned i = 0; i < sizeof(values); i++)
            if(values[i] == 0)
                return i;
        return sizeof(values);
    }

    unsigned value()const;

    inline unsigned placeOf(unsigned char value)const
    {
        if(value)
            return value - 1;
        return // value == 0
            (PUZZLE_BOARD_NUM_TILES - 1);
    }

    inline unsigned manhattanDistance(unsigned idx1, unsigned idx2)const
    {
        int x1 = (idx1 % PUZZLE_BOARD_WIDTH);
        int y1 = (idx1 / PUZZLE_BOARD_WIDTH);
        int x2 = (idx2 % PUZZLE_BOARD_WIDTH);
        int y2 = (idx2 / PUZZLE_BOARD_WIDTH);
        return abs(x1 - x2) + abs(y1 - y2);
    }

    inline bool validSwap(unsigned idx1, unsigned idx2)const
    {

        //unsigned x1 = idx1 % PUZZLE_BOARD_WIDTH;
        //unsigned y1 = idx1 / PUZZLE_BOARD_WIDTH;
        //unsigned x2 = idx2 % PUZZLE_BOARD_WIDTH;
        //unsigned y2 = idx2 / PUZZLE_BOARD_WIDTH;

        return idx1 < PUZZLE_BOARD_NUM_TILES &&
               idx2 < PUZZLE_BOARD_NUM_TILES &&
               (abs(idx1 - idx2) == PUZZLE_BOARD_WIDTH || (abs(idx1 - idx2) == 1 && ((idx1 & 3) != 0) && ((idx2 & 3) != 0)));
    }


    inline void swap(unsigned idx1, unsigned idx2)
    {
        unsigned char c = values[idx1];
        values[idx1] = values[idx2];
        values[idx2] = c;
    }

    inline bool isFinal()const
    {
        return value() == 0;
    }

    bool operator==(const State& other)const;
//#if (PUZZLE_BOARD_NUM_TILES % 4 == 0)
#if(0)
    inline bool operator==(const State& other)const
    {
        unsigned* a = (unsigned*)values;
        unsigned* b = (unsigned*)other.values;
        for(unsigned i = 0; i < PUZZLE_BOARD_NUM_TILES / sizeof(unsigned); i++)
        {
            if(*a != *b)
                return false;
        }
        return true;
    }
#else
    bool operator==(const State& other)const
    {
        for(unsigned i = 0; i < PUZZLE_BOARD_NUM_TILES; i++)
        {
            if(values[i] != other.values[i])
                return false;
        }
        return true;
    }
endif

    void reset();
    unsigned valueFuncNumBadTiles()const;
    unsigned valueFuncSumManhattanDist()const;
    unsigned valueFuncCombined()const;
    unsigned valueFuncZero()const;

    void randomize(unsigned times = 20);
    void getNextPossibleStates(std::vector<State>& results)const;
};
*/

extern unsigned (State::*valueFunc)()const;


bool stateTests();

#endif // STATE_H
