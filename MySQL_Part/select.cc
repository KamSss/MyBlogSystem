#include<iostream>
#include<cstdio>
#include<mysql/mysql.h>

int main() {
    //1.初始化句柄
    MYSQL* connect_fd = mysql_init(NULL);
    //2.建立链接
    //  1.mysql_init返回的指针
    //  2.主机地址
    //  3.用户名
    //  4.密码
    //  5.数据库名
    //  6.端口号
    //  7.unix_socket NULL
    //  8.client_flag 0
    if(mysql_real_connect(connect_fd,"127.0.0.1","root","980120","MyBlogDB",3306,NULL,0) == NULL){
        printf("链接失败！%s\n",mysql_error(connect_fd));
        return 1;
    }
    printf("链接成功！\n");
    //3.设置编码格式
    mysql_set_character_set(connect_fd,"utd8");
    //4.拼装sql语句
    char sql[1024 * 4] = {0};
    sprintf(sql,"select * from blog_table");
    //5.执行SQL语句
    int ret = mysql_query(connect_fd,sql);
    if(ret < 0){
        printf("执行sql失败！%s \n",mysql_error(connect_fd));
        return 1;
    }
    //6.遍历查询结果：
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
    //7.释放结果集
    mysql_free_result(result);
    //8.关闭句柄
    mysql_close(connect_fd);
    printf("执行成功！\n");
    return 0;
}