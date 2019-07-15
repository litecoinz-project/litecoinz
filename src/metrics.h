// Copyright (c) 2016 The Zcash developers
// Copyright (c) 2017-2018 The LitecoinZ developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"
#include "uint256.h"

#include <atomic>
#include <mutex>
#include <string>

struct AtomicCounter {
    std::atomic<uint64_t> value;

    AtomicCounter() : value {0} { }

    void increment(){
        ++value;
    }

    void decrement(){
        --value;
    }

    int get() const {
        return value.load();
    }
};

class AtomicTimer {
private:
    std::mutex mtx;
    uint64_t threads;
    int64_t start_time;
    int64_t total_time;

public:
    AtomicTimer() : threads(0), start_time(0), total_time(0) {}

    /**
     * Starts timing on first call, and counts the number of calls.
     */
    void start();

    /**
     * Counts number of calls, and stops timing after it has been called as
     * many times as start().
     */
    void stop();

    bool running();

    uint64_t threadCount();

    double rate(const AtomicCounter& count);
};

extern AtomicCounter transactionsValidated;
extern AtomicCounter ehSolverRuns;
extern AtomicCounter solutionTargetChecks;
extern AtomicTimer miningTimer;

void TrackMinedBlock(uint256 hash);

void MarkStartTime();
double GetLocalSolPS();
int EstimateNetHeightInner(int height, int64_t tipmediantime,
                           int heightLastCheckpoint, int64_t timeLastCheckpoint,
                           int64_t genesisTime, int64_t targetSpacing);
int EstimateNetHeight(int height, int64_t tipmediantime, CChainParams chainParams);

void TriggerRefresh();

void ConnectMetricsScreen();
void ThreadShowMetricsScreen();

/**
 * Rendering options:
 * LitecoinZ: img2txt -W 40 -H 20 -f utf8 -d none -g 0.7 src/qt/res/images/litecoinz.png
 */
const std::string METRICS_ART =
"              [0;1;30;90;43m%[0;1;33;93;43m%[0;33;5;43;103m;......t[0;1;33;93;43mt[0;1;30;90;43m%[0m              \n"
"          [0;1;33;93;43mS[0;33;5;43;103m..................[0;1;33;93;43m%[0m          \n"
"       [0;33;5;43;103m%....;[0;1;30;90;43m%[0m            [0;1;30;90;43m%[0;33;5;43;103m:....[0;1;33;93;43mt[0m       \n"
"     [0;33;5;43;103mS...t[0m                    [0;33;5;43;103m;...[0;1;33;93;43mt[0m     \n"
"   [0;1;30;90;43m%[0;33;5;43;103m.....ttttttttttttttttttttt[0;1;33;93;43mt[0m [0;33;5;43;103m;...[0m    \n"
"  [0;1;33;93;43mS[0;33;5;43;103m...;tttttttttttttttttt....t[0m   [0;1;33;93;43m;[0;33;5;43;103m...[0;1;33;93;43m;[0m  \n"
" [0;1;33;93;43m%[0;33;5;43;103m...[0m                    [0;33;5;43;103m...;[0m     [0;1;33;93;43m:[0;33;5;43;103m...[0;1;30;90;43m%[0m \n"
" [0;33;5;43;103m...[0;1;33;93;43m;[0m                   [0;33;5;43;103m...:[0m       [0;33;5;43;103m%...[0m \n"
"[0;33;5;43;103m;...[0m           [0;1;33;93;43mt[0;33;5;43;103m...[0;1;30;90;43m%[0m   [0;1;33;93;43mt[0;1;30;90;43m%%[0;33;5;43;103m.[0m         [0;33;5;43;103m...[0;1;30;90;43mt[0m\n"
"[0;33;5;43;103m....[0m          [0;1;30;90;43m%%[0m         [0;33;5;43;103m%[0m          [0;33;5;43;103m...[0;1;33;93;43m%[0m\n"
"[0;33;5;43;103m....[0m          [0;1;33;93;43m%[0m        [0;1;30;90;43mt[0;1;33;93;43m%[0m           [0;33;5;43;103m...[0;1;33;93;43m%[0m\n"
"[0;33;5;43;103m;...[0m         [0;33;5;43;103m.[0;1;30;90;43mt[0;1;33;93;43m:t[0m   [0;33;5;43;103mS...[0;1;30;90;43m%[0m           [0;33;5;43;103m...[0;1;30;90;43m%[0m\n"
" [0;33;5;43;103m...[0;1;30;90;43m%[0m       [0;33;5;43;103m....[0m                   [0;1;33;93;43mS[0;33;5;43;103m...[0m \n"
" [0;33;5;43;103mS...[0m      [0;33;5;43;103m:...[0m                   [0;1;30;90;43m%[0;33;5;43;103m...[0;1;33;93;43m;[0m \n"
"  [0;33;5;43;103mt...[0m    [0;33;5;43;103m;...:[0;1;33;93;43mtttttttttttttttttt[0;33;5;43;103m%...[0;1;33;93;43m%[0m  \n"
"   [0;1;33;93;43mt[0;33;5;43;103m...[0;1;33;93;43m%[0m [0;33;5;43;103m%..........................[0;1;30;90;43m%[0m   \n"
"     [0;33;5;43;103mt...[0;1;33;93;43m%[0m                    [0;33;5;43;103m%...[0;1;33;93;43mS[0m     \n"
"       [0;33;5;43;103m;....S[0;1;30;90;43m%[0m            [0;1;30;90;43mt[0;33;5;43;103mt....%[0m       \n"
"         [0;1;30;90;43mt[0;33;5;43;103m;.......;tt:.......t[0;1;30;90;43m%[0m         \n"
"              [0;1;30;90;43m%[0;1;33;93;43m%[0;33;5;43;103m;......t[0;1;33;93;43m%[0;1;30;90;43m%[0m              ";
