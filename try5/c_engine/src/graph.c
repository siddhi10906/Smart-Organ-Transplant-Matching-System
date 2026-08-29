#include <stdlib.h>
#include <string.h>
#include "graph.h"

static int idx(const Graph *g, int i, int j) { return i * g->n + j; }

void graph_init_default(Graph *g, int n) {
  g->n = n;
  g->adj = (int *)malloc(sizeof(int) * (size_t)n * (size_t)n);
  memset(g->adj, 0, sizeof(int) * (size_t)n * (size_t)n);

  for (int i = 0; i < n; i++) {
    int j = (i + 1) % n;
    g->adj[idx(g, i, j)] = 1;
    g->adj[idx(g, j, i)] = 1;
  }

  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      if (i != j && ((i + j) % 3 == 0)) {
        g->adj[idx(g, i, j)] = 1;
        g->adj[idx(g, j, i)] = 1;
      }
    }
  }
}

int graph_bfs_find_hospital(Graph *g, int start, const int *hospital_has_candidate) {
  int n = g->n;
  int *q = (int *)malloc(sizeof(int) * (size_t)n);
  int *vis = (int *)malloc(sizeof(int) * (size_t)n);
  memset(vis, 0, sizeof(int) * (size_t)n);

  int head = 0, tail = 0;
  q[tail++] = start;
  vis[start] = 1;

  while (head < tail) {
    int u = q[head++];
    if (hospital_has_candidate[u]) {
      free(q);
      free(vis);
      return u;
    }

    for (int v = 0; v < n; v++) {
      if (g->adj[idx(g, u, v)] && !vis[v]) {
        vis[v] = 1;
        q[tail++] = v;
      }
    }
  }

  free(q);
  free(vis);
  return -1;
}

int graph_bfs_distance(Graph *g, int start, int target) {
  int n = g->n;
  if (start < 0 || start >= n || target < 0 || target >= n) return -1;
  if (start == target) return 0;

  int *q = (int *)malloc(sizeof(int) * (size_t)n);
  int *vis = (int *)malloc(sizeof(int) * (size_t)n);
  int *dist = (int *)malloc(sizeof(int) * (size_t)n);
  memset(vis, 0, sizeof(int) * (size_t)n);
  for (int i = 0; i < n; i++) dist[i] = -1;

  int head = 0, tail = 0;
  q[tail++] = start;
  vis[start] = 1;
  dist[start] = 0;

  while (head < tail) {
    int u = q[head++];
    for (int v = 0; v < n; v++) {
      if (g->adj[idx(g, u, v)] && !vis[v]) {
        vis[v] = 1;
        dist[v] = dist[u] + 1;
        if (v == target) {
          int ans = dist[v];
          free(q);
          free(vis);
          free(dist);
          return ans;
        }
        q[tail++] = v;
      }
    }
  }

  free(q);
  free(vis);
  free(dist);
  return -1;
}

void graph_free(Graph *g) {
  free(g->adj);
  g->adj = NULL;
  g->n = 0;
}
