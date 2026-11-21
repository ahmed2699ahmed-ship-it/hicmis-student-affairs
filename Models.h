#ifndef MODELS_H
#define MODELS_H

#include <string>
#include <vector>

using namespace std;

// 1. كلاس يمثل الطالب وبيانات القيد
class StudentProfile {
public:
    string cardId;       // الكود الأكاديمي / رقم الكارنيه
    string nationalId;   // الرقم القومي (14 رقم - يستخدم ككلمة مرور للطالب)
    string seatNumber;   // رقم الجلوس
    string name;         // الاسم الرباعي
    string academicYear; // الفرقة الدراسية
    string department;   // الشعبة
    string phone;        // رقم الهاتف
    string status;       // حالة القيد (مقيد، مستجد، باق للإعادة، منقول بمواد)

    StudentProfile() {}
    StudentProfile(string cId, string nid, string sNum, string n, string yr, string dep, string ph, string st)
        : cardId(cId), nationalId(nid), seatNumber(sNum), name(n), academicYear(yr), department(dep), phone(ph), status(st) {}

    string toRecord() const {
        return cardId + "|" + nationalId + "|" + seatNumber + "|" + name + "|" + 
               academicYear + "|" + department + "|" + phone + "|" + status;
    }

    string toJson() const {
        return "{\"cardId\":\"" + cardId + "\",\"nationalId\":\"" + nationalId + 
               "\",\"seatNumber\":\"" + seatNumber + "\",\"name\":\"" + name + 
               "\",\"academicYear\":\"" + academicYear + "\",\"department\":\"" + department + 
               "\",\"phone\":\"" + phone + "\",\"status\":\"" + status + "\"}";
    }
};

// 2. كلاس يمثل نتيجة مقرر دراسي
class GradeRecord {
public:
    string id;           // معرف السجل
    string cardId;       // الكود الأكاديمي للطالب
    string courseCode;   // كود المقرر
    string courseName;   // اسم المادة
    int maxGrade;        // الدرجة العظمى
    int studentGrade;    // درجة الطالب
    string gradeLetter;  // التقدير (ممتاز، جيد جداً، جيد، مقبول، ضعيف)

    GradeRecord() : maxGrade(100), studentGrade(0) {}
    GradeRecord(string recId, string cId, string cCode, string cName, int maxG, int studG, string letter)
        : id(recId), cardId(cId), courseCode(cCode), courseName(cName), maxGrade(maxG), studentGrade(studG), gradeLetter(letter) {}

    string toRecord() const {
        return id + "|" + cardId + "|" + courseCode + "|" + courseName + "|" + 
               to_string(maxGrade) + "|" + to_string(studentGrade) + "|" + gradeLetter;
    }

    string toJson() const {
        return "{\"id\":\"" + id + "\",\"cardId\":\"" + cardId + "\",\"courseCode\":\"" + courseCode + 
               "\",\"courseName\":\"" + courseName + "\",\"maxGrade\":" + to_string(maxGrade) + 
               ",\"studentGrade\":" + to_string(studentGrade) + ",\"gradeLetter\":\"" + gradeLetter + "\"}";
    }
};

// 3. كلاس يمثل الرسوم الدراسية والمصروفات
class FeeRecord {
public:
    string id;           // رقم الإيصال
    string cardId;       // الكود الأكاديمي للطالب
    string academicYear; // العام الدراسي
    double totalAmount;  // إجمالي المصروفات المطلوبة
    double paidAmount;   // المبلغ المسدد
    string paymentDate;  // تاريخ السداد
    string status;       // حالة السداد (مسدد بالكامل، قسط أول، غير مسدد)

    FeeRecord() : totalAmount(0), paidAmount(0) {}
    FeeRecord(string fId, string cId, string yr, double tot, double paid, string pDate, string st)
        : id(fId), cardId(cId), academicYear(yr), totalAmount(tot), paidAmount(paid), paymentDate(pDate), status(st) {}

    string toRecord() const {
        return id + "|" + cardId + "|" + academicYear + "|" + to_string(totalAmount) + "|" + 
               to_string(paidAmount) + "|" + paymentDate + "|" + status;
    }

    string toJson() const {
        return "{\"id\":\"" + id + "\",\"cardId\":\"" + cardId + "\",\"academicYear\":\"" + academicYear + 
               "\",\"totalAmount\":" + to_string(totalAmount) + ",\"paidAmount\":" + to_string(paidAmount) + 
               ",\"paymentDate\":\"" + paymentDate + "\",\"status\":\"" + status + "\"}";
    }
};

#endif
