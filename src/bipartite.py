#!/usr/bin/env python3
import sys
from itertools import accumulate

def read_graph():
    with open(sys.argv[-1]) as file:
        graph = [line.split() for line in file.readlines()]

    graph = graph[1:]
    graph = [int(element) for line in graph for element in line]
    return tuple(graph)

def make_edges(graph):
    edges = list(enumerate(graph))
    edges = [(order, value) for order, value in edges if value > 0]
    edges = [(divmod(ordinal, V), value) for ordinal, value in edges]
    edges = [(edge, value) for edge, value in edges if edge[0] < edge[1]]
    edges = sorted(edges, key=lambda x: x[1]) # , reverse=True)
    return edges

def is_connected(solution):
    return True # TODO

def check_solution(solution):
    global solutions
    if not is_connected(solution):
        return
    elif solution[0] > solutions[0][0]:
        solutions = [solution]
    elif solution[0] == solutions[0][0]:
        solutions.append(solution)

def solve(index, total, edges, left, right):
    global counter
    counter += 1

    if index < 0:
        check_solution((total, edges, left, right))
        return
    if total + VALUE_LEFT[index] < solutions[0][0]:
        return

    edge, value = EDGES[index]

    # Add v1 v2
    if edge[0] not in right and edge[1] not in left:
        solve(index-1, total+value, edges.copy()|{edge}, left.copy()|{edge[0]}, right.copy()|{edge[1]})

    # Add v2 v1
    if edge[0] not in left and edge[1] not in right:
        solve(index-1, total+value, edges.copy()|{edge}, left.copy()|{edge[1]}, right.copy()|{edge[0]})

    # Do not add
    solve(index-1, total, edges.copy(), left.copy(), right.copy())
    return

GRAPH = read_graph()
V = int(len(GRAPH) ** (1/2))
EDGES = make_edges(GRAPH)
VALUE_LEFT = list(accumulate(element[1] for element in EDGES))
counter = 0
solutions = [(0, set(), set(), set())]
solve(len(EDGES) - 1, 0, set(), {0}, set())
print(*[sol[0] for sol in solutions])
