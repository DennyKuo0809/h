#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <iostream> 
#include "definition.hpp"
#include "einstein.hpp"

struct NODE nodes[MAXNODES];
int PV[PV_LEN];
int base_idx = 0;

void init_root_node(int id){
    nodes[id].ply = -1;
    nodes[id].p_id = -1;
    nodes[id].depth = 0;
    nodes[id].Nchild = nodes[id].Ntotal = nodes[id].sum1 = nodes[id].sum2 = 0;
    nodes[id].CsqrtlogN = nodes[id].sqrtN = nodes[id].Average = nodes[id].Variance = 0;
}

void expand(int id, std::vector< std::pair<int, int> > move_list){
    std::vector<std::pair<int, int> >::iterator it;
    int i = 0;
    for(it = move_list.begin() ; it != move_list.end() ; it ++, i++){
        nodes[base_idx].ply = get_ply(it -> first, it -> second);
        nodes[base_idx].p_id = id;
        nodes[base_idx].depth = nodes[id].depth + 1;
        nodes[base_idx].Nchild = nodes[base_idx].Ntotal = nodes[base_idx].sum1 = nodes[base_idx].sum2 = 0;
        nodes[base_idx].CsqrtlogN = nodes[base_idx].sqrtN = nodes[base_idx].Average = nodes[base_idx].Variance = 0;

        nodes[id].c_id[i] = base_idx;
        base_idx ++;
    }
    nodes[id].Nchild = i;
    return;
}


std::pair<int, int> run_simulation(BOARD_GUI* origin_b, char start){
	
    int sum1 = 0;
    int sum2 = 0;

    for(int i = 0 ; i < SIM_NUM ; i++){ // simulation {SIM_NUM} times
        /* Copy the board */
        BOARD_GUI sim_b;
        sim_b = *origin_b;

        /* Run simulation */
        while(1){
			/* check if game has end */
            if ( sim_b.winner() != Color::OTHER ) {
                /* check win/loss */
                if(sim_b.winner() == Color::R){
                    sum2 ++;
                    if(start == 'f'){
                        sum1 ++;
                    }
                    else{
                        sum1 --;
                    }
                }
                else if(sim_b.winner() == Color::B){
                    sum2 ++;
                    if(start == 's'){
                        sum1 ++;
                    }
                    else{
                        sum1 --;
                    }
                }
				break;
            }

            /* get valid movement */
            auto ml = sim_b.move_list();

            /* random select a movement */
            // auto m = ml.at(rand()%ml.size());

            /* Greedy select a movement */
            std::pair<int, int> m = std::pair<int, int>(-1, -1);
            for ( auto &move: ml ) {
                if ( sim_b.yummy(move) == 1 ) {
                    m = move;
                    break;
                }
                else if(m.first == -1 && move.second < 2){
                    m = move;
                }
            }
            if ( m.first == -1 ) {
                m = ml[rand()%ml.size()];
            }
            
            sim_b.do_move(m);
        }
    }
    return std::make_pair(sum1, sum2);
}

int simulation_1(BOARD_GUI* sim_b, char start){
    while(1){
        /* check if game has end */
        if ( sim_b -> winner() != Color::OTHER ) {
            /* check win/loss */
            if(sim_b -> winner() == Color::R){
                if(start == 'f'){
                    return 1;
                }
                else{
                    return -1;
                }
            }
            else if(sim_b -> winner() == Color::B){
                if(start == 's'){
                    return 1;
                }
                else{
                    return -1;
                }
            }
            return 0;
        }

        /* get valid movement */
        auto ml = sim_b -> move_list();

        /* random select a movement */
        // auto m = ml.at(rand()%ml.size());

        /* Greedy select a movement */
        std::pair<int, int> m = std::pair<int, int>(-1, -1);
        for ( auto &move: ml ) {
            if ( sim_b -> yummy(move) == 1 ) {
                m = move;
                break;
            }
            else if(m.first == -1 && move.second < 2){
                m = move;
            }
        }
        if ( m.first == -1 ) {
            m = ml[rand()%ml.size()];
        }
        
        sim_b -> do_move(m);
    }
}


void update_single_node(int id, int deltaS1, int deltaS2, int deltaN){
    nodes[id].sum1 += deltaS1;
    nodes[id].sum2 += deltaS2;
    nodes[id].Ntotal += deltaN;
    nodes[id].CsqrtlogN = C * sqrt(log((long double)nodes[id].Ntotal));
    nodes[id].sqrtN = sqrt((long double)nodes[id].Ntotal);
    nodes[id].Average = (long double)nodes[id].sum1 / (long double)nodes[id].Ntotal;
    nodes[id].Variance = (nodes[id].sum2 - nodes[id].Average * 
        (2 * (long double)nodes[id].sum1 - nodes[id].Average)) / 
        (long double)nodes[id].Ntotal;
}

long double UCB(int id){
    long double Range = MaxS - MinS; // can be stored as a constant
    // normalized the average score to be between 0 and 1
    long double SR = (nodes[id].Average - MinS) / Range;
    if(nodes[id].depth % 2){
        return (1.0-SR) + nodes[nodes[id].p_id].CsqrtlogN / (long double)nodes[id].sqrtN;
    }
    return SR + nodes[nodes[id].p_id].CsqrtlogN / (long double)nodes[id].sqrtN;
}

int get_PV_path(int root_id, BOARD_GUI *tmp_b){
    int idx;
    PV[0] = idx = root_id;
    std::pair<int, int> ply;

    int max_c_id, nodei;
    long double max_c_score, ucbi;

    while(nodes[idx].Nchild > 0){
        max_c_id = nodes[idx].c_id[0];
        max_c_score = UCB(nodes[idx].c_id[0]);
        for(int i = 1 ; i < nodes[idx].Nchild ; i++){
            nodei = nodes[idx].c_id[i];
            ucbi = UCB(nodei);
            if(ucbi > max_c_score){
                max_c_id = nodei;
                max_c_score = ucbi;
            }
        }
        PV[nodes[max_c_id].depth] = idx = max_c_id;
        ply = std::make_pair(move_cube(nodes[max_c_id].ply), move_dir(nodes[max_c_id].ply));
        tmp_b -> do_move(ply);
    }
    return idx; // return PV leaf
}

void back_propagation(int leaf_id, int root_id, int deltaS1, int deltaS2, int deltaN){
    int idx = leaf_id;
    while(idx != root_id){
        update_single_node(idx, deltaS1, deltaS2, deltaN);
        idx = nodes[leaf_id].p_id;
    }
    update_single_node(root_id, deltaS1, deltaS2, deltaN);
}
