#  在线博客系统
##  1.项目介绍：
  一个网页版的单用户博客系统，支持在线对博客的增加、删除、修改、查找，对博客标签的增加、删除、修改功能。
##  2.开发环境：
  在Linux CentOS7下使用C++开发，使用g++/gdb开发调试，然后用Makefile管理。（不得不说拿VSCode的远程连接阿里云真好用，比在Xshell下手写vim要方便易调试一些）。
##  3.模块划分：
1.客户端：一组HTML网页，使用Vue.js进行动态交互，使用ajax获取服务器数据。
2.服务器端：使用基于cpp-httplib第三方库（这个库可以在我的star里找），搭建的 HTTP服务器。
3.MySQL数据库：以Json作为参数，将MySQL API封装成操作数据库表的类。
**总体流程：** 用户在网页上点击某个操作的时候，比如新增博客，此时就会由客户端给服务器发送 HTTP 请求，请求包含了用户的行为，服务器根据这个行为对数据库进行操作。
![在这里插入图片描述](https://img-blog.csdnimg.cn/20190908094732544.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQyNjg1NTg4,size_16,color_FFFFFF,t_70)
### ①MySQL数据库模块：
#### 1.建库建表：
需要一张博客表，一张标签表，和一张用户表。

![在这里插入图片描述](https://img-blog.csdnimg.cn/20190908095931805.png)

![在这里插入图片描述](https://img-blog.csdnimg.cn/20190908100501321.png)

**在分布式系统下生成唯一主键出现的问题：**
主键是自增长类型，在单机上没事，但是分布式系统上，这么设计就会出问题：因为自增长可能会给不同博客分配一样的ID。
**解决办法：**
1.使用时间戳，用时间戳区分不同博客，但仍然有出错可能（时间戳恰好一致）
2.使用机房ip
3.使用主机ip
4.使用随机数
5.加锁，虽然保证了强一致性，但是加锁对效率影响实在太大了，所以不用
#### 2.练习使用MySQL C API操作数据库
主要使用mysql_real_connect()建立连接，[官方文档点这里](https://dev.mysql.com/doc/refman/8.0/en/mysql-real-connect.html)，函数参数包括：
  1.mysql_init返回的指针，链接句柄
  2.主机ip地址
  3.用户名
  4.密码
  5.数据库名
  6.主机端口号：默认3306
  7.unix_socket 预套接字 默认NULL
  8.client_flag 默认0

```/
1.初始化句柄：
	MYSQL* connect_fd = mysql_init(NULL);

2.建立连接：
    if(mysql_real_connect(connect_fd,"127.0.0.1","root","980120","MyBlogDB",3306,NULL,0) == NULL){
        printf("链接失败！%s\n",mysql_error(connect_fd));
        return 1;
    }
    printf("链接成功！\n");
    
3..设置编码格式
    mysql_set_character_set(connect_fd,"utd8")
    
4.拼装sql语句
    char sql[1024 * 4] = {0};
    sprintf(sql,"select * from blog_table");
    
5.让数据库执行SQL语句
    int ret = mysql_query(connect_fd,sql);
    if(ret < 0){
        printf("执行sql失败！%s \n",mysql_error(connect_fd));
        return 1;
    }
    
6.遍历查询结果：
    MYSQL_RES* result = mysql_store_result(connect_fd);
    if(result == NULL){
        printf("获取结果失败！%s \n",mysql_error(connect_fd));
        return 1;
    }
    //a)获取行数和列数
    int rows = mysql_num_rows(result);
    int fields = mysql_num_fields(result);
    printf("rows: %d,fields: %d\n",rows,fields);
    //b)打印结果
    for(int i = 0;i < rows;i++){
        MYSQL_ROW row = mysql_fetch_row(result);
        for(int j = 0;j < fields;j++){
            printf("%s\t",row[j]);
        }
        printf("\n");
    }
    
7.释放结果集
    mysql_free_result(result);
    
8.关闭句柄
    mysql_close(connect_fd);
    printf("执行成功！\n");
    return 0;
```
### 1.sql注入攻击
1.对参数进行校验，比如含有select和delete之类的参数，就及时排除。
2.ORM框架，用一个对象和数据库的一张表关联起来 操作对象就可以修改数据库的表 就不用直接拼接操作sql语句 也就避免了sql注入攻击（该框架依赖 反射/自省 语法机制 但是C++在这两个语法几乎没啥办法）
### 2.博客内容搜索你有什么想法
1.简单的用like
2.复杂的用倒排索引：现代搜索引擎基本都是倒排索引
又称反向索引，记录了搜索的单词在一个文件或者多个文件下的存储位置的映射。这样搜索单词可以快速锁定相关文件的位置。
### 3.你的博客实现了什么功能
1.博客/标签列表显示
2.博客/标签正文的显示
3.以markdown格式在线分屏修改博客内容
### 4.MD5的作用？
主要是校验，比如密码加密存储、文件完整性校验
### 5.你博客后端怎么实现的，你把你博客系统大概讲一下
服务器端：使用基于cpp-httplib第三方库，搭建的 HTTP服务器。 
### 6.你的博客正文存在表里 那正文一般很长 效率问题考虑过吗
最长就几十K，不存在影响
