#include <stdio.h>

#define MAXNODES 100000
#define MAXCHILD 18
#define SIM_NUM 10
#define MCTS_DEPTH 3
#define PV_LEN 10

/* For UCB computation */
#define C 0.1
#define MaxS 1.0
#define MinS 0.0

/* Timer */
#define STOP_TIME 5.0

struct NODE {
    int ply; // the ply from parent to here
    int p_id; // parent id, root’s parent is the root
    int c_id[MAXCHILD]; // children id
    int depth; // depth, 0 for the root
    int Nchild; // number of children
    int Ntotal; // total # of simulations
    long double CsqrtlogN; // c * sqrt(log(Ntotal))
    long double sqrtN; // sqrt(Ntotal)
    int sum1; // sum1: sum of scores
    int sum2; // sum2: sum of square of each score
    long double Average; // average score
    long double Variance; // variance of score
};


/********************************************
*   movement:                               *
*       0: vertical                         *
*       1: horizontal                       *
*       2: diagonal                         *
*********************************************/
const int ply_movement[18][2] = {
    {0, 0}, {0, 1}, {0, 2},
    {1, 0}, {1, 1}, {1, 2},
    {2, 0}, {2, 1}, {2, 2},
    {3, 0}, {3, 1}, {3, 2},
    {4, 0}, {4, 1}, {4, 2},
    {5, 0}, {5, 1}, {5, 2}
};
#define move_cube(x) ((x == -1)? 15 : ply_movement[x][0])
#define move_dir(x) ((x == -1)? 15 : ply_movement[x][1])
#define get_ply(x, y) ((x==15 && y==15)? (-1) :(x * 3 + y))

