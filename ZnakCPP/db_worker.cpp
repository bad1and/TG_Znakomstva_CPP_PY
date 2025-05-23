#include <iostream>
#include <sqlite3.h>
#include <nlohmann/json.hpp>
#include <string>


using namespace std; // чтобы не писать std:: каждый раз

using json = nlohmann::json;

// для одного пользователя, совпавшего по критериям
struct MatchUser {
    int tg_id;
    std::string tg_username;
    std::string in_bot_name;
    int years;
    std::string unic_your_id;
    int match_percent;
    // Метод преобразования структуры в json
    json to_json() const {
        return {
                {"tg_id", tg_id},
                {"tg_username", tg_username},
                {"in_bot_name", in_bot_name},
                {"years", years},
                {"unic_your_id", unic_your_id},
                {"match_percent", match_percent}
        };
    }
};

struct MatchNode {
    MatchUser value;
    MatchNode* prev;
    MatchNode* next;

    MatchNode(const MatchUser& val, MatchNode* p = nullptr, MatchNode* n = nullptr)
        : value(val), prev(p), next(n) {}
};

class MatchList {
    MatchNode* head = nullptr;
    MatchNode* tail = nullptr;

public:
    void push_back(const MatchUser& user) { // Добавление в конец списка
        MatchNode* node = new MatchNode(user, tail, nullptr);
        if (tail) tail->next = node;
        tail = node;
        if (!head) head = node;
    }
    // Преобразование всего списка в массив json
    json to_json_array() const {
        json result = json::array();
        for (MatchNode* n = head; n; n = n->next)
            result.push_back(n->value.to_json());
        return result;
    }
    // Деструктор
    ~MatchList() {
        MatchNode* curr = head;
        while (curr) {
            MatchNode* tmp = curr;
            curr = curr->next;
            delete tmp;
        }
    }
    // Сортировка по убыванию процента
    void sort_by_match_percent() {
        if (!head || !head->next) return;

        // Сбор всех узлов в вектор
        std::vector<MatchNode*> nodes;
        for (MatchNode* n = head; n; n = n->next) {
            nodes.push_back(n);
        }

        // Сортировка по убыванию
        std::sort(nodes.begin(), nodes.end(), [](MatchNode* a, MatchNode* b) {
            return a->value.match_percent > b->value.match_percent;
        });

        // Перестройка указ
        head = nodes[0];
        head->prev = nullptr;
        for (size_t i = 0; i < nodes.size(); ++i) {
            nodes[i]->next = (i + 1 < nodes.size()) ? nodes[i + 1] : nullptr;
            nodes[i]->prev = (i > 0) ? nodes[i - 1] : nullptr;
        }
        tail = nodes.back();
    }

};


// ошибки stderr
void log_error(const std::string &message) {
    std::cerr << "[ERROR] " << message << std::endl;
}
// ДеБуг
void log_debug(const std::string &message) {
    std::cerr << "[DEBUG] " << message << std::endl;
}
//СОздание БД если отсутствует
void init_db(sqlite3 *db) {
    const char *sql =
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

    char *err_msg = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &err_msg) != SQLITE_OK) {
        log_error(std::string("SQL error: ") + err_msg);
        sqlite3_free(err_msg);
    }
}
// Привязка текст параметра
void bind_text_param(sqlite3_stmt *stmt, int idx, const json &j, const std::string &field) {
    if (j.contains(field) && !j[field].is_null()) {
        sqlite3_bind_text(stmt, idx, j[field].get<std::string>().c_str(), -1, SQLITE_TRANSIENT);
    } else {
        sqlite3_bind_null(stmt, idx);
    }
}
// Привязка числ параметра
void bind_int_param(sqlite3_stmt *stmt, int idx, const json &j, const std::string &field, int default_val = 0) {
    if (j.contains(field) && !j[field].is_null()) {
        sqlite3_bind_int(stmt, idx, j[field].get<int>());
    } else {
        sqlite3_bind_int(stmt, idx, default_val);
    }
}

std::string handle_request(const std::string &json_str) {
    json response;
    sqlite3 *db = nullptr;

    try {
        json request = json::parse(json_str);// Парсим входной JSON
        std::string db_path = request["db_path"].get<std::string>();

        if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK) {
            response["error"] = sqlite3_errmsg(db);
            return response.dump();
        }

        init_db(db); // если бд нету создадим
        std::string action = request["action"];

        //экшены от РЕВЕСТТОВ ==> ХЭНДЛЕРЫ
        if (action == "add_user") {
            const char *sql = "INSERT OR IGNORE INTO UsersInfo ("
                    "tg_id, tg_username, first_name, last_name, number, "
                    "in_bot_name, sex, years, unic_your_id, unic_wanted_id, status"
                    ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

            sqlite3_stmt *stmt;
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
        } else if (action == "update_profile") {
            const char *sql = "UPDATE UsersInfo SET in_bot_name = ?, sex = ?, years = ? WHERE tg_id = ?";
            sqlite3_stmt *stmt;
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
        } else if (action == "update_questionnaire") {
            const char *sql = "UPDATE UsersInfo SET unic_your_id = ?, unic_wanted_id = ? WHERE tg_id = ?";
            sqlite3_stmt *stmt;
            if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_text(stmt, 1, request["unic_your_id"].get<std::string>().c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 2, request["unic_wanted_id"].get<std::string>().c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_int(stmt, 3, request["tg_id"].get<int>());
                if (sqlite3_step(stmt) == SQLITE_DONE) {
                    response["status"] = "success";
                }
                sqlite3_finalize(stmt);
            }
        } else if (action == "update_status") {
            const char *sql = "UPDATE UsersInfo SET status = ? WHERE tg_id = ?";
            sqlite3_stmt *stmt;
            if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_int(stmt, 1, request["status"].get<int>());
                sqlite3_bind_int(stmt, 2, request["tg_id"].get<int>());
                if (sqlite3_step(stmt) == SQLITE_DONE) {
                    response["status"] = "success";
                }
                sqlite3_finalize(stmt);
            }
        } else if (action == "get_users_count") {
            const char *sql = "SELECT COUNT(*) FROM UsersInfo";
            sqlite3_stmt *stmt;
            if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
                if (sqlite3_step(stmt) == SQLITE_ROW) {
                    response["count"] = sqlite3_column_int(stmt, 0);
                }
                sqlite3_finalize(stmt);
            }
        } else if (action == "get_user") {
            const char *sql = R"(
        SELECT
            id, tg_id, tg_username, first_name, last_name, number,
            in_bot_name, sex, years, unic_your_id, unic_wanted_id, status
        FROM UsersInfo
        WHERE tg_id = ?
    )";

            sqlite3_stmt *stmt;
            if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
                // Привязываем параметр
                sqlite3_bind_int(stmt, 1, request["tg_id"].get<int>());

                if (sqlite3_step(stmt) == SQLITE_ROW) {

                    response["id"] = sqlite3_column_int(stmt, 0);
                    response["tg_id"] = sqlite3_column_int(stmt, 1);


                    auto safe_column_text = [](sqlite3_stmt *stmt, int col) -> json {
                        const unsigned char *text = sqlite3_column_text(stmt, col);
                        return text ? json(reinterpret_cast<const char *>(text)) : json(nullptr);
                    };

                    response["tg_username"] = safe_column_text(stmt, 2);
                    response["first_name"] = safe_column_text(stmt, 3);
                    response["last_name"] = safe_column_text(stmt, 4);
                    response["number"] = safe_column_text(stmt, 5);
                    response["in_bot_name"] = safe_column_text(stmt, 6);
                    response["sex"] = safe_column_text(stmt, 7);
                    response["years"] = sqlite3_column_int(stmt, 8);
                    response["unic_your_id"] = safe_column_text(stmt, 9);
                    response["unic_wanted_id"] = safe_column_text(stmt, 10);
                    response["status"] = sqlite3_column_int(stmt, 11);
                } else {
                    response["error"] = "user_not_found";
                }
                sqlite3_finalize(stmt);
            } else {
                response["error"] = sqlite3_errmsg(db);
            }

        } else if (action == "get_matching_users") {
            const char* sql = "SELECT * FROM UsersInfo WHERE tg_id != ? AND sex != ? AND status = ?";
            sqlite3_stmt* stmt;

            if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
                // Привязка параметров запроса
                sqlite3_bind_int(stmt, 1, request["tg_id"].get<int>());
                sqlite3_bind_text(stmt, 2, request["sex"].get<std::string>().c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_int(stmt, 3, request["status"].get<int>());

                std::string user_unic_wanted_id = request["unic_wanted_id"].get<std::string>();
                int allowed_mismatches = request.value("max_dopusk", 2);

                MatchList list;

                // Проход по всем
                while (sqlite3_step(stmt) == SQLITE_ROW) {
                    const char* raw_unic = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
                    if (!raw_unic) continue;

                    std::string unic_your_id = raw_unic;
                    int total = std::min(unic_your_id.size(), user_unic_wanted_id.size());
                    int same = 0;
                    for (int i = 0; i < total; ++i)
                        if (unic_your_id[i] == user_unic_wanted_id[i]) ++same;

                    bool perfect = unic_your_id == user_unic_wanted_id;
                    bool similar = same >= total - allowed_mismatches;

                    if (perfect || similar) {
                        double match_percent = (total > 0) ? (100.0 * same / total) : 0.0;
                        // Создание структуры и добавление в список
                        MatchUser mu = {
                            .tg_id = sqlite3_column_int(stmt, 1),
                            .tg_username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)),
                            .in_bot_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)),
                            .years = sqlite3_column_int(stmt, 8),
                            .unic_your_id = unic_your_id,
                            .match_percent = static_cast<int>(match_percent + 0.5)
                        };
                        list.push_back(mu);
                    }
                }

                list.sort_by_match_percent();// Сортировка по убыванию

                response["matches"] = list.to_json_array();//Обратно в json
                sqlite3_finalize(stmt);
            }
        }


    } catch (const std::exception &e) {
        response["error"] = e.what();// если возникло искл
    }

    if (db) {
        sqlite3_close(db);
    }

    return response.dump();// отправка json ответа
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        log_error("Usage: db_worker <json_request>");
        return 1;
    }

    std::cout << handle_request(argv[1]);
    return 0;
}
