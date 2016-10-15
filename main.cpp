#include <iostream>
#include <random>
#include <ctime>

#include <vector>
#include <cmath>

class CacheMissCounter {
private:
    const static size_t LogLineSize = 6;
    const static size_t NWays = 12;
    const static size_t NSets = 1 << 12;
private:
    class NWaySet {
    private:
        size_t Size;
        std::vector<size_t> Tags;

        size_t Processed;
        std::vector<size_t> TimeStamps;
    public:
        NWaySet() : Size(0), Tags(NWays), Processed(0), TimeStamps(NWays) {}
    private:
        void Update(size_t pos) {
            TimeStamps[pos] = Processed;
            Processed += 1;
        }
        size_t GetMinTimestampPos() {
            size_t minPos = 0;
            for (size_t pos = 0; pos < Size; ++pos) {
                if (TimeStamps[pos] < TimeStamps[minPos]) {
                    minPos = pos;
                }
            }
            return minPos;
        }
    public:
        void Add(size_t tag) {
            size_t pos;
            if (Size == NWays) {
                pos = GetMinTimestampPos();
            } else {
                pos = Size;
                Size += 1;
            }
            Tags[pos] = tag;
            Update(pos);
        }
        bool Has(size_t tag, bool update) {
            for (size_t pos = 0; pos < Size; ++pos) {
                if (Tags[pos] == tag) {
                    if (update) {
                        Update(pos);
                    }
                    return true;
                }
            }
            return false;
        }
    };
private:
    size_t MissCount;
    std::vector<NWaySet> Sets;
public:
    CacheMissCounter() : Sets(NSets) {}

    void Poke(void* pointer) {
        size_t lineAddress = size_t(pointer) >> LogLineSize;
        size_t index = lineAddress % NSets;
        size_t tag = lineAddress / NSets;
        if (false == Sets[index].Has(tag, true)) {
            Sets[index].Add(tag);
            MissCount += 1;
        }
    }

    size_t GetMissCount() {
        return MissCount;
    }
};
//__declspec(align(16))
// __restrict
namespace {
    size_t CountMultSimpleMisses(const float* __restrict a, const float* __restrict b, float* __restrict c, int n)
    {
        CacheMissCounter cmc;
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                c[i * n + j] = 0.f;
                for (int k = 0; k < n; ++k) {
                    c[i * n + j] += a[i * n + k] * b[k * n + j];
                    cmc.Poke((void*) &a[i * n + k]);
                    cmc.Poke((void*) &b[k * n + j]);
                }
            }
        }
        return cmc.GetMissCount();
    }

    size_t CountMultSimpleBlockMisses(const float* __restrict a, const float* __restrict b, float* __restrict c, int n)
    {
        CacheMissCounter cmc;
        const int BLOCK = 10;
        for (int i = 0; i < n; i += BLOCK) {
            for (int j = 0; j < n; j += BLOCK) {
                for (int k = 0; k < n; k += BLOCK) {
                    for (int ib = 0; ib < BLOCK && i + ib < n; ++ib) {
                        for (int jb = 0; jb < BLOCK && j + jb < n; ++jb) {
                            if (k == 0)
                                c[(i + ib) * n + (j + jb)] = 0.f;
                            for (int kb = 0; k + kb < n && kb < BLOCK; ++kb) {
                                c[(i + ib) * n + j + jb] += a[(i + ib) * n + k + kb] * b[(k + kb) * n + j + jb];
                                cmc.Poke((void*) &a[(i + ib) * n + k + kb]);
                                cmc.Poke((void*) &b[(k + kb) * n + j + jb]);
                            }
                        }
                    }
                }
            }
        }
        return cmc.GetMissCount();
    }

    size_t CountMultSwappedLoopsMisses(const float* __restrict a, const float* __restrict b, float* __restrict c, int n)
    {
        CacheMissCounter cmc;
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                c[i * n + j] = 0.f;
            }

            for (int k = 0; k < n; ++k) {
                for (int j = 0; j < n; ++j) {
                    c[i * n + j] += a[i * n + k] * b[k * n + j];
                    cmc.Poke((void*) &a[i * n + k]);
                    cmc.Poke((void*) &b[k * n + j]);
                }
            }
        }
        return cmc.GetMissCount();
    }

    void FillRandom(float* a, int n)
    {
        std::default_random_engine eng;
        std::uniform_real_distribution<float> dist;

        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                a[i * n + j] = dist(eng);
            }
        }
    }
}

int main(int argc, char* argv[])
{
    const int n = atoi(argv[1]);
    std::cerr << "n = " << n << std::endl;

    float* a = new float[n * n];
    float* b = new float[n * n];
    float* c = new float[n * n];

    FillRandom(a, n);
    FillRandom(b, n);

    {
        const auto startTime = std::clock();
        size_t misses = CountMultSimpleMisses(a, b, c, n);
        const auto endTime = std::clock();

        std::cerr << "timeSimple: " << misses << '\n';
    }

    {
        const auto startTime = std::clock();
        size_t misses = CountMultSimpleBlockMisses(a, b, c, n);
        const auto endTime = std::clock();

        std::cerr << "timeBlock: " << misses << '\n';
    }

    {
        const auto startTime = std::clock();
        size_t misses = CountMultSwappedLoopsMisses(a, b, c, n);
        const auto endTime = std::clock();

        std::cerr << "timeSwapped: " << misses << '\n';
    }

    delete[] a;
    delete[] b;
    delete[] c;
}

