let students = [];
let grades = [];
let fees = [];
let loginRole = 'student'; // 'student' أو 'admin'
let currentStudent = null;

// التبديل بين تبويب دخول الطالب والإدارة
const tabStudentBtn = document.getElementById('tab-student-btn');
const tabAdminBtn = document.getElementById('tab-admin-btn');
const usernameInput = document.getElementById('username');
const passwordInput = document.getElementById('password');

tabStudentBtn.addEventListener('click', () => {
    loginRole = 'student';
    tabStudentBtn.classList.add('active-tab');
    tabAdminBtn.classList.remove('active-tab');
    usernameInput.placeholder = "الكود الأكاديمي / رقم الكارنيه";
    passwordInput.placeholder = "الرقم القومي (14 رقم)";
});

tabAdminBtn.addEventListener('click', () => {
    loginRole = 'admin';
    tabAdminBtn.classList.add('active-tab');
    tabStudentBtn.classList.remove('active-tab');
    usernameInput.placeholder = "اسم مستخدم الإدارة (admin)";
    passwordInput.placeholder = "كلمة مرور الإدارة (123456)";
});

// جلب كافة البيانات من سيرفر C++
async function fetchAllData() {
    try {
        const [resS, resG, resF] = await Promise.all([
            fetch('/api/students'),
            fetch('/api/grades'),
            fetch('/api/fees')
        ]);
        students = await resS.json();
        grades = await resG.json();
        fees = await resF.json();
        renderAdminTables();
        if (currentStudent) {
            renderStudentPortal(currentStudent);
        }
    } catch (err) {
        console.error("فشل الاتصال بسيرفر C++:", err);
    }
}

// إرسال بيانات POST إلى سيرفر C++ لحفظها في الملفات النصية
async function postToServer(endpoint, dataObj) {
    const params = new URLSearchParams();
    for (const key in dataObj) {
        params.append(key, dataObj[key]);
    }
    const response = await fetch(endpoint, {
        method: 'POST',
        headers: { 'Content-Type': 'application/x-www-form-urlencoded;charset=UTF-8' },
        body: params.toString()
    });
    return await response.json();
}

// تسجيل الدخول
document.getElementById('login-form').addEventListener('submit', async (e) => {
    e.preventDefault();
    const user = usernameInput.value.trim();
    const pass = passwordInput.value.trim();
    const errorEl = document.getElementById('login-error');
    errorEl.style.display = 'none';

    await fetchAllData();

    if (loginRole === 'admin') {
        if (user === 'admin' && pass === '123456') {
            document.getElementById('login-screen').classList.add('hidden');
            document.getElementById('admin-dashboard').classList.remove('hidden');
            renderAdminTables();
        } else {
            errorEl.textContent = "بيانات دخول الإدارة غير صحيحة (الافتراضي: admin / 123456)";
            errorEl.style.display = 'block';
        }
    } else {
        const found = students.find(s => s.cardId === user && s.nationalId === pass);
        if (found) {
            currentStudent = found;
            document.getElementById('login-screen').classList.add('hidden');
            document.getElementById('student-portal').classList.remove('hidden');
            renderStudentPortal(found);
        } else {
            errorEl.textContent = "الكود الأكاديمي أو الرقم القومي غير مسجل في قاعدة البيانات.";
            errorEl.style.display = 'block';
        }
    }
});

// تسجيل الخروج
document.getElementById('admin-logout-btn').addEventListener('click', () => {
    document.getElementById('admin-dashboard').classList.add('hidden');
    document.getElementById('login-screen').classList.remove('hidden');
});

document.getElementById('student-logout-btn').addEventListener('click', () => {
    currentStudent = null;
    document.getElementById('student-portal').classList.add('hidden');
    document.getElementById('login-screen').classList.remove('hidden');
});

// التنقل بين أقسام لوحة الإدارة
document.querySelectorAll('#admin-dashboard .menu a').forEach(link => {
    link.addEventListener('click', (e) => {
        e.preventDefault();
        document.querySelectorAll('#admin-dashboard .menu a').forEach(a => a.classList.remove('active'));
        link.classList.add('active');
        const target = link.getAttribute('data-target');
        document.querySelectorAll('#admin-dashboard .content-section').forEach(sec => sec.classList.remove('active'));
        document.getElementById(target).classList.add('active');
    });
});

// التنقل بين أقسام بوابة الطالب
document.querySelectorAll('#student-menu a').forEach(link => {
    link.addEventListener('click', (e) => {
        e.preventDefault();
        document.querySelectorAll('#student-menu a').forEach(a => a.classList.remove('active'));
        link.classList.add('active');
        const target = link.getAttribute('data-target');
        document.querySelectorAll('#student-portal .student-section').forEach(sec => sec.classList.remove('active'));
        document.getElementById(target).classList.add('active');
    });
});

// ================= إضافة وحفظ بيانات الطلاب =================
document.getElementById('add-student-form').addEventListener('submit', async (e) => {
    e.preventDefault();
    const studentObj = {
        cardId: document.getElementById('st-cardId').value.trim(),
        nationalId: document.getElementById('st-nationalId').value.trim(),
        seatNumber: document.getElementById('st-seatNumber').value.trim(),
        name: document.getElementById('st-name').value.trim(),
        academicYear: document.getElementById('st-academicYear').value,
        department: document.getElementById('st-department').value,
        phone: document.getElementById('st-phone').value.trim(),
        status: document.getElementById('st-status').value
    };

    const res = await postToServer('/api/students', studentObj);
    if (res.success) {
        e.target.reset();
        await fetchAllData();
        alert("تم حفظ بيانات الطالب في ملف قاعدة البيانات students_db.txt بنجاح!");
    }
});

async function deleteStudent(cardId) {
    if (!confirm("هل أنت متأكد من حذف سجل هذا الطالب نهائياً؟")) return;
    await postToServer('/api/students/delete', { cardId });
    await fetchAllData();
}

// ================= حساب التقدير وإضافة الدرجات =================
function calculateGradeLetter(score, max) {
    const pct = (score / max) * 100;
    if (pct >= 85) return "ممتاز (A)";
    if (pct >= 75) return "جيد جداً (B)";
    if (pct >= 65) return "جيد (C)";
    if (pct >= 50) return "مقبول (D)";
    return "ضعيف / راسب (F)";
}

document.getElementById('add-grade-form').addEventListener('submit', async (e) => {
    e.preventDefault();
    const maxG = parseInt(document.getElementById('gr-maxGrade').value);
    const studG = parseInt(document.getElementById('gr-studentGrade').value);
    const gradeObj = {
        id: "GR-" + Date.now(),
        cardId: document.getElementById('gr-cardId').value,
        courseCode: document.getElementById('gr-courseCode').value.trim(),
        courseName: document.getElementById('gr-courseName').value.trim(),
        maxGrade: maxG,
        studentGrade: studG,
        gradeLetter: calculateGradeLetter(studG, maxG)
    };

    const res = await postToServer('/api/grades', gradeObj);
    if (res.success) {
        e.target.reset();
        document.getElementById('gr-maxGrade').value = 100;
        await fetchAllData();
        alert("تم رصد وحفظ الدرجة في grades_db.txt بنجاح!");
    }
});

async function deleteGrade(id) {
    if (!confirm("حذف هذه النتيجة؟")) return;
    await postToServer('/api/grades/delete', { id });
    await fetchAllData();
}

// ================= تسجيل المصروفات والخزينة =================
document.getElementById('add-fee-form').addEventListener('submit', async (e) => {
    e.preventDefault();
    const tot = parseFloat(document.getElementById('fee-totalAmount').value);
    const paid = parseFloat(document.getElementById('fee-paidAmount').value);
    let status = "غير مسدد";
    if (paid >= tot) status = "مسدد بالكامل";
    else if (paid > 0) status = "سداد جزئي (قسط)";

    const feeObj = {
        id: document.getElementById('fee-id').value.trim(),
        cardId: document.getElementById('fee-cardId').value,
        academicYear: document.getElementById('fee-academicYear').value.trim(),
        totalAmount: tot,
        paidAmount: paid,
        paymentDate: document.getElementById('fee-paymentDate').value,
        status: status
    };

    const res = await postToServer('/api/fees', feeObj);
    if (res.success) {
        e.target.reset();
        await fetchAllData();
        alert("تم حفظ إيصال المصروفات في fees_db.txt بنجاح!");
    }
});

async function deleteFee(id) {
    if (!confirm("حذف هذا الإيصال؟")) return;
    await postToServer('/api/fees/delete', { id });
    await fetchAllData();
}

// ================= تحديث الجداول والقوائم المنسدلة =================
function renderAdminTables(filterText = "") {
    document.getElementById('total-students-count').textContent = students.length;

    // 1. جدول الطلاب
    const stBody = document.getElementById('admin-students-tbody');
    stBody.innerHTML = '';
    const filteredStudents = students.filter(s =>
        s.name.includes(filterText) || s.cardId.includes(filterText) || s.nationalId.includes(filterText)
    );

    filteredStudents.forEach(s => {
        stBody.innerHTML += `
            <tr>
                <td><strong>${s.cardId}</strong></td>
                <td>${s.seatNumber}</td>
                <td>${s.name}</td>
                <td>${s.academicYear}</td>
                <td>${s.department}</td>
                <td><span class="badge badge-green">${s.status}</span></td>
                <td>${s.phone}</td>
                <td><button class="btn btn-danger btn-sm" onclick="deleteStudent('${s.cardId}')">حذف</button></td>
            </tr>
        `;
    });

    // تحديث القوائم المنسدلة للطلاب
    const selects = ['gr-cardId', 'fee-cardId', 'cert-student-select'];
    selects.forEach(selId => {
        const el = document.getElementById(selId);
        const currentVal = el.value;
        el.innerHTML = '<option value="" disabled selected>اختر الطالب...</option>';
        students.forEach(s => {
            el.innerHTML += `<option value="${s.cardId}">${s.name} (${s.cardId})</option>`;
        });
        if (currentVal) el.value = currentVal;
    });

    // 2. جدول الدرجات
    const grBody = document.getElementById('admin-grades-tbody');
    grBody.innerHTML = '';
    grades.forEach(g => {
        const st = students.find(s => s.cardId === g.cardId);
        const stName = st ? st.name : "طالب غير مسجل";
        grBody.innerHTML += `
            <tr>
                <td>${g.cardId}</td>
                <td>${stName}</td>
                <td>${g.courseCode}</td>
                <td>${g.courseName}</td>
                <td><strong>${g.studentGrade} / ${g.maxGrade}</strong></td>
                <td><span class="badge ${g.studentGrade >= g.maxGrade/2 ? 'badge-green' : 'badge-red'}">${g.gradeLetter}</span></td>
                <td><button class="btn btn-danger btn-sm" onclick="deleteGrade('${g.id}')">حذف</button></td>
            </tr>
        `;
    });

    // 3. جدول المصروفات
    const feeBody = document.getElementById('admin-fees-tbody');
    feeBody.innerHTML = '';
    fees.forEach(f => {
        const st = students.find(s => s.cardId === f.cardId);
        const stName = st ? st.name : f.cardId;
        const rem = f.totalAmount - f.paidAmount;
        const badgeClass = rem <= 0 ? 'badge-green' : (f.paidAmount > 0 ? 'badge-orange' : 'badge-red');
        feeBody.innerHTML += `
            <tr>
                <td><strong>${f.id}</strong></td>
                <td>${stName}</td>
                <td>${f.academicYear}</td>
                <td>${f.totalAmount} ج.م</td>
                <td>${f.paidAmount} ج.م</td>
                <td>${rem} ج.م</td>
                <td><span class="badge ${badgeClass}">${f.status}</span></td>
                <td><button class="btn btn-danger btn-sm" onclick="deleteFee('${f.id}')">حذف</button></td>
            </tr>
        `;
    });
}

document.getElementById('search-student-input').addEventListener('input', (e) => {
    renderAdminTables(e.target.value.trim());
});

// ================= استخراج إفادة قيد رسمية =================
function generateCertificate() {
    const cardId = document.getElementById('cert-student-select').value;
    const s = students.find(st => st.cardId === cardId);
    if (!s) {
        alert("يرجى اختيار طالب أولاً.");
        return;
    }

    const today = new Date().toLocaleDateString('ar-EG');
    document.getElementById('cert-body').innerHTML = `
        <p>تشهد إدارة شؤون الطلاب بالمعهد العالي للحاسبات ونظم المعلومات الإدارية (HICMIS) بأن الطالب:</p>
        <p style="background:#f8f9f9; padding:15px; border-right:4px solid #154360;">
            <strong>الاسم رباعياً:</strong> ${s.name}<br>
            <strong>الرقم القومي:</strong> ${s.nationalId} &nbsp;&nbsp;|&nbsp;&nbsp; <strong>الكود الأكاديمي:</strong> ${s.cardId}<br>
            <strong>الفرقة الدراسية:</strong> ${s.academicYear} &nbsp;&nbsp;|&nbsp;&nbsp; <strong>الشعبة:</strong> ${s.department}<br>
            <strong>حالة القيد:</strong> ${s.status} &nbsp;&nbsp;|&nbsp;&nbsp; <strong>رقم الجلوس:</strong> ${s.seatNumber}
        </p>
        <p>مقيد بسجلات المعهد للعام الدراسي الحالي، وقد أُعطيت له هذه الإفادة بناءً على طلبه لتقديمها إلى من يهمه الأمر.</p>
        <p><strong>تحريراً في:</strong> ${today}</p>
    `;
    document.getElementById('certificate-paper').classList.remove('hidden');
}

// ================= عرض بوابة الطالب =================
function renderStudentPortal(s) {
    document.getElementById('st-header-name').textContent = s.name;
    document.getElementById('st-welcome-title').textContent = `أهلاً بك، ${s.name}`;
    document.getElementById('st-welcome-sub').textContent = `${s.academicYear} - شعبة ${s.department} | الحالة: ${s.status}`;

    document.getElementById('st-profile-details').innerHTML = `
        <div class="profile-item"><span>الكود الأكاديمي</span><strong>${s.cardId}</strong></div>
        <div class="profile-item"><span>الرقم القومي</span><strong>${s.nationalId}</strong></div>
        <div class="profile-item"><span>رقم الجلوس</span><strong>${s.seatNumber}</strong></div>
        <div class="profile-item"><span>الفرقة الدراسية</span><strong>${s.academicYear}</strong></div>
        <div class="profile-item"><span>الشعبة</span><strong>${s.department}</strong></div>
        <div class="profile-item"><span>حالة القيد</span><strong>${s.status}</strong></div>
    `;

    // درجات الطالب
    const myGrades = grades.filter(g => g.cardId === s.cardId);
    const stGradesTbody = document.getElementById('st-grades-tbody');
    stGradesTbody.innerHTML = myGrades.length ? '' : '<tr><td colspan="5" style="text-align:center;">لا توجد نتائج مرصودة حتى الآن.</td></tr>';
    myGrades.forEach(g => {
        stGradesTbody.innerHTML += `
            <tr>
                <td>${g.courseCode}</td>
                <td>${g.courseName}</td>
                <td>${g.maxGrade}</td>
                <td><strong>${g.studentGrade}</strong></td>
                <td><span class="badge ${g.studentGrade >= g.maxGrade/2 ? 'badge-green' : 'badge-red'}">${g.gradeLetter}</span></td>
            </tr>
        `;
    });

    // مصروفات الطالب
    const myFees = fees.filter(f => f.cardId === s.cardId);
    const stFeesTbody = document.getElementById('st-fees-tbody');
    stFeesTbody.innerHTML = myFees.length ? '' : '<tr><td colspan="7" style="text-align:center;">لا توجد إيصالات مصروفات مسجلة.</td></tr>';
    myFees.forEach(f => {
        const rem = f.totalAmount - f.paidAmount;
        stFeesTbody.innerHTML += `
            <tr>
                <td><strong>${f.id}</strong></td>
                <td>${f.academicYear}</td>
                <td>${f.totalAmount} ج.م</td>
                <td>${f.paidAmount} ج.م</td>
                <td>${rem} ج.م</td>
                <td>${f.paymentDate}</td>
                <td><span class="badge ${rem <= 0 ? 'badge-green' : 'badge-orange'}">${f.status}</span></td>
            </tr>
        `;
    });
}

// تحميل البيانات فور فتح الصفحة
window.addEventListener('DOMContentLoaded', fetchAllData);
