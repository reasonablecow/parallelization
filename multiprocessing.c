#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mpi.h>

#define MAX_PARALLEL_DEPTH 5
#define BOSS 0
#define TAG_WORK 1
#define TAG_TERMINATE 2

typedef struct {
  int value;
  int begin;
  int end;
} Edge;
int edge_cmp_desc(const void *a, const void *b)
{
    Edge edge_a = *(const Edge *) a;
    Edge edge_b = *(const Edge *) b;

    if (edge_a.value < edge_b.value) return 1;
    if (edge_a.value > edge_b.value) return -1;
    return 0;
}

int *GRAPH = NULL;
int VERTEX_COUNT = 0;
int EDGE_COUNT = 0;
Edge* EDGES;
int* EDGE_INDICES;
int* VALUE_LEFT;

typedef struct {
    int idx;
    int value;
    int *vertices;
    int *edges;
} State;
State state_new()
{
    int *vertices = (int *) malloc(VERTEX_COUNT * sizeof(int));
    for (int i = 0; i < VERTEX_COUNT; i++) vertices[i] = -1;
    return (State) {.idx = -1,
                    .value = 0,
                    .vertices = vertices,
                    .edges = (int *) calloc(EDGE_COUNT, sizeof(int))};
}
void state_free(State *state)
{
    free(state->edges);
    free(state->vertices);
}
State state_copy(State *state)
{
    int *vertices = (int *) malloc(VERTEX_COUNT * sizeof(int));
    for (int i = 0; i < VERTEX_COUNT; i++) vertices[i] = state->vertices[i];
    int *edges = (int *) malloc(EDGE_COUNT * sizeof(int));
    for (int i = 0; i < EDGE_COUNT; i++) edges[i] = state->edges[i];
    return (State) {.idx = state->idx,
                    .value = state->value,
                    .vertices = vertices,
                    .edges = edges};
}
bool state_incrementable(State *state)
{
    return (state->idx + 1 < EDGE_COUNT)? true: false;
}
void state_print(State *state)
{
    printf("{.idx = %d, .value = %d, .vertices = ", state->idx, state->value);
    for (int i = 0; i < VERTEX_COUNT; i++){
        printf("%s", (i == 0)? "{":", ");
        printf("%d", state->vertices[i]);
        if (i + 1 == VERTEX_COUNT) printf("}");
    }
    printf(", .edges = ");
    for (int i = 0; i < EDGE_COUNT; i++){
        printf("%s", (i == 0)? "{":", ");
        printf("%d", state->edges[i]);
        if (i + 1 == EDGE_COUNT) printf("}");
    }
    printf("};\n");
}

typedef struct {
    int first;
    int count;
    State *arr;
    int cpty;
} States;
States states_new()
{
    int cpty = 32;
    State *arr = (State *) malloc(cpty * sizeof(State));
    return (States) {.first = 0, .count = 0, .arr=arr, .cpty = cpty};
}
void states_add_state(States *states, State *state) {
    if (states->first + states->count == states->cpty) {
        states->cpty *= 2;
        states->arr = (State *) realloc(states->arr, sizeof(State) * states->cpty);
    }
    states->arr[states->first + states->count++] = *state;
}
States states_generate(State *state)
{
    States states = states_new();
    states_add_state(&states, state);

    while (states.arr[states.first].idx < MAX_PARALLEL_DEPTH) {
        state = &states.arr[states.first];
        if (! state_incrementable(state)) {
            break;
        }
        states.first++;
        states.count--;
        state->idx++;
        int vertex_begin = EDGES[state->idx].begin;
        int vertex_end = EDGES[state->idx].end;
        State state_0_1 = state_copy(state);
        State state_1_0 = state_copy(state);
        State state_no = state_copy(state);
        state_free(state);

        // add 0-1
        if (state_0_1.vertices[vertex_begin] != 1
                && state_0_1.vertices[vertex_end] != 0) {
            {
                state_0_1.vertices[vertex_begin] = 0;
                state_0_1.vertices[vertex_end] = 1;
                state_0_1.edges[state_0_1.idx] = 1;
                state_0_1.value += EDGES[state_0_1.idx].value;
                states_add_state(&states, &state_0_1);
            }
        }

        // add 1-0
        if (state_1_0.vertices[vertex_begin] != 0
                && state_1_0.vertices[vertex_end] != 1) {
            {
                state_1_0.vertices[vertex_begin] = 1;
                state_1_0.vertices[vertex_end] = 0;
                state_1_0.edges[state_1_0.idx] = 1;
                state_1_0.value += EDGES[state_1_0.idx].value;
                states_add_state(&states, &state_1_0);
            }
        }

        states_add_state(&states, &state_no);
    }
    return states;
}

typedef struct Max {
    State state;
    struct Max *prev;
} Max;
Max *max_new(State state, Max *prev)
{
    Max *max = (Max *) malloc(sizeof(Max));
    *max = (Max) {.state = state, .prev = prev};
    return max;
}
void max_free(Max *max)
{
    for (Max *prev; max != NULL; max = prev) {
        prev = max->prev;
        state_free(&max->state);
        free(max);
    }
}
Max *MAX = NULL;

void graph_read(char *filename)
{
    FILE *file = fopen(filename, "r");
    fscanf(file, "%d,", &VERTEX_COUNT);
    int size = (VERTEX_COUNT)*(VERTEX_COUNT);
    GRAPH = (int*) malloc(size * sizeof(int));
    for (int i = 0; i < size; i++) {
        fscanf(file, "%d,", &(GRAPH)[i]);
    }
    fclose(file);
}
void graph_print()
{
    printf("VERTEX_COUNT %d\n", VERTEX_COUNT);
    for (int row = 0; row < VERTEX_COUNT; row++) {
        for (int col = 0; col < VERTEX_COUNT; col++) {
            printf("%d ", GRAPH[row*VERTEX_COUNT + col]);
        }
        printf("\n");
    }
}


void make_edges()
{
    // Get number of EDGES
    for (int row = 0; row < VERTEX_COUNT; row++) {
        for (int col = row + 1; col < VERTEX_COUNT; col++){
            if (GRAPH[row*VERTEX_COUNT + col] > 0) {
                EDGE_COUNT += 1;
            }
        }
    }

    // Make array of edges
    EDGES = (Edge*) malloc(EDGE_COUNT * sizeof(Edge));
    int edge_idx = -1;
    int value;
    for (int begin = 0; begin < VERTEX_COUNT; begin++) {
        for (int end = begin + 1; end < VERTEX_COUNT; end++) {
            value = GRAPH[begin*VERTEX_COUNT + end];
            if (value > 0) {
                edge_idx += 1;
                EDGES[edge_idx] = (Edge) {value, begin, end};
            }
        }
    }

    qsort(EDGES, EDGE_COUNT, sizeof(Edge), edge_cmp_desc);

    // Create mapping from vertices v1,v2 to EDGES array index
    EDGE_INDICES = (int*) malloc(VERTEX_COUNT * VERTEX_COUNT * sizeof(int));
    for (int idx = 0; idx < VERTEX_COUNT*VERTEX_COUNT; idx++) {
        EDGE_INDICES[idx] = -1;
    }
    for (int idx = 0; idx < EDGE_COUNT; idx++) {
        EDGE_INDICES[EDGES[idx].begin*VERTEX_COUNT + EDGES[idx].end] = idx;
        EDGE_INDICES[EDGES[idx].end*VERTEX_COUNT + EDGES[idx].begin] = idx;
    }

    // Create value-left array.
    VALUE_LEFT = (int*) malloc(EDGE_COUNT * sizeof(int));
    int idx = EDGE_COUNT - 1;
    VALUE_LEFT[idx] = EDGES[idx].value;
    for (idx -= 1; idx >= 0; idx--) {
        VALUE_LEFT[idx] = VALUE_LEFT[idx+1] + EDGES[idx].value;
    }
}

bool is_bipartite_recursive(int vertex, int color, int* visited, int* vertices)
{
    int edge_idx;
    visited[vertex] = 1;
    vertices[vertex] = color;
    for (int other = 0; other < VERTEX_COUNT; other++) {
        edge_idx = EDGE_INDICES[vertex*VERTEX_COUNT + other];
        if (edge_idx != -1){
            if (vertices[other] == color) {
                return false;
            }
            if (visited[other] == 0) {
                if (! is_bipartite_recursive(other, (color+1)%2, visited, vertices)) {
                    return false;
                }
            }
        }
    }
    return true;
}

bool is_bipartite()
{
    bool result;
    int* visited = (int*) malloc(VERTEX_COUNT * sizeof(int));
    int* vertices = (int*) malloc(VERTEX_COUNT * sizeof(int));
    for (int idx = 0; idx < VERTEX_COUNT; idx++) {
        visited[idx] = 0;
        vertices[idx] = -1;
    }
    result = is_bipartite_recursive(0, 0, visited, vertices);
    free(visited);
    free(vertices);
    return result;
}

void is_connected_recursive(int vertex, int *edges, int *visited)
{
    visited[vertex] = 1;
    for (int vertex_other = 0; vertex_other < VERTEX_COUNT; vertex_other++) {
        int edge_idx = EDGE_INDICES[vertex * VERTEX_COUNT + vertex_other];
        if (edge_idx >= 0) {
            if (edges[edge_idx] == 1 && visited[vertex_other] == 0) {
                is_connected_recursive(vertex_other, edges, visited);
            }
        }
    }
}
bool is_connected(State *state)
{
    int *visited = (int *) calloc(VERTEX_COUNT, sizeof(int));
    is_connected_recursive(0, state->edges, visited);
    int visited_count = 0;
    for (int i = 0; i < VERTEX_COUNT; i++) visited_count += visited[i];
    free(visited);
    return (visited_count == VERTEX_COUNT)? true: false;
}

void solve(State *state)
{
    #pragma omp critical
    {
        if (state->idx > -1
                && state->edges[state->idx] == 1 // Previous state added an edge.
                && (MAX == NULL || state->value >= MAX->state.value)
                && is_connected(state)) {
            if (MAX != NULL && state->value > MAX->state.value) {
                max_free(MAX);
                MAX = NULL;
            }
            MAX = max_new(state_copy(state), MAX);
        }
    }

    state->idx++;
    if (state->idx >= EDGE_COUNT) {
        return;
    }
    if (MAX != NULL
        && state->value + VALUE_LEFT[state->idx] < MAX->state.value) {
        return;
    }

    int vertex_begin = EDGES[state->idx].begin;
    int vertex_end = EDGES[state->idx].end;

    State state_0_1 = state_copy(state);
    State state_1_0 = state_copy(state);
    State state_no = state_copy(state);

    // add 0-1
    if (state->vertices[vertex_begin] != 1
            && state->vertices[vertex_end] != 0) {
        {
            state_0_1.vertices[vertex_begin] = 0;
            state_0_1.vertices[vertex_end] = 1;
            state_0_1.edges[state->idx] = 1;
            state_0_1.value += EDGES[state->idx].value;
            solve(&state_0_1);
        }
    }

    // add 1-0
    if (state->vertices[vertex_begin] != 0
            && state->vertices[vertex_end] != 1) {
        {
            state_1_0.vertices[vertex_begin] = 1;
            state_1_0.vertices[vertex_end] = 0;
            state_1_0.edges[state->idx] = 1;
            state_1_0.value += EDGES[state->idx].value;
            solve(&state_1_0);
        }
    }

    // does not add
    {
        solve(&state_no);
    }

    {
        state_free(&state_0_1);
        state_free(&state_1_0);
        state_free(&state_no);
    }
}

void clean_up()
{
    max_free(MAX);
    free(EDGES);
    free(EDGE_INDICES);
    free(VALUE_LEFT);
    free(GRAPH);
}

int main(int argc, char **argv)
{
    if (argc != 2) return EXIT_FAILURE;
    int rank;
    int process_count;
    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &process_count);

    if (rank == 0) { // Boss Process
        graph_read(argv[1]);
        make_edges();
        if (! is_bipartite()) {

            MPI_Send(&VERTEX_COUNT, 1, MPI_INT, 1, TAG_WORK, MPI_COMM_WORLD);
            MPI_Send(&EDGE_COUNT, 1, MPI_INT, 1, TAG_WORK, MPI_COMM_WORLD);
            for (int i = 0; i < EDGE_COUNT; i++) {
                size_t length = sizeof(Edge);
                int pos = 0;
                char buffer[length];
                MPI_Pack(&EDGES[i].begin, 1, MPI_INT, buffer, length, &pos, MPI_COMM_WORLD);
                MPI_Pack(&EDGES[i].end, 1, MPI_INT, buffer, length, &pos, MPI_COMM_WORLD);
                MPI_Pack(&EDGES[i].value, 1, MPI_INT, buffer, length, &pos, MPI_COMM_WORLD);
                MPI_Send(buffer, pos, MPI_PACKED, 1, TAG_WORK, MPI_COMM_WORLD);
            }
            MPI_Send(EDGE_INDICES, VERTEX_COUNT * VERTEX_COUNT, MPI_INT, 1, TAG_WORK, MPI_COMM_WORLD);
            MPI_Send(VALUE_LEFT, EDGE_COUNT, MPI_INT, 1, TAG_WORK, MPI_COMM_WORLD);

            State state = state_new();
            state.vertices[0] = 0;
            States states = states_generate(&state);
            #pragma omp parallel for schedule(dynamic) shared(MAX)
            for (int idx = states.first; idx < states.first + states.count; idx++) {
                size_t length = (1 + 1 + VERTEX_COUNT + EDGE_COUNT) * sizeof(int);
                int position = 0;
                char buffer[length];
                MPI_Pack(&states.arr[idx].idx, 1, MPI_INT, buffer, length, &position, MPI_COMM_WORLD);
                MPI_Pack(&states.arr[idx].value, 1, MPI_INT, buffer, length, &position, MPI_COMM_WORLD);
                MPI_Pack(states.arr[idx].vertices, VERTEX_COUNT, MPI_INT, buffer, length, &position, MPI_COMM_WORLD);
                MPI_Pack(states.arr[idx].edges, EDGE_COUNT, MPI_INT, buffer, length, &position, MPI_COMM_WORLD);
                MPI_Send(buffer, position, MPI_PACKED, 1, TAG_WORK, MPI_COMM_WORLD);

                // solve(&states.arr[idx]);
                state_free(&states.arr[idx]);
            }
            free(states.arr);

            MPI_Send(NULL, 0, MPI_PACKED, 1, TAG_TERMINATE, MPI_COMM_WORLD);


            MPI_Status status;
            int value;
            while (true) {
                MPI_Recv(&value, 1, MPI_INT, 1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                if (status.MPI_TAG == TAG_TERMINATE) {
                    printf("%d\n", value);
                    break;
                }
                printf("%d ", value);
            }

        } else {
            printf("bipartite.\n");
            MPI_Send(NULL, 0, MPI_INT, 1, TAG_TERMINATE, MPI_COMM_WORLD);
        }
        for (Max *ptr = MAX; ptr != NULL; ptr = ptr->prev) {
            printf("%d%s", ptr->state.value, (ptr->prev != NULL)? " ": "\n");
        }
        clean_up();

    } else { // Worker Process
        MPI_Status status;

        MPI_Recv(&VERTEX_COUNT, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        int *vertices = (int *) malloc(sizeof(int) * VERTEX_COUNT);

        MPI_Recv(&EDGE_COUNT, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        int *edges = (int *) malloc(sizeof(int) * EDGE_COUNT);
        // printf("vertex_count %d, edge_count %d\n", VERTEX_COUNT, EDGE_COUNT);

        EDGES = (Edge *) malloc(sizeof(Edge) * EDGE_COUNT);
        for (int i = 0; i < EDGE_COUNT; i++) {
            size_t length = sizeof(Edge);
            int pos = 0;
            char buffer[length];
            MPI_Recv(buffer, length, MPI_PACKED, BOSS, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            MPI_Unpack(buffer, length, &pos, &EDGES[i].begin, 1, MPI_INT, MPI_COMM_WORLD);
            MPI_Unpack(buffer, length, &pos, &EDGES[i].end, 1, MPI_INT, MPI_COMM_WORLD);
            MPI_Unpack(buffer, length, &pos, &EDGES[i].value, 1, MPI_INT, MPI_COMM_WORLD);
        }
        EDGE_INDICES = (int*) malloc(VERTEX_COUNT * VERTEX_COUNT * sizeof(int));
        MPI_Recv(EDGE_INDICES, VERTEX_COUNT * VERTEX_COUNT, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        VALUE_LEFT = (int*) malloc(EDGE_COUNT * sizeof(int));
        MPI_Recv(VALUE_LEFT, EDGE_COUNT, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        int idx;
        int value;
        State state;
        while (true) {
            size_t length = (1 + 1 + VERTEX_COUNT + EDGE_COUNT) * sizeof(int);
            int position = 0;
            char buffer[length];
            MPI_Recv(buffer, length, MPI_PACKED, BOSS, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            if (status.MPI_TAG == TAG_TERMINATE) {
                break;
            }
            MPI_Unpack(buffer, length, &position, &idx, 1, MPI_INT, MPI_COMM_WORLD);
            MPI_Unpack(buffer, length, &position, &value, 1, MPI_INT, MPI_COMM_WORLD);
            MPI_Unpack(buffer, length, &position, vertices, VERTEX_COUNT, MPI_INT, MPI_COMM_WORLD);
            MPI_Unpack(buffer, length, &position, edges, EDGE_COUNT, MPI_INT, MPI_COMM_WORLD);
            state = (State) {.idx = idx, .value = value, .vertices = vertices,
                             .edges = edges};
            // state_print(&state);
            solve(&state);
            // for (Max *ptr = MAX; ptr != NULL; ptr = ptr->prev) {
            //     printf("%d%s", ptr->state.value, (ptr->prev != NULL)? " ": "\n");
            // }
        }
        for (Max *ptr = MAX; ptr != NULL; ptr = ptr->prev) {
            int tag = (ptr->prev != NULL)? TAG_WORK: TAG_TERMINATE;
            MPI_Send(&ptr->state.value, 1, MPI_INT, 0, tag, MPI_COMM_WORLD);
        }

        free(vertices);
        free(edges);

        max_free(MAX);
        free(EDGES);
        free(EDGE_INDICES);
        free(VALUE_LEFT);
    }
    MPI_Finalize();
    return EXIT_SUCCESS;
}
