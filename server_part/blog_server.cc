#include "httplib.h"
#include "db.hpp"
#include "signal.h"
#include <iostream>

MYSQL* mysql = NULL;

int main(){
  using namespace httplib;
  using namespace blog_system;
  //1.先和数据库建立好连接
  mysql = MySQLInit();
  //我们平常按ctrl+c退出的本质是SIGINT信号
  //这里传一个回调函数 用Lambda表示
  signal(SIGINT,[](int){ MySQLRelease(mysql); exit(0);});
  //2.创建相关数据库处理对象
  BlogTable blog_table(mysql);
  TagTable tag_table(mysql);

  //3.创建服务器，并设置路由 (这里指HTTP 服务器中的路由，和IP协议的路由不一样)，此处的路由指的是把方法 + path => 哪个处理函数 关联关系声明清楚
  Server server;

  //为什么用Lambda：通过成员函数.的方式就可以关联到方法如post 写成Lambda的方式就不用单独定义函数了，往里一写就行 在[]用捕捉方式捕捉到变量
  //新增博客
  server.Post("/blog",[&blog_table](const Request& req,Response& resp) { //R是原始字符串 转义不生效
    printf("新增博客!\n");
    //1.获取到请求中的 body 并解析成 json
    Json::Reader reader;//把字符串转成json对象用reader
    Json::FastWriter writer;
    Json::Value req_json;//把http版本请求转换成json版本（因为不关注除了json之外的协议内容 只要json）
    Json::Value resp_json;
    
    bool ret = reader.parse(req.body,req_json);
    if(!ret){
       //解析出错了 给用户返回一个发过来的数据是啥样的
       printf("解析请求失败！%s\n",req.body.c_str());
       //构造一个响应对象，告诉客户端出错
       resp_json["ok"] = false;
       resp_json["reason"] = "Parse false!";
       resp.status = 400;
       resp.set_content(writer.write(resp_json),"application/json");
       
       return;
    }

     //2.到这里说明发过来的是一个json 但还不知道是不是我要的json 所以接下来对参数进行校验
    if(req_json["title"].empty()
    || req_json["content"].empty()
    || req_json["tag_id"].empty()
    || req_json["create_time"].empty())
    {
       printf("请求数据格式错了 %s\n",req.body.c_str());
       //构造一个响应对象，告诉我们客户端出错了
       resp_json["ok"] = false;
       resp_json["reason"] = "input data format error!";
       //请求格式错了 状态码返回400
       resp.status = 400;
       resp.set_content(writer.write(resp_json),"application/json");//该函数要设置类型 json对应类型是application/json
       return;
    }

    //3.真正开始调用 MySQL 接口来操作
    ret = blog_table.Insert(req_json); //只有这一句在处理sql语句 其他部分全都是在为了错误校验 这是为了程序健壮性
    if(!ret){
        printf("博客插入失败！\n");
        resp_json["ok"] = false;
        resp_json["reason"] = "blog insert error!";
        //未预料到的错误 500
        resp.status = 500;
        resp.set_content(writer.write(resp_json),"application/json");
        return;
    }

    //4.构造一个正确的响应给客户端即可
    printf("博客插入成功！\n");
    resp_json["ok"] = true;
    resp.set_content(writer.write(resp_json),"application/json");
    return;
  });

  //查看所有博客列表
  server.Get("/blog",[&blog_table](const Request& req,Response& resp){
    printf("查看所有博客\n");
    //1.尝试获取tag_id 如果这个参数不存在 返回空字符串
    const std::string& tag_id = req.get_param_value("tag_id");
    //就不用解析请求了，也就不需要合法性判定了
    //2.调用数据库操作来获取所有博客结果
    Json::Value resp_json;
    Json::FastWriter writer;
    bool ret = blog_table.SelectALL(&resp_json,tag_id);
    if(!ret){
        resp_json["ok"] = false;
        resp_json["reason"] = "select all failed";
        resp.status = 500;
        resp.set_content(writer.write(resp_json),"application/json");
        return;
    }
    //3.构造响应结果就行了
    resp.set_content(writer.write(resp_json),"application/json");
  });

  //查看某个博客
  server.Get(R"(/blog/(\d+))",[&blog_table](const Request& req,Response& resp){
    //1.解析过去到blog_id
    int32_t blog_id = std::stoi(req.matches[1].str());
    printf("查看 id 为 %d 的博客!\n",blog_id);

    //2.直接调用数据库操作
    Json::Value resp_json;
    Json::FastWriter writer;
    bool ret = blog_table.SelectOne(blog_id,&resp_json);
    if(!ret){
        resp_json["ok"] = false;
        resp_json["reason"] = "查看指定博客失败：" + std::to_string(blog_id);
        resp.status = 404;
        resp.set_content(writer.write(resp_json),"application/json");
    }

    //3.包装一个执行正确的响应
    resp.set_content(writer.write(resp_json),"application/json");
    return;
 });

  //修改某个博客
  server.Put(R"(/blog/(\d+))",[&blog_table](const Request& req, Response& resp){
    //先获取到 博客id
    int32_t blog_id = std::stoi(req.matches[1].str());
    printf("修改 id 为 %d 的博客!\n",blog_id);
    //1.获取到请求中的 body 并解析成 json
    Json::Reader reader;
    Json::FastWriter writer;
    Json::Value req_json;
    Json::Value resp_json;
    bool ret = reader.parse(req.body,req_json);
    if(!ret){
      resp_json["ok"] = false;
      resp_json["reason"] = "修改指定博客失败：";
      resp.status = 400;
      resp.set_content(writer.write(resp_json),"application/json");
      return;
    }
    //3.校验参数是否符合预期
    if(req_json["title"].empty()
     || req_json["content"].empty()
     || req_json["tag_id"].empty()
     || req_json["create_time"].empty()){
     resp.set_content(writer.write(resp_json),"application/json");
     return;
    }
    //4.调用数据库操作来完成更新博客操作
    req_json["blog_id"] = blog_id;//从path中的带的 id 设置到json对象中
    ret = blog_table.Update(req_json);
    if(!ret){
      resp_json["ok"] = false;
      resp_json["reason"] = "修改指定博客失败";
      resp.status = 500;
      resp.set_content(writer.write(resp_json),"application/json");
      return;
    }

    //5.构造一个正确的返回结果
    resp_json["ok"] = true;
    resp.set_content(writer.write(resp_json),"application/json");
    return;
 });
//
//  //删除标签
//  server.Delete(R"(/blog/(\d+))",[&tag_table](const Request& req,Response& resp){
//    //1.获取到 博客id
//    int32_t blog_id = std::stoi(req.matches[1].str());
//    printf("删除 id 为 %d 的博客!\n",blog_id);
//    //2.调用数据库操作
//    bool ret = blog_table.Delete(blog_id);
//    if(!ret){
//        resp_json["ok"] = false;
//        resp_json["reason"] = "删除指定博客失败";
//        resp.status = 500;
//        resp.set_content(writer.write(resp_json),"application/json");
//        return;
//    }
//    //5.构造一个正确的返回结果
//    resp_json["ok"] = true;
//    resp.set_content(writer.write(resp_json),"application/json");
//    return;
//  });
//
//  //新增标签
//  server.Post("/tag",[&tag_table]{const Request& req,})Response& resp){
//      Json::Reader reader;
//      Json::FastWriter writer;
//      Json::Value req_json;
//      Json::Value resp_json;
//      //1.解析请求
//  });
//
//  //删除标签
//  server.Delete(R"(/tag/(\d+))",[&tag_table](const Request& req,Response& resp)){
//
//  });

  server.set_base_dir("./wwwroot");
  printf("开始监听！\n");
  server.listen("0.0.0.0",9093);
  return 0;
}