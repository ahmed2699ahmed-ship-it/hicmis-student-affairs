#ifndef STUDENT_AFFAIRS_ENGINE_H
#define STUDENT_AFFAIRS_ENGINE_H

#include "Models.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cstdlib>

using namespace std;

class StudentAffairsEngine {
private:
    vector<StudentProfile> students;
    vector<GradeRecord> grades;
    vector<FeeRecord> fees;

    string studentsDbPath;
    string gradesDbPath;
    string feesDbPath;

    vector<string> split(const string& str, char delimiter) {
        vector<string> tokens;
        string token;
        istringstream tokenStream(str);
        while (getline(tokenStream, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }

    // تنظيف النصوص من الرموز التي قد تكسر الفاصل | أو الـ JSON
    string sanitize(string s) {
        replace(s.begin(), s.end(), '|', '-');
        replace(s.begin(), s.end(), '"', '\'');
        replace(s.begin(), s.end(), '\n', ' ');
        replace(s.begin(), s.end(), '\r', ' ');
        return s;
    }

public:
    StudentAffairsEngine(string sPath = "students_db.txt", string gPath = "grades_db.txt", string fPath = "fees_db.txt")
        : studentsDbPath(sPath), gradesDbPath(gPath), feesDbPath(fPath) {
        loadDataFromDisk();
    }

    // تحميل البيانات الحقيقية من الهارد ديسك (بدون فرض أي بيانات وهمية)
    void loadDataFromDisk() {
        students.clear();
        ifstream sFile(studentsDbPath.c_str());
        string line;
        while (getline(sFile, line)) {
            if (line.empty()) continue;
            auto parts = split(line, '|');
            if (parts.size() >= 8) {
                students.push_back(StudentProfile(parts[0], parts[1], parts[2], parts[3], parts[4], parts[5], parts[6], parts[7]));
            }
        }
        sFile.close();

        grades.clear();
        ifstream gFile(gradesDbPath.c_str());
        while (getline(gFile, line)) {
            if (line.empty()) continue;
            auto parts = split(line, '|');
            if (parts.size() >= 7) {
                grades.push_back(GradeRecord(parts[0], parts[1], parts[2], parts[3], atoi(parts[4].c_str()), atoi(parts[5].c_str()), parts[6]));
            }
        }
        gFile.close();

        fees.clear();
        ifstream fFile(feesDbPath.c_str());
        while (getline(fFile, line)) {
            if (line.empty()) continue;
            auto parts = split(line, '|');
            if (parts.size() >= 7) {
                fees.push_back(FeeRecord(parts[0], parts[1], parts[2], atof(parts[3].c_str()), atof(parts[4].c_str()), parts[5], parts[6]));
            }
        }
        fFile.close();
    }

    // الحفظ اللحظي على الهارد ديسك (Zero-Data-Loss)
    void persistStudents() {
        ofstream sFile(studentsDbPath.c_str());
        for (const auto& s : students) {
            sFile << s.toRecord() << "\n";
        }
        sFile.close();
    }

    void persistGrades() {
        ofstream gFile(gradesDbPath.c_str());
        for (const auto& g : grades) {
            gFile << g.toRecord() << "\n";
        }
        gFile.close();
    }

    void persistFees() {
        ofstream fFile(feesDbPath.c_str());
        for (const auto& f : fees) {
            fFile << f.toRecord() << "\n";
        }
        fFile.close();
    }

    // عمليات الطلاب (إضافة، تعديل، حذف)
    bool addOrUpdateStudent(StudentProfile s) {
        s.cardId = sanitize(s.cardId);
        s.nationalId = sanitize(s.nationalId);
        s.seatNumber = sanitize(s.seatNumber);
        s.name = sanitize(s.name);
        s.academicYear = sanitize(s.academicYear);
        s.department = sanitize(s.department);
        s.phone = sanitize(s.phone);
        s.status = sanitize(s.status);

        for (auto& existing : students) {
            if (existing.cardId == s.cardId) {
                existing = s;
                persistStudents();
                return true;
            }
        }
        students.push_back(s);
        persistStudents();
        return true;
    }

    bool deleteStudent(const string& cardId) {
        auto it = remove_if(students.begin(), students.end(), [&](const StudentProfile& s) {
            return s.cardId == cardId;
        });
        if (it != students.end()) {
            students.erase(it, students.end());
            persistStudents();
            return true;
        }
        return false;
    }

    // عمليات الدرجات
    bool addGrade(GradeRecord g) {
        g.id = sanitize(g.id);
        g.cardId = sanitize(g.cardId);
        g.courseCode = sanitize(g.courseCode);
        g.courseName = sanitize(g.courseName);
        g.gradeLetter = sanitize(g.gradeLetter);

        grades.push_back(g);
        persistGrades();
        return true;
    }

    bool deleteGrade(const string& id) {
        auto it = remove_if(grades.begin(), grades.end(), [&](const GradeRecord& g) {
            return g.id == id;
        });
        if (it != grades.end()) {
            grades.erase(it, grades.end());
            persistGrades();
            return true;
        }
        return false;
    }

    // عمليات المصروفات
    bool addFee(FeeRecord f) {
        f.id = sanitize(f.id);
        f.cardId = sanitize(f.cardId);
        f.academicYear = sanitize(f.academicYear);
        f.paymentDate = sanitize(f.paymentDate);
        f.status = sanitize(f.status);

        fees.push_back(f);
        persistFees();
        return true;
    }

    bool deleteFee(const string& id) {
        auto it = remove_if(fees.begin(), fees.end(), [&](const FeeRecord& f) {
            return f.id == id;
        });
        if (it != fees.end()) {
            fees.erase(it, fees.end());
            persistFees();
            return true;
        }
        return false;
    }

    // تصدير JSON
    string getStudentsJson() const {
        string json = "[";
        for (size_t i = 0; i < students.size(); ++i) {
            json += students[i].toJson();
            if (i + 1 < students.size()) json += ",";
        }
        json += "]";
        return json;
    }

    string getGradesJson() const {
        string json = "[";
        for (size_t i = 0; i < grades.size(); ++i) {
            json += grades[i].toJson();
            if (i + 1 < grades.size()) json += ",";
        }
        json += "]";
        return json;
    }

    string getFeesJson() const {
        string json = "[";
        for (size_t i = 0; i < fees.size(); ++i) {
            json += fees[i].toJson();
            if (i + 1 < fees.size()) json += ",";
        }
        json += "]";
        return json;
    }

    size_t getStudentsCount() const { return students.size(); }
    size_t getGradesCount() const { return grades.size(); }
    size_t getFeesCount() const { return fees.size(); }
};

#endif
