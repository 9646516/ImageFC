#ifndef IMAGEFC_MCMF_H
#define IMAGEFC_MCMF_H

#include <iostream>
#include <algorithm>

struct MCMF {
    static const int INF = 1e9;
    static const int N = 500, M = 100000;
    int n, m, fst[N], f[N], a[N], p[N], q[N], d[N];
    int _x[M], _y[M], _flow[M], _cap[M], _nxt[M], _cost[M];

    void init(int _n);

    void add(int from, int to, int cap, int cost);

    bool SPFA(int S, int T, int &flow, int &cost);

    int MinCost(int S, int T);
};

#endif