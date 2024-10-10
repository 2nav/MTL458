#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <unordered_map>
using namespace std;

typedef struct
{
    int vpn;
    int last_used;
} LRU_entry;

// Added for Set
typedef struct
{
    long int *elements;
    int size;
    int capacity;
} set;

void initSet(set *s)
{
    s->elements = (long int *)malloc(1028 * sizeof(long int));
    s->size = 0;
    s->capacity = 1028;
}

int binarySearch(set *s, int element)
{
    int left = 0;
    int right = s->size - 1;
    while (left <= right)
    {
        int mid = left + (right - left) / 2;
        if (s->elements[mid] == element)
            return mid;
        if (s->elements[mid] < element)
            left = mid + 1;
        else
            right = mid - 1;
    }
    return left;
}

bool contains(set *s, int element)
{
    int pos = binarySearch(s, element);
    return (pos < s->size && s->elements[pos] == element);
}

bool insert(set *s, int element)
{
    if (s->size >= s->capacity)
    {
        printf("Provided Set Constraints have been exceeded (1024).\n");
        return false;
    }

    int pos = binarySearch(s, element);
    if (pos < s->size && s->elements[pos] == element)
        return false;
    for (int i = s->size; i > pos; i--)
        s->elements[i] = s->elements[i - 1];

    s->elements[pos] = element;
    s->size++;

    return true;
}

bool removeByElement(set *s, int element)
{
    int pos = binarySearch(s, element);

    if (pos == s->size || s->elements[pos] != element)
        return false;
    for (int i = pos; i < s->size - 1; i++)
        s->elements[i] = s->elements[i + 1];

    s->size--;
    return true;
}

bool removeByIndex(set *s, int index)
{
    if (index < 0 || index >= s->size)
        return false;
    for (int i = index; i < s->size - 1; i++)
        s->elements[i] = s->elements[i + 1];

    s->size--;
    return true;
}

int size(set *s)
{
    return s->size;
}
// End of Set

// int FIFO(int vpn_bits, int offset_bits, int tlb_count, int num_access, int va_list[]){
//     int tlb[tlb_count];
//     for(int i=0;i<tlb_count;i++)  tlb[i] = -1;

//     int tlb_ptr = 0;
//     int tlb_hits = 0;

//     for(int i = 0; i < num_access; i++){
//         int va = va_list[i];
//         int vpn = va >> offset_bits;
//         int hit_flag = 0;
//         for(int j = 0; j < tlb_count; j++){
//             if(tlb[j] == vpn){
//                 hit_flag = 1;
//                 tlb_hits++;
//                 break;
//             }
//         }
//         if(hit_flag == 0){
//             tlb[tlb_ptr] = vpn;
//             tlb_ptr = (tlb_ptr + 1) % tlb_count;
//         }
//     }

//     return tlb_hits;
// }

int FIFO(int vpn_bits, int offset_bits, int tlb_count, int num_access, long int va_list[])
{
    set tlb;
    initSet(&tlb);

    int tlb_ptr = 0;
    int physical_tlb[tlb_count];

    int tlb_hits = 0;
    for (int i = 0; i < num_access; i++)
    {
        long int va = va_list[i];
        long int vpn = va >> offset_bits;
        if (contains(&tlb, vpn))
        {
            tlb_hits++;
        }
        else
        {
            if (size(&tlb) == tlb_count)
            {
                removeByElement(&tlb, physical_tlb[tlb_ptr]);
            }
            insert(&tlb, vpn);

            physical_tlb[tlb_ptr] = vpn;
            tlb_ptr = (tlb_ptr + 1) % tlb_count;
        }
    }

    return tlb_hits;
}

// int LIFO(int vpn_bits, int offset_bits, int tlb_count, int num_access, int va_list[]){
//     int tlb[tlb_count];
//     for(int i=0;i<tlb_count;i++)  tlb[i] = -1;

//     int tlb_ptr = 0;
//     int tlb_hits = 0;

//     for(int i = 0; i < num_access; i++){
//         int va = va_list[i];
//         int vpn = va >> offset_bits;
//         int hit_flag = 0;
//         for(int j = 0; j < tlb_count; j++){
//             if(tlb[j] == vpn){
//                 hit_flag = 1;
//                 tlb_hits++;
//                 break;
//             }
//         }
//         if(hit_flag == 0){
//             tlb[tlb_ptr] = vpn;
//             tlb_ptr = min(tlb_ptr + 1, tlb_count - 1);
//         }
//     }

//     return tlb_hits;
// }

int LIFO(int vpn_bits, int offset_bits, int tlb_count, int num_access, long int va_list[])
{
    set tlb;
    initSet(&tlb);

    int tlb_ptr = 0;
    int physical_tlb[tlb_count];

    int tlb_hits = 0;
    for (int i = 0; i < num_access; i++)
    {
        long int va = va_list[i];
        long int vpn = va >> offset_bits;
        if (contains(&tlb, vpn))
        {
            tlb_hits++;
        }
        else
        {
            if (size(&tlb) == tlb_count)
            {
                removeByElement(&tlb, physical_tlb[tlb_ptr]);
            }
            insert(&tlb, vpn);

            physical_tlb[tlb_ptr] = vpn;
            tlb_ptr = min(tlb_ptr + 1, tlb_count - 1);
            ;
        }
    }

    return tlb_hits;
}

// int LRU(int vpn_bits, int offset_bits, int tlb_count, int num_access, int va_list[]){
//     LRU_entry tlb[tlb_count];
//     for(int i=0;i<tlb_count;i++){
//         tlb[i].vpn = -1;
//         tlb[i].last_used = -1;
//     }

//     int tlb_ptr = 0;
//     int tlb_hits = 0;
//     int min_last_used_index;

//     for(int i = 0; i < num_access; i++){
//         int va = va_list[i];
//         int vpn = va >> offset_bits;
//         int hit_flag = 0;
//         min_last_used_index = INT64_MAX;
//         for(int j = 0; j < tlb_count; j++){
//             if(tlb[j].vpn == vpn){
//                 hit_flag = 1;
//                 tlb[j].last_used = i;
//                 tlb_hits++;
//                 break;
//             }
//             min_last_used_index = min(min_last_used_index, tlb[j].last_used);
//         }
//         if(hit_flag == 0){
//             if(tlb_ptr > tlb_count-1){
//                 tlb[min_last_used_index].vpn = vpn;
//                 tlb[min_last_used_index].last_used = i;
//             }
//             else{
//                 tlb[tlb_ptr].vpn = vpn;
//                 tlb[tlb_ptr].last_used = i;
//                 tlb_ptr++;
//             }
//         }
//     }

//     return tlb_hits;
// }

int LRU(int vpn_bits, int offset_bits, int tlb_count, int num_access, long int va_list[])
{
    set tlb;
    initSet(&tlb);

    set used_index;
    initSet(&used_index);

    long int index_to_vpn[tlb_count];
    unordered_map<long int, int> vpn_to_index;

    cout << "Initialized" << endl;

    int tlb_hits = 0;

    for (int i = 0; i < num_access; i++)
    {
        long int va = va_list[i];
        long int vpn = va >> offset_bits;
        cout << "VPN: " << vpn << endl;
        if (contains(&tlb, vpn))
        {
            cout << "Hit" << endl;
            tlb_hits++;
            removeByElement(&used_index, vpn_to_index[vpn]); // Removing Old Index from Index Set
            insert(&used_index, i);
            index_to_vpn[i] = vpn;
            cout << "Removed Old Index" << endl;
            cout << vpn << " " << i << endl;
            for (auto x : vpn_to_index)
            {
                cout << x.first << " " << x.second << endl;
            }
            vpn_to_index[vpn] = i;
            cout << "Lmao" << endl;
        }
        else
        {
            cout << "Miss" << endl;
            if (size(&tlb) == tlb_count)
            {
                int index = used_index.elements[0];
                cout << "Removing VPN: " << index_to_vpn[index] << endl;
                removeByElement(&tlb, index_to_vpn[index]);
                removeByIndex(&used_index, 0);
            }
            insert(&tlb, vpn);
            insert(&used_index, i);
            index_to_vpn[i] = vpn;
            vpn_to_index[vpn] = i;
        }
        cout << "Ended" << endl;
    }
    cout << "Out" << endl;
    cout << tlb_hits << endl;
    return tlb_hits;
}

long int Hex_to_Int(string va_str)
{
    long int result = 0;
    for (int i = 0; i < 8; i++)
    {
        char c = va_str[i];
        int digit;
        if (c >= '0' && c <= '9')
        {
            digit = c - '0';
        }
        else if (c >= 'A' && c <= 'F')
        {
            digit = c - 'A' + 10;
        }
        else if (c >= 'a' && c <= 'f')
        {
            digit = c - 'a' + 10;
        }
        else
            return 0; // Incorrect case

        result = (result << 4) | digit;
    }
    return result;
}

int OPT(int vpn_bits, int offset_bits, int tlb_count, int num_access, long int va_list[])
{
    return 0;
}

void TLB_hits(int addr_sz, int page_sz, int tlb_count, int num_access)
{
    // Size of Page gives us the number of bits for offset
    double tmp = log2(page_sz);
    int offset_bits = 10 + ceil(tmp);
    int vpn_bits = 32 - offset_bits;

    long int va_list[num_access];
    for (int i = 0; i < num_access; i++)
    {
        string va;
        cin >> va;
        va_list[i] = Hex_to_Int(va);
    }

    // cout << FIFO(vpn_bits, offset_bits, tlb_count, num_access, va_list) << " ";
    // cout << LIFO(vpn_bits, offset_bits, tlb_count, num_access, va_list) << " ";
    cout << LRU(vpn_bits, offset_bits, tlb_count, num_access, va_list) << " ";
    // cout << "Yaha bhi" << " ";
    cout << OPT(vpn_bits, offset_bits, tlb_count, num_access, va_list) << endl;
}

int main()
{
    int T;
    scanf("%d", &T);
    while (T--)
    {
        int S, P, K, N;
        scanf("%d %d %d %d", &S, &P, &K, &N);
        TLB_hits(S, P, K, N);
    }
}