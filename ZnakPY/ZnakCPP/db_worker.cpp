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
        "tg_username TEXT NOT NULL,"
        "first_name TEXT NOT NULL,"
        "last_name TEXT NOT NULL,"
        "number INTEGER NOT NULL,"
        "in_bot_name TEXT NOT NULL,"
        "sex TEXT NOT NULL,"
        "years INTEGER NOT NULL,"
        "unic_your_id TEXT NOT NULL,"
        "unic_wanted_id TEXT NOT NULL,"
        "status BOOLEAN NOT NULL);";

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

    // Добавляем этот блок ДО вызова init_db()
    if (request["action"] == "init_db") {
        init_db(db); // Создаем таблицы
        response["status"] = "db_created";
        sqlite3_close(db);
        return response.dump();
    }

    init_db(db);
    std::string action = request["action"];

    if (action == "add_user") {
        const char* sql = "INSERT OR IGNORE INTO UsersInfo ("
                         "tg_id, tg_username, first_name, last_name, number, "
                         "in_bot_name, sex, years, unic_your_id, unic_wanted_id, status"
                         ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            // Обязательные поля
            sqlite3_bind_int(stmt, 1, request["tg_id"].get<int>());
            sqlite3_bind_text(stmt, 2, request["username"].get<std::string>().c_str(), -1, SQLITE_STATIC);

            // Необязательные поля
            auto bind_optional = [&request](sqlite3_stmt* stmt, int idx, const char* field) {
                if (request.contains(field)) {
                    sqlite3_bind_text(stmt, idx, request[field].get<std::string>().c_str(), -1, SQLITE_STATIC);
                } else {
                    sqlite3_bind_null(stmt, idx);
                }
            };

            bind_optional(stmt, 3, "first_name");
            bind_optional(stmt, 4, "last_name");
            sqlite3_bind_int(stmt, 5, request.value("number", 0));
            bind_optional(stmt, 6, "in_bot_name");
            bind_optional(stmt, 7, "sex");
            sqlite3_bind_int(stmt, 8, request.value("years", 0));
            bind_optional(stmt, 9, "unic_your_id");
            bind_optional(stmt, 10, "unic_wanted_id");
            sqlite3_bind_int(stmt, 11, request.value("status", 1));

            if (sqlite3_step(stmt) == SQLITE_DONE) {
                response["status"] = sqlite3_changes(db) > 0 ? "created" : "exists";
            }
            sqlite3_finalize(stmt);
        }
    }

    sqlite3_close(db);
    return response.dump();
}

int main(int argc, char* argv[]) {
    if (argc < 2) return 1;
    std::cout << handle_request(argv[1]);
    return 0;
}