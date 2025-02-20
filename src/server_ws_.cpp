#include <websocketpp/config/asio_no_tls.hpp>

#include <websocketpp/server.hpp>

#include <iostream>

typedef websocketpp::server<websocketpp::config::asio> server;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;
#include "nlohmann/json.hpp"
#include<string>
#include<iostream>
#include<fstream>
#include <json/json.h>
#include <iostream>
using namespace std;
using namespace Json;
using namespace std;
using json =nlohmann::json;

// pull out the type of messages sent by our config
typedef server::message_ptr message_ptr;
namespace ns
{
    class student
    {
    public:
         string name;
    int age;
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

// namespace 
namespace ns
{
    class style
    {
    public:
        string name;
        //int age;
        //string phone;
    };
    void from_json(const json& j,style &p)
    {
        j.at("name").get_to(p.name);
        //j.at("age").get_to(p.age);
        //j.at("phone").get_to(p.phone);
    }

    void to_json(json& j,const style& s)
    {
        j = json{{"name",s.name}};
    }

    class order //class_room
    {
    public:
    string sessionID;//teacher_name;
    //std::vector<style> student_list;
    std::vector<style> style_list;
    };

    void from_json(const json& j, order& p)
    {
        std::cout << "ns from_json to object" <<std::endl;
        std::cout <<  j <<std::endl;
        std::cout <<  j.at("sessionID") <<std::endl;
        std::cout <<  "sessionID BEgin"  <<std::endl;
        //std::string nameee = j.at(0);
        json k = j["sessionID"];
        std::cout <<  "sessionID000 end"  <<std::endl;

        std::cout <<  k <<std::endl;
        std::cout <<  "sessionID end"  <<std::endl;
        j.at("sessionID").get_to(p.sessionID);
        std::cout << p.sessionID <<std::endl;

        for(auto &style_t:j["StyleImageName"])
        {
            //student s;
            std::cout << style_t << std::endl;
            style s;
            from_json(style_t,s);
            //p.student_list.push_back(s);
            p.style_list.push_back(s);
        }
    }
    //void to_json(json&j, const class_room& s)
    void to_json(json&j, const order& s)
    {
        j = json{ {"sessionID", s.sessionID } };
        for(auto& style_t:s.style_list)
        {
        json j_style;
        to_json(j_style,style_t);
        j["style_list"].push_back(j_style);
        }
    }

}



// Define a callback to handle incoming messages
void on_message(server* s, websocketpp::connection_hdl hdl, message_ptr msg) {
    std::cout << "on_message called with hdl: " << hdl.lock().get()
              << " and message: " << msg->get_payload()
              << std::endl;
    std::cout << msg->get_payload().data() << std::endl;
    std::cout << "The shit message from clientddddd:"<< std::endl;
    std::cout << "The message from client from apifox:"<< std::endl;

    //nlohmann::json commands;
    //std::cout << commands << std::endl;
    std::cout << "hello..."<< std::endl;
    //Json::Value root = msg->get_payload().data();
    //std::cout << root << std::endl;
    std::cout << msg->get_payload().data() << std::endl;
    //commands = nlohmann::json (msg->get_payload().data());//
    //std::cout << commands << std::endl;
    std::cout << "world..."<< std::endl;
    ns::order cr;
    std::cout << "cr..."<< std::endl;
    //std::cout << commands << std::endl;
    ns::from_json(msg->get_payload().data(), cr);
    std::cout << "cr..."<< std::endl;
    std::cout << "hello222..."<< std::endl;
    std::cout << cr.style_list[0].name <<std::endl;
    std::cout << cr.sessionID << "/0.jpg" <<std::endl;

    std::cout <<"hello I am a server"<<std::endl;

    // check for a special command to instruct the server to stop listening so
    // it can be cleanly exited.
    if (msg->get_payload() == "stop-listening") {
        s->stop_listening();
        return;
    }

    try {
        s->send(hdl, msg->get_payload(), msg->get_opcode());
    } catch (websocketpp::exception const & e) {
        std::cout << "Echo failed because: "
                  << "(" << e.what() << ")" << std::endl;
    }
}


int main() {
    // Create a server endpoint
    server echo_server;
    std::cout << "hello, server!"<< std::endl;
    Json::Value root; // root
	Json::FastWriter writer;

	Json::Value Person; // 子Value
	Person["name"] = "tony";
	Person["age"] = 22;
	// 注意append方法的使用场景，只适用在添加数组item
	root.append(Person);

	string jsonStr = writer.write(root); // json到字符串

	cout << "demo_write_simple ==============\n";
	cout << jsonStr << endl;

    try {
        // Set logging settings
        echo_server.set_access_channels(websocketpp::log::alevel::all);
        echo_server.clear_access_channels(websocketpp::log::alevel::frame_payload);

        // Initialize Asio
        echo_server.init_asio();

        // Register our message handler
        echo_server.set_message_handler(bind(&on_message,&echo_server,::_1,::_2));

        // Listen on port 9002
        echo_server.listen(9002);

        // Start the server accept loop
        echo_server.start_accept();

        // Start the ASIO io_service run loop
        echo_server.run();
    } catch (websocketpp::exception const & e) {
        std::cout << e.what() << std::endl;
    } catch (...) {
        std::cout << "other exception" << std::endl;
    }
}
