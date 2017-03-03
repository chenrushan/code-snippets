// g++ -o mysql_demo mysql_demo.cpp -lmysqlclient -std=c++11
#include <getopt.h>
#include <mysql/mysql.h>
#include <iostream>
#include <functional>
#include <string.h>
#include <sstream>
#include <vector>
#include <algorithm>

// ======================================================================
// SQLHandler class implementation
// ======================================================================

class SQLHandler {
public:
    using RowHandler = std::function<int(MYSQL_ROW)>;

    SQLHandler(std::string ip, std::string user, std::string passwd,
               std::string db);
    ~SQLHandler();

    int HandleSQL(const std::string& sql, RowHandler rhdl);

private:
    std::string ip_;
    std::string user_;
    std::string passwd_;
    std::string db_;
    MYSQL* mysql_ = nullptr;
};

// ----------------------------------------------------------------------

SQLHandler::SQLHandler(std::string ip, std::string user, std::string passwd,
                       std::string db)
    : ip_(ip), user_(user), passwd_(passwd), db_(db) {
    // init
    mysql_ = mysql_init(nullptr);
    if (mysql_ == nullptr) {
        throw "fail to init mysql";
    }

    // connect
    fprintf(stderr, "connect to mysql with ip[%s] user[%s] db[%s] begin\n",
            ip.data(), user.data(), db.data());
    if (mysql_real_connect(mysql_, ip.data(), user.data(), passwd.data(),
                           db.data(), 0, nullptr, 0) == nullptr) {
        throw "fail to connect to mysql";
    }
}

// ----------------------------------------------------------------------

SQLHandler::~SQLHandler() { mysql_close(mysql_); }

// ----------------------------------------------------------------------

// 处理 sql 查询 @sql，并通过 callback function @rhdl 处理查询结果
int SQLHandler::HandleSQL(const std::string& sql, RowHandler rhdl) {
    // sql-query
    fprintf(stderr, "sql-query [%s] begin\n", sql.data());
    if (mysql_query(mysql_, sql.data()) != 0) {
        fprintf(stderr, "sql-query fail: %s\n", mysql_error(mysql_));
        return -1;
    }

    // store-res
    MYSQL_RES* pres = nullptr;
    if ((pres = mysql_store_result(mysql_)) == nullptr) {
        fprintf(stderr, "get-query-result fail: %s\n", mysql_error(mysql_));
        return -1;
    }

    // get-rows
    auto rows = mysql_num_rows(pres);
    fprintf(stderr, "rows [%llu] fetched\n", rows);

    // handle all records
    for (int i = 0; i < (int)rows; ++i) {
        MYSQL_ROW row = mysql_fetch_row(pres);
        if (rhdl(row) != 0) {
            fprintf(stderr, "fail to handle row\n");
            return -1;
        }
    }
    mysql_free_result(pres);

    return 0;
}

// ======================================================================

struct Record {
    char comp[16];
    char date[16];
    double pre_close;
    double act_pre_close;

    Record(const char* c, const char* d, const char* pc, const char* apc) {
        strcpy(comp, c);
        strcpy(date, d);
        pre_close = atof(pc);
        act_pre_close = atof(apc);
    }

    bool operator<(const Record& that) const {
        auto c = strcmp(comp, that.comp);
        if (c < 0) {
            return true;
        }
        if (c > 0) {
            return false;
        }
        c = strcmp(date, that.date);
        if (c < 0) {
            return true;
        }
        if (c > 0) {
            return false;
        }
        return true;
    }

    std::string ToString() const {
        std::ostringstream os;
        os << comp << " " << date << " " << pre_close << " " << act_pre_close;
        return os.str();
    }
};

// ----------------------------------------------------------------------

int main(int argc, char* argv[]) {
    // get all daily market data
    std::vector<Record> records;
    SQLHandler sqlhdl("192.168.2.6", "work", "work123456", "dingfu_data");
    sqlhdl.HandleSQL(
        "select ticker,tradeDate,preClosePrice,actPreClosePrice from "
        "df_tonglian_rhq",
        [&records](MYSQL_ROW row) {
            records.emplace_back(row[0], row[1], row[2], row[3]);
            return 0;
        });
    std::cout << "get " << records.size() << " records\n";
    if (records.size() == 0) {
        std::cerr << "no records found\n";
        return -1;
    }

    return 0;
}
