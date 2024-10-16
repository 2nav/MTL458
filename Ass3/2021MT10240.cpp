#include <iostream>
#include <cstdint>
// #include <vector>
#include <cmath>
// #include <map>
// #include <queue>
#include <unordered_map>
// #include <set>

constexpr int ADDR_BITS = 32;

// queue implementation for FIFO
class Queue
{
    int front, rear, size;
    unsigned capacity;
    uint32_t *array;

public:
    void init(unsigned capacity)
    {
        front = size = 0;
        rear = capacity - 1;
        this->capacity = capacity;
        array = new uint32_t[capacity];
    }

    void push(uint32_t item)
    {
        if (size == capacity)
            return;
        rear = (rear + 1) % capacity;
        array[rear] = item;
        size++;
    }

    void pop()
    {
        if (size == 0)
            return;
        front = (front + 1) % capacity;
        size--;
    }

    uint32_t top()
    {
        if (size == 0)
            return -1;
        return array[front];
    }
};

// stack implementation for LIFO
class Stack
{
    int top;
    unsigned capacity;
    uint32_t *array;

public:
    void init(unsigned capacity)
    {
        top = -1;
        this->capacity = capacity;
        array = new uint32_t[capacity];
    }

    void push(uint32_t item)
    {
        if (top == capacity - 1)
            return;
        array[++top] = item;
    }

    void pop()
    {
        if (top == -1)
            return;
        top--;
    }

    uint32_t front()
    {
        if (top == -1)
            return -1;
        return array[top];
    }
};

/**
 * @brief FIFO TLB replacement algorithm
 *
 * @param vpns - an array of virtual page numbers
 * @param K - size of the TLB
 * @param N - number of memory accesses
 * @return int - number of TLB hits
 */
int fifo(uint32_t *vpns, int K, int N)
{
    std::unordered_map<uint32_t, int> tlb;
    // std::queue<uint32_t> fifo_q;
    Queue fifo_q;
    fifo_q.init(K);

    int size = 0;
    int hits = 0;
    for (int i = 0; i < N; i++)
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
                // fifo_q.push(vpns[i]); // push the vpn to the queue
                fifo_q.push(vpns[i]);
                size++;
            }
            else
            {
                // int front = fifo_q.front();
                uint32_t front = fifo_q.top();
                fifo_q.pop();         // pop the front of the queue
                tlb.erase(front);     // erase the front of the queue from the tlb
                tlb[vpns[i]] = 1;     // insert the vpn to the tlb
                fifo_q.push(vpns[i]); // push the vpn to the queue
            }
        }
    }
    return hits;
}

/**
 * @brief LIFO TLB replacement algorithm
 *
 * @param vpns - an array of virtual page numbers
 * @param K - size of the TLB
 * @param N - number of memory accesses
 * @return int - number of TLB hits
 */
int lifo(uint32_t *vpns, int K, int N)
{
    std::unordered_map<uint32_t, int> tlb;
    // std::vector<uint32_t> lifo_q;
    Stack lifo_q;
    lifo_q.init(K);

    int size = 0;
    int hits = 0;
    for (int i = 0; i < N; i++)
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
                // lifo_q.push_back(vpns[i]); // push the vpn to the queue
                lifo_q.push(vpns[i]);
                size++;
            }
            else
            {
                // int back = lifo_q.back();
                uint32_t back = lifo_q.front();
                // lifo_q.pop_back();         // pop the back of the queue
                lifo_q.pop();
                tlb.erase(back);  // erase the back of the queue from the tlb
                tlb[vpns[i]] = 1; // insert the vpn to the tlb
                // lifo_q.push_back(vpns[i]); // push the vpn to the queue
                lifo_q.push(vpns[i]);
            }
        }
    }
    return hits;
}

/**
 * @brief LRU TLB replacement algorithm
 *
 * @param vpns - an array of virtual page numbers
 * @param K - size of the TLB
 * @param N - number of memory accesses
 * @return int - number of TLB hits
 */
int lru(uint32_t *vpns, int K, int N)
{
    std::unordered_map<uint32_t, int> tlb; // maps vpn to its index in last_access array
    int last_access[K];                    // keeps track of the last access time for each TLB slot
    uint32_t vpn_keys[K];                  // keeps track of the vpn in each TLB slot
    int hits = 0;
    int count = 0;

    // Initialize last_access array with a high value (indicating empty)
    for (int i = 0; i < K; ++i)
    {
        last_access[i] = INT32_MAX;
    }

    for (int i = 0; i < N; ++i)
    {
        if (tlb.find(vpns[i]) != tlb.end())
        {
            // Hit: Update last accessed time for the existing entry
            hits++;
            last_access[tlb[vpns[i]]] = i; // Update the access time
        }
        else
        {
            // Miss: Insert or replace an entry
            if (count < K)
            {
                // There's space in the TLB; add new entry
                tlb[vpns[i]] = count;      // Add entry in TLB
                last_access[count] = i;    // Record access time
                vpn_keys[count] = vpns[i]; // Record the VPN
                count++;
            }
            else
            {
                // TLB is full; find the LRU entry to replace
                int lru_index = 0;
                for (int j = 1; j < K; ++j)
                {
                    if (last_access[j] < last_access[lru_index])
                    {
                        lru_index = j;
                    }
                }

                // Erase the LRU entry from the TLB
                tlb.erase(vpn_keys[lru_index]);

                // Replace the LRU entry with the new entry
                tlb[vpns[i]] = lru_index;
                last_access[lru_index] = i;
                vpn_keys[lru_index] = vpns[i];
            }
        }
    }

    return hits;
}

/**
 * @brief Optimal TLB replacement algorithm
 *
 * @param vpns - an array of virtual page numbers
 * @param K - size of the TLB
 * @param N - number of memory accesses
 * @return int - number of TLB hits
 */
int optimal(uint32_t *vpns, int K, int N)
{
    // std::map<int, int> tlb;
    // std::set<std::pair<int, uint32_t>> tlb; // {-next memory access time, vpn}, so stores the vpn in desscending order of next memory access time
    // std::unordered_map<uint32_t, std::queue<int>> tlb_q; // map of vpn to queue of memory access times
    std::unordered_map<uint32_t, Queue> tlb_q; // map of vpn to queue of memory access times
    std::pair<int, int> tlb_alt[K];            // {next memory access time, vpn} for each tlb slot
    int tlb_size = 0;
    const int inf = 1e7; // infinity as 10^6 is max memory accesses, vpns.size() <= 10^6

    // count frequency of each vpn
    std::unordered_map<uint32_t, unsigned int> freq;
    for (int i = 0; i < N; i++)
    {
        freq[vpns[i]]++;
    }

    // initialize the queues
    for (int i = 0; i < N; i++)
    {
        if (tlb_q.find(vpns[i]) == tlb_q.end())
            tlb_q[vpns[i]].init(freq[vpns[i]] + 1); // store the memory access time for each vpn in ascending order
    }

    for (int i = 0; i < N; i++)
    {
        tlb_q[vpns[i]].push(i); // store the memory access time for each vpn in ascending order
    }

    for (auto &it : tlb_q)
    {
        it.second.push(inf); // push infinity to the end of the queue to show that the vpn is not accessed anymore
    }
    int hits = 0;
    for (int i = 0; i < N; i++)
    {
        bool found = false;
        int found_index = 0;
        for (int j = 0; j < tlb_size; j++)
        {
            if (tlb_alt[j].second == vpns[i])
            {
                found = true;
                found_index = j;
                break;
            }
        }
        if (found)
        {
            hits++;
            // tlb.erase({-tlb_q[vpns[i]].front(), vpns[i]});  // update the vpn in the tlb
            tlb_q[vpns[i]].pop(); // pop the front of the queue, front of queue = current memory access
            // tlb.insert({-tlb_q[vpns[i]].front(), vpns[i]}); // insert the vpn to the tlb
            tlb_alt[found_index].first = tlb_q[vpns[i]].top();
        }
        else
        {

            tlb_q[vpns[i]].pop(); // pop the front of the queue, front of queue = current memory access
            if (tlb_size < K)
            {
                // tlb.insert({-tlb_q[vpns[i]].front(), vpns[i]}); // insert the vpn to the tlb
                tlb_alt[tlb_size] = {tlb_q[vpns[i]].top(), vpns[i]};
                tlb_size++;
            }
            else
            {
                // auto it = tlb.begin();
                // tlb.erase(it);                                  // erase the farthest in future vpn from the tlb
                // tlb.insert({-tlb_q[vpns[i]].front(), vpns[i]}); // insert the vpn to the tlb
                int farthest = 0;
                for (int j = 1; j < K; j++)
                {
                    if (tlb_alt[j].first > tlb_alt[farthest].first)
                    {
                        farthest = j;
                    }
                }
                tlb_alt[farthest] = {tlb_q[vpns[i]].top(), vpns[i]};
            }
        }
    }
    return hits;
}

int main()
{

    // redirect input from input.txt
    // freopen("in.txt", "r", stdin);
    int t;
    std::cin >> t;
    while (t--)
    {
        int S, P, K;
        std::cin >> std::dec >> S >> P >> K;
        // TODO : Implement address bounds
        // address bounds not needed apparently
        // assuming page size to be in power of 2
        int page_offset_bits = log2(P) + 10;
        // std::cout << S << " " << P << " " << K << " " << page_offset_bits << std::endl;

        int N;
        std::cin >> N;
        // std::cout << N << std::endl;
        // std::vector<uint32_t> addrs(N), vpns(N);
        uint32_t addrs[N], vpns[N];
        for (int i = 0; i < N; i++)
        {
            uint32_t addr;
            std::cin >> std::hex >> addr;
            // std::cout << addr << std::endl;
            // convert hex to decimal
            addrs[i] = addr;
            vpns[i] = addr >> page_offset_bits;
            // std::cout << vpns[i] << std::endl;
        }
        int fifo_hits = fifo(vpns, K, N);
        int lifo_hits = lifo(vpns, K, N);
        int lru_hits = lru(vpns, K, N);
        int optimal_hits = optimal(vpns, K, N);
        std::cout << fifo_hits << " " << lifo_hits << " " << lru_hits << " " << optimal_hits << std::endl;
    }
}