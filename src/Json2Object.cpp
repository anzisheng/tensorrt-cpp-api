//#include "stdafx.h"
#include "nlohmann/json.hpp"
#include<string>
#include<iostream>
#include<fstream>

using namespace std;
using json =nlohmann::json;

/* 嵌套结构json字符串
{
    "teacher_name": "wangwu",
    "student_list": 
    [
        {
        "name": "zhangsan",
        "age": 16,
        "phone": "12345678"
        },
        {
        "name": "lisi",
        "age": 17,
        "phone": "123456"
        }
    ]
}
*/

namespace ns
{
    class student
    {
    public:
    string name;
    string age;
    string phone;
};
void from_json(const json& j,student&p)
{
    j.at("name").get_to(p.name);
    j.at("age").get_to(p.age);
    j.at("phone").get_to(p.phone);
}

void to_json(json& j,const student& s)
{
    j = json{{"name",s.name},{"age",s.age},{"phone",s.phone}};
}

class class_room
{
public:
string teacher_name;
std::vector<student> student_list;

};

void from_json(const json& j, class_room& p)
{

j.at("teacher_name").get_to(p.teacher_name);

for(auto &student_t:j["student_list"])
{
student s;
from_json(student_t,s);
p.student_list.push_back(s);

}
}
void to_json(json&j, const class_room& s)
{
j = json{ {"teacher_name", s.teacher_name } };
for(auto& student_t:s.student_list)
{
json j_student;
to_json(j_student,student_t);
j["student_list"].push_back(j_student);
}
}


}


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

int main()
{
nlohmann::json commands;
read_json_file("test.json", commands);


cout << commands<<endl;


cout << "00000000" <<endl;
ns::class_room cr;
cout << "1111112222" <<endl;
ns::from_json(commands, cr);
cout << "1111113333" <<endl;
std::cout << cr.student_list[0].age <<std::endl;
cr.student_list[0].age +=10;
cout << "111111" <<endl;
json j_2;
ns::to_json(j_2, cr);
cout << "22222" <<endl;
std::cout<<"j_2:"<< j_2 << std::endl;
return 0;
}