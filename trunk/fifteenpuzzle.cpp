#include "fifteenpuzzle.h"
#include "state.h"
#include <list>
#include <map>
#include <set>
#include <algorithm>
#include <vector>
using namespace std;
//#define LOG(...) cerr << __VA_ARGS__;
#define LOG(...)

struct AStarNode
{
    State state;
    AStarNode* root;
    unsigned path_lenght;
    unsigned value;

    AStarNode(AStarNode* new_root, State new_state)
        : state(new_state), root(new_root)
    {
        path_lenght = root->path_lenght;
        value = new_state.value() + path_lenght;
    }

     AStarNode(State new_state)
         : state(new_state), root(0), path_lenght(0)
     {
         value = new_state.value();
     }
};

static struct LessValueClass
{
    bool operator() (const AStarNode* n1, const AStarNode* n2)
    {
        return n1->value < n2->value;
    }
} LessValue;

/**
  * This is a comparator for States so that we can find closed states fast. For that I defined an
  * Order of States so the can be sorted and searched in a binary tree.
  */
struct StateOrdering
{
    bool operator() (const AStarNode* n1, const AStarNode* n2)
    {
        return n1->state.values < n2->state.values;
    }
};

SolverThread::SolverThread(QObject *parent)
    : QThread(parent), _enabled(true)
{
}

SolverThread::~SolverThread()
{
}

void SolverThread::run()
{
    if(_enabled)
        _solveAStar();
    emit solvingDone();
}

void SolverThread::startSolving(std::vector<State>* solution, State s)
{
    _solution = solution;
    _solution->clear();
    _start = s;
    start();
}

/**
  * A* Solver for fifteen puzzle.
  */
void SolverThread::_solveAStar()
{
    _solution->clear();
    if(_start.isFinal())
        return;

    typedef std::list<AStarNode*> OpenedContainer;
    typedef std::set<AStarNode*, StateOrdering> ClosedContainer;

    OpenedContainer opened;
    ClosedContainer closed;

    // start
    AStarNode* current = new AStarNode(_start);
    opened.push_front(current);

    while(not opened.empty())
    {
        emit updateOpenedClosed(opened.size(), closed.size());
        LOG("opened: " << opened.size() << " closed: " << closed.size());

        // finding a tile with best score
        OpenedContainer::iterator iter = std::min_element(opened.begin(), opened.end(), LessValue);
        AStarNode* current = *iter;
        opened.erase(iter);
        closed.insert(current);
        LOG(" choosing state with smallest value: " << current->state.value() << endl << current->state);

        if(current->state.isFinal())
        {
            do
            {
                _solution->push_back(current->state);
                current = current->root;
            }
            while(current->root);
            break;
        }
        else
        {
            std::vector<State> next_states;
            current->state.getNextPossibleStates(next_states);

            for(std::vector<State>::iterator state_iter = next_states.begin(); state_iter != next_states.end(); ++state_iter)
            {
                const State& state = *state_iter;
                LOG("processing state with value " << state.value() << ": " << endl << state);
                //AStarNode

                // if this state is not yet closed
                AStarNode tmp(state);
                // AStarNode a and b are equal when they cannot be ordered => not (a < b) and not (b < a)
                ClosedContainer::iterator c = closed.find(&tmp);
                if(c == closed.end())
                {
                    OpenedContainer::iterator o = find(opened.begin(), opened.end(), &tmp);
                    // and was not yet examined or found but worse than the one in "opened"
                    if((o != opened.end()) && ((*o)->value <= state.value()))
                        continue;
                }
                // is the state in "closed" is worse than the current one?
                else if((*c)->value <= state.value())
                        continue;

                LOG("putting this state to opened" << endl);
                AStarNode* new_node = new AStarNode(current, state);
                opened.push_back(new_node);
            }
        }
    }

    for(ClosedContainer::iterator it = closed.begin(); it != closed.end(); ++it)
        delete *it;
}
