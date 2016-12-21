/*************************************************************************
/
/ filename: file_reading.h
/
/ description: File describes the classes necessary for reading both the input and configuration files.
/
/ authors: Saha, Dhrubo
/
/ class: CSE 331
/ instructor: Zheng
/ assignment: Lab Project #1
/
/ assigned: Oct 31, 2016
/ due: Nov 14, 2016
/
/************************************************************************/



#ifndef FILE_READING_INCLUDED
#define FILE_READING_INCLUDED

#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <vector>

#include "cache.h"

using std::ifstream;
using std::ofstream;


void conf_file_reading(ifstream &fin,int *conf_arr);
void input_file_reading(ifstream &fin,cache *mem,ofstream &output);
std::vector<std::string> explode( const std::string &delimiter, const std::string &str);

#endif      // End of file
