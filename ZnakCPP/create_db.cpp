#include <iostream>
#include <sqlite3.h>
#include <unistd.h>
#include <libgen.h>
#include <limits.h>
#include <cstring>

int create_database(const std::string& db_path) {
    sqlite3* db;
    char* err_msg = nullptr;

    int rc = sqlite3_open(db_path.c_str(), &db);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
        return rc;
    }

    const char* sql = "CREATE TABLE IF NOT EXISTS UsersInfo ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "tg_id BIGINT UNIQUE NOT NULL,"
                      "tg_username VARCHAR(120) NOT NULL,"
                      "first_name VARCHAR(120),"
                      "last_name VARCHAR(120),"
                      "number INTEGER NOT NULL,"
                      "in_bot_name VARCHAR(120) NOT NULL,"
                      "sex VARCHAR(120) NOT NULL,"
                      "years INTEGER NOT NULL,"
                      "unic_your_id VARCHAR(120) NOT NULL,"
                      "unic_wanted_id VARCHAR(120) NOT NULL,"
                      "status BOOLEAN NOT NULL"
                      ");";

    rc = sqlite3_exec(db, sql, nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << err_msg << std::endl;
        sqlite3_free(err_msg);
    } else {
        std::cout << "Table created successfully." << std::endl;
    }

    sqlite3_close(db);
    return rc;
}

int main(int argc, char* argv[]) {
    char exe_path[PATH_MAX];
    if (realpath(argv[0], exe_path) == nullptr) {
        perror("realpath");
        return 1;
    }

    char* dir = dirname(exe_path);
    std::string db_path = std::string(dir) + "/maindb.sqlite3";

    if (create_database(db_path) == SQLITE_OK) {
        std::cout << "Database created at: " << db_path << std::endl;
    } else {
        std::cout << "Failed to create database." << std::endl;
    }

    return 0;
}
