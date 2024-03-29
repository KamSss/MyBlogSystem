-- 把SQL语句写到文件中，方便批量使用和后期对表进行修改
-- 如何把SQL文件导入到MySQL:重定向
--  >标准输出重定向 2>标准错误重定向 <标准输入重定向
-- mysql -uroot -p < db.sql

-- 分布式系统下生成唯一主键会出现的问题：
--       主键是自增类型，如果这个数据库是分布式的不是单机的，这么设计就会出问题：自增长会给不同博客分配一样的ID
-- 解决办法：
-- 1.使用时间戳 2.使用机房id 3.使用主机ip 4.使用随机数 5.加锁（不推荐，因为加锁会大大降低效率，为了效率牺牲了数据的强一致性）
create database MyBlogDB;
use MyBlogDB;

drop table if exists blog_table;
create table blog_table(
    blog_id int not null primary key auto_increment comment '博客id',
    title varchar(50) comment '标题',
    content text comment '正文',
    tag_id int comment '标签id',
    create_time varchar(50) comment '创建时间'
);

-- drop table if exists tag_table;
create table tag_table(
    -- 为了方便这里设计成一篇博客只能属于一个标签
    tag_id int not null primary key auto_increment comment '标签id',
    tag_name varchar(50) comment '标签内容'
);

create table if not exists user_table(
	user_id int primary key auto_increment comment '用户id',
	user_name varchar(32) unique comment '用户名',
	user_password varchar(32) comment '用户密码'
	);
