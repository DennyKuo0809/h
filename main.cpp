// Copyright (C) 2019 Yueh-Ting Chen (eopXD)
/*! \file greedy.cpp
	\brief baseline agents
	 -D GREEDY, greedy movement evaluated by eval() below
	 -D CONSERVATIVE, conservative strategy, eats opponent, or do hor/vert,
	  if none of the above move exists, do random
	 -D RANDOM, do random move from move_list()
	 Add log: -D PLAYER1/PLAYER2
	\author Yueh-Ting Chen (eopXD)
	\course Theory of Computer Game (TCG)
	\modified by kjchan (2022)
*/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <string.h>

#include <iostream>
#include <fstream>
#include <utility>
#include <chrono>


#include "mcts.hpp"
// #include "definition.hpp"
// #include "einstein.hpp"

char start;
char init[2][NUM_CUBE+1] = {};
BOARD_GUI *b, tmp_b, sim_b;
bool myturn;
inline void flip_bit ( bool &_ ) { _ = !_; }
char num, dir;
std::fstream flog;
void logger ( std::string logfile ) {
	flog.open(logfile, std::fstream::out);
	if ( !flog.is_open() ) {
		throw std::runtime_error("error opening file\n");
	}
}
void print_board(std::string info, BOARD_GUI *pb){
	flog << "[" <<  info <<"]\n" << std::flush;
	for(int i = 0 ; i < 6 ; i ++){
		for(int j = 0 ; j < 7 ; j ++){
			if(pb -> now[7 * i + j].c != NULL){
				flog << " " << pb -> now[7 * i + j].c -> c << pb -> now[7 * i + j].c -> num << " |" << std::flush;
			}
			else{
				flog << "    |" << std::flush;
			}
		}
		flog << "\n----|----|----|----|----|----|----|\n" << std::flush;
	}
	return;
}

using PII = std::pair<int, int>;
// PII m_sim[30*12];


int main () 
{
	logger("mylog");
	auto timer = [] ( bool reset = false ) -> double {
		static decltype(std::chrono::steady_clock::now()) tick, tock;
		if ( reset ) {
			tick = std::chrono::steady_clock::now();
			return (0);
		}
		tock = std::chrono::steady_clock::now();
		return (std::chrono::duration_cast<std::chrono::duration<double>>(tock-tick).count());
	};

	srand(time(NULL));

	do {
		/* get initial positions */
		for ( int i=0; i<2; ++i ) { for ( int j=0; j<NUM_CUBE; ++j ) {
				init[i][j] = getchar();
		}}
		init[0][NUM_CUBE] = init[1][NUM_CUBE] = '\0';
		start = getchar();

		flog << init[0] << " " << init[1] << std::endl << std::flush;
		flog << start << std::endl << std::flush;

		/* game start s*/
		b = new BOARD_GUI(init[0], init[1]);
		b->no_hl = 1;



		for ( myturn=(start=='f'); b->winner()==Color::OTHER; flip_bit(myturn) ) {
			if ( myturn ) {
/***************************************** TODO *****************************************************/
				/* Reset the timer */
				timer(true);
				auto mlist = b -> move_list();

				// flog << "[INFO] {MOVE LIST} "; 
				// for(int i = 0 ; i < mlist.size() ; i++){
				// 	flog << "(" << mlist[i].first << ", " << mlist[i].second << ") ";
				// }
				// flog << "\n" << std::flush;

				if(mlist[0].first == 15){
#ifdef INFO
					flog << "[INFO] PASS!!\n" << std::flush;
#endif					
					b->do_move(mlist[0]);
					continue;
				}
#ifdef DEBUG
				print_board("init", b);
				flog << "[INFO] base_idx for root: " << base_idx << "\n" << std::flush;
#endif	
				/* Initialize the root */
				int root_id = base_idx;
				init_root_node(root_id);
				base_idx ++;
#ifdef DEBUG
				flog << "[INFO] base_idx for root: " << root_id << "\n" << std::flush;
#endif		
				/* MCTS */
				for(int do_mcts = 1; ; do_mcts ++){
#ifdef INFO
					flog << "[INFO] mcts time: " << do_mcts << "\n" << std::flush;
#endif
					/* Copy current state */
					tmp_b = *b;

					/* Find PV path to leaf */
					if(timer(false) > STOP_TIME){
						break;
					}
					int leaf_id = get_PV_path(root_id, &tmp_b); // will do move on tmp_b
#ifdef INFO
					flog << "[INFO] {Found PV path} got pv leaf id: " << leaf_id << "\n" << std::flush;
#endif
					/* Get valid movement */
					auto mlv = tmp_b.move_list();

					/* Expand */
					if(nodes[leaf_id].Nchild == 0){ // leaf
						expand(leaf_id, mlv);
					}
#ifdef INFO
					flog << "[INFO] {Expand} expand " << nodes[leaf_id].Nchild << " childs\n" << std::flush;
#endif
					/* Simulation */
					int num_win, num_win_square, num_sim;
					int total_num_win, total_num_win_square, total_num_sim;
					int childi, result;
					PII sim_ply;
					bool timeout = false;

					total_num_sim = total_num_win = total_num_win_square = 0;
					for(int i = 0 ; i < nodes[leaf_id].Nchild ; i++){

						childi = nodes[leaf_id].c_id[i];
						sim_ply = std::make_pair(move_cube(nodes[childi].ply), move_dir(nodes[childi].ply));
						tmp_b.do_move(sim_ply);
						
						// flog << "[INFO] timer before simulation: " << timer(false) << "\n" << std::flush;
					
						num_sim = num_win = num_win_square = 0;
						timeout = false;
						for(int sim = 0 ; sim < SIM_NUM ; sim ++){
							if(timer(false) > STOP_TIME){ // check timer before simulation
								timeout = true;
								break;
							}

							int sim_step = 0;
							flog << "[INFO] timer before copy " << timer(false) << std::flush;
							sim_b = tmp_b;
							flog << "[INFO] timer after copy " << timer(false) << std::flush;
							/* Simulation */
							while(sim_b.winner() == Color::OTHER){
								// flog << "[INFO] timer single simulation {begin}: " << timer(false) << "\n" << std::flush;
								/* check if game has end */
								
								// flog << "[INFO] timer single simulation {check}: " << timer(false) << "\n" << std::flush;
								/* get valid movement */
								auto ml = sim_b.move_list();
								// flog << "[INFO] timer single simulation {ml}: " << timer(false) << "\n" << std::flush;

								flog << "[INFO] {ml}: ";
								for(int mo = 0 ; mo < ml.size() ; mo ++){
									flog << "(" << ml[mo].first << ", " << ml[mo].second << ") ";
								}
								flog << "\n" << std::flush;

								/* random select a movement */
								// auto m = ml.at(rand()%ml.size());

								/* Greedy select a movement */
								std::pair<int, int> m = std::pair<int, int>(-1, -1);
								// for ( auto &move: ml ) {
								// 	if ( sim_b.yummy(move) == 1 ) {
								// 		m = move;
								// 		break;
								// 	}
								// 	else if(m.first == -1 && move.second < 2){
								// 		m = move;
								// 	}
								// }
								if ( m.first == -1 ) {
									m = ml[rand()%ml.size()];
								}
								print_board("Before do move in sim", &sim_b);
								flog << "[INFO] timer single simulation {select}: " << timer(false) << "\n" << std::flush;
								flog << "[INFO] movement {select}: (" << m.first << ", " << m.second << ")\n" << std::flush;

								sim_b.do_move(m);
								sim_step ++;
								// flog << "[INFO] timer single simulation {do}: " << timer(false) << "\n" << std::flush;

							}
							/* Undo movement in simulation */
							// flog << "[INFO] timer before single simulation {before copy}: " << timer(false) << "\n" << std::flush;
							/* check win/loss */
							if(sim_b.winner() == Color::R){
								if(start == 'f'){
									result =  1;
								}
								else{
									result =  -1;
								}
							}
							else if(sim_b.winner() == Color::B){
								if(start == 's'){
									result =  1;
								}
								else{
									result =  -1;
								}
							}
							else{
								result = 0;
							}
							// flog << "[INFO] Result: " << result << " in single simulation\n" << std::flush;
							// for(int st = 0 ; st < sim_step ; st ++){
							// 	// flog << "[INFO] In undo move : (color) " <<  tmp_b.history.back().whose_move << "\n" << std::flush; //<< " (move) cube " << tmp_b.history.back().c -> num << "\n" << std::flush; //"from " <<  tmp_b.history.back().start_pos << "to " << tmp_b.history.back().end_pos  << "\n"<< std::flush; 
							// 	tmp_b.undo_move();
							// }
							num_win += result;
							num_sim += 1;
							num_win_square += (int)(result != 0);
						}
						// flog << "[INFO] timer before single update: " << timer(false) << "\n" << std::flush;
						update_single_node(childi, num_win, num_win_square, num_sim);
						// flog << "[INFO] timer after single update: " << timer(false) << "\n" << std::flush;
						total_num_win += num_win;
						total_num_sim += num_sim;
						total_num_win_square += num_win_square;

						tmp_b.undo_move();
						if(timeout){

							flog << "[INFO] {Timeout} with timer: " << timer(false) << "\n" << std::flush;

							break;
						}
					}
#ifdef INFO
					flog << "[INFO] {Simulation Result} total: " << total_num_sim << " | win: " << total_num_win << "\n" << std::flush;
					flog << "[INFO] timer after simulation: " << timer(false) << "\n" << std::flush;
					flog << "[INFO] {Back Propogation Start} from depth " << nodes[leaf_id].depth << " to depth " << nodes[root_id].depth << "\n";
#endif
					/* Back Propagation */
					while(leaf_id != root_id){
#ifdef INFO
						flog << "[INFO] Back Propagation on depth: " << nodes[leaf_id].depth <<  "\n";
						// flog << "[INFO] timer between back propagation: " << timer(false) << "\n" << std::flush;
#endif
						update_single_node(leaf_id, total_num_win, total_num_win_square, total_num_sim);
						leaf_id = nodes[leaf_id].p_id;
					}
					update_single_node(root_id, total_num_win, total_num_win_square, total_num_sim);
#ifdef INFO
					flog << "[INFO] {Back Propagation End}\n" ;
					flog << "[INFO] timer after back propagation: " << timer(false) << "\n" << std::flush;
#endif
					if(timer(false) > STOP_TIME){
						break;
					}
					// delete tmp_b;
				}

				/* Find the child with highst win rate of root */
				int cid = nodes[root_id].c_id[0];
				long double num_win = nodes[cid].Average;
				long double win_most_num = nodes[cid].Average;
				PII m = std::make_pair(move_cube(nodes[cid].ply), move_dir(nodes[cid].ply)); /* movement to send out */

				for(int i = 1; i < nodes[root_id].Nchild ; i++){
					cid = nodes[root_id].c_id[i];
					
					num_win = nodes[cid].Average;
					if(num_win > win_most_num){
						m = std::make_pair(move_cube(nodes[cid].ply), move_dir(nodes[cid].ply));
						win_most_num = num_win;
					}
				}

#ifdef INFO			
				/* Debug Log */
				print_board("before send", b);
				flog << "[movement] cube: " << m.first << " move: " << m.second <<"\n" << std::flush;
				flog << "\n" << std::flush;
#endif
				flog << "[INFO] Total simulation: " << nodes[root_id].Ntotal << "\n" << std::flush;
/***************************************** TODO *****************************************************/

				b->do_move(m);
				flog << myturn << " " << b->send_move(m) << "\n================" << std::endl << std::flush;
				std::cout << b->send_move(m) << std::flush;
			}
			else {
				num = getchar()-'0';
				dir = getchar()-'0';

				flog << "FROM ANOTHER " << myturn << " " << int(num) <<  " " << int(dir) << std::endl << std::flush;
				// print_board("do another's move", b);
				if ( num == 16 ) {
					b->undo_move();
					b->undo_move();
					flip_bit(myturn);
				}
				else {
					b->do_move(num, dir);
				}
			}
		}
		flog << "winner: " << b->winner() << std::endl << std::flush;
		/* game end */
		delete b;
	} while ( getchar()=='y' ); 

	return (0);
}
