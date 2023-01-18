#include <iostream>
#include <mysql/mysql.h>
#include <cassert>
#include <string.h>

// mysql api
// https://dev.mysql.com/doc/c-api/5.7/en/c-api-function-reference.html
using namespace std;

static void test1(MYSQL* con) {
    MYSQL_RES* res = nullptr;
    int num = 0;
    unsigned int num_fields;
    MYSQL_FIELD *fields = nullptr;
    int num_row = 0;
    MYSQL_ROW_OFFSET offset;
    MYSQL_ROW row;
    unsigned long* len;

    if(mysql_query(con, "CREATE DATABASE IF NOT EXISTS test")) {
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
}

#define MAX_TEST_QUERY_LENGTH 300 /* MAX QUERY BUFFER LENGTH */
#define myheader(str)
#define myerror(...)    printf("Query err %d,%s \r\n", mysql_errno(mysql), mysql_error(mysql))
#define DIE_UNLESS(exp) assert(exp);
#define myquery(RES)				\
{						\
 int r= (RES);					\
 if (r)						\
 myerror(NULL);					\
 DIE_UNLESS(r == 0);				\
}

#define check_stmt(stmt)			\
{						\
 if ( stmt == 0)				\
 myerror(NULL);					\
 DIE_UNLESS(stmt != 0);				\
}

char *my_stpmov(char *dst, const char *src)
{
    while ((*dst++ = *src++)) ;
    return dst-1;
}

static inline char *my_stpcpy(char *dst, const char *src)
{
    return my_stpmov(dst, src);
}

static MYSQL_STMT *STDCALL
mysql_simple_prepare(MYSQL *mysql_arg, const char *query)
{
 MYSQL_STMT *stmt= mysql_stmt_init(mysql_arg);
 if (stmt && mysql_stmt_prepare(stmt, query, (ulong)strlen(query)))
 {
   mysql_stmt_close(stmt);
   return 0;
 }
 return stmt;
}
#define mysterror(...)
#define check_execute(stmt, r)			\
{						\
 if (r)						\
 mysterror(stmt, NULL);				\
 DIE_UNLESS(r == 0);				\
}

static void verify_param_count(MYSQL_STMT *stmt, long exp_count)
{
 long param_count= mysql_stmt_param_count(stmt);
 DIE_UNLESS(param_count == exp_count);
}

static void test2(MYSQL* mysql) {
    MYSQL_STMT *stmt;
    int        rc;
    char query[MAX_TEST_QUERY_LENGTH];

    myheader("test_prepare_simple");

    rc= mysql_query(mysql, "DROP TABLE IF EXISTS test_prepare_simple");
    myquery(rc);

    rc= mysql_query(mysql, "CREATE TABLE test_prepare_simple("
                            "id int, name varchar(50))");
    myquery(rc);

    /* insert */
    my_stpcpy(query, "INSERT INTO test_prepare_simple VALUES(?, ?)");
    stmt= mysql_simple_prepare(mysql, query);
    check_stmt(stmt);

    verify_param_count(stmt, 2);
    mysql_stmt_close(stmt);

    /* update */
    my_stpcpy(query, "UPDATE test_prepare_simple SET id=? "
                    "WHERE id=? AND CONVERT(name USING utf8)= ?");
    stmt= mysql_simple_prepare(mysql, query);
    check_stmt(stmt);

    verify_param_count(stmt, 3);
    mysql_stmt_close(stmt);

    /* delete */
    my_stpcpy(query, "DELETE FROM test_prepare_simple WHERE id=10");
    stmt= mysql_simple_prepare(mysql, query);
    check_stmt(stmt);

    verify_param_count(stmt, 0);

    rc= mysql_stmt_execute(stmt);
    check_execute(stmt, rc);
    mysql_stmt_close(stmt);

    /* delete */
    my_stpcpy(query, "DELETE FROM test_prepare_simple WHERE id=?");
    stmt= mysql_simple_prepare(mysql, query);
    check_stmt(stmt);

    verify_param_count(stmt, 1);

    mysql_stmt_close(stmt);

    /* select */
    my_stpcpy(query, "SELECT * FROM test_prepare_simple WHERE id=? "
                    "AND CONVERT(name USING utf8)= ?");
    stmt= mysql_simple_prepare(mysql, query);
    check_stmt(stmt);

    verify_param_count(stmt, 2);

    mysql_stmt_close(stmt);

    /* now fetch the results ..*/
    rc= mysql_commit(mysql);
}

int main(int argc, char** argv) {
    mysql_library_init(0, nullptr, nullptr);
    
    auto* con = mysql_init(nullptr);
    if(!mysql_real_connect(con, "localhost", "cbookshu", "cs123456", "", 3306, nullptr, 0)) {
        cout << "error" << mysql_error(con) << endl;
        exit(1);
    }
    if(mysql_query(con, "USE test")) {
        cout << "error" << mysql_error(con) << endl;
        exit(1);
    }

    test1(con);

    test2(con);

    mysql_close(con);
    mysql_library_end();
    return 0;
}
