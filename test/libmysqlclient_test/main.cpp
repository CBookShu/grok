#include <iostream>
#include <mysql/mysql.h>

// mysql api
// https://dev.mysql.com/doc/c-api/5.7/en/c-api-function-reference.html
using namespace std;

int main(int argc, char** argv) {
    mysql_library_init(0, nullptr, nullptr);
    MYSQL *con = nullptr;
    MYSQL_RES* res = nullptr;
    int num = 0;
    unsigned int num_fields;
    MYSQL_FIELD *fields = nullptr;
    int num_row = 0;
    MYSQL_ROW_OFFSET offset;
    MYSQL_ROW row;
    unsigned long* len;

    con = mysql_init(nullptr);
    if(!mysql_real_connect(con, "localhost", "cbookshu", "cs123456", "", 3306, nullptr, 0)) {
        cout << "error" << mysql_error(con) << endl;
        goto exist;
    }
    if(mysql_query(con, "CREATE DATABASE IF NOT EXISTS test")) {
        cout << "error" << mysql_error(con) << endl;
        goto exist;
    }
    if(mysql_query(con, "USE test")) {
        cout << "error" << mysql_error(con) << endl;
        goto exist;
    }
    if(mysql_query(con,
    "CREATE TABLE IF NOT EXISTS `testtable` (\
        `f1` int(11) DEFAULT NULL,\
        `f2` varchar(255) DEFAULT NULL,\
        `f3` datetime DEFAULT NULL ON UPDATE CURRENT_TIMESTAMP\
        ) ENGINE=InnoDB DEFAULT CHARSET=latin1;")) {
        cout << "error" << mysql_error(con) << endl;
        goto exist;
    }
    if(mysql_query(con,"TRUNCATE testtable")) {
        cout << "error" << mysql_error(con) << endl;
        goto exist;
    }
    if(mysql_query(con, "INSERT INTO testtable VALUES(1, 'test')")) {
        cout << "error" << mysql_error(con) << endl;
        goto exist;
    }

    if(mysql_query(con, "SELECT * FROM testtable")) {
        cout << "error" << mysql_error(con) << endl;
        goto exist;
    } 
    res = mysql_store_result(con);
    if(!res) {
        cout << "error" << mysql_error(con) << endl;
        goto exist;
    }
    // header
    num_fields= mysql_num_fields(res);
    fields= mysql_fetch_fields(res);
    for (int i = 0; i < num_fields; ++i) {
        cout << fields[i].name << endl;
    }
    // data
    num_row = mysql_num_rows(res);
    mysql_data_seek(res, 0);
    for (int i = 0; i < num_row; ++i) {
        row = mysql_fetch_row(res);
        len = mysql_fetch_lengths(res);
        for(int j = 0; j < num_fields; ++j) {
            std::string s(row[j], len[j]);
            printf("%s,%d[%s]\r\n", fields[j].name, fields[j].type,row[j]);
        }
    }

exist:
    if(res) {mysql_free_result(res);}
    if(con) {mysql_close(con);}
    mysql_library_end();
    return 0;
}
