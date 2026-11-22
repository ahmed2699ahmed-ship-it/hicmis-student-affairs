#include "StudentAffairsEngine.h"
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <algorithm>
#include <map>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
    #define CLOSE_SOCKET closesocket
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    #define SOCKET int
    #define INVALID_SOCKET -1
    #define CLOSE_SOCKET close
#endif

using namespace std;

StudentAffairsEngine engine;

string readFileBinary(string filePath) {
    #ifdef _WIN32
    replace(filePath.begin(), filePath.end(), '/', '\\');
    #endif
    ifstream file(filePath.c_str(), ios::binary);
    if (!file.is_open()) return "";
    stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

string getContentType(const string& path) {
    if (path.rfind(".html") != string::npos) return "text/html; charset=UTF-8";
    if (path.rfind(".css") != string::npos)  return "text/css; charset=UTF-8";
    if (path.rfind(".js") != string::npos)   return "application/javascript; charset=UTF-8";
    if (path.rfind(".json") != string::npos) return "application/json; charset=UTF-8";
    if (path.rfind(".jpeg") != string::npos || path.rfind(".jpg") != string::npos) return "image/jpeg";
    if (path.rfind(".png") != string::npos)  return "image/png";
    if (path.rfind(".svg") != string::npos)  return "image/svg+xml";
    return "text/plain";
}

// فك تشفير URL (UTF-8 / Arabic Support)
string urlDecode(const string& str) {
    string ret;
    char ch;
    int i, ii;
    for (i = 0; i < (int)str.length(); i++) {
        if (str[i] == '%') {
            if (i + 2 < (int)str.length()) {
                sscanf(str.substr(i + 1, 2).c_str(), "%x", &ii);
                ch = static_cast<char>(ii);
                ret += ch;
                i = i + 2;
            }
        } else if (str[i] == '+') {
            ret += ' ';
        } else {
            ret += str[i];
        }
    }
    return ret;
}

// تحليل بيانات الفورم المرسلة (key1=val1&key2=val2)
map<string, string> parseFormBody(const string& body) {
    map<string, string> params;
    istringstream ss(body);
    string pair;
    while (getline(ss, pair, '&')) {
        size_t eq = pair.find('=');
        if (eq != string::npos) {
            string key = urlDecode(pair.substr(0, eq));
            string val = urlDecode(pair.substr(eq + 1));
            params[key] = val;
        }
    }
    return params;
}

void sendJsonResponse(SOCKET client, const string& jsonData, int statusCode = 200) {
    string statusText = (statusCode == 200) ? "200 OK" : "400 Bad Request";
    string response = "HTTP/1.1 " + statusText + "\r\n"
                      "Content-Type: application/json; charset=UTF-8\r\n"
                      "Access-Control-Allow-Origin: *\r\n"
                      "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
                      "Access-Control-Allow-Headers: Content-Type\r\n"
                      "Content-Length: " + to_string(jsonData.length()) + "\r\n"
                      "Connection: close\r\n\r\n" + jsonData;
    send(client, response.c_str(), (int)response.length(), 0);
}

int main() {
    int port = 8080;
    const char* envPort = getenv("PORT");
    if (envPort != NULL) {
        port = atoi(envPort);
    }

#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif

    SOCKET server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == INVALID_SOCKET) return 1;

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) return 1;
    listen(server_fd, 25);

    cout << "====================================================================\n";
    cout << "   HICMIS Student Affairs Engine - High-Performance C++ Backend     \n";
    cout << "   Author: Mennatullah Ragab, Youssef Ahmed, Habiba Abdullahtullah Ragab | Computer Systems & MIS                     \n";
    cout << "====================================================================\n";
    cout << " [*] C++ Core Engine: READY (Loaded: " << engine.getStudentsCount() << " Students, " 
         << engine.getGradesCount() << " Grades, " << engine.getFeesCount() << " Fee Records)\n";
    cout << " [*] Persistent Storage: students_db.txt | grades_db.txt | fees_db.txt\n";
    cout << " [*] Listening on Port: " << port << " (http://localhost:" << port << ")\n";
    cout << "====================================================================\n\n";

    while (true) {
        sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        SOCKET client = accept(server_fd, (struct sockaddr*)&clientAddr, &clientLen);
        if (client == INVALID_SOCKET) continue;

        char buffer[16384] = {0};
        int bytesRead = recv(client, buffer, sizeof(buffer) - 1, 0);
        if (bytesRead <= 0) {
            CLOSE_SOCKET(client);
            continue;
        }

        string request(buffer, bytesRead);

        // قراءة بقية الـ Body إذا كان الطلب POST
        size_t headerEnd = request.find("\r\n\r\n");
        string headers = (headerEnd != string::npos) ? request.substr(0, headerEnd) : request;
        string body = (headerEnd != string::npos) ? request.substr(headerEnd + 4) : "";

        size_t clPos = headers.find("Content-Length:");
        if (clPos == string::npos) clPos = headers.find("content-length:");
        if (clPos != string::npos) {
            int contentLength = atoi(headers.substr(clPos + 15).c_str());
            while ((int)body.length() < contentLength) {
                char extra[4096] = {0};
                int r = recv(client, extra, sizeof(extra) - 1, 0);
                if (r <= 0) break;
                body.append(extra, r);
            }
        }

        size_t firstSpace = headers.find(' ');
        size_t secondSpace = headers.find(" HTTP/");
        string method = (firstSpace != string::npos) ? headers.substr(0, firstSpace) : "GET";
        string path = "/";

        if (firstSpace != string::npos && secondSpace != string::npos) {
            path = headers.substr(firstSpace + 1, secondSpace - (firstSpace + 1));
        }

        size_t qPos = path.find('?');
        if (qPos != string::npos) path = path.substr(0, qPos);
        if (path == "/" || path.empty()) path = "/index.html";

        if (method == "OPTIONS") {
            sendJsonResponse(client, "{}");
        }
        // ================= مسارات الـ API =================
        else if (method == "GET" && path == "/api/students") {
            sendJsonResponse(client, engine.getStudentsJson());
        }
        else if (method == "GET" && path == "/api/grades") {
            sendJsonResponse(client, engine.getGradesJson());
        }
        else if (method == "GET" && path == "/api/fees") {
            sendJsonResponse(client, engine.getFeesJson());
        }
        // إضافة أو تحديث طالب وحفظه فوراً في students_db.txt
        else if (method == "POST" && path == "/api/students") {
            auto p = parseFormBody(body);
            if (!p["cardId"].empty() && !p["name"].empty()) {
                StudentProfile s(p["cardId"], p["nationalId"], p["seatNumber"], p["name"],
                                 p["academicYear"], p["department"], p["phone"], p["status"]);
                engine.addOrUpdateStudent(s);
                cout << "[+] Saved Student: " << s.cardId << " - " << s.name << "\n";
                sendJsonResponse(client, "{\"success\":true}");
            } else {
                sendJsonResponse(client, "{\"success\":false,\"error\":\"Missing fields\"}", 400);
            }
        }
        // حذف طالب
        else if (method == "POST" && path == "/api/students/delete") {
            auto p = parseFormBody(body);
            bool ok = engine.deleteStudent(p["cardId"]);
            sendJsonResponse(client, ok ? "{\"success\":true}" : "{\"success\":false}");
        }
        // إضافة درجة وحفظها فوراً في grades_db.txt
        else if (method == "POST" && path == "/api/grades") {
            auto p = parseFormBody(body);
            if (!p["cardId"].empty() && !p["courseCode"].empty()) {
                GradeRecord g(p["id"], p["cardId"], p["courseCode"], p["courseName"],
                              atoi(p["maxGrade"].c_str()), atoi(p["studentGrade"].c_str()), p["gradeLetter"]);
                engine.addGrade(g);
                cout << "[+] Saved Grade for Student: " << g.cardId << " (" << g.courseCode << ")\n";
                sendJsonResponse(client, "{\"success\":true}");
            } else {
                sendJsonResponse(client, "{\"success\":false}", 400);
            }
        }
        // حذف درجة
        else if (method == "POST" && path == "/api/grades/delete") {
            auto p = parseFormBody(body);
            bool ok = engine.deleteGrade(p["id"]);
            sendJsonResponse(client, ok ? "{\"success\":true}" : "{\"success\":false}");
        }
        // إضافة إيصال مصروفات وحفظه فوراً في fees_db.txt
        else if (method == "POST" && path == "/api/fees") {
            auto p = parseFormBody(body);
            if (!p["cardId"].empty() && !p["id"].empty()) {
                FeeRecord f(p["id"], p["cardId"], p["academicYear"],
                            atof(p["totalAmount"].c_str()), atof(p["paidAmount"].c_str()),
                            p["paymentDate"], p["status"]);
                engine.addFee(f);
                cout << "[+] Saved Fee Receipt: " << f.id << " for " << f.cardId << "\n";
                sendJsonResponse(client, "{\"success\":true}");
            } else {
                sendJsonResponse(client, "{\"success\":false}", 400);
            }
        }
        // حذف إيصال مصروفات
        else if (method == "POST" && path == "/api/fees/delete") {
            auto p = parseFormBody(body);
            bool ok = engine.deleteFee(p["id"]);
            sendJsonResponse(client, ok ? "{\"success\":true}" : "{\"success\":false}");
        }
        // ================= مسار الملفات الثابتة =================
        else {
            string localPath = (path[0] == '/') ? path.substr(1) : path;
            string fileContent = readFileBinary(localPath);
            if (!fileContent.empty()) {
                string mime = getContentType(localPath);
                string response = "HTTP/1.1 200 OK\r\nContent-Type: " + mime + 
                                  "\r\nContent-Length: " + to_string(fileContent.length()) + 
                                  "\r\nAccess-Control-Allow-Origin: *\r\nConnection: close\r\n\r\n" + fileContent;
                send(client, response.c_str(), (int)response.length(), 0);
            } else {
                string notFound = "404 Not Found on HICMIS C++ Server";
                string response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: " + 
                                  to_string(notFound.length()) + "\r\nConnection: close\r\n\r\n" + notFound;
                send(client, response.c_str(), (int)response.length(), 0);
            }
        }

        CLOSE_SOCKET(client);
    }

    CLOSE_SOCKET(server_fd);
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
