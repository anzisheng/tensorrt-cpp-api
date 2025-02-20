#include "nlohmann/json.hpp"
#include<string>
#include<iostream>
#include<fstream>
#include <json/json.h>
#include <iostream>
#include <json/json.h>
using namespace std;
using namespace Json;
using namespace std;
using json =nlohmann::json;

#include <websocketpp/config/asio_no_tls.hpp>

#include <websocketpp/server.hpp>

#include <iostream>

typedef websocketpp::server<websocketpp::config::asio> server;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

// pull out the type of messages sent by our config
typedef server::message_ptr message_ptr;


//namespace ns
//{
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
        std::cout << "from_json to object" <<std::endl;
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

//}


// Define a callback to handle incoming messages
void on_message(server* s, websocketpp::connection_hdl hdl, message_ptr msg) {
    std::cout << "on_message called with hdl: " << hdl.lock().get()
              << " and message: " << msg->get_payload()
              << std::endl;
    std::cout << "The message from client:"<< std::endl;
    std::cout << msg->get_payload()<<std::endl;
    std::cout << "The message change to json:"<< std::endl;
    std::cout << msg->get_payload().data()<<std::endl;
    nlohmann::json commands = msg->get_payload().data();
    std::cout << "to raw string:"<<commands << std::endl;
    std::string jsonString = commands;
    // 创建一个Json::CharReaderBuilder
    Json::CharReaderBuilder builder;

    // 创建一个Json::Value对象
    Json::Value root;
 
    // 创建一个错误信息字符串
    std::string errors;
    
    // 解析JSON字符串
    std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    bool parsingSuccessful = reader->parse(jsonString.c_str(), jsonString.c_str() + jsonString.size(), &root, &errors);
    if (!parsingSuccessful) {
            // 打印错误信息并退出
            std::cout << "Error parsing JSON: " << errors << std::endl;
            //return 1;
        }
    
    // 提取并打印数据
    std::cout << "Name: " << root["sessionID"].asString() << std::endl;

    //json commands = json(msg->get_payload().data())["sessionID"];
    //std::cout << "the sessioId is :"<<commands.at("sessionID")<<std::endl;

    //order cr;
    //from_json(commands, cr);

    std::cout <<"hello I am server"<<std::endl;

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

    // // 创建一个Json::Value对象
    // Json::Value root;
 
    // // 向对象中添加数据
    // root["name"] = "John Doe";
    // root["age"] = 30;
    // root["isAlive"] = true;
    // root["address"]["city"] = "New York";
    // root["address"]["state"] = "NY";
 
    // // 创建一个Json::StreamWriterBuilder
    // Json::StreamWriterBuilder writer;
 
    // // 将Json::Value对象转换为字符串
    // std::string output = Json::writeString(writer, root);
 
    // // 打印输出
    // std::cout << output << std::endl;
 
    
    // JSON字符串
    //std::string jsonString = R"({"name":"John Doe","age":30,"isAlive":true,"address":{"city":"New York","state":"NY"}})";
    std::string jsonString = R"({	
	"sessionID":"1111111111111111111112222222222222",
	"styleName":[
	{
		"name" : "1.jpg"		
	},
	{
		"name" : "2.jpg"		
	},
	{
		"name" : "3.jpg"
	}
	]	
})";
 
    // 创建一个Json::CharReaderBuilder
    Json::CharReaderBuilder builder;
 
    // 创建一个Json::Value对象
    Json::Value root;
 
    // 创建一个错误信息字符串
    std::string errors;
 
    // 解析JSON字符串
    std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    bool parsingSuccessful = reader->parse(jsonString.c_str(), jsonString.c_str() + jsonString.size(), &root, &errors);
 
    if (!parsingSuccessful) {
        // 打印错误信息并退出
        std::cout << "Error parsing JSON: " << errors << std::endl;
        return 1;
    }
 
    // 提取并打印数据
    std::cout << "Name: " << root["sessionID"].asString() << std::endl;
    // std::cout << "Age: " << root["age"].asInt() << std::endl;
    // std::cout << "Is Alive: " << (root["isAlive"].asBool() ? "Yes" : "No") << std::endl;
    // std::cout << "City: " << root["address"]["city"].asString() << std::endl;
    // std::cout << "State: " << root["address"]["state"].asString() << std::endl;



    // Create a server endpoint
    server echo_server;
    std::cout << "hello, server!"<< std::endl;
    

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
