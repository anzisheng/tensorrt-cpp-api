#include "nlohmann/json.hpp"
#include<string>
#include<iostream>
#include<fstream>
#include "json_info.h"
using namespace namespace Json_nm;

void read_json_file(string file_name, json& commands)
{
ifstream jsonFile(file_name);
jsonFile >> commands;

}

void write_json_file(string file_name, json& commands)
{
std::ofstream file(file_name);
file << commands;
}