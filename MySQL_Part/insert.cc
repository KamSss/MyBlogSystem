//通过这个程序测试一下 MYSQL API 
//实现数据的插入功能
//注意事项：
//  1.在 mysql 官网搜索文档：mysql_real_connect
//  2.mysql 默认端口号3306 是可以配置的
#include<iostream>
#include<cstdio>
#include<cstdlib>
//编译器默认从 /usr/include 目录中查找头文件，mysql.h
//是在一个mysql的目录中
#include<mysql/mysql.h>

int main(){
    //1.创建一个数据库连接句柄
    MYSQL* connect_fd = mysql_init(NULL);
    //2.和数据库建立连接//和 TCP 三次握手有联系，但要区分开,这是在应用层建立链接。
    //     链接过程中需要指定一些必要的消息
    //          1.链接句柄
    //          2.服务器的ip地址
    //          3.用户名
    //          4.密码
    //          5.数据库名（MyBlogDB）
    //          6.服务器的端口号
    //          7.unix_sock NULL 预套接字 
    //          8.client_flag 0
    //==NULL代表数据库连接失败 数据库连接失败是很常见的事 
   if(mysql_real_connect(connect_fd,"127.0.0.1","root","980120","MyBlogDB",3306,NULL,0) == NULL)
   {
    printf("链接失败！%s\n",mysql_error(connect_fd));
    return 1;
    }
    printf("连接成功！\n");
   //3.设置编码方式
   //mysql server 部分最初安装的时候已经设成了 utf8
   //也得在客户端设成 utf8
   mysql_set_character_set(connect_fd,"utf8");
   //4.拼接 SQL 语句
   char sql[1024*4] = {0};
   char title[] = "立一个flag";
   char content[] = "我要当程序员";
   int tag_id = 1;
   char date[] = "2019/07/25";
   sprintf(sql,"insert into blog_table value(null,'%s','%s',%d,'%s')",title,content,tag_id,date);
   printf("sql: %s\n",sql);
   
   //5.让 数据库 服务器 执行 sql
   int ret = mysql_query(connect_fd,sql);
   if(ret < 0){
       printf("执行 sql 失败！%s\n",mysql_error(connect_fd));
       mysql_close(connect_fd);
   }
   printf("插入成功！\n");
   //断开连接
   mysql_close(connect_fd);
   return 0;
}