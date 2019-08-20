#include<cstdio>
#include<cstdlib>
#include<mysql/mysql.h>
#include<jsoncpp/json/json.h>
#include<memory>

namespace blog_system{
static MYSQL* MySQLInit(){
  //初始化一个 MYSQL 句柄并建立连接
  //1.创建一个句柄
  MYSQL* connect_fd = mysql_init(NULL);
  //2.和数据库建立链接
  if(mysql_real_connect(connect_fd,"127.0.0.1","root","980120","MyBlogDB",3306,NULL,0) == NULL){
	  printf("链接失败！%s\n",mysql_error(connect_fd));
	  return NULL;
	  }
	  //3.设定字符编码
	  mysql_set_character_set(connect_fd,"utf8");
      return connect_fd;
	  }
	  //释放句柄断开连接
	  static void MySQLRelease(MYSQL* connect_fd){
		  mysql_close(connect_fd);
		  }
//创建一个类，用于操作博客表的类
class BlogTable{
public:
	//通过这个构造函数获取到一个数据库的操作句柄
	BlogTable(MYSQL* connect_fd) : mysql_(connect_fd){
	}
	//以下操作相关参数都统一使用 JSON 的方式
	//Json:value jsoncpp 中最核心的类
	//Json:Value 就表示一个具体的 JSON 对象
	//形如：
	//    {   
	//		  blog_id:1,
	//        title:"我的第一篇博客",
	//     	  create_time:"2019/07/28",
	//        tag_id:1
	//    }
	//最大的好处是方便扩展
	bool Insert(const Json::Value& blog){
        const std::string& content = blog["content"].asString();
        //为啥 to 的缓冲区长度是 2*size+1 这是官方文档的要求
        //为啥不用new动态开辟空间，而是用智能指针？因为new每次都要delete，万一忘记del就会有问题。
        std::unique_ptr<char> to(new char[content.size() * 2 + 1]);
        mysql_real_escape_string(mysql_,to.get(),content.c_str(),content.size());
		//正文都要动态生成，那sql语句的长度就更不确定了，所以也动态生成
        std::unique_ptr<char> sql(new char[content.size() * 2 + 4096]);//把 4095 留给标题、tag_id、create_time
		//拼 sql 语句
        sprintf(sql.get(),"insert into blog_table values(null,'%s','%s','%d','%s')",
            blog["title"].asCString(),
            to.get(),
            blog["tag_id"].asInt(),
            blog["create_time"].asCString());
        int ret = mysql_query(mysql_,sql.get());
        if(ret != 0){
          printf("执行插入博客失败！%s\n",mysql_error(mysql_));
		  return false;
        }
		printf("执行插入博客成功！\n");
		return true;
	}
			
	//blogs 作为一个输出型参数
	bool SelectALL(Json::Value* blogs,const std::string& tag_id = ""){
		//查找不需要太长的 sql ，固定长度就行了
		char sql[1024 * 4] = {0};
		if(tag_id == ""){
			//此时不需要按照 tag 来筛选结果
			sprintf(sql,"select blog_id,title,tag_id,create_time from blog_table");
		}else{
			//此时需要按 tag 来筛选
			sprintf(sql,"select blog_id,title,tag_id,create_time from blog_table where tag_id = %d",std::stoi(tag_id));
		}
		int ret = mysql_query(mysql_,sql);
		if(ret != 0){
			printf("执行查找所有博客失败！%s\n",mysql_error(mysql_));
			return false;
		}
		//到这里 已经把select执行成功了 下面开始遍历查询结果
		MYSQL_RES* result = mysql_store_result(mysql_);
		int rows = mysql_num_rows(result);
		//遍历结果集合，然后把结果写到 blogs 参数中，返回给调用者（输出型参数blogs）
		for(int i = 0;i < rows;++i){
			MYSQL_ROW row = mysql_fetch_row(result);
			Json::Value blog;
			//row方括号中的下标 和上面select语句中写的列的顺序是对应的
			blog["blog_id"] = atoi(row[0]);
			blog["title"] = row[1];
			blog["tag_id"] = atoi(row[2]);
			blog["create_time"] = row[3];
			//append之后 blogs这个json对象就包含了数组，数组每一个下标就是查询内容
			blogs->append(blog);
		}
		//mysql查询的结果集合需要记得及时释放
		mysql_free_result(result);
		printf("执行查找所有博客成功！共查找到 %d 条博客\n",rows);
		return true;
	}
			
	//blog 同样是输出型参数，根据当前的 blog_id 在数据库中找到具体的博客内容通过 blog 参数返回给调用者
	bool SelectOne(int32_t blog_id,Json::Value* blog){
		char sql[1024] = {0};
		sprintf(sql,"select blog_id,title,content,tag_id,create_time from blog_table where blog_id = %d",blog_id);
		int ret = mysql_query(mysql_,sql);
		if(ret != 0){
			printf("执行查找博客失败 %s\n",mysql_error(mysql_));
			return false;
		}
		MYSQL_RES* result = mysql_store_result(mysql_);
		int rows = mysql_num_rows(result);
		if(rows != 1){
			printf("查询错误！查找到的博客不唯一！有 %d 条\n", rows);
			return false;
		}
		MYSQL_ROW row = mysql_fetch_row(result);
		(*blog)["blog_id"] = atoi(row[0]);
		(*blog)["title"] = row[1];
		(*blog)["content"] = row[2];
		(*blog)["tag_id"] = atoi(row[3]);
		(*blog)["create_time"] = row[4];
		return true;
	}
			
	//数据库修改\更新操作
	bool Update(const Json::Value& blog){
		const std::string& content = blog["content"].asString();
        //为啥 to 的缓冲区长度是 2*size+1 这是官方文档的要求
        //为啥不用new动态开辟空间，而是用智能指针？因为new每次都要delete，万一忘记del就会有问题。
        std::unique_ptr<char> to(new char[content.size() * 2 + 1]);
        mysql_real_escape_string(mysql_,to.get(),content.c_str(),content.size());
		//正文都要动态生成，那sql语句的长度就更不确定了，所以也动态生成
        std::unique_ptr<char> sql(new char[content.size() * 2 + 4096]);//把 4095 留给标题、tag_id、create_time
		//拼 sql 语句
        sprintf(sql.get(),"update blog_table set title = '%s',content = '%s',tag_id = %d where blog_id = %d",
            blog["title"].asCString(),
            to.get(),
            blog["tag_id"].asInt(),
            blog["blog_id"].asInt());
        int ret = mysql_query(mysql_,sql.get());
        if(ret != 0){
          printf("更新博客失败！%s\n",mysql_error(mysql_));
		  return false;
        }
		printf("更新博客成功！\n");
		return true;
	}
			
	bool Delete(int32_t blog_id){
		char sql[1024 * 4] = {0};
		sprintf(sql,"delete from blog_table where blog_id = %d",blog_id);
		int ret = mysql_query(mysql_,sql);
        if(ret != 0){
          printf("删除博客失败！%s\n",mysql_error(mysql_));
		  return false;
        }
		printf("删除博客成功！\n");
		return true;
	}
private:
	MYSQL* mysql_;
};

class TagTable{
public:
	TagTable(MYSQL* mysql) : mysql_(mysql){}
		
	bool Insert(const Json::Value& tag){
		char sql[1024 * 4];
		sprintf(sql,"insert into tag_table values(null,'%s')",tag["tag_name"].asCString());
		int ret = mysql_query(mysql_,sql);
		if(ret != 0){
			printf("插入标签失败！%s\n", mysql_error(mysql_));
			return false;
		}
		printf("插入标签成功！\n");
		return true;
	}
		
	bool Delete(int32_t tag_id){
		char sql[1024 * 4] = {0};
		sprintf(sql,"delete from tag_table where tag_id = %d",tag_id);
		int ret = mysql_query(mysql_,sql);
		if(ret != 0){
			printf("删除标签失败！%d\n", mysql_error(mysql_));
			return false;
		}
		printf("删除标签成功！\n");
		return true;
	}
		
	bool SelectALL(Json::Value* tags){
		char sql[1024 * 4] = {0};
		sprintf(sql,"select tag_id,tag_name from tag_table");
		int ret = mysql_query(mysql_,sql);
		if(ret != 0){
			printf("查询标签失败！%d\n", mysql_error(mysql_));
			return false;
		}
		MYSQL_RES* result = mysql_store_result(mysql_);
		int rows = mysql_num_rows(result);
		for(int i = 0;i < rows;++i){
			MYSQL_ROW row = mysql_fetch_row(result);
			Json::Value tag;
			tag["tag_id"] = atoi(row[0]);
			tag["tag_name"] = row[1];
			tags->append(tag);
		}
		printf("查询标签成功！共找到 %d 个！\n",rows);
		return true;
	}
private:
	MYSQL* mysql_;
};

}