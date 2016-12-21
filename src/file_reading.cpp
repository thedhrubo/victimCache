/*************************************************************************
/
/ filename: file_reading.cpp
/
/ description: The functions required to read the files are implemented here
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


#include "../include/file_reading.h"
using namespace std;

void conf_file_reading(ifstream &fin, int *conf_arr) {
    /* Configuration file is read in this function and the parameters are
     * stored in the 'array' passed.
     *
     * Index        property
     *   0          Line Size
     *   1          associativity
     *   2          Data Size
     *   3          Replacement policy
     *   4          Miss penalty 
     *   5          Write Allocate
     * Hit time is default 1
     */
    int i = 0;

    while (!fin.eof() && i < 7) {
        std::string line;
        getline(fin, line);

        if (line[0] == '#' || line[0] == '\n' || line[0] == 0)
            continue;

        conf_arr[i] = atoi(line.c_str());
        //conf_arr[i]=0 signifies default
        i++;
    }

}

long unsigned int hex2int(std::string &hex_string) {
    int i;
    unsigned long int val = 0;
    for (i = 0; i < hex_string.size(); i++) {
        if (hex_string[i] >= '0' && hex_string[i] <= '9')
            val = val * 16 + (hex_string[i] - '0');
        else if (hex_string[i] >= 'a' && hex_string[i] <= 'f')
            val = val * 16 + (hex_string[i] - 'a') + 10;
    }

    return val;
}

vector<string> explode(const string &delimiter, const string &str) {
    vector<string> arr;

    int strleng = str.length();
    int delleng = delimiter.length();
    if (delleng == 0)
        return arr; //no change

    int i = 0;
    int k = 0;
    while (i < strleng) {
        int j = 0;
        while (i + j < strleng && j < delleng && str[i + j] == delimiter[j])
            j++;
        if (j == delleng)//found delimiter
        {
            arr.push_back(str.substr(k, i - k));
            i += delleng;
            k = i;
        } else {
            i++;
        }
    }
    arr.push_back(str.substr(k, i - k));
    return arr;
}

void input_file_reading(ifstream &fin, cache*mem, ofstream &output) {
    std::vector<std::string> str_array;
    while (!fin.eof()) {
        unsigned long int val;
        std::string req, val_string, digit;
        int read_write, instruction_no;

        std::string line;
        getline(fin, line);
        if (line[0] == '\n' || line[0] == '\x0')
            continue;

        char ptr[line.size() + 1];

        strcpy(ptr, line.c_str());
        req = strtok(ptr, " ");

        val_string = strtok(NULL, " ");
        digit = strtok(NULL, " ");
        instruction_no = std::atoi(digit.c_str());
        val = hex2int(val_string);
        read_write = ((req.compare("l") == 0) ? 1 : ((req.compare("s") == 0) ? 2 : 0));
        /* read_write = 1 (for RD)
         * read_write = 2 (for WR)
         * read_write = 0 (otherwise, incorrect input)
         */
        //	std::cout<<"val ="<<val<<std::endl;
        if (read_write)
            mem->read_write_request(val, read_write, instruction_no); // this function is defined in cache.cpp

    }
    output << fixed;
    output << "Total Hit Rate : " << mem->total_hit_rates() << "%\r\nLoad Hit Rate : " << mem->load_hit_rates() << "%\r\nStore Hit Rate : " << mem->store_hit_rates() 
            << "%\r\nTotal Run Time : " << mem->TRT() << "\r\nAverage Memory Access Latency : "  << mem->AMAL() 
            <<"\r\ntotal victim cache hit:" << mem->victimCache()
            << std::endl;

}
