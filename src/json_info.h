#ifndef JSON_INFO_
#define JSON_INFO_
#include "nlohmann/json.hpp"
#include<string>
#include<iostream>
#include<fstream>
namespace Json_nm
{
void read_json_file(string file_name, json& commands);
void write_json_file(string file_name, json& commands);
};
#endif