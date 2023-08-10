struct cell {

    cell(int& n, int _i, int _j) {
        i = _i;
        j = _j;
        cnt = n;
        ans = 0;
        used = false;
        maybe = vector<int>(n, 1);
        maybe_old = vector<int>(n, 1);
    }

    void rollback() {
        maybe = maybe_old;
        used = false;
        ans = -1;
        cnt = 0;
        for (int& num: maybe) {
            cnt += (num>0);
        }
    }

    void fix() {
        maybe_old = maybe;
    }

    int cnt, ans, i, j;
    vector<int> maybe;
    vector<int> maybe_old;
    bool confidence, used;
};

struct sudoku {

    sudoku() {
        create_board(9);
    }

    sudoku(int n) {
        create_board(n);
    }

    sudoku(vector<vector<char>>& board) {
        //printf("@sudoku(board);\n");
        create_board(board.size());
        for (size_t i = 0; i < n; ++i) {
            for (size_t j = 0; j < n; ++j) {
                if (board[i][j] != '.') {
                    detect_cell(i, j, board[i][j]-'1');

                    //printf("Detected %d at [%d][%d]\n", cells[i][j].ans+1, i, j);
                }
            }
        }
    }

    void create_board(int _n) {
        //printf("@create_board(%d);\n", n);
        n = _n;

        //создаем все клеки
        cells = vector<vector<cell*>>(n);
        for (size_t i = 0; i < n; ++i) {
            cells[i] = vector<cell*>();
            for (size_t j = 0; j < n; ++j) {
                cells[i].push_back(new cell(n, i, j));
                indet_cells.insert(cells[i][j]);
            }
        }

        //для каждой клетки определяем все соседние
        for (size_t i = 0; i < n; ++i) {
            for (size_t j = 0; j < n; ++j) {
                for (int k = 0; k < n; ++k) {
                   if (k!=i) {neighbor_cells[cells[i][j]].insert(cells[k][j]);}
                   if (k!=j) {neighbor_cells[cells[i][j]].insert(cells[i][k]);} 
                }

                size_t start_i = i/3*3, end_i = start_i+3;
                size_t start_j = j/3*3, end_j = start_j+3;
                
                for (size_t cur_i = start_i; cur_i < end_i; ++cur_i) {
                    for (size_t cur_j = start_j; cur_j < end_j; ++cur_j) {
                        if (i != cur_i || j != cur_j) {
                            neighbor_cells[cells[i][j]].insert(cells[cur_i][cur_j]);
                        }
                    }
                }
            }
        }
    }

    void print() {
        for (size_t i = 0; i < n; ++i) {
            for (size_t j = 0; j < n; ++j) {
                cout << (cells[i][j]->cnt != 1? char('.'): char('1' + cells[i][j]->ans)) << ((j+1)%3==0? '\t': ' ');
            }
            cout << ((i+1)%3==0? "\n\n": "\n");
        }
    }

    void cells_fix() {
        for (cell* indet_cell: indet_cells) {
            indet_cell->fix();
            unreliable_cells.insert(indet_cell);
        }
    }

    void cells_rollback() {
        for (cell* unreliable_cell: unreliable_cells) {
            unreliable_cell->rollback();
            if (det_cells.find(unreliable_cell) != det_cells.end()) {                
                det_cells.erase(unreliable_cell);
                indet_cells.insert(unreliable_cell);
            }
        } 
        unreliable_cells.clear();
    }

    bool is_correct() {
        for (cell* det_cell: det_cells) {
            for (cell* neighbor_cell: neighbor_cells[det_cell]) {
                if (
                    det_cell->cnt==1 &&
                    neighbor_cell->cnt == 1 &&
                    det_cell->ans == neighbor_cell->ans
                ) {
                    return false;
                }
            }
        }
        return true;
    }


    void detect_cell(int i, int j, int num) {
        //printf("@detect_cell(%d, %d, %d);\n", i, j, num+1);
        indet_cells.erase(cells[i][j]);
        det_cells.insert(cells[i][j]);
        useful_cells.insert(cells[i][j]);
        cells[i][j]->ans = num;
        cells[i][j]->cnt = 1;
    }

    void erase_from_cell(int i, int j, int num) {
        //printf("@erase_from_cell(%d, %d, %d);\n", i, j, num+1);
        if (!cells[i][j]->maybe[num] || cells[i][j]->cnt == 1) return;

        cells[i][j]->maybe[num] = false;
        if (--cells[i][j]->cnt == 1) {
            for (size_t k = 0; k < cells[i][j]->maybe.size(); ++k) {
                if (cells[i][j]->maybe[k])  {
                    detect_cell(i, j, k);
                    break;
                }
            }
            //printf("Detected %d\n", ans+1);
            //print();
        }
    }

    void use_cell(size_t i, size_t j) {
        //printf("@use_cell(%d, %d);\n", i, j);
        //printf("Used [%zd][%zd] = %d\n", i, j, ans);
        cells[i][j]->used = true;
        for (cell* neighbor_cell: neighbor_cells[cells[i][j]]) {
            erase_from_cell(neighbor_cell->i, neighbor_cell->j, cells[i][j]->ans);
            //printf("from cells[%zd][%zd] erased %d, cnt:%d\n", cur_i, cur_j, ans+1, cells[cur_i][cur_j].cnt);
        }
        useful_cells.erase(cells[i][j]);
    }

    void solve() {
        cell* main_indet_cell = nullptr;
        int main_indet_cell_maybe;

        //printf("@solve();\n");
        while (det_cells.size() < n*n) {
            prev_detected = det_cells.size();

            ///*
            /*
            for (size_t i = 0; i < n; ++i) {
                for (size_t j = 0; j < n; ++j) {
                    if (board[i][j] == '.') cells[i][j].print();
                } 
            }
            */
            //cout << "______________________\n\n";
            //*/

            for (cell* useful_cell: useful_cells) {
                if (useful_cell->cnt == 1 && !useful_cell->used) {
                    use_cell(useful_cell->i, useful_cell->j);
                }   
            }
            
            for (size_t i = 0; i < n; i+=3) {
                for (size_t j = 0; j < n; j+=3) {
    
                    size_t start_i = i, end_i = start_i+3;
                    size_t start_j = j, end_j = start_j+3;
                    
                    vector<size_t> maybe(n);

                    for (size_t cur_i = start_i; cur_i < end_i; ++cur_i) {
                        for (size_t cur_j = start_j; cur_j < end_j; ++cur_j) {
                            if (cells[cur_i][cur_j]->cnt > 1) {
                                for (size_t k = 0; k < n; ++k) {
                                    maybe[k] += cells[cur_i][cur_j]->maybe[k];
                                }
                            } else {
                                maybe[cells[cur_i][cur_j]->ans] = 10; 
                            }
                        }
                    }

                    /*
                    for (size_t k = 0; k < n; ++k) {
                        cout << maybe[k] << ' ';
                    }
                    cout << '\n';
                    */

                    for (size_t k = 0; k < n; ++k) {
                        if (maybe[k] == 1) {
                            for (size_t cur_i = start_i; cur_i < end_i; ++cur_i) {
                                for (size_t cur_j = start_j; cur_j < end_j; ++cur_j) {
                                    if (cells[cur_i][cur_j]->cnt > 1 && cells[cur_i][cur_j]->maybe[k]) {
                                        //printf("Detected %d at [%d][%d]\n", k+1, cur_i, cur_j);
                                        detect_cell(cur_i, cur_j, k);
                                        //cells[cur_i][cur_j].use(board, cur_i, cur_j, cells);
                                    }
                                    
                                }
                            }
                        }
                    }
                     
                }
            } //cout << '\n';
            //*/
   
            if (!is_correct()) {
                //printf("okay, it seems that [%d][%d] does not have %d\n", main_indet_cell->i+1, main_indet_cell->j+1, main_indet_cell_maybe+1);
                erase_from_cell(main_indet_cell->i, main_indet_cell->j, main_indet_cell_maybe);
                cells_rollback();
                //print();
            } else {
                //printf("I dont know...\n");
            }
            if (det_cells.size() == prev_detected) {
                //cout << "HMMMM\n";

                cells_rollback();                
                cells_fix();
                auto it = indet_cells.begin();
                advance(it, rand()%indet_cells.size());
                main_indet_cell = *it;

                if (!main_indet_cell) continue;
                for (int k = rand()%n; k < n; ++k) {
                    if (main_indet_cell->maybe[k]) {
                        main_indet_cell_maybe = k;
                        //printf("maybe in [%d][%d] %d?\n", main_indet_cell->i+1, main_indet_cell->j+1, main_indet_cell_maybe+1);
                        detect_cell(main_indet_cell->i, main_indet_cell->j, k);
                        break;
                    }
                }
                
            }
        }
    }

    void fill(vector<vector<char>>& _board) {
        for (size_t i = 0; i < n; ++i) {
            for (size_t j = 0; j < n; ++j) {
                _board[i][j] = (cells[i][j]->cnt > 1? '.': '1'+cells[i][j]->ans);
            }
        }
    }

    int n, prev_detected;
    vector<vector<cell*>> cells;
    map<cell*, set<cell*>> neighbor_cells; 
    set<cell*> useful_cells;
    set<cell*> det_cells;
    set<cell*> indet_cells;
    set<cell*> unreliable_cells;
};

class Solution {
public:

    void solveSudoku(vector<vector<char>>& board) {

        sudoku sudo(board);
        sudo.solve();
        sudo.fill(board);
       
    }
};
