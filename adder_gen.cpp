#include <iostream>
#include <list>
#include <vector>
#include <cstdlib>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <unordered_map>
#include <map>
#include <random>
#include "PrefixAdder.hpp"
#include <set>

using namespace std;

vector<list<short int>> solutions;
vector<short int> fo_vector;
short int index = 3;        //starting adder
short int max_index;   //final adder
int approx;             //how many bits my adder looks back
int level;              //parameter for level pruning
int delta_pruning;   //parameter for dynamic size pruning
int repeat;          //parameter for repeatability pruning
int max_fo;      //parameter for max fan-out pruning
string nameFile;

int global_counter;
int global_counter_2;
int firstSolution = 0;
int min_size = 0;
int starting_acc;
vector<int> min_size_helper;

int fo_cnt = 0;

int max_per_size;
int adders_to_keep;

long solution_number_for_bw;

vector<string> files_to_clean;

int get_max_fanout(list<short int>& sol);
float er_uniform(list<short int>& sol);
void print_ER(list<short int>& adder, vector<float>& error_rates);

int LSB(list<short int>& List, list<short int>::iterator& thisNode) {

  int cnt = 0;
  list <short int>::iterator thisIter = thisNode;
  int flag = 0;

  if (thisNode != List.begin()) thisIter--;

  for (list<short int>::iterator it = List.begin(); it != List.end(); it++) {
    if (*it == index) cnt++;
  }

  //CASE 1: no recentNodes yet
  if (cnt == 0) return index;

  //CASE 2a: node!=index
  else if (*thisIter == 1) return 0;

  //CASE 2b: node!=index
  else if (*thisIter == 2){
    int counter=0;
    for(list<short int>::iterator iter8=List.begin(); iter8!=List.end(); iter8++){
      if(*iter8 == *thisIter) counter++;
    }
    if (counter == 1) return 0;
    else{
      int count_two = 0;
      for(list<short int>::iterator iter7=List.begin(); iter7!=thisNode; iter7++){
        if(*iter7 == *thisIter) count_two++;
      }
      if(count_two == 1) return 1;
      else return 0;
    }
  }

  //CASE 3: recentNode at begin of list
  else if (thisNode == List.begin()) return *thisNode - 1;

  //CASE 4: looks for the first smaller index(bit) right before it
  else if (*thisIter < *thisNode) return LSB(List, thisIter);

  
  else if (*thisIter == *thisNode) return LSB(List, thisIter) - 1;

  else {
    list<short int>::iterator iter = List.begin();
    flag = 0;
    for (list<short int>::iterator it = List.begin(); it != thisNode; it++) {
      if (*thisNode >= *it) {
        flag = 1;
        iter = it;
      }
    }
    if (flag == 0) return *thisNode - 1;
    else if (*iter == *thisNode) return LSB(List, iter) - 1;
    else return LSB(List, iter);
  }
}

list<short int>::iterator msb_parent(list<short int>& List2, list<short int>::iterator& thisNode2) {

  list<short int>::iterator iter2 = List2.begin();
  list<short int>::iterator iter = List2.begin();
  int flag = 0;

  if (thisNode2 == List2.begin()) return List2.end(); //the msb parent is an input bit, not a node

  for (iter2 = List2.begin(); iter2 != thisNode2; iter2++) {
    if (*iter2 == *thisNode2) {
      flag = 1;
      iter = iter2;
    }
  }
  if (flag == 0) return List2.end(); //the msb parent is an input bit, not a node
  else return iter;
}

list<short int>::iterator lsb_parent(list<short int>& List3, list<short int>::iterator& thisNode3) {

  list<short int>::iterator iter = List3.begin();
  int flag = 0;
  list<short int>::iterator it2 = thisNode3;

  if (thisNode3 != List3.begin()) it2--;

  if (*it2 == *thisNode3 && thisNode3 != List3.begin()) return List3.end(); //case 1: msb right before the node, so the lsb_parent is an input bit

  for (list<short int>::iterator it = List3.begin(); it != thisNode3; it++) {
    if (*it <= *thisNode3) {
      flag = 1;
      if (*it < *thisNode3) iter = it;
      else if (*it == *thisNode3) flag = 0; //it is an input node
    }
  }
  if (flag == 0) return List3.end(); //case 2: the lsb parent is an input bit, not a node
  else return iter; //case 3: the lsb_parent is a node
}

void fanout(list<short int>& List5, list<short int>::iterator& search_bit) {

  list<short int>::iterator lsb = lsb_parent(List5, search_bit);
  list<short int>::iterator msb = msb_parent(List5, search_bit);

  int distance_helper = distance(List5.begin(), search_bit);
  vector<short int>::iterator fo_insertion_helper = next(fo_vector.begin(), distance_helper);
  vector<short int>::iterator new_fo = fo_vector.insert(fo_insertion_helper, 0);

  if (msb != List5.end()) {
    int positionHelper = distance(List5.begin(), msb); //finds the position of the lsb_parent in the nodeList
    vector<short int>::iterator fo = next(fo_vector.begin(), positionHelper);  //finds the fo of the lsb_parent in fo_vector
    *fo = *fo + 1; //augments f.o. +1
  }
  if (lsb != List5.end()) {
    int positionHelper = distance(List5.begin(), lsb); //finds the position of the lsb_parent in the nodeList
    vector<short int>::iterator fo2 = next(fo_vector.begin(), positionHelper);  //finds the fo of the lsb_parent in fo_vector
    *fo2 = *fo2 + 1; //augments f.o. +1
  }
}

int print_fanout(list<short int>& List5, list<short int>::iterator& search_bit) {

  int f_out = 0;

  for (list<short int>::iterator it = List5.begin(); it != List5.end(); it++) {
    if ((lsb_parent(List5, it)) == search_bit || (msb_parent(List5, it)) == search_bit) {
      f_out++;
    }
  }
  return f_out;
}

void fanout_erase(list<short int>& Listt, list<short int>::iterator& search_bit) {

  list<short int>::iterator lsb = lsb_parent(Listt, search_bit);
  list<short int>::iterator msb = msb_parent(Listt, search_bit);

  int fo_eraser_helper = distance(Listt.begin(), search_bit);
  vector<short int>::iterator fo_eraser = next(fo_vector.begin(), fo_eraser_helper);
  fo_vector.erase(fo_eraser);

  if (msb != Listt.end()) {
    int positionHelper = distance(Listt.begin(), msb); //finds the position of the lsb_parent in the nodeList
    vector<short int>::iterator fo = next(fo_vector.begin(), positionHelper);  //finds the fo of the lsb_parent in fo_vector
    *fo = *fo - 1; //augments f.o. +1
  }
  if (lsb != Listt.end()) {
    int positionHelper = distance(Listt.begin(), lsb); //finds the position of the lsb_parent in the nodeList
    vector<short int>::iterator fo2 = next(fo_vector.begin(), positionHelper);  //finds the fo of the lsb_parent in fo_vector
    *fo2 = *fo2 - 1; //augments f.o. +1
  }
}

int depth(list<short int>& List5, list<short int>::iterator& thisNode5) {

  int i, positionHelper, positionHelper2;
  vector<short int>levelList;
  vector<short int>::iterator iter, iter2 = levelList.begin();

  thisNode5++;

  for (list<short int>::iterator it = List5.begin(); it != thisNode5; it++) {
    list<short int>::iterator lsb = lsb_parent(List5, it);
    list<short int>::iterator msb = msb_parent(List5, it);

    if (lsb != List5.end() && msb != List5.end()) {
      positionHelper = distance(List5.begin(), lsb);
      positionHelper2 = distance(List5.begin(), msb);

      iter2 = next(levelList.begin(), positionHelper2);
      iter = next(levelList.begin(), positionHelper);

      if (*iter <= *iter2) {
        i = *iter2 + 1;
        levelList.push_back(i);
      }
      else {
        i = *iter + 1;
        levelList.push_back(i);
      }
    }
    else if (lsb != List5.end()) {
      positionHelper = distance(List5.begin(), lsb); //finds the position of the lsb_parent in the nodeList
      iter = next(levelList.begin(), positionHelper); //finds the level of the lsb_parent in levelList
      i = *iter + 1; //adds one more level
      levelList.push_back(i); //iterators point to the same nodes,...
                              //..in nodeList the int is the bit and in levelList the int is the level of that bit
    }
    else if (msb != List5.end()) {
      positionHelper = distance(List5.begin(), msb);
      iter = next(levelList.begin(), positionHelper);
      i = *iter + 1;
      levelList.push_back(i);
    }
    else levelList.push_back(1); //if connected directly to an input bit
  }
  thisNode5--;
  return levelList.back();
}

int max_depth(list<short int>& List5) {

  int i, positionHelper, positionHelper2;
  vector<short int>levelList;
  vector<short int>::iterator iter, iter2 = levelList.begin();

  auto thisNode5 = List5.end();
  --thisNode5;

  int maximum_depth = -1;

  thisNode5++;

  for (list<short int>::iterator it = List5.begin(); it != thisNode5; it++) {
    list<short int>::iterator lsb = lsb_parent(List5, it);
    list<short int>::iterator msb = msb_parent(List5, it);

    if (lsb != List5.end() && msb != List5.end()) {
      positionHelper = distance(List5.begin(), lsb);
      positionHelper2 = distance(List5.begin(), msb);

      iter2 = next(levelList.begin(), positionHelper2);
      iter = next(levelList.begin(), positionHelper);

      if (*iter <= *iter2) {
        i = *iter2 + 1;
        levelList.push_back(i);
        if (i > maximum_depth) {
          maximum_depth = i;
        }
      }
      else {
        i = *iter + 1;
        levelList.push_back(i);
        if (i > maximum_depth) {
          maximum_depth = i;
        }
      }
    }
    else if (lsb != List5.end()) {
      positionHelper = distance(List5.begin(), lsb); //finds the position of the lsb_parent in the nodeList
      iter = next(levelList.begin(), positionHelper); //finds the level of the lsb_parent in levelList
      i = *iter + 1; //adds one more level
      levelList.push_back(i); //iterators point to the same nodes,...
                              //..in nodeList the int is the bit and in levelList the int is the level of that bit
      if (i > maximum_depth) {
        maximum_depth = i;
      }
    }
    else if (msb != List5.end()) {
      positionHelper = distance(List5.begin(), msb);
      iter = next(levelList.begin(), positionHelper);
      i = *iter + 1;
      levelList.push_back(i);
      if (i > maximum_depth) {
        maximum_depth = i;
      }
    }
    else levelList.push_back(1); //if connected directly to an input bit
  }
  thisNode5--;
  return maximum_depth;
}

void generate_txt() {
  int keepIndex = index;
  int beginList = 0;
  string numFile  = to_string(global_counter);
  string Index = to_string(index);
  string filename = Index + "__" + "file" + numFile + ".txt";

  ofstream file;
  file.open(filename.c_str());

  int numOfSol = 0;
  for(auto& sol: solutions){
    beginList = 0;
    numOfSol++;
    for(auto& node: sol){
      beginList++;
      if(beginList==1) file<<"S["<<numOfSol<<"]={ ";
      file<<node<<" ";
    }
    file<<"}\n";
  }
  index = keepIndex;
  file.close();

  solutions.clear();
  solutions.shrink_to_fit();
  files_to_clean.push_back(filename);
}

bool buildRecursive(list<short int>& nodeList, list<short int>::iterator& recentNode, list<short int>::iterator currIter) {
  bool flag;
  int node, searchIndex, k, lsb;
  static int flag4 = 0;
  int flag3 = 0;  //flag3 is used to avoid infinite recursion

  lsb = LSB(nodeList, recentNode);

  //all bits are approximate:
  if (index < approx) k = index;
  else k = approx;

  if (*recentNode == index && lsb == 0) {
    if (!flag4){
      if (solutions.size() >= 1000000){
        global_counter++;
        generate_txt();
      }
      if (firstSolution == 0) {
        min_size = nodeList.size();
        min_size_helper[index] = min_size;
        firstSolution = 1;
      }
      fo_cnt = 0;
      solutions.push_back(nodeList);
      solution_number_for_bw++;
      if (nodeList.size() < min_size) {
        min_size = nodeList.size();
        min_size_helper[index] = min_size;
      }
    }
    return true;
  }

  searchIndex = lsb - 1;
  list<short int>::iterator newIter = nodeList.begin();
  newIter = nodeList.insert(currIter, index);

  fanout(nodeList, newIter); //for fanout calculation

  int lsb2 = LSB(nodeList, newIter);

  ///PRUNING///
  int R = 0;
  flag4 = 0;

  for (list<short int>::iterator iter = nodeList.begin(); iter != nodeList.end(); iter++) {
    if (*iter == index && (LSB(nodeList, iter) != 0)) R++;
    else R = 0;

    if (R > repeat) {
      flag4 = 1;
      if (R >= repeat + 2) break;
    }
  }

  if (*max_element(fo_vector.begin(), fo_vector.end()) > max_fo) {
    flag4 = 1; //to avoid saving solutions that violate the pruning parameters
    fo_cnt ++;
  }

  if (depth(nodeList, newIter) > level || R == repeat + 2 || nodeList.size() > min_size+delta_pruning) {
    fo_cnt = 0; //for fanout pruning
    fanout_erase(nodeList, newIter); //for fanout calculation
    nodeList.erase(newIter);
    return false;
  }
  ///END OF PRUNING///

  if ((*newIter == index) && (lsb2 <= index - k) && (lsb2 != 0) && (!flag4) && (index >= starting_acc)) {
    if (solutions.size() >= 1000000){
      global_counter++;
      generate_txt();
    }
    if (firstSolution == 0) {
      min_size = nodeList.size();
      min_size_helper[index] = min_size;
      firstSolution = 1;
    }
    if (nodeList.size() < min_size){
      min_size = nodeList.size();
      min_size_helper[index] = min_size;
    }
    solutions.push_back(nodeList);
    solution_number_for_bw++;
  }

  flag = buildRecursive(nodeList, newIter, currIter);

  fanout_erase(nodeList, newIter); //for fanout calculation
  nodeList.erase(newIter);
  fo_cnt = 0; //for fanout pruning
  if (flag == true) return false;

  do {
    if (currIter == nodeList.end()) currIter = nodeList.begin();

    node = *currIter;
    currIter++;
    if (currIter == nodeList.end() && node != searchIndex) flag3 = 1; //case to avoid the infinite recursion

  } while (node != searchIndex);

  if (flag3 == 1) return false; //case to avoid the infinite recursion
  buildRecursive(nodeList, recentNode, currIter);
}

void read_from_txt(int &j, unordered_map<int, unordered_map<int, unordered_map<int, int>>>& size_occurences, int& solutions_for_next_bit_width){

  string numFile  = to_string(j);
  string Index = to_string(index-1);
  string filename = Index + "__" + "file" + numFile + ".txt";

  ifstream myReadFile;
  string line;
  list<list<short int>>mylist;
  list<short int>innerlist;

  myReadFile.open(filename.c_str());
  if(myReadFile.is_open()){
    while(!myReadFile.eof()){
      myReadFile >> line;
      if(line.find('{') == -1){
        if(line.find('}') == -1) innerlist.push_back( atoi(line.c_str()) );
        else mylist.push_back(innerlist);
      }
      else innerlist.clear();
    }
  }
  int Count=0;
  for(auto& lis: mylist){
    Count++;
    if(Count < mylist.size()){
      bool found_sol = false; // find_sol(lis, index -1);
      int max_sol_depth = max_depth(lis);
      int max_fanout = get_max_fanout(lis);
      if((lis.size() <= min_size_helper[index-1]+delta_pruning) && (size_occurences[lis.size()][max_sol_depth][max_fanout] < max_per_size)) { 
        size_occurences[lis.size()][max_sol_depth][max_fanout]++;
        solutions_for_next_bit_width++;
        list<short int> List = lis;
        list<short int>::iterator arrow = List.begin();
        list<short int>::iterator lastNode = List.begin();

        fo_vector.clear();
        fo_vector.resize(List.size());
        for(list<short int>::iterator it=List.begin(); it!=List.end(); it++) fanout(List, it);
          buildRecursive(List, lastNode, arrow);
      } 
    }
  }
  myReadFile.close();
}

void generateSolutions(list<short int>& List6, list<short int>::iterator& thisNode6, list<short int>::iterator& curIt) {
  vector<list<short int>> solutions2;

  fo_vector.clear();
  fo_vector.resize(List6.size());
  for(list<short int>::iterator it=List6.begin(); it!=List6.end(); it++) fanout(List6, it);

  min_size_helper[2] = 3; //min size helper is used for remembering previous min sizes when reading from txt files
  min_size = 5555;        //random big value that will never be reached
  cout << "Generating solutions for " << index + 1 << " bits: ";
  solution_number_for_bw = 0;
  buildRecursive(List6, thisNode6, curIt);

  ///pass the second 3-bit sequence
  List6 = { 2,2,1 };
  thisNode6 = List6.begin();
  curIt = List6.begin();
  fo_vector.clear();
  fo_vector.resize(List6.size());
  for(list<short int>::iterator it=List6.begin(); it!=List6.end(); it++) fanout(List6, it);

  buildRecursive(List6, thisNode6, curIt);
  cout << solution_number_for_bw << " solutions generated." << endl;

  index++;

  while (index <= max_index) {
    solution_number_for_bw = 0;
    cout << "Generating solutions for " << index + 1 << " bits: ";
    firstSolution  = 0;
    global_counter = 0;
    int solutions_for_next_bit_width = 0;
    // cout << "sols "  << solutions.size() << endl;
    // cout << "\nbit " << index << endl;

    unordered_map<int, unordered_map<int, unordered_map<int, int>>> size_occurences; 

    //process only the solutions that obide the delta pruning
    for(auto& sol: solutions){
      int max_sol_depth = max_depth(sol);
      int max_fanout = get_max_fanout(sol);
      if((sol.size() <= min_size + delta_pruning) && (size_occurences[sol.size()][max_sol_depth][max_fanout] < max_per_size)) {
        size_occurences[sol.size()][max_sol_depth][max_fanout]++;
        solutions2.push_back(sol);
      } 
    }

    solutions.clear();
    min_size = 555;

    solutions_for_next_bit_width += solutions2.size();

    for (auto& sols2 : solutions2) {
      List6 = sols2;
      thisNode6 = List6.begin();
      curIt = List6.begin();

      fo_vector.clear();
      fo_vector.resize(List6.size());
      for(list<short int>::iterator it=List6.begin(); it!=List6.end(); it++) fanout(List6, it);
      buildRecursive(List6, thisNode6, curIt);
    }
    solutions2.clear();

    //find solutions from the sequences of the txt files
    for(int i=1; i<=global_counter_2; i++) {
      read_from_txt(i, size_occurences, solutions_for_next_bit_width);
    } 

    cout << solution_number_for_bw << " solutions generated." << endl;
    global_counter_2 = global_counter;
    index++;
  }
}

int get_max_fanout(list<short int>& sol) 
{
  int max_fanout = 0;

  for (list<short int>::iterator node=sol.begin(); node!=sol.end(); node++) {
    int  current_fanout = print_fanout(sol, node);
    if (current_fanout > max_fanout) 
      max_fanout = current_fanout;
  }

  return max_fanout;
}

struct ListCmp {
    bool operator()(const list<short int>& lhs, const list<short int>& rhs) const { 
        return lhs.size() < rhs.size(); 
    }
};

void print_csv(multiset<list<short int>, ListCmp>& adders_to_report) {
  string filename = nameFile +"_report.csv";

  cout << "Generating csv file..." << endl;

  ofstream file;
  file.open(filename.c_str());

  int adder_index = 0;

  file << "adder,level,min-carry-chain,max-carry-chain,max-fanout,#operators,EF-uniform,MRE-uniform,EF-mixed,MRE-mixed" << endl;

  for(auto it = adders_to_report.begin(); it != adders_to_report.end(); ++it) {
    list<short int> sol = *it;
    int flag, level, current_fanout, max_level = 0;
    int max_fanout = 0;

    for (list<short int>::iterator node=sol.begin(); node!=sol.end(); node++) {
      current_fanout = print_fanout(sol, node);
      if (current_fanout > max_fanout) max_fanout = current_fanout;

      if (*node > 2) {
        list<short int>::iterator it=node;
        it++;
        for(list<short int>::iterator iter=it; iter!=sol.end(); iter++) {
          if(*iter==*node){
            flag = 1;
            break;
          }
        }
        if (flag == 0) {
          index = *node;
          level = depth(sol,node);
          if (level > max_level) max_level=level;
        }
      }
      flag=0;
    }
    vector<float> errors(4, 0.0);
    print_ER(sol, errors);

    int min_carry_chain = max_index + 1;
    int max_carry_chain = -1;
    for (int i = 1; i <= max_index; i++) {
      for (auto n = sol.end(); n-- != sol.begin();) {
        if (*n == i) {
          int lsb = LSB(sol, n);
          int carry_chain =  i - lsb + 1;
          if (carry_chain < min_carry_chain && lsb != 0) {
            min_carry_chain = carry_chain;
          }
          if (carry_chain > max_carry_chain) {
            max_carry_chain = carry_chain;
          }
          break;
        }
      }
    }


    ///PRINT CHARACTERISTICS
    file << nameFile << "_" << adder_index << "," << max_level << ",";
    file << min_carry_chain << "," << max_carry_chain << ",";
    file << max_fanout << ","; 
    file << sol.size() << ",";
    file << errors[0] << "," << errors[1] << "," << errors[2] << "," << errors[3] << endl;
    max_level = 0;
    max_fanout = 0;
    adder_index++;
  }
  file.close();
  cout << "Wrote results in " << filename << endl;
}

void print_ER(list<short int>& adder, vector<float>& error_rates) {
  vector<int> sol_vec;
  for (auto i : adder) {
    sol_vec.push_back(i);
  }
  PrefixAdder add(max_index + 1, sol_vec);

  for (int mode = 0; mode < 2; mode++) {
    int errors = 0;
    float re = 0;
    random_device rd;
    mt19937 gen(rd());
    int min_limit = max((long) -2147483648, (long) -pow(2, max_index));
    int max_limit = min((long) 2147483647, (long) pow(2, max_index) - 1);
    uniform_int_distribution<> dist(min_limit, max_limit);
    uniform_real_distribution<> dist_coin(0, 1);
    int sigma = 256;
    normal_distribution<> dist_norm(0, sigma);
    float uniform_chance = 0.5;
    if (mode == 0) {
      uniform_chance = 1.0;
    }

    for (int i = 0; i < 50000; i++) {

      float coin = dist_coin(gen);
      int a, b;
      if (coin < uniform_chance) {
        a = dist(gen);
        b = dist(gen);
      } else {
        a = (short) round(dist_norm(gen));
        b = (short) round(dist_norm(gen));
      }
      int sum = add.calculate_sum(a, b);
      int true_sum = a + b;

      if (sum != true_sum) {
        errors++;
        if (true_sum != 0) {
          re += abs(((sum - true_sum)/true_sum));
        }
      }
    }

    if (mode == 0) {
      error_rates[0] = errors / 50000.0;
      error_rates[1] = re / 50000.0;
    } else {
      error_rates[2] = errors / 50000.0;
      error_rates[3] = re / 50000.0;
    }
  }
}

void generate_verilog(const list<short int>& adder, string& adder_name) {
  int n = max_index + 1;
  int k = adder.size();
   
  auto it = adder.begin();

  ofstream file;
  string filename = adder_name + ".sv";
  file.open(filename);

  file<<"module " << adder_name << "\n(\n";
  file<<" input logic ["<<n-1<<":0] A,\n";
  file<<" input logic ["<<n-1<<":0] B,\n";
  file<<" output logic ["<<n<<":0] S\n);\n\n";

  file<<" logic ["<<n-1<<":0] P;\n";
  file<<" logic ["<<n-1<<":0] G;\n";

  file<<"\n\n//PREPROCESSING STAGE\n";
  for(int i=0; i<n; i++){
    file<<"assign G["<<i<<"] = A["<<i<<"] & B["<<i<<"];\n";
    file<<"assign P["<<i<<"] = A["<<i<<"] ^ B["<<i<<"];\n\n";
  }

  file<<"\n//PARALLEL PREFIX LOGIC\n";
  file<<" logic ["<<k-1<<":0] P_mid;\n";
  file<<" logic ["<<k-1<<":0] G_mid;\n";
  file<<" logic ["<<n-1<<":0] G_new;\n\n";

  file<<"assign G_new[0] = G[0];\n";

  for(it=adder.begin(); it!=adder.end(); it++) {

    int flag = 0;
    int counter = 0;

    if(it==adder.begin()) {
      file<<"assign G_mid["<<0<<"] = G["<<*it<<"] | (P["<<*it<<"] & G["<<*it-1<<"]);\n";
      file<<"assign P_mid["<<0<<"] = P["<<*it<<"] & P["<<*it-1<<"];\n\n";
    }
    else {
      auto itr = adder.begin();
      auto itr2 = adder.begin();
      auto itr3 = it;
      itr3--;

      int counter=0;
      for(auto iter=adder.begin(); iter!=it; iter++){
        if(*iter==*it){ //same nodes before
          counter++;
          itr2 = iter;
        }
        if(*iter<*it){ //keep the last smaller node
          flag=1;
          itr = iter;
        }
      }
      if(flag==1 && counter==0) { //smaller nodes before, no same nodes before
        file<<"assign G_mid["<<distance(adder.begin(),it)<<"] = G["<<*it<<"] | (P["<<*it<<"] & G_mid["<<distance(adder.begin(),itr)<<"]);\n";
        file<<"assign P_mid["<<distance(adder.begin(),it)<<"] = P["<<*it<<"] & P_mid["<<distance(adder.begin(),itr)<<"];\n\n";
      }
      else if(flag==1 && counter!=0) { //smaller nodes before, same nodes before
        if(*itr3==*it){
          file<<"assign G_mid["<<distance(adder.begin(),it)<<"] = G_mid["<<distance(adder.begin(),itr2)<<"] | (P_mid["<<distance(adder.begin(),itr2)<<"] & G["<<*itr-counter-1<<"]);\n";
          file<<"assign P_mid["<<distance(adder.begin(),it)<<"] = P_mid["<<distance(adder.begin(),itr2)<<"] & P["<<*itr-counter-1<<"];\n\n";
        }
        else {
          file<<"assign G_mid["<<distance(adder.begin(),it)<<"] = G_mid["<<distance(adder.begin(),itr2)<<"] | (P_mid["<<distance(adder.begin(),itr2)<<"] & G_mid["<<distance(adder.begin(),itr)<<"]);\n";
          file<<"assign P_mid["<<distance(adder.begin(),it)<<"] = P_mid["<<distance(adder.begin(),itr2)<<"] & P_mid["<<distance(adder.begin(),itr)<<"];\n\n";
        }
      }
      else if(flag==0 && counter==0) { //no smaller nodes before, no same nodes before(only bigger)
        file<<"assign G_mid["<<distance(adder.begin(),it)<<"] = G["<<*it<<"] | (P["<<*it<<"] & G["<<*it-1<<"]);\n";
        file<<"assign P_mid["<<distance(adder.begin(),it)<<"] = P["<<*it<<"] & P["<<*it-1<<"];\n\n";
      }
      else if(flag==0 && counter!=0) { //no smaller nodes before, same nodes before
        file<<"assign G_mid["<<distance(adder.begin(),it)<<"] = G_mid["<<distance(adder.begin(),itr2)<<"] | (P_mid["<<distance(adder.begin(),itr2)<<"] & G["<<*it-counter-1<<"]);\n";
        file<<"assign P_mid["<<distance(adder.begin(),it)<<"] = P_mid["<<distance(adder.begin(),itr2)<<"] & P["<<*it-counter-1<<"];\n\n";
      }
    }
    int flag2=0;
    auto itr4 = it;
    itr4++;
    for(auto itr5=itr4; itr5!=adder.end(); itr5++) {
      if(*itr5==*it) {
        flag2 = 1;
        break;
      }
    }
    if(flag2==0) {
      file<<"assign G_new["<<*it<<"] = G_mid["<<distance(adder.begin(),it)<<"];\n";
    }
  }

  file<<"\n//CALCULATION OF SUM\n";
  file<<"assign S[0] = P[0];\n";
  for(int i=1; i<n; i++) {
    file<<"assign S["<<i<<"] = P["<<i<<"] ^ G_new["<<i-1<<"];\n";
  }
  file<<"assign S["<<n<<"] = G_new["<<n-1<<"] ^ A[" << n-1 << "] ^ B[" << n-1 << "];\n";
  file<<"\nendmodule";
  file.close();
}

void clean_files() {
  for (string s : files_to_clean) {
    remove(s.c_str());
  }
}

int main(int argc, char** argv) {

  if(argc != 7){
    cout << "Wrong argument number! \n Provide 6 command line arguments in the following order:" << endl;
    cout << "bit width, minimum carry chain, maximum levels, maximum fanout, number of adders to report, verilog file name" << endl;
    return 1;
  }

  auto start = chrono::steady_clock::now();

  max_index = stoi(argv[1]) - 1;
  approx = stoi(argv[2]) - 1;
  level = stoi(argv[3]);
  delta_pruning = round(0.7 * (max_index + 1));
  max_fo = stoi((argv[4]));
  repeat = 1;
  adders_to_keep = stoi(argv[5]);
  if (adders_to_keep > 100000) {
    cout << "Maximum number of solutions that can be reported: 100.000" << std::endl;
    return 1;
  }
  nameFile = argv[6];
  starting_acc = 0;
  max_per_size = 500;
  global_counter = 0;

  min_size_helper.resize(max_index + 1);

  vector<list<short int>> solutions3;
  list<short int> List = { 1, 2};
  list<short int>::iterator arrow = List.begin(); 
  list<short int>::iterator lastNode = List.begin(); 

  generateSolutions(List, lastNode, arrow);

  index = max_index;

  //put the vector in a file
  global_counter++;
  generate_txt();

  //remove solutions that violate the delta pruning and print their characteristics
  int cnt_of_sols = 0;
  int file_counter = 0;

  list<short int> smallest_sol;
  int smallest_size = 1000;
  int smallest_size_fanout = -1;

  map<int, int> size_map;


  // auto cmp = [](list<short int>& a, list<short int>& b) { return a.size() < b.size() };
  // set<list<short int>, decltype(cmp)> adders_to_report(cmp);
  multiset<list<short int>, ListCmp> adders_to_report;


  for(int i=1; i <= (global_counter_2 + 1); i++) {
    string numFile  = to_string(i);
    string Index = to_string(max_index);
    string filename = Index + "__" + "file" + numFile + ".txt";

    ifstream myReadFile;
    string line;
    list<list<short int>>mylist;
    vector<list<short int>>last_sols;
    list<short int>innerlist;

    myReadFile.open(filename.c_str());
    if(myReadFile.is_open()){
      while(!myReadFile.eof()){
        myReadFile >> line;
        if(line.find('{') == -1){
          if(line.find('}') == -1) innerlist.push_back( atoi(line.c_str()) );
          else mylist.push_back(innerlist);
        }
        else innerlist.clear();
      }
    }
    int Count = 0;


    for(auto& lis: mylist){
      Count++;
      if(Count < mylist.size()){
        if(lis.size() <= min_size+delta_pruning){
          size_map[lis.size()]++;
          last_sols.push_back(lis);
          cnt_of_sols++;
          if (adders_to_keep != 0) {
            if (cnt_of_sols <= adders_to_keep) {
              adders_to_report.insert(lis);
            } else {
              list<short int> largest_adder = *adders_to_report.rbegin();
              if (lis.size() < largest_adder.size()) {
                auto it = adders_to_report.end();
                --it;
                adders_to_report.erase(it);
                adders_to_report.insert(lis);
              }
            }
          }
          int fanout = get_max_fanout(lis);
          if (lis.size() < smallest_size) {
            smallest_size = lis.size();
            smallest_size_fanout = fanout;
            smallest_sol = lis;
          } 
        }
      }
    }
    index = max_index;
    // file_counter++;
    // print_characteristics(file_counter, last_sols);
    last_sols.clear();
    last_sols.shrink_to_fit();
    myReadFile.close();
  }

  clean_files();

  print_csv(adders_to_report);

  cout << "Generating verilog files..." << endl;
  int adder_num = 0;
  for (auto it = adders_to_report.begin(); it != adders_to_report.end(); ++ it) {
    string adder_name = nameFile + "_" + to_string(adder_num);
    generate_verilog(*it, adder_name);
    adder_num++;
  }  
  cout << "Wrote verilog files in " << nameFile << "_x.sv" << endl;

  cout << "Final solution number after pruning: "   << cnt_of_sols << endl;


  cout << "adder size -> # solutions with this size" << endl;
  for (auto& it : size_map) {
    cout << it.first << " -> " << it.second << endl;
  }

  auto end = chrono::steady_clock::now();
  chrono::duration<double> elapsed_seconds = end - start;
  cout << "elapsed time: " << elapsed_seconds.count()/60 << "m\n";
  cout << "elapsed time: " << elapsed_seconds.count() << "s\n";

  return 0;
}
