#include "mcmf.h"

void MCMF::init(int _n) {
    n = _n, m = 1;
    memset(fst, 0, sizeof(fst));
}

void MCMF::add(int from, int to, int cap, int cost) {
    m++;
    _x[m] = from, _y[m] = to, _cap[m] = cap, _cost[m] = cost, _flow[m] = 0;
    _nxt[m] = fst[from], fst[from] = m;
    m++;
    _x[m] = to, _y[m] = from, _cap[m] = 0, _cost[m] = -cost, _flow[m] = 0;
    _nxt[m] = fst[to], fst[to] = m;
}

bool MCMF::SPFA(int S, int T, int &flow, int &cost) {
    int head = 0, tail = 0, qmod = n + 1, x, y;
    memset(f, 0, sizeof(f));
    for (int i = 1; i <= n; i++) {
        d[i] = 1e9;
    }
    d[S] = 0;
    q[tail = tail % qmod + 1] = S;
    a[S] = 1e9;
    while (head != tail) {
        f[x = q[head = head % qmod + 1]] = 0;
        for (int i = fst[x]; i; i = _nxt[i]) {
            y = _y[i];
            if (_flow[i] < _cap[i] && d[x] + _cost[i] < d[y]) {
                d[y] = d[x] + _cost[i];
                a[y] = std::min(a[x], _cap[i] - _flow[i]);
                p[y] = i;
                if (!f[y]) {
                    f[q[tail = tail % qmod + 1] = y] = 1;
                }
            }
        }
    }
    if (d[T] > 5e8) {
        return false;
    } else {
        flow += a[T];
        cost += d[T] * a[T];
        for (x = T; x != S; x = _x[p[x]]) {
            _flow[p[x]] += a[T];
            _flow[p[x] ^ 1] -= a[T];
        }
        return true;
    }
}

int MCMF::MinCost(int S, int T) {
    int flow = 0, cost = 0;
    while (SPFA(S, T, flow, cost));
    if (!flow)return 1e9;
    else return cost;
}
