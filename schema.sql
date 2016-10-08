 drop table if exists device_second_level_data;
create table device_second_level_data(
	id integer PRIMARY KEY AUTOINCREMENT,
	length integer,
	date timestamp default (strftime('%Y.%m.%d %H:%M:%f','now','localtime')),
	data blob
);
 drop table if exists device_running_statement;
create table device_running_statement(
	id integer PRIMARY KEY AUTOINCREMENT,
	date timestamp default (strftime('%Y.%m.%d %H:%M:%f','now','localtime')),
	statement text
);

 drop table if exists device_alarm;
create table device_alarm(
	id integer PRIMARY KEY AUTOINCREMENT,
	date timestamp default (strftime('%Y.%m.%d %H:%M:%f','now','localtime')),
	length integer,
	data blob
);

