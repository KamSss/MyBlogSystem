.PHONY:all
all:insert select

insert:insert.cc
	g++ -o $@ $^ -L/usr/lib64/mysql -lmysqlclient -g

select:select.cc
	g++ -o $@ $^ -L/usr/lib64/mysql -lmysqlclient -g

.PHONY:clean
clean:
	rm insert select