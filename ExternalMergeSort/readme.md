objectives

1. single-threaded EMS vs. multi-threaded EMS in execution time
2. single-threaded EMS vs. multi-threaded EMS in read/write pattern
3. cause analysis


//In sqlite3 shell

create table test (a real, b real, c real, d real, e real, f real, g real, h real, i real, j real, k real, l real m real);
.mode csv
.import testdata.csv test

<for checking execution time>
.timer on

<Thread setting>
PRAGMA threads = 0; //single thread
PRAGMA threads = 8; //multi threads
	
<set the number of pages of cache>
PRAGMA cache_size = 1000;

<not to print the result of query on console>
.output nul 		//in windows
.output /dev/null	//in linux

<test query>
select * from test order by a;
