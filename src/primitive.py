#!/usr/bin/env python3
import sys
from dataclasses import dataclass


@dataclass
class Edge:
    value: int
    begin: int
    end: int

    def __init__(self, value, begin, end):
        self.value = value
        self.begin = begin
        self.end = end

@dataclass
class State:
    idx: int
    value: int
    vertices: list[int]
    edges: list[int]

    def __init__(self, idx, value, vertices, edges):
        self.idx = idx
        self.value = value
        self.vertices = vertices
        self.edges = edges

global VERTEX_COUNT
EDGES: list[Edge] = []
EDGE_COUNT: int
EDGE_INDICES: list[int]
VALUE_LEFT: list[int] = []

BEST: list[State] = [State(-1, -1, None, None) for i in range(100)]
BEST_IDX: int = -1

def read_graph():
    global GRAPH
    with open(sys.argv[-1]) as file:
        graph = [line.split() for line in file.readlines()]

    graph = graph[1:]
    graph = [int(element) for line in graph for element in line]
    GRAPH = tuple(graph)

def make_edges():
    global EDGE_COUNT
    global EDGES
    global EDGE_INDICES
    global VALUE_LEFT
    EDGE_COUNT = 0
    for row in range(VERTEX_COUNT):
        for col in range(row+1, VERTEX_COUNT):
            if GRAPH[row*VERTEX_COUNT+col] > 0:
                EDGE_COUNT += 1

    EDGES = [Edge(-1, -1, -1) for i in range(EDGE_COUNT)]

    edge_idx = -1
    for begin in range(VERTEX_COUNT):
        for end in range(begin+1, VERTEX_COUNT):
            value = GRAPH[begin * VERTEX_COUNT + end]
            if value > 0:
                edge_idx += 1
                EDGES[edge_idx] = Edge(value, begin, end)

    EDGES = sorted(EDGES, key=lambda e: e.value)

    EDGE_INDICES = [-1 for i in range(VERTEX_COUNT**2)]
    for idx in range(EDGE_COUNT):
        EDGE_INDICES[EDGES[idx].begin * VERTEX_COUNT + EDGES[idx].end] = idx
        EDGE_INDICES[EDGES[idx].end * VERTEX_COUNT + EDGES[idx].begin] = idx

    VALUE_LEFT = [0 for i in range(EDGE_COUNT)]
    VALUE_LEFT[EDGE_COUNT-1] = EDGES[EDGE_COUNT-1].value
    for idx in range(EDGE_COUNT-1-1, -1, -1):
        VALUE_LEFT[idx] = VALUE_LEFT[idx+1] + EDGES[idx].value


def is_connected_recursive(vertex, edges, visited):
    visited[vertex] = 1
    for vertex_other in range(VERTEX_COUNT):
        edge_idx = EDGE_INDICES[vertex * VERTEX_COUNT + vertex_other]
        if edge_idx >= 0:
            if edges[edge_idx] == 1 and visited[vertex_other] == 0:
                is_connected_recursive(vertex_other, edges, visited)

def is_connected(state):
    visited = [0 for i in range(VERTEX_COUNT)]
    is_connected_recursive(0, state.edges, visited)
    visited_count = 0
    for idx in range(VERTEX_COUNT):
        if visited[idx] == 1:
            visited_count += 1

    if visited_count == VERTEX_COUNT:
        return True
    else:
        return False

def is_bipartite_recursive(vertex, color, visited, vertices):
    visited[vertex] = 1
    vertices[vertex] = color
    for vertex_other in range(VERTEX_COUNT):
        edge_idx = EDGE_INDICES[vertex * VERTEX_COUNT + vertex_other]
        if edge_idx >= 0:
            if vertices[vertex_other] == color:
                return False
            if visited[vertex_other] == 0:
                if not is_bipartite_recursive(vertex_other, (color+1)%2, visited, vertices):
                    return False
    return True

def is_bipartite(vertices):
    visited = [0 for i in range(VERTEX_COUNT)]
    return is_bipartite_recursive(0, 0, visited, vertices)

def state_new(idx, value):
    vertices = [-1 for i in range(VERTEX_COUNT)]
    edges = [0 for i in range(EDGE_COUNT)]
    return State(idx, value, vertices, edges)

def state_copy(state):
    vertices = [color for color in state.vertices]
    edges = [used for used in state.edges]
    return State(state.idx, state.value, vertices, edges)

def state_free(state):
    ...

def solve(state):
    global BEST
    global BEST_IDX

    # Critical section
    if (state.edges[state.idx] == 1
        and state.value >= BEST[0].value
        and is_connected(state)):
        state_best = state_copy(state)
        if state.value == BEST[0].value:
            BEST_IDX += 1
        else:
            for i in range(BEST_IDX):
                state_free(BEST[i])
            BEST_IDX = 0
        BEST[BEST_IDX] = state_best

    state.idx += 1

    if state.idx >= EDGE_COUNT:
        return

    if state.value + VALUE_LEFT[state.idx] < BEST[0].value:
        return

    vertex_begin = EDGES[state.idx].begin
    vertex_end = EDGES[state.idx].end

    # add 0-1
    if state.vertices[vertex_begin] != 1 and state.vertices[vertex_end] != 0:
        state_0_1 = state_copy(state)
        state_0_1.vertices[vertex_begin] = 0
        state_0_1.vertices[vertex_end] = 1
        state_0_1.edges[state.idx] = 1
        state_0_1.value += EDGES[state.idx].value
        solve(state_0_1)

    # add 1-0
    if state.vertices[vertex_begin] != 0 and state.vertices[vertex_end] != 1:
        state_1_0 = state_copy(state)
        state_1_0.vertices[vertex_begin] = 1
        state_1_0.vertices[vertex_end] = 0
        state_1_0.edges[state.idx] = 1
        state_1_0.value += EDGES[state.idx].value
        solve(state_1_0)

    # does not add
    solve(state)


if __name__ == '__main__':
    global GRAPH
    read_graph()
    VERTEX_COUNT = int(len(GRAPH) ** (1/2))
    make_edges()

    vertices = [-1 for color in range(VERTEX_COUNT)]
    if not is_bipartite(vertices):
        state = state_new(-1, 0)
        state.vertices[0] = 0
        BEST[0] = state_copy(state)
        solve(state)
        for best in BEST[:BEST_IDX + 1]:
            print(best.value, end=' ') if best != BEST[BEST_IDX] else print(best.value)
    else:
        print('bipartite')
