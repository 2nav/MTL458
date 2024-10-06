#include <iostream>
#include <cstdint>
#include <vector>
#include <cmath>
#include <map>
#include <queue>
#include <unordered_map>
#include <set>

constexpr int ADDR_BITS = 32;

int fifo(std::vector<uint32_t> &vpns, int K)
{
    std::unordered_map<int, int> tlb;
    std::queue<int> fifo_q;
    int size = 0;
    int hits = 0;
    for (int i = 0; i < vpns.size(); i++)
    {
        if (tlb.find(vpns[i]) != tlb.end())
        {
            hits++;
        }
        else
        {
            if (size < K)
            {
                tlb[vpns[i]] = 1;
                fifo_q.push(vpns[i]);
                size++;
            }
            else
            {
                int front = fifo_q.front();
                fifo_q.pop();
                tlb.erase(front);
                tlb[vpns[i]] = 1;
                fifo_q.push(vpns[i]);
            }
        }
    }
    return hits;
}

int lifo(std::vector<uint32_t> &vpns, int K)
{
    std::unordered_map<int, int> tlb;
    std::vector<int> lifo_q;
    int size = 0;
    int hits = 0;
    for (int i = 0; i < vpns.size(); i++)
    {
        if (tlb.find(vpns[i]) != tlb.end())
        {
            hits++;
        }
        else
        {
            if (size < K)
            {
                tlb[vpns[i]] = 1;
                lifo_q.push_back(vpns[i]);
                size++;
            }
            else
            {
                int back = lifo_q.back();
                lifo_q.pop_back();
                tlb.erase(back);
                tlb[vpns[i]] = 1;
                lifo_q.push_back(vpns[i]);
            }
        }
    }
    return hits;
}

int lru(std::vector<uint32_t> &vpns, int K)
{
    // std::map<int, int> tlb;
    std::set<std::pair<int, int>> lru_q;
    std::unordered_map<int, int> tlb;
    int hits = 0;
    for (int i = 0; i < vpns.size(); i++)
    {
        if (tlb.find(vpns[i]) != tlb.end())
        {
            hits++;
            // tlb.erase(vpns[i]);
            // tlb[vpns[i]] = i;
            lru_q.erase({tlb[vpns[i]], vpns[i]}); // erase the old entry
            lru_q.insert({i, vpns[i]});           // insert the new entry to update LRU
        }
        else
        {
            if (tlb.size() < K)
            {
                // tlb[vpns[i]] = i;
                lru_q.insert({i, vpns[i]}); // insert entry to LRU
                tlb[vpns[i]] = i;           // insert entry to TLB
            }
            else
            {
                // tlb.erase(tlb.begin());
                // tlb[vpns[i]] = i;
                auto it = lru_q.begin();    // get the LRU entry
                tlb.erase(it->second);      // erase the LRU entry from TLB
                lru_q.erase(it);            // erase the LRU entry from LRU
                lru_q.insert({i, vpns[i]}); // insert the new entry to LRU
                tlb[vpns[i]] = i;           // insert the new entry to TLB
            }
        }
    }
    return hits;
}

int optimal(std::vector<uint32_t> &vpns, int K)
{
    std::map<int, int> tlb;
    int hits = 0;
    for (int i = 0; i < vpns.size(); i++)
    {
        if (tlb.find(vpns[i]) != tlb.end())
        {
            hits++;
        }
        else
        {
            if (tlb.size() < K)
            {
                tlb[vpns[i]] = i;
            }
            else
            {
                int max = -1;
                int max_vpn = -1;
                for (auto it = tlb.begin(); it != tlb.end(); it++)
                {
                    int vpn = it->first;
                    int j = i + 1;
                    for (; j < vpns.size(); j++)
                    {
                        if (vpns[j] == vpn)
                        {
                            break;
                        }
                    }
                    if (j > max)
                    {
                        max = j;
                        max_vpn = vpn;
                    }
                }
                tlb.erase(max_vpn);
                tlb[vpns[i]] = i;
            }
        }
    }
    return hits;
}

int main()
{
    int t;
    std::cin >> t;
    while (t--)
    {
        int S, P, K;
        std::cin >> S >> P >> K;

        // assuming page size to be in power of 2
        int page_offset_bits = log2(P) + 10;

        int N;
        std::cin >> N;
        std::vector<uint32_t> addrs(N), vpns(N);
        for (int i = 0; i < N; i++)
        {
            uint32_t addr;
            std::cin >> std::hex >> addr;
            // std::cout << addr << std::endl;
            addrs[i] = addr;
            vpns[i] = addr >> page_offset_bits;
            std::cout << vpns[i] << std::endl;
        }
        std::cout << fifo(vpns, K) << " " << lifo(vpns, K) << " " << lru(vpns, K) << std::endl;
    }
}