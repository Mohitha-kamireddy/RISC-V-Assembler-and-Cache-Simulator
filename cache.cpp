#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <iomanip>
#include <map>
using namespace std;

struct Config
{
    long cacheSize = 0;
    int blockSize = 0;
    int assoc = 0;
    string writePolicy;
    bool writeAllocate = true;
    int hitTime = 1;
    int missPenalty = 50;
    int matrixN = 0;
    int elemSize = 1;
    long baseAddr = 0;
};

struct cachestats
{
    long accesses = 0;
    long reads = 0;
    long writes = 0;
    long hits = 0;
    long misses = 0;
    long writebacks = 0;
    long bytesMoved = 0;
    long cycles = 0;
    double hitRate = 0.0;
    double missRate = 0.0;
    double amat = 0.0;
};

struct cachelines
{
    bool valid = false;
    bool dirty = false;
    long tag = -1;
    long lastUsed = 0;
};

using CacheSet = vector<cachelines>;
using Cache = vector<CacheSet>;

bool read_file(const string &path, Config &cfg)
{
    ifstream in(path);
    if (!in)
        return false;
    string line;
    while (getline(in, line))
    {
        auto pos = line.find('#');
        if (pos != string::npos)
            line = line.substr(0, pos);
        auto eq = line.find('=');
        if (eq == string::npos)
            continue;
        string key = line.substr(0, eq);
        string val = line.substr(eq + 1);
        auto cut = [&](string &s)
        {
            auto a = s.find_first_not_of(" \t\r\n");
            auto b = s.find_last_not_of(" \t\r\n");
            if (a == string::npos)
            {
                s = "";
                return;
            }
            s = s.substr(a, b - a + 1);
        };
        cut(key);
        cut(val);
        if (key == "cacheSize")
            cfg.cacheSize = stol(val);
        else if (key == "blockSize")
            cfg.blockSize = stoi(val);
        else if (key == "assoc")
            cfg.assoc = stoi(val);
        else if (key == "writePolicy")
            cfg.writePolicy = val;
        else if (key == "writeAllocate")
            cfg.writeAllocate = (val != "0");
        else if (key == "hitTime")
            cfg.hitTime = stoi(val);
        else if (key == "missPenalty")
            cfg.missPenalty = stoi(val);
        else if (key == "matrixN")
            cfg.matrixN = stoi(val);
        else if (key == "elemSize")
            cfg.elemSize = stoi(val);
        else if (key == "baseAddr")
            cfg.baseAddr = stol(val);
    }
    return true;
}

int log2_int(long x)
{
    int r = 0;
    while ((1L << r) < x)
        r++;
    return r;
}

struct addressParts
{
    long setIndex;
    long tag;
};

addressParts splitAddress(long addr, int blockSize, long SETS)
{
    int blockOffsetBits = log2_int(blockSize);
    int setBits = (SETS > 1) ? log2_int(SETS) : 0;
    long setMask = (1UL << setBits) - 1;
    long setIndex = (setBits > 0) ? ((addr >> blockOffsetBits) & setMask) : 0;
    long tag = addr >> (blockOffsetBits + setBits);
    return {setIndex, tag};
}

bool access(Cache &cache, long addr, bool isWrite, const Config &cfg, cachestats &st, long &globalTime)
{
    long SETS = cache.size();
    addressParts ap = splitAddress(addr, cfg.blockSize, SETS);
    st.accesses++;
    if (isWrite)
        st.writes++;
    else
        st.reads++;
    CacheSet &set = cache[ap.setIndex];
    for (auto &line : set)
    {
        if (line.valid && line.tag == ap.tag)
        {
            st.hits++;
            st.cycles += cfg.hitTime;
            line.lastUsed = globalTime++;
            if (isWrite)
            {
                if (cfg.writePolicy == "write_back")
                {
                    line.dirty = true;
                }
                else
                {
                    st.bytesMoved += cfg.elemSize;
                    st.cycles += cfg.missPenalty;
                }
            }
            return true;
        }
    }
    st.misses++;
    st.cycles += cfg.hitTime + cfg.missPenalty;
    if (isWrite && !cfg.writeAllocate)
    {
        st.bytesMoved += cfg.elemSize;
        return false;
    }
    cachelines *def_line = nullptr;
    for (auto &l : set)
        if (!l.valid)
        {
            def_line = &l;
            break;
        }
    if (!def_line)
    {
        def_line = &set[0];
        for (auto &l : set)
            if (l.lastUsed < def_line->lastUsed)
                def_line = &l;
    }
    if (def_line->valid && def_line->dirty)
    {
        st.writebacks++;
        st.bytesMoved += cfg.blockSize;
        st.cycles += cfg.missPenalty;
    }
    st.bytesMoved += cfg.blockSize;
    def_line->valid = true;
    def_line->tag = ap.tag;
    def_line->lastUsed = globalTime++;
    if (isWrite)
    {
        if (cfg.writePolicy == "write_back")
            def_line->dirty = true;
        else
        {
            def_line->dirty = false;
            st.bytesMoved += cfg.elemSize;
            st.cycles += cfg.missPenalty;
        }
    }
    else
    {
        def_line->dirty = false;
    }
    return false;
}

vector<long> tracing(const Config &cfg)
{
    vector<long> trace;
    int N = cfg.matrixN;
    int sz = cfg.elemSize;
    long base = cfg.baseAddr;
    auto A = [&](int i, int j)
    { return base + (long)(i * N + j) * sz; };
    auto B = [&](int i, int j)
    { return base + (long)N * N * sz + (long)(i * N + j) * sz; };
    auto C = [&](int i, int j)
    { return base + 2L * N * N * sz + (long)(i * N + j) * sz; };
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            for (int k = 0; k < N; k++)
            {
                trace.push_back(A(i, k));
                trace.push_back(B(k, j));
                trace.push_back(C(i, j));
            }
    return trace;
}

cachestats simulation(const Config &cfg, int overrideMode)
{
    cachestats st;
    long numBlocks = cfg.cacheSize / cfg.blockSize;
    int assoc = cfg.assoc;
    if (overrideMode == 0)
        assoc = 1;
    if (overrideMode == 2)
        assoc = numBlocks;
    if (assoc < 1)
        assoc = 1;
    long SETS = numBlocks / assoc;
    if (SETS < 1)
        SETS = 1;
    Cache cache(SETS, CacheSet(assoc));
    vector<long> trace = tracing(cfg);
    long gt = 1;
    for (int i = 0; i < trace.size(); i++)
    {
        bool isWrite = ((i % 3) == 2);
        access(cache, trace[i], isWrite, cfg, st, gt);
    }
    st.hitRate = (st.accesses > 0) ? double(st.hits) / st.accesses : 0.0;
    st.missRate = 1.0 - st.hitRate;
    st.amat = cfg.hitTime + st.missRate * cfg.missPenalty;
    return st;
}

string modeName(int m)
{
    if (m == 0)
        return "Direct-Mapped";
    if (m == 1)
        return "Set-Associative";
    return "Fully-Associative";
}

int main(int argc, char **argv)
{
    string file = "config.txt";
    if (argc > 1)
        file = argv[1];
    Config cfg;
    if (!read_file(file, cfg))
    {
        cout << "Error reading config file.\n";
        return 1;
    }
    vector<int> modes = {0, 1, 2};
    vector<string> policies = {"write_back", "write_through"};
    cout << fixed << setprecision(4);
    cout << "\n**** Results ****\n";
    for (int m : modes)
    {
        for (auto wp : policies)
        {
            Config local = cfg;
            local.writePolicy = wp;
            cachestats st = simulation(local, m);
            cout << "\n";
            cout << "Mapping: " << modeName(m) << "\n";
            cout << "WritePolicy: " << wp << "\n";
            cout << "Accesses: " << st.accesses << "\n";
            cout << "Hits: " << st.hits << "\n";
            cout << "Misses: " << st.misses << "\n";
            cout << "HitRate: " << st.hitRate << "\n";
            cout << "AMAT computed as: hittime + missrate * misspenalty (cycles).\n";
            cout << "AMAT: " << st.amat << " cycles\n";
            cout << "BytesMoved: " << st.bytesMoved << "\n";
            cout << "Cycles: " << st.cycles << "\n";
            double bytesPerAccess = (st.accesses > 0) ? double(st.bytesMoved) / st.accesses : 0.0;
            double bytesPerCycle = (st.cycles > 0) ? double(st.bytesMoved) / st.cycles : 0.0;
            cout << "Bandwidth computed as: total bytes moved / total cycles\n";
            cout << "Bytes/Cycle or Bandwidth :  " << bytesPerCycle << "\n";
        }
    }
    cout << "\n";
    return 0;
}