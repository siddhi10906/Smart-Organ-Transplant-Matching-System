#ifndef GRAPH_H
#define GRAPH_H

typedef struct {
  int n;
  int *adj;
} Graph;

void graph_init_default(Graph *g, int n);
int graph_bfs_find_hospital(Graph *g, int start, const int *hospital_has_candidate);
int graph_bfs_distance(Graph *g, int start, int target);
void graph_free(Graph *g);

#endif
