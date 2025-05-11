// #include <iostream>
// #include <sqlite3.h>
// #include <nlohmann/json.hpp>
// #include <string>
//
// using json = nlohmann::json;
//
// void init_db(sqlite3* db) {
//     const char* sql =
//         "CREATE TABLE IF NOT EXISTS UsersInfo ("
//         "id INTEGER PRIMARY KEY AUTOINCREMENT,"
//         "tg_id BIGINT UNIQUE NOT NULL,"
//         "tg_username TEXT NOT NULL,"
//         "first_name TEXT NOT NULL,"
//         "last_name TEXT NOT NULL,"
//         "number TEXT NOT NULL,"
//         "in_bot_name TEXT NOT NULL,"
//         "sex TEXT NOT NULL,"
//         "years INTEGER NOT NULL,"
//         "unic_your_id TEXT NOT NULL,"
//         "unic_wanted_id TEXT NOT NULL,"
//         "status BOOLEAN NOT NULL);";
//
//     sqlite3_exec(db, sql, nullptr, nullptr, nullptr);
// }
//
// std::string handle_request(const std::string& json_str) {
//     json request = json::parse(json_str);
//     json response;
//     sqlite3* db;
//
//
//
//     if (sqlite3_open(request["db_path"].get<std::string>().c_str(), &db) != SQLITE_OK) {
//         response["error"] = "Cannot open database";
//         return response.dump();
//     }
//
//     if (request["action"] == "init_db") {
//         init_db(db);
//         response["status"] = "db_created";
//         sqlite3_close(db);
//         return response.dump();
//     }
//
//     init_db(db);
//     std::string action = request["action"];
//
//     if (action == "add_user") {
//         const char* sql = "INSERT OR IGNORE INTO UsersInfo ("
//                          "tg_id, tg_username, first_name, last_name, number, "
//                          "in_bot_name, sex, years, unic_your_id, unic_wanted_id, status"
//                          ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
//
//         sqlite3_stmt* stmt;
//         if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
//             sqlite3_bind_int(stmt, 1, request["tg_id"].get<int>());
//             sqlite3_bind_text(stmt, 2, request["tg_username"].get<std::string>().c_str(), -1, SQLITE_STATIC);
//
//             auto bind_optional = [&request](sqlite3_stmt* stmt, int idx, const char* field) {
//                 if (request.contains(field)) {
//                     std::string value = request.value(field, "");
//                     sqlite3_bind_text(stmt, idx, value.c_str(), -1, SQLITE_TRANSIENT);
//
//                 } else {
//                     sqlite3_bind_null(stmt, idx);
//                 }
//             };
//
//             bind_optional(stmt, 3, "first_name");
//             bind_optional(stmt, 4, "last_name");
//             std::string phone = request.value("number", "");
//             sqlite3_bind_text(stmt, 5, phone.c_str(), -1, SQLITE_TRANSIENT);
//
//             bind_optional(stmt, 6, "in_bot_name");
//             bind_optional(stmt, 7, "sex");
//             sqlite3_bind_int(stmt, 8, request.value("years", 0));
//             bind_optional(stmt, 9, "unic_your_id");
//             bind_optional(stmt, 10, "unic_wanted_id");
//             sqlite3_bind_int(stmt, 11, request.value("status", 1));
//
//             if (sqlite3_step(stmt) == SQLITE_DONE) {
//                 response["status"] = sqlite3_changes(db) > 0 ? "created" : "exists";
//             }
//             sqlite3_finalize(stmt);
//         }
//     }
//
//     else if (action == "get_user") {
//         const char* sql = "SELECT * FROM UsersInfo WHERE tg_id = ?";
//         sqlite3_stmt* stmt;
//         if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
//             sqlite3_bind_int(stmt, 1, request["tg_id"].get<int>());
//             if (sqlite3_step(stmt) == SQLITE_ROW) {
//                 response = {
//                     {"id", sqlite3_column_int(stmt, 0)},
//                     {"tg_id", sqlite3_column_int(stmt, 1)},
//                     {"tg_username", reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2))},
//                     {"first_name", reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3))},
//                     {"last_name", reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4))},
//                     {"number", sqlite3_column_int(stmt, 5)},
//                     {"in_bot_name", reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6))},
//                     {"sex", reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7))},
//                     {"years", sqlite3_column_int(stmt, 8)},
//                     {"unic_your_id", reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9))},
//                     {"unic_wanted_id", reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10))},
//                     {"status", sqlite3_column_int(stmt, 11)}
//                 };
//             }
//             sqlite3_finalize(stmt);
//         }
//     }
//
//     else if (action == "update_profile") {
//         const char* sql = "UPDATE UsersInfo SET in_bot_name = ?, sex = ?, years = ? WHERE tg_id = ?";
//         sqlite3_stmt* stmt;
//         if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
//             sqlite3_bind_text(stmt, 1, request["in_bot_name"].get<std::string>().c_str(), -1, SQLITE_STATIC);
//             sqlite3_bind_text(stmt, 2, request["sex"].get<std::string>().c_str(), -1, SQLITE_STATIC);
//             sqlite3_bind_int(stmt, 3, request["years"].get<int>());
//             sqlite3_bind_int(stmt, 4, request["tg_id"].get<int>());
//             if (sqlite3_step(stmt) == SQLITE_DONE) {
//                 response["status"] = "success";
//             }
//             sqlite3_finalize(stmt);
//         }
//     }
//
//     else if (action == "update_questionnaire") {
//         const char* sql = "UPDATE UsersInfo SET unic_your_id = ?, unic_wanted_id = ? WHERE tg_id = ?";
//         sqlite3_stmt* stmt;
//         if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
//             sqlite3_bind_text(stmt, 1, request["unic_your_id"].get<std::string>().c_str(), -1, SQLITE_STATIC);
//             sqlite3_bind_text(stmt, 2, request["unic_wanted_id"].get<std::string>().c_str(), -1, SQLITE_STATIC);
//             sqlite3_bind_int(stmt, 3, request["tg_id"].get<int>());
//             if (sqlite3_step(stmt) == SQLITE_DONE) {
//                 response["status"] = "success";
//             }
//             sqlite3_finalize(stmt);
//         }
//     }
//
//     else if (action == "update_status") {
//         const char* sql = "UPDATE UsersInfo SET status = ? WHERE tg_id = ?";
//         sqlite3_stmt* stmt;
//         if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
//             sqlite3_bind_int(stmt, 1, request["status"].get<int>());
//             sqlite3_bind_int(stmt, 2, request["tg_id"].get<int>());
//             if (sqlite3_step(stmt) == SQLITE_DONE) {
//                 response["status"] = "success";
//             }
//             sqlite3_finalize(stmt);
//         }
//     }
//
//     else if (action == "get_users_count") {
//         const char* sql = "SELECT COUNT(*) FROM UsersInfo";
//         sqlite3_stmt* stmt;
//         if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
//             if (sqlite3_step(stmt) == SQLITE_ROW) {
//                 response["count"] = sqlite3_column_int(stmt, 0);
//             }
//             sqlite3_finalize(stmt);
//         }
//     }
//
//     sqlite3_close(db);
//     return response.dump();
// }
//
// int main(int argc, char* argv[]) {
//     if (argc < 2) return 1;
//     std::cout << handle_request(argv[1]);
//     return 0;
// }
#include <iostream>
#include <sqlite3.h>
#include <nlohmann/json.hpp>
#include <string>
#include <signal.h>
#include <cstring>

using json = nlohmann::json;

void log_error(const std::string& message) {
    std::cerr << "[ERROR] " << message << std::endl;
}

void log_debug(const std::string& message) {
    std::cerr << "[DEBUG] " << message << std::endl;
}

void init_db(sqlite3* db) {
    const char* sql =
        "CREATE TABLE IF NOT EXISTS UsersInfo ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "tg_id BIGINT UNIQUE NOT NULL,"
        "tg_username TEXT NOT NULL,"
        "first_name TEXT,"
        "last_name TEXT,"
        "number TEXT,"
        "in_bot_name TEXT,"
        "sex TEXT,"
        "years INTEGER,"
        "unic_your_id TEXT,"
        "unic_wanted_id TEXT,"
        "status INTEGER NOT NULL DEFAULT 1);";

    char* err_msg = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &err_msg) != SQLITE_OK) {
        log_error(std::string("SQL error: ") + err_msg);
        sqlite3_free(err_msg);
    }
}

void bind_text_param(sqlite3_stmt* stmt, int idx, const json& j, const std::string& field) {
    if (j.contains(field) && !j[field].is_null()) {
        sqlite3_bind_text(stmt, idx, j[field].get<std::string>().c_str(), -1, SQLITE_TRANSIENT);
    } else {
        sqlite3_bind_null(stmt, idx);
    }
}

void bind_int_param(sqlite3_stmt* stmt, int idx, const json& j, const std::string& field, int default_val = 0) {
    if (j.contains(field) && !j[field].is_null()) {
        sqlite3_bind_int(stmt, idx, j[field].get<int>());
    } else {
        sqlite3_bind_int(stmt, idx, default_val);
    }
}

std::string handle_request(const std::string& json_str) {
    json response;
    sqlite3* db = nullptr;

    try {
        json request = json::parse(json_str);
        std::string db_path = request["db_path"].get<std::string>();

        if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK) {
            response["error"] = sqlite3_errmsg(db);
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
                sqlite3_bind_int(stmt, 1, request["tg_id"].get<int>());
                sqlite3_bind_text(stmt, 2, request["tg_username"].get<std::string>().c_str(), -1, SQLITE_TRANSIENT);

                bind_text_param(stmt, 3, request, "first_name");
                bind_text_param(stmt, 4, request, "last_name");
                bind_text_param(stmt, 5, request, "number");
                bind_text_param(stmt, 6, request, "in_bot_name");
                bind_text_param(stmt, 7, request, "sex");
                bind_int_param(stmt, 8, request, "years");
                bind_text_param(stmt, 9, request, "unic_your_id");
                bind_text_param(stmt, 10, request, "unic_wanted_id");
                bind_int_param(stmt, 11, request, "status", 1);

                if (sqlite3_step(stmt) == SQLITE_DONE) {
                    response["status"] = sqlite3_changes(db) > 0 ? "created" : "exists";
                }
                sqlite3_finalize(stmt);
            } else {
                response["error"] = sqlite3_errmsg(db);
            }
        }
        // ... (остальные обработчики действий)


        else if (action == "update_profile") {
                const char* sql = "UPDATE UsersInfo SET in_bot_name = ?, sex = ?, years = ? WHERE tg_id = ?";
                sqlite3_stmt* stmt;
                if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
                    sqlite3_bind_text(stmt, 1, request["in_bot_name"].get<std::string>().c_str(), -1, SQLITE_TRANSIENT);
                    sqlite3_bind_text(stmt, 2, request["sex"].get<std::string>().c_str(), -1, SQLITE_TRANSIENT);
                    sqlite3_bind_int(stmt, 3, request["years"].get<int>());
                    sqlite3_bind_int(stmt, 4, request["tg_id"].get<int>());
                    if (sqlite3_step(stmt) == SQLITE_DONE) {
                        response["status"] = "success";
                    }
                    sqlite3_finalize(stmt);
                }
            }

        else if (action == "update_questionnaire") {
            const char* sql = "UPDATE UsersInfo SET unic_your_id = ?, unic_wanted_id = ? WHERE tg_id = ?";
            sqlite3_stmt* stmt;
            if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_text(stmt, 1, request["unic_your_id"].get<std::string>().c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 2, request["unic_wanted_id"].get<std::string>().c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_int(stmt, 3, request["tg_id"].get<int>());
                if (sqlite3_step(stmt) == SQLITE_DONE) {
                    response["status"] = "success";
                }
                sqlite3_finalize(stmt);
            }
        }


    } catch (const std::exception& e) {
        response["error"] = e.what();
    }

    if (db) {
        sqlite3_close(db);
    }

    return response.dump();
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        log_error("Usage: db_worker <json_request>");
        return 1;
    }

    std::cout << handle_request(argv[1]);
    return 0;
}