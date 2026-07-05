#include "crow_all.h"
#include <iostream> 
#include <fstream> 
#include <string> 
#include <queue> 
#include <ctime> 
#include <sstream> 
#include <vector> 
#include <iomanip> 
#include <cctype> 
#include <map> 

using namespace std; 
 
struct Patient { 
    int id; 
    string name; 
    string phone; 
    string dob; 
    int age; 
}; 
 
struct TempBuffer { 
    bool active = false; 
    bool isEmergency = false; 
    bool processed = false; 
    bool inpatient = false; 
    vector<string> records; 
    double medAmount = 0; 
    int daysAdmitted = 0; 
}; 
 
// Active Queues
queue<int> outpatientQueue; 
priority_queue<int> emergencyQueue; 
queue<int> inpatientQueue; 
 
// File Storage Records
const string PATIENT_FILE = "patients.txt"; 
const string HISTORY_FILE = "history.txt"; 
 
// Emergency Buffers
map<int, TempBuffer> buffers; 
 
// --------------------------------------------------------------------------- 
// Core Diagnostic Utility Functions
// --------------------------------------------------------------------------- 
bool isUniqueId(int id) { 
    ifstream fin(PATIENT_FILE); 
    if (!fin) return true; 
    Patient p; 
    while (fin >> p.id >> p.name >> p.phone >> p.dob >> p.age) { 
        if (p.id == id) { fin.close(); return false; }
    } 
    fin.close(); 
    return true; 
} 
 
bool patientExists(int id) { 
    ifstream fin(PATIENT_FILE); 
    if (!fin) return false; 
    Patient p; 
    while (fin >> p.id >> p.name >> p.phone >> p.dob >> p.age) { 
        if (p.id == id) { fin.close(); return true; }
    } 
    fin.close(); 
    return false; 
} 
 
bool isUniquePhone(const string &phone) { 
    ifstream fin(PATIENT_FILE); 
    if (!fin) return true; 
    Patient p; 
    while (fin >> p.id >> p.name >> p.phone >> p.dob >> p.age) { 
        if (p.phone == phone) { fin.close(); return false; }
    } 
    fin.close(); 
    return true; 
} 
 
bool isProcessed(int id) { 
    queue<int> temp = outpatientQueue; 
    while (!temp.empty()) { 
        if (temp.front() == id) return true; 
        temp.pop(); 
    } 
    priority_queue<int> temp2 = emergencyQueue; 
    while (!temp2.empty()) { 
        if (temp2.top() == id) return true; 
        temp2.pop(); 
    } 
    queue<int> temp3 = inpatientQueue; 
    while (!temp3.empty()) { 
        if (temp3.front() == id) return true; 
        temp3.pop(); 
    } 
    return false; 
} 

string getCurrentDateTime() { 
    time_t now = time(0); 
    tm *lt = localtime(&now); 
    char buf[50]; 
    sprintf(buf, "%02d-%02d-%04d %02d:%02d:%02d", 
            lt->tm_mday, lt->tm_mon + 1, lt->tm_year + 1900, 
            lt->tm_hour, lt->tm_min, lt->tm_sec); 
    return string(buf); 
} 
 
bool appendHistoryRecord(int id, const string &record) { 
    ofstream fout(HISTORY_FILE, ios::app); 
    if (!fout) return false; 
    fout << id << "|" << getCurrentDateTime() << "|" << record << "\n"; 
    fout.close(); 
    return true; 
} 

// Helper utility to safely extract clean strings from raw JSON payloads without relying on heavy parser frameworks
string extractJsonField(const string& body, const string& field) {
    size_t pos = body.find("\"" + field + "\"");
    if (pos == string::npos) return "";
    size_t start = body.find(":", pos);
    if (start == string::npos) return "";
    
    // Advance past separator
    start++;
    while(start < body.size() && (body[start] == ' ' || body[start] == '"')) start++;
    
    size_t end = start;
    while(end < body.size() && body[end] != '"' && body[end] != ',' && body[end] != '}' && body[end] != '\r' && body[end] != '\n') end++;
    
    string val = body.substr(start, end - start);
    // Trim trailing quotes if needed
    if(!val.empty() && val.back() == '"') val.pop_back();
    return val;
}

// --------------------------------------------------------------------------- 
// Main Distributed Web Router Loop[cite: 3]
// --------------------------------------------------------------------------- 
int main() { 
    crow::SimpleApp app;

    // 1. Core Web GUI Dashboard Access Point[cite: 3]
    CROW_ROUTE(app, "/")([](){
        auto page = crow::mustache::load_text("index.html"); 
        return page;
    });

    // 2. Fetch Active Patient Directory API[cite: 3]
    CROW_ROUTE(app, "/api/patients")([](){
        ifstream fin(PATIENT_FILE);
        ostringstream json_out;
        json_out << "[";
        
        if (fin) {
            Patient p;
            bool first = true;
            while (fin >> p.id >> p.name >> p.phone >> p.dob >> p.age) {
                if (!first) json_out << ",";
                json_out << "{\"id\":" << p.id 
                         << ",\"name\":\"" << p.name 
                         << "\",\"phone\":\"" << p.phone 
                         << "\",\"dob\":\"" << p.dob 
                         << "\",\"age\":" << p.age << "}";
                first = false;
            }
            fin.close();
        }
        json_out << "]";
        return crow::response(200, "application/json", json_out.str());
    });

    // 3. Register New Patient API Endpoint[cite: 3]
    CROW_ROUTE(app, "/api/patients/add").methods(crow::HTTPMethod::POST)([](const crow::request& req){
        string body = req.body;
        string s_id = extractJsonField(body, "id");
        string name = extractJsonField(body, "name");
        string phone = extractJsonField(body, "phone");
        string dob = extractJsonField(body, "dob");
        string s_age = extractJsonField(body, "age");

        if(s_id.empty() || name.empty() || phone.empty()) {
            return crow::response(400, "Missing required database fields.");
        }

        int id = stoi(s_id);
        int age = stoi(s_age);

        if (!isUniqueId(id)) return crow::response(400, "Patient ID collision detected.");
        if (!isUniquePhone(phone)) return crow::response(400, "Phone number matches existing registry.");

        ofstream fout(PATIENT_FILE, ios::app);
        fout << id << " " << name << " " << phone << " " << dob << " " << age << "\n";
        fout.close();

        return crow::response(200, "Patient record successfully synchronized to local disk.");
    });

    // 4. Clinical Triage Router API[cite: 3]
    CROW_ROUTE(app, "/api/patients/process").methods(crow::HTTPMethod::POST)([](const crow::request& req){
        string body = req.body;
        string s_id = extractJsonField(body, "id");
        string type = extractJsonField(body, "type");

        if(s_id.empty()) return crow::response(400, "Invalid ID provided.");
        int id = stoi(s_id);

        if (!patientExists(id)) return crow::response(404, "Target patient record does not exist.");

        if (type == "outpatient") {
            outpatientQueue.push(id);
        } else if (type == "emergency") {
            emergencyQueue.push(id);
            inpatientQueue.push(id);
        } else {
            return crow::response(400, "Unsupported storage triage pipeline specification.");
        }
        return crow::response(200, "Patient successfully routed inside runtime DSA tracking queues.");
    });

    // 5. Live Data Structure Stream Status API[cite: 3]
    CROW_ROUTE(app, "/api/queues")([](){
        ostringstream json_out;
        json_out << "{";

        // Map Sequential Outpatient Queue State[cite: 3]
        json_out << "\"outpatient\":[";
        queue<int> q1 = outpatientQueue;
        bool first = true;
        while(!q1.empty()){
            if(!first) json_out << ",";
            json_out << q1.front();
            first = false;
            q1.pop();
        }
        json_out << "],";

        // Map Max-Heap Emergency Priority Queue State[cite: 3]
        json_out << "\"emergency\":[";
        priority_queue<int> q2 = emergencyQueue;
        first = true;
        while(!q2.empty()){
            if(!first) json_out << ",";
            json_out << q2.top();
            first = false;
            q2.pop();
        }
        json_out << "],";

        // Map Inpatient Queue State[cite: 3]
        json_out << "\"inpatient\":[";
        queue<int> q3 = inpatientQueue;
        first = true;
        while(!q3.empty()){
            if(!first) json_out << ",";
            json_out << q3.front();
            first = false;
            q3.pop();
        }
        json_out << "]}";

        return crow::response(200, "application/json", json_out.str());
    });

    // 6. Clinical History Tracking Query Engine API[cite: 3]
    CROW_ROUTE(app, "/api/patients/history/<int>")([](int id){
        ifstream fin(HISTORY_FILE);
        ostringstream json_out;
        json_out << "[";
        
        if (fin) {
            string line;
            bool first = true;
            while (getline(fin, line)) {
                if (line.empty()) continue;
                stringstream ss(line);
                string sid, date, record;
                getline(ss, sid, '|');
                getline(ss, date, '|');
                getline(ss, record);
                
                if (stoi(sid) == id) {
                    if (!first) json_out << ",";
                    json_out << "{\"date\":\"" << date << "\",\"record\":\"" << record << "\"}";
                    first = false;
                }
            }
            fin.close();
        }
        json_out << "]";
        return crow::response(200, "application/json", json_out.str());
    });

    // Launch server asynchronously on port 18080[cite: 3]
    app.port(18080).multithreaded().run();
}
