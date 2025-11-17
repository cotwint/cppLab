#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <algorithm>
#include <cmath>  
#include <functional>
#include <limits>
#include <cctype>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

using namespace std;

// 字段类型枚举
enum FieldType {
    FIELD_INT,
    FIELD_STRING,
    FIELD_DOUBLE
};

// 运算符枚举
enum Operator {
    EQUAL,          // ==
    NOT_EQUAL,      // !=
    GREATER,        // >
    LESS,           // <
    GREATER_EQUAL,  // >=
    LESS_EQUAL,     // <=
    CONTAINS        // 字符串包含 (仅用于STRING类型)
};

// 字段定义结构
struct Field {
    string name;
    FieldType type;
    
    Field(string n = "", FieldType t = FIELD_STRING) : name(n), type(t) {}
};

// 记录值
struct Value {
    FieldType type;
    union {
        int intVal;
        double doubleVal;
    };
    string strVal;
    
    Value() : type(FIELD_STRING), intVal(0) {}
    
    string toString() const {
        switch(type) {
            case FIELD_INT: return to_string(intVal);
            case FIELD_DOUBLE: return to_string(doubleVal);
            case FIELD_STRING: return strVal;
            default: return "";
        }
    }
};

// 记录节点（链表节点）
struct RecordNode {
    map<string, Value> data;  // 字段名 -> 值
    RecordNode* next;
    
    RecordNode() : next(nullptr) {}
};

// ==================== 工具类 ====================
class DatabaseUtils {
public:
    // 辅助函数：创建INT类型的Value
    static Value makeIntValue(int val) {
        Value v;
        v.type = FIELD_INT;
        v.intVal = val;
        return v;
    }

    // 辅助函数：创建DOUBLE类型的Value
    static Value makeDoubleValue(double val) {
        Value v;
        v.type = FIELD_DOUBLE;
        v.doubleVal = val;
        return v;
    }

    // 辅助函数：创建STRING类型的Value
    static Value makeStringValue(const string& val) {
        Value v;
        v.type = FIELD_STRING;
        v.strVal = val;
        return v;
    }

    // 通用判断函数：判断记录是否满足条件
    // recordNode: 要判断的记录节点
    // key: 字段名
    // op: 运算符
    // value: 用于比较的值
    static bool evaluateCondition(const RecordNode* recordNode, const string& key, Operator op, const Value& value) {
        // 检查记录是否包含该键
        auto it = recordNode->data.find(key);
        if (it == recordNode->data.end()) {
            return false;  // 键不存在,返回false
        }
        
        const Value& recordValue = it->second;
        
        // 类型必须匹配(除了CONTAINS运算符只能用于STRING)
        if (recordValue.type != value.type) {
            return false;
        }
        
        // 根据类型和运算符进行比较
        switch (recordValue.type) {
            case FIELD_INT:
                switch (op) {
                    case EQUAL: return recordValue.intVal == value.intVal;
                    case NOT_EQUAL: return recordValue.intVal != value.intVal;
                    case GREATER: return recordValue.intVal > value.intVal;
                    case LESS: return recordValue.intVal < value.intVal;
                    case GREATER_EQUAL: return recordValue.intVal >= value.intVal;
                    case LESS_EQUAL: return recordValue.intVal <= value.intVal;
                    default: return false;
                }
                
            case FIELD_DOUBLE:
                switch (op) {
                    case EQUAL: return fabs(recordValue.doubleVal - value.doubleVal) < 1e-9;
                    case NOT_EQUAL: return fabs(recordValue.doubleVal - value.doubleVal) >= 1e-9;
                    case GREATER: return recordValue.doubleVal > value.doubleVal;
                    case LESS: return recordValue.doubleVal < value.doubleVal;
                    case GREATER_EQUAL: return recordValue.doubleVal >= value.doubleVal;
                    case LESS_EQUAL: return recordValue.doubleVal <= value.doubleVal;
                    default: return false;
                }
                
            case FIELD_STRING:
                switch (op) {
                    case EQUAL: return recordValue.strVal == value.strVal;
                    case NOT_EQUAL: return recordValue.strVal != value.strVal;
                    case GREATER: return recordValue.strVal > value.strVal;
                    case LESS: return recordValue.strVal < value.strVal;
                    case GREATER_EQUAL: return recordValue.strVal >= value.strVal;
                    case LESS_EQUAL: return recordValue.strVal <= value.strVal;
                    case CONTAINS: return recordValue.strVal.find(value.strVal) != string::npos;
                    default: return false;
                }
                
            default:
                return false;
        }
    }

    // 重载版本：直接接受RecordNode的data成员
    static bool evaluateCondition(const map<string, Value>& data, const string& key, Operator op, const Value& value) {
        RecordNode temp;
        temp.data = data;
        return evaluateCondition(&temp, key, op, value);
    }
};

//数据库

class Database {
private:
    string name;
    vector<Field> fields;  // 表结构，初始化后不可更改
    RecordNode* RecordHead;
    int recordCount;
    
    // 验证记录是否符合表结构
    bool validateRecord(const map<string, Value>& record) const {
        // 检查记录字段数量是否匹配
        if (record.size() != fields.size()) {
            cout << "错误：记录字段数量不匹配。期望 " << fields.size() 
                 << " 个字段，实际 " << record.size() << " 个字段" << endl;
            return false;
        }
        
        // 检查每个必需字段是否存在且类型正确
        for (const auto& field : fields) {
            auto it = record.find(field.name);
            
            // 检查字段是否存在
            if (it == record.end()) {
                cout << "错误：缺少必需字段 \"" << field.name << "\"" << endl;
                return false;
            }
            
            // 检查字段类型是否匹配
            if (it->second.type != field.type) {
                cout << "错误：字段 \"" << field.name << "\" 类型不匹配。期望 ";
                switch (field.type) {
                    case FIELD_INT: cout << "INT"; break;
                    case FIELD_DOUBLE: cout << "DOUBLE"; break;
                    case FIELD_STRING: cout << "STRING"; break;
                }
                cout << "，实际 ";
                switch (it->second.type) {
                    case FIELD_INT: cout << "INT"; break;
                    case FIELD_DOUBLE: cout << "DOUBLE"; break;
                    case FIELD_STRING: cout << "STRING"; break;
                }
                cout << endl;
                return false;
            }
        }
        
        // 检查是否有未定义的字段
        for (const auto& item : record) {
            bool found = false;
            for (const auto& field : fields) {
                if (field.name == item.first) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                cout << "错误：字段 \"" << item.first << "\" 未在表结构中定义" << endl;
                return false;
            }
        }
        
        return true;
    }
    
public:
    // 构造函数：必须提供数据库名称和表结构定义
    Database(const string& name, const vector<Field>& schema) 
        : name(name), fields(schema), RecordHead(nullptr), recordCount(0) {
        if (fields.empty()) {
            throw invalid_argument("错误：表结构不能为空");
        }
        cout << "数据库 \"" << name << "\" 创建成功，包含 " << fields.size() << " 个字段：";
        for (size_t i = 0; i < fields.size(); i++) {
            cout << fields[i].name;
            if (i < fields.size() - 1) cout << ", ";
        }
        cout << endl;
    }
    
    ~Database() {
        RecordNode* current = RecordHead;
        while (current != nullptr) {
            RecordNode* toDelete = current;
            current = current->next;
            delete toDelete;
        }
    }
    
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    
    // 显示表结构
    void displaySchema() const {
        cout << "========== 数据库表结构: " << name << " ==========" << endl;
        for (const auto& field : fields) {
            cout << "  字段名: " << field.name << " | 类型: ";
            switch (field.type) {
                case FIELD_INT: cout << "INT"; break;
                case FIELD_DOUBLE: cout << "DOUBLE"; break;
                case FIELD_STRING: cout << "STRING"; break;
            }
            cout << endl;
        }
        cout << "============================================" << endl;
    }

    void add_element_to_database(const map<string, Value>& record){
        if (record.empty()) {
            cout << "错误：不能添加空记录" << endl;
            return;
        }
        
        if (!validateRecord(record)) {
            cout << "错误：记录验证失败" << endl;
            return;
        }
        
        RecordNode* newNode = new RecordNode();
        newNode->data = record;
        newNode->next = RecordHead;
        RecordHead = newNode;

        recordCount++;
        cout << "记录添加成功，目前共 " << recordCount << " 条记录" << endl;
    }

    // 删除满足条件的记录
    // predicate: 判断函数,接受一个记录的引用,返回true表示需要删除该记录
    void remove_elements_in_database(const function<bool(const map<string, Value>&)>& predicate){
        RecordNode* current = RecordHead;
        RecordNode* prev = nullptr;
        int removedCount = 0;

        while (current != nullptr) {
            if (predicate(current->data)) {
                // 需要删除当前节点
                RecordNode* toDelete = current;  
                if (prev == nullptr) {
                    // 删除头节点
                    RecordHead = current->next;
                    current = RecordHead;
                } else {
                    // 删除中间或尾节点
                    prev->next = current->next;
                    current = current->next;
                }
                delete toDelete;
                recordCount--;
                removedCount++;
            } else {
                // 不删除,继续遍历
                prev = current;
                current = current->next;
            }
        }

        cout << "成功删除 " << removedCount << " 条记录,目前共 " << recordCount << " 条记录" << endl;
    }

    // 查找满足条件的记录
    // predicate: 判断函数,接受一个记录的引用,返回true表示该记录满足条件
    // 返回: 包含所有满足条件的记录指针的列表
    vector<RecordNode*> locate_elements_with_features(const function<bool(const map<string, Value>&)>& predicate) {
        vector<RecordNode*> result;
        RecordNode* current = RecordHead;

        while (current != nullptr) {
            if (predicate(current->data)) {
                result.push_back(current);
            }
            current = current->next;
        }

        cout << "找到 " << result.size() << " 条满足条件的记录" << endl;
        return result;
    }
    
    // 显示所有记录
    void display_all_elements() {
        if (RecordHead == nullptr) {
            cout << "数据库 \"" << name << "\" 中没有记录" << endl;
            return;
        }

        cout << "========== 数据库: " << name << " ==========" << endl;
        cout << "共有 " << recordCount << " 条记录" << endl;
        cout << "======================================" << endl;

        RecordNode* current = RecordHead;
        int index = 1;

        while (current != nullptr) {
            cout << "记录 #" << index << ":" << endl;
            
            // 遍历当前记录的所有字段
            for (const auto& field : current->data) {
                cout << "  " << field.first << ": " << field.second.toString() << endl;
            }
            
            cout << "--------------------------------------" << endl;
            current = current->next;
            index++;
        }
        
        cout << "======================================" << endl;
    }

    string getName() const {
        return name;
    }
    // predicate: 判断是否需要更新的条件
    // updater: 更新函数,接受记录引用并修改
    void update_elements_in_database(
        const function<bool(const map<string, Value>&)>& predicate,
        const function<void(map<string, Value>&)>& updater
    ) {
        RecordNode* current = RecordHead;
        int updatedCount = 0;
        int failedCount = 0;

        while (current != nullptr) {
            if (predicate(current->data)) {
                // 先保存原记录，以便验证失败时恢复
                map<string, Value> backup = current->data;
                
                // 执行更新
                updater(current->data);
                
                // 验证更新后的记录是否仍符合表结构
                if (!validateRecord(current->data)) {
                    cout << "警告：更新后的记录不符合表结构，已恢复原记录" << endl;
                    current->data = backup;  // 恢复原记录
                    failedCount++;
                } else {
                    updatedCount++;
                }
            }
            current = current->next;
        }

        cout << "成功更新 " << updatedCount << " 条记录";
        if (failedCount > 0) {
            cout << "，" << failedCount << " 条记录更新失败";
        }
        cout << endl;
    }
    
    int getRecordCount() const {
        return recordCount;
    }

    const vector<Field>& getSchema() const {
        return fields;
    }
};

// ==================== 数据库管理系统类 ====================
class DatabaseManagementSystem {
private:
    map<string, Database*> databases;  // 数据库名 -> 数据库指针
    Database* currentDatabase;         // 当前操作的数据库
    string currentDatabaseName;        // 当前数据库名称
    
public:
    DatabaseManagementSystem() : currentDatabase(nullptr), currentDatabaseName("") {
        cout << "========== 数据库管理系统已启动 ==========" << endl;
    }
    
    ~DatabaseManagementSystem() {
        // 清理所有数据库
        for (auto& pair : databases) {
            delete pair.second;
        }
        databases.clear();
        cout << "========== 数据库管理系统已关闭 ==========" << endl;
    }
    
    // 禁用拷贝构造和赋值
    DatabaseManagementSystem(const DatabaseManagementSystem&) = delete;
    DatabaseManagementSystem& operator=(const DatabaseManagementSystem&) = delete;
    
    // 创建新数据库
    bool createDatabase(const string& name, const vector<Field>& schema) {
        // 检查数据库名是否已存在
        if (databases.find(name) != databases.end()) {
            cout << "错误：数据库 \"" << name << "\" 已存在" << endl;
            return false;
        }
        
        // 检查表结构是否为空
        if (schema.empty()) {
            cout << "错误：数据库表结构不能为空" << endl;
            return false;
        }
        
        try {
            // 创建新数据库
            Database* newDb = new Database(name, schema);
            databases[name] = newDb;
            
            // 如果这是第一个数据库，自动设为当前数据库
            if (currentDatabase == nullptr) {
                currentDatabase = newDb;
                currentDatabaseName = name;
                cout << "已自动切换到数据库 \"" << name << "\"" << endl;
            }
            
            cout << "数据库 \"" << name << "\" 创建成功！目前共有 " << databases.size() << " 个数据库" << endl;
            return true;
        } catch (const exception& e) {
            cout << "创建数据库失败: " << e.what() << endl;
            return false;
        }
    }
    
    // 删除数据库
    bool deleteDatabase(const string& name) {
        auto it = databases.find(name);
        if (it == databases.end()) {
            cout << "错误：数据库 \"" << name << "\" 不存在" << endl;
            return false;
        }
        
        // 如果要删除的是当前数据库，需要先切换
        if (currentDatabaseName == name) {
            cout << "警告：正在删除当前操作的数据库 \"" << name << "\"" << endl;
            currentDatabase = nullptr;
            currentDatabaseName = "";
        }
        
        // 删除数据库
        delete it->second;
        databases.erase(it);
        
        cout << "数据库 \"" << name << "\" 已删除。目前共有 " << databases.size() << " 个数据库" << endl;
        
        // 如果还有其他数据库且当前数据库为空，提示用户切换
        if (currentDatabase == nullptr && !databases.empty()) {
            cout << "提示：请使用 useDatabase() 切换到其他数据库" << endl;
        }
        
        return true;
    }
    
    // 切换当前操作的数据库
    bool useDatabase(const string& name) {
        auto it = databases.find(name);
        if (it == databases.end()) {
            cout << "错误：数据库 \"" << name << "\" 不存在" << endl;
            return false;
        }
        
        currentDatabase = it->second;
        currentDatabaseName = name;
        cout << "已切换到数据库 \"" << name << "\"" << endl;
        return true;
    }
    
    // 显示所有数据库
    void showAllDatabases() const {
        cout << "========== 所有数据库列表 ==========" << endl;
        if (databases.empty()) {
            cout << "  (无数据库)" << endl;
        } else {
            cout << "  共有 " << databases.size() << " 个数据库：" << endl;
            int index = 1;
            for (const auto& pair : databases) {
                cout << "  " << index << ". " << pair.first;
                if (pair.first == currentDatabaseName) {
                    cout << " [当前]";
                }
                cout << " (" << pair.second->getRecordCount() << " 条记录)" << endl;
                index++;
            }
        }
        cout << "======================================" << endl;
    }
    
    // 显示当前操作的数据库信息
    void showCurrentDatabase() const {
        cout << "========== 当前数据库信息 ==========" << endl;
        if (currentDatabase == nullptr) {
            cout << "  当前未选择任何数据库" << endl;
            if (!databases.empty()) {
                cout << "  提示：使用 useDatabase() 切换到已有数据库" << endl;
            }
        } else {
            cout << "  数据库名称: " << currentDatabaseName << endl;
            cout << "  记录数量: " << currentDatabase->getRecordCount() << endl;
            cout << "  表结构信息:" << endl;
            currentDatabase->displaySchema();
        }
        cout << "======================================" << endl;
    }
    
    // 获取当前数据库指针（供外部操作使用）
    Database* getCurrentDatabase() {
        if (currentDatabase == nullptr) {
            cout << "警告：当前未选择任何数据库" << endl;
        }
        return currentDatabase;
    }
    
    // 获取当前数据库名称
    string getCurrentDatabaseName() const {
        return currentDatabaseName;
    }
    
    // 检查数据库是否存在
    bool databaseExists(const string& name) const {
        return databases.find(name) != databases.end();
    }
    
    // 获取数据库数量
    int getDatabaseCount() const {
        return databases.size();
    }
};

// ==================== 命令解析类 ====================
class CommandParser {
private:
    DatabaseManagementSystem& dbms;

    static void prompt(const string& message) {
        cout << message;
        cout.flush();
    }

    static string trim(const string& input) {
        size_t start = 0;
        while (start < input.size() && isspace(static_cast<unsigned char>(input[start]))) {
            start++;
        }
        size_t end = input.size();
        while (end > start && isspace(static_cast<unsigned char>(input[end - 1]))) {
            end--;
        }
        return input.substr(start, end - start);
    }

    static string toLower(string value) {
        transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
            return static_cast<char>(tolower(c));
        });
        return value;
    }

    static bool parseInt(const string& text, int& outValue) {
        try {
            size_t pos = 0;
            long long temp = stoll(text, &pos);
            if (pos != text.size()) {
                return false;
            }
            if (temp < std::numeric_limits<int>::min() || temp > std::numeric_limits<int>::max()) {
                return false;
            }
            outValue = static_cast<int>(temp);
            return true;
        } catch (...) {
            return false;
        }
    }

    static bool parseDoubleValue(const string& text, double& outValue) {
        try {
            size_t pos = 0;
            outValue = stod(text, &pos);
            return pos == text.size();
        } catch (...) {
            return false;
        }
    }

    static bool parseFieldType(const string& text, FieldType& outType) {
        string lowered = toLower(text);
        if (lowered == "int") {
            outType = FIELD_INT;
            return true;
        }
        if (lowered == "double") {
            outType = FIELD_DOUBLE;
            return true;
        }
        if (lowered == "string") {
            outType = FIELD_STRING;
            return true;
        }
        return false;
    }

    static bool parseOperatorToken(const string& token, Operator& op) {
        string lowered = toLower(token);
        if (lowered == "==" || lowered == "=") {
            op = EQUAL;
            return true;
        }
        if (lowered == "!=") {
            op = NOT_EQUAL;
            return true;
        }
        if (lowered == ">") {
            op = GREATER;
            return true;
        }
        if (lowered == "<") {
            op = LESS;
            return true;
        }
        if (lowered == ">=") {
            op = GREATER_EQUAL;
            return true;
        }
        if (lowered == "<=") {
            op = LESS_EQUAL;
            return true;
        }
        if (lowered == "contains") {
            op = CONTAINS;
            return true;
        }
        return false;
    }

    static string stripQuotes(const string& text) {
        if (text.size() >= 2) {
            char first = text.front();
            char last = text.back();
            if ((first == '"' && last == '"') || (first == '\'' && last == '\'')) {
                return text.substr(1, text.size() - 2);
            }
        }
        return text;
    }

    static bool findFieldByName(const Database* db, const string& name, Field& outField) {
        const vector<Field>& schema = db->getSchema();
        for (const auto& field : schema) {
            if (field.name == name) {
                outField = field;
                return true;
            }
        }
        return false;
    }

    static bool convertValueByField(const Field& field, const string& text, Value& outValue) {
        string trimmed = trim(text);
        Value converted;
        switch (field.type) {
            case FIELD_INT: {
                int temp;
                if (!parseInt(trimmed, temp)) {
                    return false;
                }
                converted = DatabaseUtils::makeIntValue(temp);
                break;
            }
            case FIELD_DOUBLE: {
                double temp;
                if (!parseDoubleValue(trimmed, temp)) {
                    return false;
                }
                converted = DatabaseUtils::makeDoubleValue(temp);
                break;
            }
            case FIELD_STRING: {
                converted = DatabaseUtils::makeStringValue(stripQuotes(trimmed));
                break;
            }
            default:
                return false;
        }
        outValue = converted;
        return true;
    }

    static void printHelp() {
        cout << "可用命令:" << endl;
        cout << "  create <name>           - 创建数据库" << endl;
        cout << "  open <name>             - 切换当前数据库" << endl;
        cout << "  add                     - 向当前数据库追加记录" << endl;
        cout << "  locate for <cond>       - 按条件定位记录" << endl;
        cout << "  delete for <cond>       - 按条件删除记录" << endl;
        cout << "  show databases          - 显示所有数据库" << endl;
        cout << "  show current            - 显示当前数据库信息" << endl;
        cout << "  help                    - 显示帮助" << endl;
        cout << "  exit                    - 退出程序" << endl;
    }

    static void displayRecords(const vector<RecordNode*>& records) {
        if (records.empty()) {
            cout << "未找到符合条件的记录" << endl;
            return;
        }
        cout << "========== 匹配记录 ==========" << endl;
        int index = 1;
        for (const auto* node : records) {
            cout << "记录 #" << index << ":" << endl;
            for (const auto& entry : node->data) {
                cout << "  " << entry.first << ": " << entry.second.toString() << endl;
            }
            cout << "--------------------------------------" << endl;
            index++;
        }
        cout << "================================" << endl;
    }

    bool buildCondition(Database* db, const string& rawCondition, string& fieldName, Operator& op, Value& value) {
        string condition = trim(rawCondition);
        if (condition.empty()) {
            cout << "错误：条件不能为空" << endl;
            return false;
        }

        istringstream iss(condition);
        string opToken;
        if (!(iss >> fieldName)) {
            cout << "错误：未能解析字段名" << endl;
            return false;
        }

        if (!(iss >> opToken)) {
            cout << "错误：未能解析运算符" << endl;
            return false;
        }

        string valuePart;
        getline(iss, valuePart);
        valuePart = trim(valuePart);
        if (valuePart.empty()) {
            cout << "错误：未能解析比较值" << endl;
            return false;
        }

        if (!parseOperatorToken(opToken, op)) {
            cout << "错误：不支持的运算符 " << opToken << endl;
            return false;
        }

        Field field;
        if (!findFieldByName(db, fieldName, field)) {
            cout << "错误：字段 " << fieldName << " 不存在于当前数据库" << endl;
            return false;
        }

        if (op == CONTAINS && field.type != FIELD_STRING) {
            cout << "错误：CONTAINS 运算符仅适用于 STRING 字段" << endl;
            return false;
        }

        if (!convertValueByField(field, valuePart, value)) {
            cout << "错误：值 \"" << valuePart << "\" 无法转换为指定字段类型" << endl;
            return false;
        }

        return true;
    }

    bool buildSchema(vector<Field>& schema) {
        string line;
    prompt("请输入字段数量：");
        if (!getline(cin, line)) {
            return false;
        }
        line = trim(line);
        if (line.empty()) {
            cout << "错误：字段数量不能为空" << endl;
            return false;
        }
        int count = 0;
        if (!parseInt(line, count) || count <= 0) {
            cout << "错误：字段数量必须为正整数" << endl;
            return false;
        }

        for (int i = 0; i < count; ++i) {
            prompt(string("请输入第 ") + to_string(i + 1) + " 个字段的名称：");
            if (!getline(cin, line)) {
                return false;
            }
            line = trim(line);
            if (line.empty()) {
                cout << "错误：字段名称不能为空" << endl;
                return false;
            }
            string fieldName = line;

            prompt(string("请输入字段 ") + fieldName + " 的类型 (int/double/string)：");
            if (!getline(cin, line)) {
                return false;
            }
            line = trim(line);
            FieldType type;
            if (!parseFieldType(line, type)) {
                cout << "错误：字段类型无效" << endl;
                return false;
            }

            schema.emplace_back(fieldName, type);
        }
        return true;
    }

    bool buildRecord(Database* db, map<string, Value>& record) {
        const vector<Field>& schema = db->getSchema();
        string line;
        for (const auto& field : schema) {
            bool valid = false;
            while (!valid) {
                prompt(string("请输入字段 ") + field.name + " 的值：");
                if (!getline(cin, line)) {
                    return false;
                }
                if (line.empty() && field.type != FIELD_STRING) {
                    cout << "错误：该字段值不能为空" << endl;
                    continue;
                }
                Value value;
                if (!convertValueByField(field, line, value)) {
                    cout << "错误：输入的值与字段类型不匹配" << endl;
                    continue;
                }
                record[field.name] = value;
                valid = true;
            }
        }
        return true;
    }

    void handleCreateCommand(istringstream& iss) {
        string name;
        if (!(iss >> name)) {
            prompt("请输入数据库名称：");
            string line;
            if (!getline(cin, line)) {
                return;
            }
            name = trim(line);
        }

        if (name.empty()) {
            cout << "错误：数据库名称不能为空" << endl;
            return;
        }

        cout << "[创建] 即将创建数据库 \"" << name << "\"，接下来请依次输入字段数量与字段信息" << endl;

        vector<Field> schema;
        if (!buildSchema(schema)) {
            cout << "提示：数据库创建取消" << endl;
            return;
        }

        if (dbms.createDatabase(name, schema)) {
            cout << "[创建] 数据库 \"" << name << "\" 已准备就绪，可使用 open " << name
                 << " 切换并通过 add 添加记录" << endl;
        }
    }

    void handleOpenCommand(istringstream& iss) {
        string name;
        if (!(iss >> name)) {
            prompt("请输入要打开的数据库名称：");
            string line;
            if (!getline(cin, line)) {
                return;
            }
            name = trim(line);
        }

        if (name.empty()) {
            cout << "错误：数据库名称不能为空" << endl;
            return;
        }

        cout << "[打开] 正在尝试打开数据库 \"" << name << "\"" << endl;
        if (dbms.useDatabase(name)) {
            cout << "[打开] 已切换至 \"" << name << "\"，可执行 add 追加记录或 locate for ... 查询" << endl;
        }
    }

    void handleAddCommand() {
        Database* db = dbms.getCurrentDatabase();
        if (db == nullptr) {
            cout << "错误：请先使用 open 命令选择数据库" << endl;
            return;
        }

        cout << "[追加] 将按当前表结构的字段顺序逐项输入记录值，字符串可直接输入或使用引号" << endl;
        map<string, Value> record;
        if (!buildRecord(db, record)) {
            cout << "提示：追加记录取消" << endl;
            return;
        }

        db->add_element_to_database(record);
        cout << "[追加] 可继续使用 add 增加更多记录，或 locate for ... 查看符合条件的记录" << endl;
    }

    void handleLocateCommand(istringstream& iss) {
        string keyword;
        if (!(iss >> keyword) || toLower(keyword) != "for") {
            cout << "错误：locate 命令格式应为 locate for <条件>" << endl;
            return;
        }

        string condition;
        getline(iss, condition);
        condition = trim(condition);
        if (condition.empty()) {
            cout << "错误：缺少定位条件" << endl;
            return;
        }

        Database* db = dbms.getCurrentDatabase();
        if (db == nullptr) {
            cout << "错误：请先使用 open 命令选择数据库" << endl;
            return;
        }

        string fieldName;
        Operator op;
        Value value;
        if (!buildCondition(db, condition, fieldName, op, value)) {
            return;
        }

        cout << "[定位] 正在根据条件 \"" << condition << "\" 查找记录" << endl;
        auto matches = db->locate_elements_with_features([&](const map<string, Value>& data) {
            return DatabaseUtils::evaluateCondition(data, fieldName, op, value);
        });

        displayRecords(matches);
    }

    void handleDeleteCommand(istringstream& iss) {
        string keyword;
        if (!(iss >> keyword) || toLower(keyword) != "for") {
            cout << "错误：delete 命令格式应为 delete for <条件>" << endl;
            return;
        }

        string condition;
        getline(iss, condition);
        condition = trim(condition);
        if (condition.empty()) {
            cout << "错误：缺少删除条件" << endl;
            return;
        }

        Database* db = dbms.getCurrentDatabase();
        if (db == nullptr) {
            cout << "错误：请先使用 open 命令选择数据库" << endl;
            return;
        }

        string fieldName;
        Operator op;
        Value value;
        if (!buildCondition(db, condition, fieldName, op, value)) {
            return;
        }

        cout << "[删除] 正在删除满足条件 \"" << condition << "\" 的记录" << endl;
        db->remove_elements_in_database([&](const map<string, Value>& data) {
            return DatabaseUtils::evaluateCondition(data, fieldName, op, value);
        });
        cout << "[删除] 如需确认结果，可使用 locate for ... 或 show current" << endl;
    }

public:
    explicit CommandParser(DatabaseManagementSystem& system) : dbms(system) {}

    void run() {
        cout << "输入 help 查看命令列表，输入 exit 退出程序" << endl;
        string line;
        while (true) {
            prompt("> ");
            if (!getline(cin, line)) {
                cout << endl;
                break;
            }
            line = trim(line);
            if (line.empty()) {
                continue;
            }

            string lowered = toLower(line);
            if (lowered == "exit" || lowered == "quit") {
                break;
            }

            handleCommand(line);
        }
    }

    void handleCommand(const string& commandLine) {
        istringstream iss(commandLine);
        string command;
        iss >> command;
        string lowered = toLower(command);

        if (lowered == "help") {
            printHelp();
        } else if (lowered == "create") {
            handleCreateCommand(iss);
        } else if (lowered == "open") {
            handleOpenCommand(iss);
        } else if (lowered == "add") {
            handleAddCommand();
        } else if (lowered == "locate") {
            handleLocateCommand(iss);
        } else if (lowered == "delete") {
            handleDeleteCommand(iss);
        } else if (lowered == "show") {
            string target;
            iss >> target;
            string targetLower = toLower(target);
            if (targetLower == "databases") {
                dbms.showAllDatabases();
            } else if (targetLower == "current") {
                dbms.showCurrentDatabase();
            } else {
                cout << "错误：未知的 show 参数" << endl;
            }
        } else {
            cout << "错误：未知命令，输入 help 查看帮助" << endl;
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    DatabaseManagementSystem dbms;
    CommandParser parser(dbms);
    parser.run();
    return 0;
}

