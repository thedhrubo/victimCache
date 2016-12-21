/*************************************************************************
/
/ filename: cache.cpp
/
/ description: This file contains the necessary functions required for cache execution.
/
/ authors: Saha, Dhrubo
/
/ class: CSE 331
/ instructor: Zheng
/ assignment: Lab Project #2
/
/ assigned: Oct 31, 2016
/ due: Nov 14, 2016
/
/************************************************************************/

#include "../include/cache.h"

cache::cache(int *arr) {
    /* Configuration details */

    blockSize = arr[0];
    associativity = arr[1];
    if (associativity == 0) {
        fully_associative = true;
        associativity = 1;
    }
    data_size = arr[2];
    rp = arr[3];
    cache_miss_penalty = arr[4];
    wp = arr[5];
    cache_hit_time = 1; // cache hit time is default for all the time

    /* ----- */

    /* Initialize the clock (timestamp) and hits/misses to zero */
    timestamp = n_hit = n_miss = 0;
    /* ----- */


    /* Total size of the cache */
    chacheSize = data_size * 1024; // Cache will be in KB so 16KB = 16*1024

    victimSize = arr[6];

    num_sets = (chacheSize / (blockSize * associativity)); // Cache will be in KB so 16KB = 16*1024
    /* ------ */
    //        totalChacheBlocks = chacheSize / blockSize;
    //        totalCacheSets = totalChacheBlocks;        // as the associativity will be direct mapped or fully associative
    //        offsetbits_tot = log2(blockSize);
    //        if(associativity == 1)
    //            indexbits_tot = log2(totalCacheSets);
    //        else indexbits_tot = 0;
    //        tagbits_tot = 32 - (indexbits_tot + offsetbits_tot);
    //        ramSize = ((mem_capacity * 1024 * 1024));
    //        ramMemmoryAllocation();

    mem = new element*[associativity];
    unsigned int i, j;
    for (i = 0; i < associativity; i++) {
        mem[i] = new element[num_sets * blockSize];
    }

    for (i = 0; i < associativity; i++) {
        for (j = 0; j < num_sets; j++)
            mem[i][j].valid = false;
    }
    i = j = 0;
    victimcache = new victimElement*[associativity];
    for (i = 0; i < associativity; i++) {
        victimcache[i] = new victimElement[victimSize];
    }

    for (i = 0; i < associativity; i++) {
        for (j = 0; j < victimSize; j++)
            victimcache[i][j].valid = false;
    }
}

void cache::ramMemmoryAllocation() {
    int i;
    ramAllocation = (unsigned int*) malloc(ramSize * 4);

    for (i = 0; i < (int) (ramSize / SIZEOFINT); i++) {
        updateDataRam(i, 0);
    }

}

void cache::updateDataRam(int index, unsigned int data) {
    ramAllocation[index] = data;
}

bool cache::inCache(long unsigned int val) {
    // Increase time
    timestamp++;

    long unsigned int line_val = val % blockSize;
    long unsigned int index = (val / blockSize) % num_sets;
    long unsigned int tag = (val / blockSize) / num_sets;


    /* Calculate the array index where the value needs to be checked */
    long unsigned int arr_index = (index * blockSize) + line_val;
    unsigned long int layer;


    /* Check in the 2-D array for the required tag at the
     * corresponding index in 'each' 1-D array
     */

    for (layer = 0; layer < associativity; layer++) {
        if (mem[layer][arr_index].valid) {
            if (mem[layer][arr_index].tag == tag) {

                //	printf("%lu %lu %lu %lu\n",tag,index,line_val,timestamp);
                mem[layer][arr_index].time_accessed = timestamp;
                return true;
            }
        }
    }

    /* If not found, return false */
    return false;
}

bool cache::inVictimeCache(long unsigned int val) {
    // Increase time
    timestamp++;

    long unsigned int line_val = val % blockSize;
    long unsigned int index = (val / blockSize) % num_sets;
    long unsigned int tag = (val / blockSize) / num_sets;


    /* Calculate the array index where the value needs to be checked */
    long unsigned int arr_index = (index * blockSize) + line_val;
    unsigned long int layer;


    /* Check in the 2-D array for the required tag at the
     * corresponding index in 'each' 1-D array
     */

    for (layer = 0; layer < associativity; layer++) {
        for (int i = 0; i < victimSize; i++) {
            if (victimcache[layer][i].valid) {
                if (victimcache[layer][i].tag == tag) {
                    
                    int templayer = layer;
                    int tempi = i; 
                    bool tempvalid = mem[layer][arr_index].valid;
                    unsigned long int temptag = mem[layer][arr_index].tag;
                    unsigned long int temptime_stored = mem[layer][arr_index].time_stored;
                    unsigned long int temptime_accessed = mem[layer][arr_index].time_accessed;
                    
                    //	printf("%lu %lu %lu %lu\n",tag,index,line_val,timestamp);
                    mem[layer][arr_index].tag = victimcache[layer][i].tag;
                    mem[layer][arr_index].valid = victimcache[layer][i].valid;
                    mem[layer][arr_index].time_stored = victimcache[layer][i].time_stored;
                    mem[layer][arr_index].time_accessed = victimcache[layer][i].time_accessed;
                    
                    victimcache[templayer][tempi].tag = temptag;
                    victimcache[templayer][tempi].valid = tempvalid;
                    victimcache[templayer][tempi].time_stored = temptime_stored;
                    victimcache[templayer][tempi].time_accessed = temptime_accessed;
                    
                    return true;
                }
            }
        }
    }

    /* If not found, return false */
    return false;
}

bool cache::insert(unsigned long int val) {
    // Increase time
    timestamp++;

    long unsigned int line_val = val % blockSize;
    long unsigned int index = (val / blockSize) % num_sets;
    long unsigned int tag = (val / blockSize) / num_sets;

    /* Calculate the entry point in the array */

    long unsigned int arr_index = (index * blockSize) + line_val;
    long unsigned int layer;
    //printf("%lu %lu %lu %lu\n",tag,index,line_val,timestamp);
    for (layer = 0; layer < associativity && (mem[layer][arr_index].valid); layer++);
    //std::cerr<<layer<<","<<arr_index<<::endl;
    if (layer != associativity) {
        // If appropriate spot is found, write it there
        mem[layer][arr_index].tag = tag;
        mem[layer][arr_index].valid = true;
        mem[layer][arr_index].time_stored = timestamp;
        mem[layer][arr_index].time_accessed = timestamp;
        return true;
    } else {
        /* If associativityiativity = 1, replace the only place in the memory
         * where it can be entered.
         */

        if (associativity == 1) {
            layer = 0;

            //            victimcache[layer][arr_index].tag = mem[layer][arr_index].tag;
            //            victimcache[layer][arr_index].valid = mem[layer][arr_index].valid;
            //            victimcache[layer][arr_index].time_stored = mem[layer][arr_index].time_stored;
            //            victimcache[layer][arr_index].time_accessed = mem[layer][arr_index].time_accessed;
            if (mem[layer][arr_index].valid != false) {
                for (int i = 0; i < victimSize; i++) {
                    if (victimcache[layer][i].valid == false) {
                        victimcache[layer][i].tag = mem[layer][arr_index].tag;
                        victimcache[layer][i].set = arr_index;
                        victimcache[layer][i].valid = mem[layer][arr_index].valid;
                        victimcache[layer][i].time_stored = mem[layer][arr_index].time_stored;
                        victimcache[layer][i].time_accessed = mem[layer][arr_index].time_accessed;
                        break;
                    }
                    if (i == victimSize - 1) {
                        unsigned long int minimum_time = 0;
                        unsigned int checkIndex;
                        minimum_time--;
                        if (rp == 0) {    // Random
                            srand(time(NULL));
                            int replace_index = rand() % associativity;
                            victimcache[layer][replace_index].tag = mem[layer][arr_index].tag;
                            victimcache[layer][replace_index].set = arr_index;
                            victimcache[layer][replace_index].valid = mem[layer][arr_index].valid;
                            victimcache[layer][replace_index].time_stored = mem[layer][arr_index].time_stored;
                            victimcache[layer][replace_index].time_accessed = mem[layer][arr_index].time_accessed;

                        } else if (rp == 1) {  // FIFO
                            unsigned int i, replace_index;
                            unsigned long int min_time = 0;
                            min_time--; // -1 = MAX of unsigned int
                            for (i = 0; i < victimSize; i++) {
                                if (victimcache[layer][i].time_stored < min_time) {
                                    min_time = victimcache[layer][i].time_stored;
                                    replace_index = i;
                                }
                            }
                            victimcache[layer][replace_index].tag = mem[layer][arr_index].tag;
                            victimcache[layer][replace_index].set = arr_index;
                            victimcache[layer][replace_index].valid = mem[layer][arr_index].valid;
                            victimcache[layer][replace_index].time_stored = mem[layer][arr_index].time_stored;
                            victimcache[layer][replace_index].time_accessed = mem[layer][arr_index].time_accessed;

                        } else if (rp == 2) {  // LRU
                            unsigned int i, replace_index;
                            unsigned long int min_time = 0;
                            min_time--; // -1 = MAX of unsigned int
                            for (i = 0; i < victimSize; i++) {
                                if (victimcache[layer][i].time_accessed < min_time) {
                                    min_time = victimcache[layer][i].time_accessed;
                                    replace_index = i;
                                }
                            }
                            victimcache[layer][replace_index].tag = mem[layer][arr_index].tag;
                            victimcache[layer][replace_index].set = arr_index;
                            victimcache[layer][replace_index].valid = mem[layer][arr_index].valid;
                            victimcache[layer][replace_index].time_stored = mem[layer][arr_index].time_stored;
                            victimcache[layer][replace_index].time_accessed = mem[layer][arr_index].time_accessed;
                        } else {
                            std::cerr << "Unknown replacement policy" << std::endl;
                            exit(2); // 2 => unknown replacement policy
                        }
                    }
                }
            }
            mem[layer][arr_index].tag = tag;
            mem[layer][arr_index].valid = true;
            mem[layer][arr_index].time_stored = timestamp;
            mem[layer][arr_index].time_accessed = timestamp;
            return true;
        }

        /* Replace one of the places on the basis of the replacement policy described*/

        if (rp == 0) {
            /* Random replacement */
            srand(time(NULL));
            int replace_index = rand() % associativity;

            //            victimcache[replace_index][arr_index].tag = mem[replace_index][arr_index].tag;
            //            victimcache[replace_index][arr_index].valid = mem[replace_index][arr_index].valid;
            //            victimcache[replace_index][arr_index].time_stored = mem[replace_index][arr_index].time_stored;
            //            victimcache[replace_index][arr_index].time_accessed = mem[replace_index][arr_index].time_accessed;
            for (int i = 0; i < victimSize; i++) {
                if (victimcache[replace_index][i].valid == false) {
                    victimcache[replace_index][i].tag = mem[replace_index][arr_index].tag;
                    victimcache[replace_index][i].valid = mem[replace_index][arr_index].valid;
                    victimcache[replace_index][i].time_stored = mem[replace_index][arr_index].time_stored;
                    victimcache[replace_index][i].time_accessed = mem[replace_index][arr_index].time_accessed;
                    break;
                }
            }

            mem[replace_index][arr_index].tag = tag;
            mem[replace_index][arr_index].valid = true;
            mem[replace_index][arr_index].time_stored = timestamp;
            mem[replace_index][arr_index].time_accessed = timestamp;

            return true;
        } else if (rp == 1) {
            /* FIFO replacement
             * Compare the time_stored and replace the one
             * with the minimum value
             */

            unsigned int i, replace_index;
            unsigned long int min_time = 0;
            min_time--; // -1 = MAX of unsigned int
            for (i = 0; i < associativity; i++) {

                if (mem[i][arr_index].time_stored < min_time) {
                    min_time = mem[i][arr_index].time_stored;
                    replace_index = i;
                }

            }

            /* Finally, replace value at the obtained index */

            //            victimcache[replace_index][arr_index].tag = mem[replace_index][arr_index].tag;
            //            victimcache[replace_index][arr_index].valid = mem[replace_index][arr_index].valid;
            //            victimcache[replace_index][arr_index].time_stored = mem[replace_index][arr_index].time_stored;
            //            victimcache[replace_index][arr_index].time_accessed = mem[replace_index][arr_index].time_accessed;
            for (int i = 0; i < victimSize; i++) {
                if (victimcache[replace_index][i].valid == false) {
                    victimcache[replace_index][i].tag = mem[replace_index][arr_index].tag;
                    victimcache[replace_index][i].valid = mem[replace_index][arr_index].valid;
                    victimcache[replace_index][i].time_stored = mem[replace_index][arr_index].time_stored;
                    victimcache[replace_index][i].time_accessed = mem[replace_index][arr_index].time_accessed;
                    break;
                }
                if (i == victimSize - 1) {
                    unsigned long int minimum_time = 0;
                    unsigned int checkIndex;
                    minimum_time--;
                    for (i = 0; i < victimSize; i++) {

                        if (victimcache[replace_index][i].time_stored < minimum_time) {
                            min_time = victimcache[replace_index][i].time_stored;
                            checkIndex = i;
                        }

                    }
                    victimcache[replace_index][checkIndex].tag = mem[replace_index][arr_index].tag;
                    victimcache[replace_index][checkIndex].valid = mem[replace_index][arr_index].valid;
                    victimcache[replace_index][checkIndex].time_stored = mem[replace_index][arr_index].time_stored;
                    victimcache[replace_index][checkIndex].time_accessed = mem[replace_index][arr_index].time_accessed;
                }

                mem[replace_index][arr_index].tag = tag;
                mem[replace_index][arr_index].valid = true;
                mem[replace_index][arr_index].time_stored = timestamp;
                mem[replace_index][arr_index].time_accessed = timestamp;
                return true;
            }
        } else if (rp == 2) {
            /* LRU replacement
             * Compare the time_accessed and replace the one with the minimum value
             */

            unsigned long int i, replace_index;
            unsigned long int min_time = 0;
            min_time--; // -1 = MAX of unsigned int
            for (i = 0; i < associativity; i++) {
                if (mem[i][arr_index].time_accessed < min_time) {
                    min_time = mem[i][arr_index].time_accessed;
                    replace_index = i;
                }
            }

            /* Finally, replace value at the obtained index */
            //            
            //            victimcache[replace_index][arr_index].tag = mem[replace_index][arr_index].tag;
            //            victimcache[replace_index][arr_index].valid = mem[replace_index][arr_index].valid;
            //            victimcache[replace_index][arr_index].time_stored = mem[replace_index][arr_index].time_stored;
            //            victimcache[replace_index][arr_index].time_accessed = mem[replace_index][arr_index].time_accessed;

            for (int i = 0; i < victimSize; i++) {
                if (victimcache[replace_index][i].valid == false) {
                    victimcache[replace_index][i].tag = mem[replace_index][arr_index].tag;
                    victimcache[replace_index][i].valid = mem[replace_index][arr_index].valid;
                    victimcache[replace_index][i].time_stored = mem[replace_index][arr_index].time_stored;
                    victimcache[replace_index][i].time_accessed = mem[replace_index][arr_index].time_accessed;
                    break;
                }
            }

            mem[replace_index][arr_index].tag = tag;
            mem[replace_index][arr_index].valid = true;
            mem[replace_index][arr_index].time_stored = timestamp;
            mem[replace_index][arr_index].time_accessed = timestamp;
            return true;

        } else {
            std::cerr << "Unknown replacement policy" << std::endl;
            exit(2); // 2 => unknown replacement policy
        }


    }


}

void cache::read_write_request(unsigned long int val, int read_write, int instruction_no) {
    /*
     * read_write = 1 (for load)
     * read_write = 2 (for store)
     */

    /* Irrespective of the read_write request, check if the value is already present */
    total_instruction_no = total_instruction_no + instruction_no;
    if (read_write == 1) {
        total_load++;
    } else if (read_write == 2) {
        total_store++;
    } else {
        std::cerr << "Unknown Instruction" << std::endl;
        exit(3); // Unknown write policy
    }
    if (!inCache(val)) {
        n_miss++;
        if (!inVictimeCache(val)) {
            n_victimMiss++;
            if (insert(val)) {
                if (read_write == 1)
                    return;
                else {
                    if (wp == 0) {
                        // Write back, increase time accordingly
                        timestamp = timestamp + 2;
                        return;
                    } else if (wp == 1) {
                        // Write through
                        return;
                    } else {
                        std::cerr << "Unknown write policy" << std::endl;
                        exit(3); // Unknown write policy
                    }
                }
            }
        } else {
            n_victimHit++;
            if (read_write == 1) {
                n_load_victimHit++;
                return;
            } else {
                n_store_victimHit++;
                if (wp == 0) {
                    // Write back, increase time accordingly
                    return;
                } else if (wp == 1) {
                    timestamp = timestamp + 2;
                    return;
                } else {
                    std::cerr << "Unknown write policy" << std::endl;
                    exit(3); // Unknown write policy
                }
            }
        }
    } else {
        n_hit++;
        //	printf("%lx %lu %d\n",val,val,read_write);
        /* In case of read request, return */
        if (read_write == 1) {
            n_load_hit++;
            return;
        } else {
            n_store_hit++;
            if (wp == 0) {
                // Write back, increase time accordingly
                return;
            } else if (wp == 1) {
                timestamp = timestamp + 2;
                return;
            } else {
                std::cerr << "Unknown write policy" << std::endl;
                exit(3); // Unknown write policy
            }
        }
    }

}

float cache::total_hit_rates() {
    total_instruction = n_hit + n_miss ;
    total_hit_rate = ((n_hit + n_victimHit) / total_instruction)*100;
    return total_hit_rate;
}

float cache::load_hit_rates() {
    load_hit_rate = ((n_load_hit + n_load_victimHit) / total_load)*100;
    return load_hit_rate;
}

float cache::store_hit_rates() {
    store_hit_rate = ((n_store_hit + n_store_victimHit) / total_store)*100;
    return store_hit_rate;
}

float cache::misses() {
    return n_miss;
}

unsigned long int cache::time_taken() {
    return timestamp;
}

double cache::AMAL() {
    total_instruction = n_hit + n_miss;
    return (1 + (n_victimMiss / total_instruction) * cache_miss_penalty) ;
}

float cache::TRT() {
    total_instruction = n_hit + n_miss;
    return (total_instruction * cache_hit_time + (n_miss * cache_miss_penalty) + total_instruction_no);

}

float cache::victimCache() {
    return n_victimHit;

}
