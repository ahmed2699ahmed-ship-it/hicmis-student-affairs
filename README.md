# منظومة HICMIS الذكية لشؤون الطلاب والقيد الأكاديمي

نظام Full-Stack متكامل مبني بلغة **C++** كخادم ويب عالي الأداء (Backend) وواجهة ويب عربية متجاوبة (**HTML5 / CSS3 / Vanilla JS**) مع تخزين دائم في ملفات نصية (`Zero-Data-Loss`).

## هيكل المشروع
- `server.cpp`: خادم الويب (HTTP Server) مبني بـ Sockets يدعم `GET` و `POST` لحفظ واسترجاع البيانات لحظياً.
- `Models.h`: تعريف كلاسات البيانات (`StudentProfile`, `GradeRecord`, `FeeRecord`).
- `StudentAffairsEngine.h`: محرك إدارة البيانات والقراءة والكتابة الفورية في الملفات النصية.
- `students_db.txt` / `grades_db.txt` / `fees_db.txt`: قواعد البيانات النصية الفارغة الجاهزة لاستقبال بياناتك الحقيقية.
- `index.html`, `css/style.css`, `js/app.js`: واجهة النظام (بوابة شؤون الطلاب + بوابة الطالب).

## طريقة التشغيل على Visual Studio Code
1. افتح مجلد المشروع في **VS Code**.
2. افتح الـ Terminal (`Ctrl + ~`).
3. **على نظام Windows (MinGW/GCC):**
   ```bash
   g++ -O3 server.cpp -o server.exe -lws2_32
   ./server.exe
   ```
4. **على نظام Linux / Ubuntu:**
   ```bash
   g++ -O3 server.cpp -o server
   ./server
   ```
5. افتح المتصفح على الرابط: `http://localhost:8080`

## بيانات الدخول الافتراضية للإدارة
- **اسم المستخدم:** `admin`
- **كلمة المرور:** `123456`
- *(بعد تسجيل أي طالب من لوحة الإدارة، يستطيع الطالب الدخول فوراً باستخدام الكود الأكاديمي والرقم القومي الخاص به).*
