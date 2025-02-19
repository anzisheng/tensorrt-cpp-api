//trtexec --onnx=gfpgan_1.4.onnx --saveEngine=gfpgan_1.4.engine.Orin.fp16.1.1++ --fp16

#include "buffers.h"
//#include "faceswap_fromMNist.h"
#include "cmd_line_parser.h"
#include "logger.h"
#include "engine.h"
#include <chrono>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/opencv.hpp>
#include "yolov8.h"
//#include "faceswap.h"
#include "faceswap_trt.h"
//#include "face68landmarks.h"
#include "Face68Landmarks_trt.h"
#include "facerecognizer_trt.h"
#include "faceenhancer_trt.h"
//#include "faceenhancer.h"
//#include "faceenhancer_trt.h"
//#include "faceenhancer_trt2.h"
#include "faceswap_trt.h"
#include "SampleOnnxMNIST.h"
#include "nlohmann/json.hpp" 

//#include "faceswap.h"
#include "engine.h"
#include "utile.h"
#include "yolov8.h"
#include <vector>
//#include "json_info.h"
#include "nlohmann/json.hpp"
using namespace std;
using namespace cv;
using json =nlohmann::json;

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


//function:
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
//////////////////////////////////////
#include <thread>
#include <iostream>
#include <queue>
#include <functional>
#include <fstream>
#include <mutex>
#include <condition_variable>
using namespace std;

template <typename T>
class MutexSafe
{
private:
    mutex _mutex;
    T* _resource;
    T* operator ->(){}
    T& operator &(){}
public:
    MutexSafe(T* resource):_resource(resource){}
    ~MutexSafe(){delete _resource;}
    void lock()
    {
        _mutex.lock();
    }
    void unlock()
    {
        _mutex.unlock();
    }
    bool try_lock()
    {
        return _mutex.try_lock();
    }
    mutex& Mutex()
    {
        return _mutex;
    }
    T& Acquire (unique_lock<MutexSafe<T>>& lock)
    {
        MutexSafe<T> *_safe = lock.mutex();
        if(&_safe->Mutex()!=&_mutex)
        {
            throw "wrong lock object passed to Acquire function.\n";
        }
        return *_resource;
    }
    T& Acquire (unique_lock<mutex>& lock)
    {
        if(lock.mutex()!=&_mutex)
        {
            throw "wrong lock object passed to Acquire function.\n";
        }
        return *_resource;
    }
};

template <typename MsgType>
class MsgQueue
{
private:
    queue<MsgType> _queue; 
    mutex _mutex;
    condition_variable _enqCv;
    condition_variable _deqCv;
    int _limit;
public:
    MsgQueue(int limit=3):_limit(limit){
        std::cout << "队列长度最大为："<< _limit << "。" << std::endl;
    }
    void Enqueue(MsgType & msg)
    {
        unique_lock<mutex> lock(_mutex);
        if(_queue.size()>=_limit)
        {
             cout<<"queue is full, wait()..."<<endl;
            _enqCv.wait(lock,[this]{return _queue.size()<_limit;});
        }
        _queue.push(msg);
        cout << "now ,the queue length is : " <<_queue.size() <<endl;
        cout << "I have added one task, any one thread can dequeue the task"<<endl;
        _deqCv.notify_one();
    }
    MsgType& Dequeue()
    {
        unique_lock<mutex> lock(_mutex);
        if(_queue.size()<=0)
        {
            cout<<"queue is empty, wait()..."<<endl;
            _deqCv.wait(lock, [this]{return _queue.size()>0;});
            //cout << "I am back when queue size is " <<_queue.size() <<endl;
        }
        MsgType& msg = _queue.front();
        cout << "get the first rask: " <<msg.photo << " " << msg.style <<endl;
        _queue.pop();
        _enqCv.notify_one();
        return msg;
    }
    int Size()
    {
        unique_lock<mutex> lock(_mutex);
        return _queue.size();
    }
};

struct CustomerTask
{
    string task;
    float money;
    string photo;
    string style;
    string swap;
    CustomerTask(){}
    CustomerTask(string photo_filename, string style_filename):photo(photo_filename), style(style_filename){}
    CustomerTask(const CustomerTask& cp):task(cp.task),money(cp.money){}
    string SwapFace()
    {
        cout << "swap face" << endl;
        cout << photo << "   " <<style <<endl;
        return "swap...";
    }
    void ExecuteTask()
    {
        //swap = SwapFace();
        //cout << "swap is " << swap;
        //return;
        ///////////////////////////
        //swap face process
        cout << "begin to swap facing ..."<< photo << style <<endl;
        YoloV8Config config;
        std::string onnxModelPath;
        std::string onnxModelPathLandmark;

        std::string inputImage = this->photo;        
        std::string outputImage = this->style;
        cout << "inputImage::: " << inputImage << " and  " << outputImage <<endl;

        YoloV8 yoloV8("yoloface_8n.onnx", config); //
        //std::cout << "what's the fuck..."<< std::endl;
        Face68Landmarks_trt detect_68landmarks_net_trt("2dfan4.onnx", config);
        FaceEmbdding_trt face_embedding_net_trt("arcface_w600k_r50.onnx", config);

        SwapFace_trt swap_face_net_trt("inswapper_128.onnx", config, 1);
        samplesCommon::BufferManager buffers(swap_face_net_trt.m_trtEngine_faceswap->m_engine);
        
        //samplesCommon::Args args; // 接收用户传递参数的变量
        //SampleOnnxMNIST sample(initializeSampleParams(args)); // 定义一个sample实例
        //FaceEnhance enhance_face_net("gfpgan_1.4.onnx");
        FaceEnhance_trt enhance_face_net_trt("gfpgan_1.4.onnx", config, 1);
        //FaceEnhance_trt2 enhance_face_net_trt2("gfpgan_1.4.onnx", config);
        samplesCommon::BufferManager buffers_enhance(enhance_face_net_trt.m_trtEngine_enhance->m_engine);

            preciseStopwatch stopwatch;
     // Read the input image
    cv::Mat img = cv::imread(inputImage);
    cv::Mat source_img = img.clone();
    
    cout << "inputImage: " << inputImage <<endl;

    std::vector<Object>objects = yoloV8.detectObjects(img);
    
    // Draw the bounding boxes on the image
#ifdef SHOW
    yoloV8.drawObjectLabels(source_img, objects);
    // Save the image to disk
    const auto outputName = inputImage.substr(0, inputImage.find_last_of('.')) + "_annotated.jpg";
    cv::imwrite(outputName, source_img);
    std::cout << "Saved annotated image to: " << outputName << std::endl;
#endif

   
    std::vector<cv::Point2f> face_landmark_5of68_trt;
    //std::cout <<"begin to detect landmark"<<std::endl;
    std::vector<cv::Point2f> face68landmarks_trt = detect_68landmarks_net_trt.detectlandmark(img, objects[0], face_landmark_5of68_trt);
    #ifdef SHOW
    //std::cout << "face68landmarks_trt size: " <<face68landmarks_trt.size()<<std::endl;
    //std::cout << "face_landmark_5of68_trt size: " <<face_landmark_5of68_trt.size()<<std::endl;
    for(int i =0; i < face68landmarks_trt.size(); i++)
	{
		//destFile2 << source_face_embedding[i] << " " ;
        cout << face68landmarks_trt[i] << " ";
	}    
    for(int i =0; i < face_landmark_5of68_trt.size(); i++)
	{
		//destFile2 << source_face_embedding[i] << " " ;
        cout << face_landmark_5of68_trt[i] << " ";
	}
    #endif

    
    vector<float> source_face_embedding = face_embedding_net_trt.detect(source_img, face_landmark_5of68_trt);

       
    cv::Mat target_img = cv::imread(outputImage);
    cv::Mat target_img2 =target_img.clone();

    std::vector<Object>objects_target = yoloV8.detectObjects(target_img);

    #ifdef SHOW
    // Draw the bounding boxes on the image
    yoloV8.drawObjectLabels(target_img2, objects_target);
    cout << "Detected " << objects_target.size() << " objects" << std::endl;
    // Save the image to disk
    const auto outputName_target = outputImage.substr(0, outputImage.find_last_of('.')) + "_annotated.jpg";
    cv::imwrite(outputName_target, target_img2);
    std::cout << "Saved annotated image to: " << outputName_target << std::endl;
#endif
     
	// int position = 0; ////一张图片里可能有多个人脸，这里只考虑1个人脸的情况
	// vector<Point2f> target_landmark_5(5);    
	// detect_68landmarks_net_trt.detectlandmark(target_img, objects_target[position], target_landmark_5);
    
    // cv::Mat swapimg = swap_face_net_trt.process(target_img, source_face_embedding, target_landmark_5, buffers);



// #ifdef SHOW
//     // Draw the bounding boxes on the image
//     yoloV8.drawObjectLabels(target_img2, objects_target);
//     cout << "Detected " << objects_target.size() << " objects" << std::endl;
//     // Save the image to disk
//     const auto outputName_target = outputImage.substr(0, outputImage.find_last_of('.')) + "_annotated.jpg";
//     cv::imwrite(outputName_target, target_img2);
//     std::cout << "Saved annotated image to: " << outputName_target << std::endl;
// #endif
     
	int position = 0; ////一张图片里可能有多个人脸，这里只考虑1个人脸的情况
	vector<Point2f> target_landmark_5(5);    
	detect_68landmarks_net_trt.detectlandmark(target_img, objects_target[position], target_landmark_5);
    
    cv::Mat swapimg = swap_face_net_trt.process(target_img, source_face_embedding, target_landmark_5, buffers);
    //imwrite("target_img.jpg", target_img);
//#ifdef SHOW        
    //std::cout << "swap_face_net.process end" <<std::endl;
    //imwrite("swapimg.jpg", swapimg);
//#endif    
    
    cv::Mat resultimg = enhance_face_net_trt.process(swapimg, target_landmark_5, buffers_enhance);
    
    imwrite("resultimgend.jpg", resultimg);

    //if (!sample.build()) // 【主要】在build方法中构建网络，返回构建网络是否成功的状态
    // {
    //     cout<<"bad build"<<endl;
    //     return 0;////sample::gLogger.reportFail(sampleTest);
    // }
    // //if (!sample.infer()) // 【主要】读取图像并进行推理，返回推理是否成功的状态
    // {
    //      cout<<"bad build"<<endl;
    //     return 0;////sample::gLogger.reportFail(sampleTest);
    // }
	
    //preciseStopwatch stopwatch;
    auto totalElapsedTimeMs = stopwatch.elapsedTime<float, std::chrono::milliseconds>();
    cout << "total time is " << totalElapsedTimeMs/1000 <<" S"<<endl;
   

        cout << "end"<<endl;
        ////////////////////////////


        if(money>0)
            cout<<"Task "<<task<<" is executed at $"<<money<<endl;
        else
            cout<<"Bank closed because the price is $"<<money<<endl;
        
    }
};

//typedef MsgQueue<CustomerTask> TaskQueueType;
typedef MsgQueue<CustomerTask> TaskQueueType;
typedef MutexSafe<TaskQueueType> TaskQueueSafe;
class ThreadPool
{
private:
    int _limit;
    vector<thread*> _workerThreads;
    TaskQueueType& _taskQueue;
    bool _threadPoolStop=false;
public:
  
    void ExecuteTask()
    {
        while(1)
        {
            CustomerTask task=_taskQueue.Dequeue();
            cout << "get the task from queue:" << task.photo << " and " << task.style <<endl;
            task.ExecuteTask();
            //task.SwapFace();

            if(task.money<0)
            {
                _threadPoolStop=true;
                _taskQueue.Enqueue(task);//tell other threads to stop
            }
            if(_threadPoolStop)
            {
                cout<< "thread finshed!"<<endl;
                return;
            }
            //the sleep function simulates that the task takes a while to finish 
            std::this_thread::sleep_for(std::chrono::milliseconds(rand()%100));
        }
    }

        ThreadPool(TaskQueueType& taskQueue,int limit=3):_limit(limit),_taskQueue(taskQueue){
        for(int i=0;i<_limit;++i)
        {  //生成 _limit个线程对象，并放到vetor里面
            _workerThreads.push_back(new thread(&ThreadPool::ExecuteTask, this));
        }
    }
    ~ThreadPool(){
        for(auto threadObj: _workerThreads)
        {
            if(threadObj->joinable())
                {
                    threadObj->join();
                    delete threadObj;
                }
        }
    }
};


void TestLeaderFollower()
{
    TaskQueueType taskQueue(5);//可以最多放五个任务的队列
    
    ThreadPool pool(taskQueue,1);//放着3个线程的线程池
    //the sleep function simulates the situation that all  
    //worker threads are waiting for the empty message queue at the beginning
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    for(int i=0;i<1;i++)
    {
        string style_name = fmt::format("{}.jpg", i+1);
        cout <<"style image: "<< style_name<<endl; 
        CustomerTask task("0.jpg",style_name);

        //task.photo = "0.jpg";
        
        //cout <<  style_name<<endl; 
        //task.style = style_name; 

        task.money= i+1;
        if(task.money >5)
            task.task = "deposit $";
        else
            task.task = "withdraw $";
        cout << "first task is " <<task.photo << " and " << task.style<< endl;     
        taskQueue.Enqueue(task);
        cout <<"current queue length:" << taskQueue.Size()<<endl;
    }

}

/////////
//process
string swap_faces(string photo, string style){
        //tensorrt part
    YoloV8Config config;
    std::string onnxModelPath;
    std::string onnxModelPathLandmark;
    std::string inputImage = photo;// "1.jpg";
    std::string outputImage =style;// "12.jpg";
    
    //std::string inputImage = "12.jpg";
    //std::string outputImage = "1.jpg";
    std::cout << "world 000..."<< std::endl;
    YoloV8 yoloV8("yoloface_8n.onnx", config); //
    std::cout << "what's the fuck..."<< std::endl;
    Face68Landmarks_trt detect_68landmarks_net_trt("2dfan4.onnx", config);
    FaceEmbdding_trt face_embedding_net_trt("arcface_w600k_r50.onnx", config);
   
  
    //SwapFace swap_face_net("inswapper_128.onnx");
    SwapFace_trt swap_face_net_trt("inswapper_128.onnx", config, 1);
    samplesCommon::BufferManager buffers(swap_face_net_trt.m_trtEngine_faceswap->m_engine);
    
    //samplesCommon::Args args; // 接收用户传递参数的变量
    //SampleOnnxMNIST sample(initializeSampleParams(args)); // 定义一个sample实例
    //FaceEnhance enhance_face_net("gfpgan_1.4.onnx");
    FaceEnhance_trt enhance_face_net_trt("gfpgan_1.4.onnx", config, 1);
    //FaceEnhance_trt2 enhance_face_net_trt2("gfpgan_1.4.onnx", config);
    samplesCommon::BufferManager buffers_enhance(enhance_face_net_trt.m_trtEngine_enhance->m_engine);

    //cout << "gfpgan_1.4.onnx trted"<<endl;
    preciseStopwatch stopwatch;
     // Read the input image
    cv::Mat img = cv::imread(inputImage);
    cv::Mat source_img = img.clone();
    

    std::vector<Object>objects = yoloV8.detectObjects(img);
    
    // Draw the bounding boxes on the image
#ifdef SHOW
    yoloV8.drawObjectLabels(source_img, objects);
    // Save the image to disk
    const auto outputName = inputImage.substr(0, inputImage.find_last_of('.')) + "_annotated.jpg";
    cv::imwrite(outputName, source_img);
    std::cout << "Saved annotated image to: " << outputName << std::endl;
#endif
    

    
    std::vector<cv::Point2f> face_landmark_5of68_trt;
    //std::cout <<"begin to detect landmark"<<std::endl;
    std::vector<cv::Point2f> face68landmarks_trt = detect_68landmarks_net_trt.detectlandmark(img, objects[0], face_landmark_5of68_trt);
    #ifdef SHOW
    //std::cout << "face68landmarks_trt size: " <<face68landmarks_trt.size()<<std::endl;
    //std::cout << "face_landmark_5of68_trt size: " <<face_landmark_5of68_trt.size()<<std::endl;
    for(int i =0; i < face68landmarks_trt.size(); i++)
	{
		//destFile2 << source_face_embedding[i] << " " ;
        cout << face68landmarks_trt[i] << " ";
	}    
    for(int i =0; i < face_landmark_5of68_trt.size(); i++)
	{
		//destFile2 << source_face_embedding[i] << " " ;
        cout << face_landmark_5of68_trt[i] << " ";
	}
    #endif

    
    vector<float> source_face_embedding = face_embedding_net_trt.detect(source_img, face_landmark_5of68_trt);

       
    cv::Mat target_img = cv::imread(outputImage);
    cv::Mat target_img2 =target_img.clone();

    std::vector<Object>objects_target = yoloV8.detectObjects(target_img);
   

#ifdef SHOW
    // Draw the bounding boxes on the image
    yoloV8.drawObjectLabels(target_img2, objects_target);
    cout << "Detected " << objects_target.size() << " objects" << std::endl;
    // Save the image to disk
    const auto outputName_target = outputImage.substr(0, outputImage.find_last_of('.')) + "_annotated.jpg";
    cv::imwrite(outputName_target, target_img2);
    std::cout << "Saved annotated image to: " << outputName_target << std::endl;
#endif
     
	int position = 0; ////一张图片里可能有多个人脸，这里只考虑1个人脸的情况
	vector<Point2f> target_landmark_5(5);    
	detect_68landmarks_net_trt.detectlandmark(target_img, objects_target[position], target_landmark_5);
    
    cv::Mat swapimg = swap_face_net_trt.process(target_img, source_face_embedding, target_landmark_5, buffers);
    //imwrite("target_img.jpg", target_img);
//#ifdef SHOW        
    //std::cout << "swap_face_net.process end" <<std::endl;
    //imwrite("swapimg.jpg", swapimg);
//#endif    
    
    cv::Mat resultimg = enhance_face_net_trt.process(swapimg, target_landmark_5, buffers_enhance);
    string result = fmt::format("{}_{}.jpg",  photo.substr(0, photo.rfind(".")), style.substr(0, style.rfind(".")));
    //imwrite("resultimgend.jpg", resultimg);
    imwrite(result, resultimg);
    return result;


    //if (!sample.build()) // 【主要】在build方法中构建网络，返回构建网络是否成功的状态
    // {
    //     cout<<"bad build"<<endl;
    //     return 0;////sample::gLogger.reportFail(sampleTest);
    // }
    // //if (!sample.infer()) // 【主要】读取图像并进行推理，返回推理是否成功的状态
    // {
    //      cout<<"bad build"<<endl;
    //     return 0;////sample::gLogger.reportFail(sampleTest);
    // }
	
    //preciseStopwatch stopwatch;
    auto totalElapsedTimeMs = stopwatch.elapsedTime<float, std::chrono::milliseconds>();
    cout << "total time is " << totalElapsedTimeMs/1000 <<" S"<<endl;

    return "resultimgend.jpg";

    
}

////////////


class TaskSocket
{
    public:
    string photo;
    string style;
    TaskSocket(string photo_file, string style_file):photo(photo_file), style(style_file){}

};
std::queue<TaskSocket> messageQueue; // 消息队列
std::mutex mtx; // 互斥锁
std::condition_variable cvs; // 条件变量

// 生产者线程函数，向消息队列中添加消息
void producerFunction() {
    for (int i = 1; i <= 12; ++i) {
        string temp_style = fmt::format("{}.jpg", i);
        
        // 创建消息
        TaskSocket message("c3.jpg", temp_style);

        // 将消息添加到队列
        {
            std::lock_guard<std::mutex> lock(mtx);
            messageQueue.push(message);
            std::cout << "Produced message: " << message.photo<<message.style << std::endl;
        }

        // 通知等待的消费者线程
        cvs.notify_one();

        // 模拟一些工作
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

// 消费者线程函数，从消息队列中获取消息
void consumerFunction() {
    while (true) {
        // 等待消息队列非空
        std::unique_lock<std::mutex> lock(mtx);
        cvs.wait(lock, [] { return !messageQueue.empty(); });

        // 从队列中获取消息
        TaskSocket message = messageQueue.front();
        messageQueue.pop();
        std::cout << "Consumed message: " << message.photo <<" and " <<message.style << std::endl;
        cout << swap_faces(message.photo,message.style) ;


        // 检查是否为终止信号
        // if (message.style == "12.jpg") {
        //     break;
        // }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}
///////////////////////////////////////////////////////////
//websocket
#include <websocketpp/config/asio_no_tls.hpp>

#include <websocketpp/server.hpp>

#include <iostream>

typedef websocketpp::server<websocketpp::config::asio> server;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

// pull out the type of messages sent by our config
typedef server::message_ptr message_ptr;

// Define a callback to handle incoming messages
void on_message(server* s, websocketpp::connection_hdl hdl, message_ptr msg) {
    std::cout << "on_message called with hdl: " << hdl.lock().get()
              << " and message: " << msg->get_payload()
              << std::endl;
    std::cout << "The message from client:"<< std::endl;
    std::cout << msg->get_payload()<<std::endl;
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


/////////////////////////////////////////////////


int main(int argc, char *argv[]) {


//////////



////////

  /*
	std::cout << "hello..."<< std::endl;
    //1. read the received message.
    nlohmann::json commands;
    read_json_file("info.json", commands);
    std::cout << "world..."<< std::endl;
    ns::order cr;
    std::cout << commands << std::endl;
    ns::from_json(commands, cr);
    std::cout << "hello222..."<< std::endl;
    std::cout << cr.style_list[0].name <<std::endl;
    std::cout << cr.sessionID << "/0.jpg" <<std::endl;

    */ 
    //TestLeaderFollower();
    cout << "++++++++++++++++++++++++++++++++++++" <<endl;
    //tensorrt part
    YoloV8Config config;
    std::string onnxModelPath;
    std::string onnxModelPathLandmark;
    //read input image from task_socket.
    std::string inputImage = "1.jpg";
    std::string outputImage = "3.jpg";
    
        // 创建生产者线程和消费者线程
    std::thread producer(producerFunction);
    std::thread consumer(consumerFunction);

    // 等待线程执行完成
    producer.join();
    consumer.join();
    cout << "-----------------------"<<endl;
    //cout << swap_faces(inputImage,outputImage) ;

    return 0;
    //std::string inputImage = "12.jpg";
    //std::string outputImage = "1.jpg";
    std::cout << "world 000..."<< std::endl;
    YoloV8 yoloV8("yoloface_8n.onnx", config); //
    std::cout << "what's the fuck..."<< std::endl;
    Face68Landmarks_trt detect_68landmarks_net_trt("2dfan4.onnx", config);
    FaceEmbdding_trt face_embedding_net_trt("arcface_w600k_r50.onnx", config);
   
  
    //SwapFace swap_face_net("inswapper_128.onnx");
    SwapFace_trt swap_face_net_trt("inswapper_128.onnx", config, 1);
    samplesCommon::BufferManager buffers(swap_face_net_trt.m_trtEngine_faceswap->m_engine);
    
    //samplesCommon::Args args; // 接收用户传递参数的变量
    //SampleOnnxMNIST sample(initializeSampleParams(args)); // 定义一个sample实例
    //FaceEnhance enhance_face_net("gfpgan_1.4.onnx");
    FaceEnhance_trt enhance_face_net_trt("gfpgan_1.4.onnx", config, 1);
    //FaceEnhance_trt2 enhance_face_net_trt2("gfpgan_1.4.onnx", config);
    samplesCommon::BufferManager buffers_enhance(enhance_face_net_trt.m_trtEngine_enhance->m_engine);

    //cout << "gfpgan_1.4.onnx trted"<<endl;
    preciseStopwatch stopwatch;
     // Read the input image
    cv::Mat img = cv::imread(inputImage);
    cv::Mat source_img = img.clone();
    

    std::vector<Object>objects = yoloV8.detectObjects(img);
    
    // Draw the bounding boxes on the image
#ifdef SHOW
    yoloV8.drawObjectLabels(source_img, objects);
    // Save the image to disk
    const auto outputName = inputImage.substr(0, inputImage.find_last_of('.')) + "_annotated.jpg";
    cv::imwrite(outputName, source_img);
    std::cout << "Saved annotated image to: " << outputName << std::endl;
#endif
    

    
    std::vector<cv::Point2f> face_landmark_5of68_trt;
    //std::cout <<"begin to detect landmark"<<std::endl;
    std::vector<cv::Point2f> face68landmarks_trt = detect_68landmarks_net_trt.detectlandmark(img, objects[0], face_landmark_5of68_trt);
    #ifdef SHOW
    //std::cout << "face68landmarks_trt size: " <<face68landmarks_trt.size()<<std::endl;
    //std::cout << "face_landmark_5of68_trt size: " <<face_landmark_5of68_trt.size()<<std::endl;
    for(int i =0; i < face68landmarks_trt.size(); i++)
	{
		//destFile2 << source_face_embedding[i] << " " ;
        cout << face68landmarks_trt[i] << " ";
	}    
    for(int i =0; i < face_landmark_5of68_trt.size(); i++)
	{
		//destFile2 << source_face_embedding[i] << " " ;
        cout << face_landmark_5of68_trt[i] << " ";
	}
    #endif

    
    vector<float> source_face_embedding = face_embedding_net_trt.detect(source_img, face_landmark_5of68_trt);

       
    cv::Mat target_img = cv::imread(outputImage);
    cv::Mat target_img2 =target_img.clone();

    std::vector<Object>objects_target = yoloV8.detectObjects(target_img);
   

#ifdef SHOW
    // Draw the bounding boxes on the image
    yoloV8.drawObjectLabels(target_img2, objects_target);
    cout << "Detected " << objects_target.size() << " objects" << std::endl;
    // Save the image to disk
    const auto outputName_target = outputImage.substr(0, outputImage.find_last_of('.')) + "_annotated.jpg";
    cv::imwrite(outputName_target, target_img2);
    std::cout << "Saved annotated image to: " << outputName_target << std::endl;
#endif
     
	int position = 0; ////一张图片里可能有多个人脸，这里只考虑1个人脸的情况
	vector<Point2f> target_landmark_5(5);    
	detect_68landmarks_net_trt.detectlandmark(target_img, objects_target[position], target_landmark_5);
    
    cv::Mat swapimg = swap_face_net_trt.process(target_img, source_face_embedding, target_landmark_5, buffers);
    //imwrite("target_img.jpg", target_img);
//#ifdef SHOW        
    //std::cout << "swap_face_net.process end" <<std::endl;
    //imwrite("swapimg.jpg", swapimg);
//#endif    
    
    cv::Mat resultimg = enhance_face_net_trt.process(swapimg, target_landmark_5, buffers_enhance);
    
    imwrite("resultimgend.jpg", resultimg);

    //if (!sample.build()) // 【主要】在build方法中构建网络，返回构建网络是否成功的状态
    // {
    //     cout<<"bad build"<<endl;
    //     return 0;////sample::gLogger.reportFail(sampleTest);
    // }
    // //if (!sample.infer()) // 【主要】读取图像并进行推理，返回推理是否成功的状态
    // {
    //      cout<<"bad build"<<endl;
    //     return 0;////sample::gLogger.reportFail(sampleTest);
    // }
	
    //preciseStopwatch stopwatch;
    auto totalElapsedTimeMs = stopwatch.elapsedTime<float, std::chrono::milliseconds>();
    cout << "total time is " << totalElapsedTimeMs/1000 <<" S"<<endl;

    return 0;

/*
    CommandLineArguments arguments;

    std::string logLevelStr = getLogLevelFromEnvironment();
    spdlog::level::level_enum logLevel = toSpdlogLevel(logLevelStr);
    spdlog::set_level(logLevel);

    // Parse the command line arguments
    // if (!parseArguments(argc, argv, arguments)) {
    //     return -1;
    // }

    //std::string flag = "trt_model";
    //arguments.trtModelPath = "../models/yoloface_8n.engine.Orin.fp16.1.1";
    //const std::string inputImage = "../inputs/2.jpg";




    // Specify our GPU inference configuration options
    Options options;
    // Specify what precision to use for inference
    // FP16 is approximately twice as fast as FP32.
    options.precision = Precision::FP16;
    // If using INT8 precision, must specify path to directory containing
    // calibration data.
    options.calibrationDataDirectoryPath = "";
    // Specify the batch size to optimize for.
    options.optBatchSize = 1;
    // Specify the maximum batch size we plan on running.
    options.maxBatchSize = 1;
    // Specify the directory where you want the model engine model file saved.
    options.engineFileDir = ".";

    Engine<float> engine(options);

    // Define our preprocessing code
    // The default Engine::build method will normalize values between [0.f, 1.f]
    // Setting the normalize flag to false will leave values between [0.f, 255.f]
    // (some converted models may require this).

    // For our YoloV8 model, we need the values to be normalized between
    // [0.f, 1.f] so we use the following params
    std::array<float, 3> subVals{0.f, 0.f, 0.f};
    std::array<float, 3> divVals{1.f, 1.f, 1.f};
    bool normalize = true;
    // Note, we could have also used the default values.

    // If the model requires values to be normalized between [-1.f, 1.f], use the
    // following params:
    //    subVals = {0.5f, 0.5f, 0.5f};
    //    divVals = {0.5f, 0.5f, 0.5f};
    //    normalize = true;

    if (!arguments.onnxModelPath.empty()) {
        // Build the onnx model into a TensorRT engine file, and load the TensorRT
        // engine file into memory.
        bool succ = engine.buildLoadNetwork(arguments.onnxModelPath, subVals, divVals, normalize);
        if (!succ) {
            throw std::runtime_error("Unable to build or load TensorRT engine.");
        }
    } else {
        // Load the TensorRT engine file directly
        bool succ = engine.loadNetwork(arguments.trtModelPath, subVals, divVals, normalize);
        if (!succ) {
            const std::string msg = "Unable to load TensorRT engine.";
            spdlog::error(msg);
            throw std::runtime_error(msg);
        }
    }

    // Read the input image
    // TODO: You will need to read the input image required for your model
    //const std::string inputImage = "../inputs/2.jpg";
    auto cpuImg = cv::imread(inputImage);
    if (cpuImg.empty()) {
        const std::string msg = "Unable to read image at path: " + inputImage;
        spdlog::error(msg);
        throw std::runtime_error(msg);
    }

    // Upload the image GPU memory
    cv::cuda::GpuMat img;
    img.upload(cpuImg);

    // The model expects RGB input
    cv::cuda::cvtColor(img, img, cv::COLOR_BGR2RGB);

    // In the following section we populate the input vectors to later pass for
    // inference
    const auto &inputDims = engine.getInputDims();
    std::vector<std::vector<cv::cuda::GpuMat>> inputs;

    // Let's use a batch size which matches that which we set the
    // Options.optBatchSize option
    size_t batchSize = options.optBatchSize;

    // TODO:
    // For the sake of the demo, we will be feeding the same image to all the
    // inputs You should populate your inputs appropriately.
    for (const auto &inputDim : inputDims) { // For each of the model inputs...
        std::vector<cv::cuda::GpuMat> input;
        for (size_t j = 0; j < batchSize; ++j) { // For each element we want to add to the batch...
            // TODO:
            // You can choose to resize by scaling, adding padding, or a combination
            // of the two in order to maintain the aspect ratio You can use the
            // Engine::resizeKeepAspectRatioPadRightBottom to resize to a square while
            // maintain the aspect ratio (adds padding where necessary to achieve
            // this).
            auto resized = Engine<float>::resizeKeepAspectRatioPadRightBottom(img, inputDim.d[1], inputDim.d[2]);
            // You could also perform a resize operation without maintaining aspect
            // ratio with the use of padding by using the following instead:
            //            cv::cuda::resize(img, resized, cv::Size(inputDim.d[2],
            //            inputDim.d[1])); // TRT dims are (height, width) whereas
            //            OpenCV is (width, height)
            input.emplace_back(std::move(resized));
        }
        inputs.emplace_back(std::move(input));
    }

    // Warm up the network before we begin the benchmark
    spdlog::info("Warming up the network...");
    std::vector<std::vector<std::vector<float>>> featureVectors;
    for (int i = 0; i < 100; ++i) {
        bool succ = engine.runInference(inputs, featureVectors);
        if (!succ) {
            const std::string msg = "Unable to run inference.";
            spdlog::error(msg);
            throw std::runtime_error(msg);
        }
    }

    // Benchmark the inference time
    size_t numIterations = 1;//1000;
    spdlog::info("Running benchmarks ({} iterations)...", numIterations);
    preciseStopwatch stopwatch;
    for (size_t i = 0; i < numIterations; ++i) {
        featureVectors.clear();
        engine.runInference(inputs, featureVectors);
    }
    auto totalElapsedTimeMs = stopwatch.elapsedTime<float, std::chrono::milliseconds>();
    auto avgElapsedTimeMs = totalElapsedTimeMs / numIterations / static_cast<float>(inputs[0].size());

    spdlog::info("Benchmarking complete!");
    spdlog::info("======================");
    spdlog::info("Avg time per sample: ");
    spdlog::info("Avg time per sample: {} ms", avgElapsedTimeMs);
    spdlog::info("Batch size: {}", inputs[0].size());
    spdlog::info("Avg FPS: {} fps", static_cast<int>(1000 / avgElapsedTimeMs));
    spdlog::info("======================\n");

    // Print the feature vectors
    for (size_t batch = 0; batch < featureVectors.size(); ++batch) {
        for (size_t outputNum = 0; outputNum < featureVectors[batch].size(); ++outputNum) {
            spdlog::info("Batch {}, output {}", batch, outputNum);
            std::string output;
            int i = 0;
            for (const auto &e : featureVectors[batch][outputNum]) {
                output += std::to_string(e) + " ";
                if (++i == 10) {
                    output += "...";
                    break;
                }
            }
            spdlog::info("{}", output);
        }
    }

    // TODO: If your model requires post processing (ex. convert feature vector
    // into bounding boxes) then you would do so here.

    return 0;*/
}
