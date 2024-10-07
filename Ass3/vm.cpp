#include <iostream>
#include <cstdint>
#include <vector>
#include <cmath>
// #include <map>
#include <queue>
#include <unordered_map>
#include <set>

constexpr int ADDR_BITS = 32;

int fifo(std::vector<uint32_t> &vpns, int K)
{
    std::unordered_map<uint32_t, int> tlb;
    std::queue<uint32_t> fifo_q;
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
                fifo_q.push(vpns[i]); // push the vpn to the queue
                size++;
            }
            else
            {
                int front = fifo_q.front();
                fifo_q.pop();         // pop the front of the queue
                tlb.erase(front);     // erase the front of the queue from the tlb
                tlb[vpns[i]] = 1;     // insert the vpn to the tlb
                fifo_q.push(vpns[i]); // push the vpn to the queue
            }
        }
    }
    return hits;
}

int lifo(std::vector<uint32_t> &vpns, int K)
{
    std::unordered_map<uint32_t, int> tlb;
    std::vector<uint32_t> lifo_q;
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
                lifo_q.push_back(vpns[i]); // push the vpn to the queue
                size++;
            }
            else
            {
                int back = lifo_q.back();
                lifo_q.pop_back();         // pop the back of the queue
                tlb.erase(back);           // erase the back of the queue from the tlb
                tlb[vpns[i]] = 1;          // insert the vpn to the tlb
                lifo_q.push_back(vpns[i]); // push the vpn to the queue
            }
        }
    }
    return hits;
}

int lru(std::vector<uint32_t> &vpns, int K)
{
    // std::map<int, int> tlb;
    std::set<std::pair<int, uint32_t>> lru_q;
    std::unordered_map<uint32_t, int> tlb;
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
    // std::map<int, int> tlb;
    std::set<std::pair<int, uint32_t>> tlb;              // {-next memory access time, vpn}, so stores the vpn in desscending order of next memory access time
    std::unordered_map<uint32_t, std::queue<int>> tlb_q; // map of vpn to queue of memory access times
    const int inf = 1e7;                                 // infinity as 10^6 is max memory accesses, vpns.size() <= 10^6

    for (int i = 0; i < vpns.size(); i++)
    {
        tlb_q[vpns[i]].push(i); // store the memory access time for each vpn in ascending order
    }

    for (auto it : tlb_q)
    {
        it.second.push(inf); // push infinity to the end of the queue to show that the vpn is not accessed anymore
    }

    int hits = 0;
    for (int i = 0; i < vpns.size(); i++)
    {
        if (tlb.find({-i, vpns[i]}) != tlb.end())
        {
            hits++;
            tlb.erase({tlb_q[vpns[i]].front(), vpns[i]});   // update the vpn in the tlb
            tlb_q[vpns[i]].pop();                           // pop the front of the queue, front of queue = current memory access
            tlb.insert({-tlb_q[vpns[i]].front(), vpns[i]}); // insert the vpn to the tlb
        }
        else
        {
            // for (auto it : tlb)
            // {
            //     std::cout << it.first << " " << it.second << "  ";
            // }
            // std::cout << std::endl;
            tlb_q[vpns[i]].pop(); // pop the front of the queue, front of queue = current memory access
            if (tlb.size() < K)
            {
                tlb.insert({-tlb_q[vpns[i]].front(), vpns[i]}); // insert the vpn to the tlb
            }
            else
            {
                auto it = tlb.begin();
                tlb.erase(it);                                  // erase the farthest in future vpn from the tlb
                tlb.insert({-tlb_q[vpns[i]].front(), vpns[i]}); // insert the vpn to the tlb
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
        // TODO : Implement address bounds
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
        int fifo_hits = fifo(vpns, K);
        int lifo_hits = lifo(vpns, K);
        int lru_hits = lru(vpns, K);
        int optimal_hits = optimal(vpns, K);
        std::cout << fifo_hits << " " << lifo_hits << " " << lru_hits << " " << optimal_hits << std::endl;
    }
}