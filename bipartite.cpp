#include <algorithm>
#include <fstream>
#include <iostream>
#include <numeric>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <typeinfo>
#include <vector>

using namespace std;

vector<pair<pair<int, int>, int>> EDGES;
vector<int> VALUE_LEFT;
vector<tuple<int, set<pair<int, int>>, set<int>, set<int>>> SOLUTIONS;
int COUNTER = 0;

vector<int> read_graph(char* path)
{
    vector<int> graph;
    string line;
    ifstream file(path);
    if (file.is_open()){
        while(getline(file,line)){
            // cout << line << '\n';
            stringstream iss(line);
            int number;
            while(iss >> number){
                graph.push_back(number);
            }
        }
        file.close();
    }
    return graph;
}

vector<pair<pair<int, int>, int>> make_edges(vector<int> graph,
                                             int vertex_count)
{
    vector<pair<pair<int, int>, int>> edges;
    int value;
    for (int v1 = 0; v1 < vertex_count; v1++){
        for (int v2 = v1 + 1; v2 < vertex_count; v2++){
            value = graph[v1*vertex_count+v2];
            if (value != 0){
                edges.push_back(make_pair(make_pair(v1, v2), value));
            }
        }
    }
    sort(edges.begin(), edges.end(), [](auto &left, auto &right) {
        return left.second < right.second;
    });
    return edges;
}

void check_solution(int total,
                    set<pair<int, int>> & edges,
                    set<int> & left,
                    set<int> & right)
{
    if (total > get<0>(SOLUTIONS[0])){
        SOLUTIONS.clear();
        SOLUTIONS.push_back(make_tuple(total, edges, left, right));
    } else if (total == get<0>(SOLUTIONS[0])) {
        SOLUTIONS.push_back(make_tuple(total, edges, left, right));
    }
}

void solve(int index,
           int total,
           set<pair<int, int>> & edges,
           set<int> & left,
           set<int> & right)
{
    if (index < 0) {
        check_solution(total, edges, left, right);
        return;
    }
    if (total + VALUE_LEFT[index] < get<0>(SOLUTIONS[0])) {
        return;
    }
    COUNTER++;
    // cout << COUNTER << " Solution count: " << SOLUTIONS.size() << " Total: " << get<0>(SOLUTIONS[0]) << endl;

    pair<int, int> edge = EDGES[index].first;
    int v1 = edge.first;
    int v2 = edge.second;
    int value = EDGES[index].second;

    // Add edge left - v1, right - v2
    if (right.count(v1) == 0 && left.count(v2) == 0) {
        set<pair<int, int>> edges_tmp(edges);
        edges_tmp.insert(edge);
        set<int> left_tmp(left);
        left_tmp.insert(v1);
        set<int> right_tmp(right);
        right_tmp.insert(v2);
        solve(index - 1, total + value, edges_tmp, left_tmp, right_tmp);
    }

    // Add edge left - v2, right - v1
    if (left.count(v1) == 0 && right.count(v2) == 0) {
        set<pair<int, int>> edges_tmp(edges);
        edges_tmp.insert(edge);
        set<int> left_tmp(left);
        left_tmp.insert(v2);
        set<int> right_tmp(right);
        right_tmp.insert(v1);
        solve(index - 1, total + value, edges_tmp, left_tmp, right_tmp);
    }

    // Add no edge
    solve(index - 1, total, edges, left, right);
}

int sum_edges(pair<pair<int, int>, int> first,
              pair<pair<int, int>, int> second)
{
    return first.second + second.second;
}

int main(int argc, char** argv)
{
    // cout << "typeid(argv[1]): "<< typeid(argv[1]).name() << endl;
    vector<int> graph = read_graph(argv[1]);
    int vertex_count = graph.front();
    graph.erase(graph.begin());
    // cout << "vertex_count: " << vertex_count << " graph: ";
    // for (auto i: graph)
    //     cout << i << ' ';
    // cout << endl;

    EDGES = make_edges(graph, vertex_count);
    // partial_sum(EDGES.begin(), EDGES.end(), VALUE_LEFT.begin(), sum_edges);
    VALUE_LEFT.push_back(EDGES[0].second);
    for (size_t i = 1; i < EDGES.size(); i++){
        VALUE_LEFT.push_back(VALUE_LEFT.back() + EDGES[i].second);
    }

    set<pair<int, int>> edges;
    set<int> left = {0};
    set<int> right;
    SOLUTIONS.push_back(make_tuple(0, edges, left, right));
    solve(EDGES.size() - 1, 0, edges, left, right);
    // cout << "Calls: " << COUNTER << " Max weight: " << get<0>(SOLUTIONS[0]) << " Solution count: " << SOLUTIONS.size() << endl;
    for (auto &solution : SOLUTIONS) {
        cout << get<0>(solution) << ((&solution != &SOLUTIONS.back())? " ": "\n");
    }
    return 0;
}
