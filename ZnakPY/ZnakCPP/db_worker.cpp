#include <iostream>
#include <sqlite3.h>
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

void init_db(sqlite3* db) {
    const char* sql =
        "CREATE TABLE IF NOT EXISTS UsersInfo ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "tg_id BIGINT UNIQUE NOT NULL,"
        "tg_username VARCHAR(120) NOT NULL);";
    sqlite3_exec(db, sql, nullptr, nullptr, nullptr);
}

std::string handle_request(const std::string& json_str) {
    json request = json::parse(json_str);
    json response;
    sqlite3* db;

    if (sqlite3_open(request["db_path"].get<std::string>().c_str(), &db) != SQLITE_OK) {
        response["error"] = "Cannot open database";
        return response.dump();
    }

    init_db(db);

    try {
        std::string action = request["action"];

        if (action == "add_user") {
            const char* sql = "INSERT OR REPLACE INTO UsersInfo (tg_id, tg_username) VALUES (?, ?)";
            sqlite3_stmt* stmt;
            if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_int(stmt, 1, request["tg_id"].get<int>());
                sqlite3_bind_text(stmt, 2, request["username"].get<std::string>().c_str(), -1, SQLITE_STATIC);

                if (sqlite3_step(stmt) == SQLITE_DONE) {
                    response["status"] = "success";
                }
                sqlite3_finalize(stmt);
            }
        }
    } catch (...) {
        response["error"] = "Invalid request";
    }

    sqlite3_close(db);
    return response.dump();
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <json_request>" << std::endl;
        return 1;
    }
    std::cout << handle_request(argv[1]);
    return 0;
}