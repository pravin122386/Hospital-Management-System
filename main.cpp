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
 
 
 
10 | Page  
// Queues 
queue<int> outpatientQueue; 
priority_queue<int> emergencyQueue; 
queue<int> inpatientQueue; 
 
// Files 
const string PATIENT_FILE = "patients.txt"; 
const string HISTORY_FILE = "history.txt"; 
 
// Multiple Buffers 
map<int, TempBuffer> buffers; 
 
// --------------------------------------------------------------------------- 
// Utility Functions 
// --------------------------------------------------------------------------- 
bool isUniqueId(int id) { 
    ifstream fin(PATIENT_FILE); 
    if (!fin) return true; 
    Patient p; 
    while (fin >> p.id >> p.name >> p.phone >> p.dob >> p.age) 
{ 
        if (p.id == id) return false; 
    } 
    fin.close(); 
    return true; 
} 
 
bool patientExists(int id) { 
    ifstream fin(PATIENT_FILE); 
    if (!fin) return false; 
    Patient p; 
 
 
11 | Page  
    while (fin >> p.id >> p.name >> p.phone >> p.dob >> p.age) 
{ 
        if (p.id == id) return true; 
    } 
    fin.close(); 
    return false; 
} 
 
bool isUniquePhone(const string &phone) { 
    ifstream fin(PATIENT_FILE); 
    if (!fin) return true; 
    Patient p; 
    while (fin >> p.id >> p.name >> p.phone >> p.dob >> p.age) 
{ 
        if (p.phone == phone) return false; 
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
 
 
12 | Page  
    queue<int> temp3 = inpatientQueue; 
    while (!temp3.empty()) { 
        if (temp3.front() == id) return true; 
        temp3.pop(); 
    } 
    return false; 
} 
 
// --------------------------------------------------------------------------- 
// 1. Add Patient 
// --------------------------------------------------------------------------- 
void addPatient() { 
    Patient p; 
    cout << "\nEnter Patient ID: "; 
    cin >> p.id; 
 
    if (!isUniqueId(p.id)) { 
        cout << "Patient ID already exists.\n"; 
        return; 
    } 
 
    cout << "Enter Name: "; 
    cin >> p.name; 
    cout << "Enter Phone Number: "; 
    cin >> p.phone; 
 
    if (!isUniquePhone(p.phone)) { 
        cout << "Phone number already exists.\n"; 
        return; 
    } 
 
 
 
13 | Page  
    cout << "Enter DOB (dd-mm-yyyy): "; 
    cin >> p.dob; 
    cout << "Enter Age: "; 
    cin >> p.age; 
 
    ofstream fout(PATIENT_FILE, ios::app); 
    fout << p.id << " " << p.name << " " << p.phone << " " << 
p.dob << " " << p.age << "\n"; 
    fout.close(); 
 
    cout << "Patient added successfully.\n"; 
} 
 
// --------------------------------------------------------------------------- 
// 2. Search Patient 
// --------------------------------------------------------------------------- 
void searchPatient() { 
    string phone; 
    cout << "\nEnter phone number to search: "; 
    cin >> phone; 
 
    ifstream fin(PATIENT_FILE); 
    if (!fin) { 
        cout << "No patient records found.\n"; 
        return; 
    } 
 
    Patient p; 
    bool found = false; 
    while (fin >> p.id >> p.name >> p.phone >> p.dob >> p.age) 
{ 
 
 
14 | Page  
        if (p.phone == phone) { 
            cout << "\nPatient Found:\n"; 
            cout << "ID: " << p.id << "\nName: " << p.name << 
endl; 
            found = true; 
            break; 
        } 
    } 
    fin.close(); 
 
    if (!found) 
        cout << "No record found for phone number: " << phone 
<< endl; 
} 
 
// --------------------------------------------------------------------------- 
// 3. Process Patient 
// --------------------------------------------------------------------------- 
void processPatient() { 
    int id; 
    char type; 
    cout << "\nEnter Patient ID to process: "; 
    cin >> id; 
 
    if (!patientExists(id)) { 
        cout << "This patient is not registered. Please add the 
patient first.\n"; 
        return; 
    } 
 
    cout << "Type of patient (a - Outpatient, b - Emergency): "; 
 
 
15 | Page  
    cin >> type; 
    type = tolower(type); 
 
    if (type == 'a') { 
        outpatientQueue.push(id); 
        cout << "Added to Outpatient Queue.\n"; 
    } else if (type == 'b') { 
        emergencyQueue.push(id); 
        inpatientQueue.push(id); 
        cout << "Added to Emergency and Inpatient Queues.\n"; 
    } else { 
        cout << "Invalid type.\n"; 
    } 
} 
 
// --------------------------------------------------------------------------- 
// Date & History Functions 
// --------------------------------------------------------------------------- 
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
 
 
16 | Page  
    fout << id << "|" << getCurrentDateTime() << "|" << record 
<< "\n"; 
    fout.close(); 
    return true; 
} 
 
// --------------------------------------------------------------------------- 
// 7. Emergency Buffer with multiple Temp IDs 
// --------------------------------------------------------------------------- 
void emergencyBufferStart() { 
    int tempId; 
    cout << "\nEnter Temporary Buffer ID: "; 
    cin >> tempId; 
 
    if (buffers[tempId].active) { 
        cout << "This temporary buffer ID already exists. Choose 
a different one.\n"; 
        return; 
    } 
 
    TempBuffer &buf = buffers[tempId]; 
    buf.active = true; 
    buf.isEmergency = true; 
    buf.processed = true; 
    buf.inpatient = true; 
    buf.records.clear(); 
    buf.medAmount = 0; 
    buf.daysAdmitted = 0; 
 
    cout << "\n*** Emergency Buffer Activated for Temp ID " 
<< tempId << " ***\n"; 
 
 
17 | Page  
    cin.ignore(); 
    while (true) { 
        string treatment; 
        cout << "\nEnter treatment name for buffer (or just 
press ENTER to stop): "; 
        getline(cin, treatment); 
        if (treatment.empty()) break; 
        string rec = string("Emergency Treatment: ") + 
treatment; 
        buf.records.push_back(rec); 
        cout << "Added to buffer.\n"; 
    } 
    cout << "\nBuffer setup complete for Temp ID " << tempId 
<< ".\n"; 
} 
 
// --------------------------------------------------------------------------- 
// Medication for normal patients 
// --------------------------------------------------------------------------- 
void medication() { 
    int id; 
    cout << "\nEnter Patient ID for medication: "; 
    cin >> id; 
    cin.ignore(); 
 
    if (!patientExists(id)) { 
        cout << "This patient is not registered. Please add 
first.\n"; 
        return; 
    } 
 
 
 
18 | Page  
    if (!isProcessed(id)) { 
        cout << "Patient not yet processed. Please process 
before medication.\n"; 
        return; 
    } 
 
    bool isEmergency = false; 
    priority_queue<int> tempE = emergencyQueue; 
    while (!tempE.empty()) { 
        if (tempE.top() == id) { 
            isEmergency = true; 
            break; 
        } 
        tempE.pop(); 
    } 
 
    if (isEmergency) { 
        cout << "\n*** Emergency Patient Detected ***\n"; 
        string treatment; 
        cout << "Enter treatment name: "; 
        getline(cin, treatment); 
 
        stringstream record; 
        record << "Emergency Treatment: " << treatment; 
 
        appendHistoryRecord(id, record.str()); 
 
        bool alreadyInQueue = false; 
        queue<int> tempQ = inpatientQueue; 
        while (!tempQ.empty()) { 
            if (tempQ.front() == id) { 
 
 
19 | Page  
                alreadyInQueue = true; 
                break; 
            } 
            tempQ.pop(); 
        } 
        if (!alreadyInQueue) inpatientQueue.push(id); 
 
        cout << "Emergency treatment record saved. Patient 
added to inpatient queue.\n"; 
        return; 
    } 
 
    string pres; 
    cout << "Enter doctor's prescription: "; 
    getline(cin, pres); 
 
    appendHistoryRecord(id, pres); 
 
    char ch; 
    cout << "Enter 'c' if patient is inpatient: "; 
    cin >> ch; 
    if (tolower(ch) == 'c') { 
        inpatientQueue.push(id); 
        cout << "Patient added to Inpatient Queue.\n"; 
    } 
 
    cout << "Prescription saved successfully.\n"; 
} 
 
// --------------------------------------------------------------------------- 
// Get latest prescription 
 
 
20 | Page  
// --------------------------------------------------------------------------- 
string getLatestPrescription(int id) { 
    ifstream fin(HISTORY_FILE); 
    if (!fin) return "No prescription found."; 
 
    string line, lastPres; 
    while (getline(fin, line)) { 
        if (line.find(to_string(id) + "|") == 0 && line.find("Billed") 
== string::npos) 
            lastPres = line; 
    } 
    fin.close(); 
 
    if (lastPres.empty()) return "No prescription found."; 
    size_t pos = lastPres.find_last_of('|'); 
    return (pos != string::npos) ? lastPres.substr(pos + 1) : "No 
prescription found."; 
} 
 
// --------------------------------------------------------------------------- 
// Billing For ID 
// --------------------------------------------------------------------------- 
void billingForId(int id) { 
    if (!patientExists(id)) { 
        cout << "This patient is not registered. Please add the 
patient first.\n"; 
        return; 
    } 
 
    if (!isProcessed(id)) { 
        cout << "Patient not processed yet. Please process 
 
 
21 | Page  
before billing.\n"; 
        return; 
    } 
 
    string pres = getLatestPrescription(id); 
    if (pres == "No prescription found.") { 
        cout << "\nBilling not possible — no prescription found 
for this patient.\n"; 
        cout << "Please record medication first.\n"; 
        return; 
    } 
 
    cout << "\n--- Prescription / Treatment ---\n" << pres << 
"\n"; 
 
    bool isInpatient = false; 
    queue<int> tempQ = inpatientQueue; 
    while (!tempQ.empty()) { 
        if (tempQ.front() == id) { 
            isInpatient = true; 
            break; 
        } 
        tempQ.pop(); 
    } 
 
    double total = 0, medAmount = 0; 
 
    if (isInpatient) { 
        int days; 
        cout << "Enter number of days admitted: "; 
        cin >> days; 
 
 
22 | Page  
        cout << "Enter medication amount: "; 
        cin >> medAmount; 
        total = (1000 * days) + medAmount + (800 * days); 
        cout << "\nInpatient Bill: " << total << endl; 
    } else { 
        cout << "Enter medication amount: "; 
        cin >> medAmount; 
        total = medAmount + 500; 
        cout << "\nOutpatient Bill: " << total << endl; 
    } 
 
    queue<int> tempOut; 
    while (!outpatientQueue.empty()) { 
        if (outpatientQueue.front() != id) 
            tempOut.push(outpatientQueue.front()); 
        outpatientQueue.pop(); 
    } 
    outpatientQueue = tempOut; 
 
    priority_queue<int> tempPQ; 
    while (!emergencyQueue.empty()) { 
        if (emergencyQueue.top() != id) 
            tempPQ.push(emergencyQueue.top()); 
        emergencyQueue.pop(); 
    } 
    emergencyQueue = tempPQ; 
 
    queue<int> tempIn; 
    while (!inpatientQueue.empty()) { 
        if (inpatientQueue.front() != id) 
            tempIn.push(inpatientQueue.front()); 
 
 
23 | Page  
        inpatientQueue.pop(); 
    } 
    inpatientQueue = tempIn; 
 
    stringstream billRecord; 
    billRecord << "Billed " << total << (isInpatient ? " 
(Inpatient)" : " (Outpatient)"); 
    appendHistoryRecord(id, billRecord.str()); 
 
    cout << "\nBilling completed and patient removed from all 
queues.\n"; 
} 
 
void billing() { 
    int id; 
    cout << "\nEnter Patient ID for billing: "; 
    cin >> id; 
    billingForId(id); 
} 
 
// --------------------------------------------------------------------------- 
// 8. Assign Buffer to Patient (Temp ID + Patient ID) 
// --------------------------------------------------------------------------- 
void assignBufferToPatient() { 
    int tempId; 
    cout << "\nEnter Temporary Buffer ID to assign: "; 
    cin >> tempId; 
 
    if (!buffers[tempId].active) { 
        cout << "No active buffer found with this Temp ID.\n"; 
        return; 
 
 
24 | Page  
    } 
 
    int id; 
    cout << "Enter Patient ID to assign buffer to: "; 
    cin >> id; 
    if (!patientExists(id)) { 
        cout << "This patient is not registered. Please add the 
patient first.\n"; 
        return; 
    } 
 
    TempBuffer &buf = buffers[tempId]; 
    for (const string &rec : buf.records) { 
        appendHistoryRecord(id, rec); 
    } 
 
    if (buf.inpatient) inpatientQueue.push(id); 
    if (buf.isEmergency) emergencyQueue.push(id); 
 
    cout << "\nBuffer records assigned to patient ID " << id << 
".\n"; 
 
    cout << "\nNow running billing for patient ID " << id << 
"...\n"; 
    billingForId(id); 
 
    buffers.erase(tempId); 
    cout << "\nTemporary buffer " << tempId << " deleted.\n"; 
} 
 
// --------------------------------------------------------------------------- 
 
 
25 | Page  
// 6. Search History 
// --------------------------------------------------------------------------- 
void searchHistory() { 
    int id; 
    cout << "\nEnter patient ID to view history: "; 
    cin >> id; 
 
    ifstream fin(HISTORY_FILE); 
    if (!fin) { 
        cout << "No history records available.\n"; 
        return; 
    } 
 
    string line; 
    bool found = false; 
    cout << "\n--- History for Patient ID " << id << " ---\n"; 
    while (getline(fin, line)) { 
        if (line.empty()) continue; 
        stringstream ss(line); 
        string sid, date, record; 
        getline(ss, sid, '|'); 
        getline(ss, date, '|'); 
        getline(ss, record); 
 
        try { 
            if (!sid.empty() && stoi(sid) == id) { 
                cout << "Date: " << date << "\nRecord: " << record 
<< "\n\n"; 
                found = true; 
            } 
        } catch (...) { 
 
 
26 | Page  
            continue; 
        } 
    } 
    fin.close(); 
 
    if (!found) 
        cout << "No history found for Patient ID: " << id << endl; 
} 
 
// --------------------------------------------------------------------------- 
// Show All Buffer Status (Fixed Version) 
// --------------------------------------------------------------------------- 
void showBufferStatus() { 
    if (buffers.empty()) { 
        cout << "\nNo temporary buffers available.\n"; 
        return; 
    } 
 
    cout << "\n--- Temporary Buffers ---\n"; 
    for (auto it = buffers.begin(); it != buffers.end(); ++it) { 
        int tempId = it->first; 
        const TempBuffer &buf = it->second; 
 
        cout << "Temp ID: " << tempId 
             << " | Active: " << (buf.active ? "Yes" : "No") 
             << " | Emergency: " << (buf.isEmergency ? "Yes" : 
"No") 
             << " | Inpatient: " << (buf.inpatient ? "Yes" : "No") 
             << " | Records: " << buf.records.size() 
             << " | Med Amount: " << buf.medAmount 
             << " | Days Admitted: " << buf.daysAdmitted 
 
 
27 | Page  
             << endl; 
    } 
} 
// --------------------------------------------------------------------------- 
// Main Menu 
// --------------------------------------------------------------------------- 
int main() { 
    int choice; 
    do { 
        cout << "\n====== HOSPITAL MANAGEMENT SYSTEM 
======\n"; 
        cout << "1. Add Patient\n"; 
        cout << "2. Search Patient\n"; 
        cout << "3. Process Patient\n"; 
        cout << "4. Medication (by Patient ID)\n"; 
        cout << "5. Billing\n"; 
        cout << "6. Search History\n"; 
        cout << "7. Emergency Buffer (no ID - temporary)\n"; 
        cout << "8. Assign Buffer to Patient (Temp ID -> Patient 
ID)\n"; 
        cout << "9. Exit\n"; 
        cout << "Enter your choice: "; 
        cin >> choice; 
 
        switch (choice) { 
            case 1: addPatient(); break; 
            case 2: searchPatient(); break; 
            case 3: processPatient(); break; 
            case 4: medication(); break; 
            case 5: billing(); break; 
            case 6: searchHistory(); break; 
 
 
28 | Page  
            case 7: emergencyBufferStart(); break; 
            case 8: assignBufferToPatient(); break; 
            case 9: cout << "Exiting program...\n"; break; 
            default: cout << "Invalid choice. Try again.\n"; 
        } 
 
        if (choice >= 1 && choice <= 9 && choice != 9) { 
            showBufferStatus(); 
        } 
 
    } while (choice != 9); 
 
    return 0; 
}