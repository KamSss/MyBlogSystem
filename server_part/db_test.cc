//此文件是为了测试刚才封装的db.hpp(mysql封装操作)
//这种单个模块测试 叫 单元测试
//gtest谷歌提供的单元测试框架。自己加入项目中！
#include "db.hpp"

void TestBlogTable(){
    //格式化生成json序列化
    //fastwriter更高效 styledwriter更好看
    Json::StyledWriter writer;
    MYSQL* mysql = blog_system::MySQLInit();
    blog_system::BlogTable blog_table(mysql);
    bool ret = false;

    //测试插入：成功！
    Json::Value blog;
    //blog["title"] = "我的第一篇测试博客";
    //blog["content"] = "我要拿30w年薪";
    //blog["tag_id"] = 1;
    //blog["create_time"] = "2019/8/20";
    //bool ret = blog_table.Insert(blog);
    //printf("insert:%d\n", ret);

    //测试查找：成功！
    //Json::Value blogs;
    //ret = blog_table.SelectALL(&blogs);
    //printf("select all %d\n", ret);
    //printf("blogs:%s\n",writer.write(blogs).c_str());

    //测试查找单个博客: 成功！
    //ret = blog_table.SelectOne(1,&blog);
    //printf("select one %d\n", ret);
    //printf("blog: %s\n", writer.write(blog).c_str());

    //测试修改博客：成功！
    //blog["blog_id"] = 1;
    //blog["title"] = "测试一哈，有bug我就哭给你看";
    //blog["content"] = "1.测试一下单引号和换行是否可以正确识别\n创建一个'单引号' 看到这里说明成功啦";
    //ret = blog_table.Update(blog);
    //printf("update %d\n", ret);

    //测试删除博客：成功！
    ret = blog_table.Delete(1);
    printf("delete %d\n", ret);

    blog_system::MySQLRelease(mysql);
}

void TestTagTable(){
    Json::StyledWriter writer;
    MYSQL* mysql = blog_system::MySQLInit();
    blog_system::TagTable tag_table(mysql);

    //测试插入：成功！
    //json::Value tag;
    //tag["tag_name"] = "C语言";
    //bool ret = tag_table.Insert(tag);
    //printf("insert %d\n", ret);

    //测试查找：成功！
    //Json::Value tags;
    //bool ret = tag_table.SelectALL(&tags);
    //printf("select all %d\n", ret);
    //printf("tags: %s",writer.write(tags).c_str());

    //测试删除：成功！
    bool ret = tag_table.Delete(1);
    printf("delete %d\n",ret);
    blog_system::MySQLRelease(mysql);
}
int main() {
    //TestBlogTable();
    TestTagTable();
    return 0;
}