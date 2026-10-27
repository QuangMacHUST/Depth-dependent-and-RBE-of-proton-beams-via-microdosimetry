# Mục tiêu

Định lượng sự thay đổi của các tham số α, β, tỷ số α/β và RBE dọc đường cong Bragg, gồm vùng plateau, SOBP, đỉnh Bragg và vùng distal fall-off.
So sánh kết quả với giá trị lâm sàng RBE = 1.1 và với các mô hình RBE thực nghiệm như McNamara, Wedenberg và Carabe.

# Cơ sở khoa học

Hiệu ứng sinh học của proton phụ thuộc vào cách năng lượng phân bố ở mức vi mô (LET, phổ năng lượng tuyến y), không chỉ phụ thuộc vào liều hấp thụ vĩ mô.
MKM liên hệ năng lượng tuyến trung bình theo liều ($y_D$) hoặc $y^*$ (hiệu chỉnh bão hòa) với α theo công thức: $\alpha = \alpha_0 + \dfrac{\beta}{\rho \pi r_d^2} \, y^*$, với giả định β không đổi.

# Các bước thực hiện

1.Mô phỏng vĩ mô (Geant4 tiêu chuẩn): chùm proton 70–230 MeV, hoặc SOBP dựng từ tổ hợp trọng số nhiều năng lượng, chiếu vào phantom nước.
Physics list: QGSP_BIC_EMZ hoặc QGSP_BIC_HP_EMZ.
Ghi đường cong liều theo độ sâu (PDD), phổ năng lượng proton và hạt thứ cấp (hạt nhân giật lùi, alpha, deuteron) tại từng độ sâu, cùng LET_d và LET_t.
2. Mô phỏng vi mô (Geant4-DNA): lấy phổ năng lượng tại từng độ sâu làm nguồn hạt, chiếu vào vùng nhạy hình cầu đường kính khoảng 1 µm trong nước.
Physics: G4EmDNAPhysics_option4 hoặc option6 (CPA100), có so sánh với option2.
Tính f(y), d(y), $\bar{y}_F$, $\bar{y}_D$ và $y^*$.
3. Mô hình sinh học: đưa $y^*$ vào MKM với bộ tham số của dòng tế bào chuẩn (ví dụ HSG hoặc V79: α₀, β, r_d) để tính α(z), β(z), α/β(z), tỷ lệ sống sót và RBE(z) ở một mức sống sót tham chiếu.
4. Phân tích độ nhạy: khảo sát ảnh hưởng của kích thước vùng nhạy, lựa chọn physics option, ngưỡng năng lượng và đóng góp của hạt thứ cấp nặng.

# Cấu trúc kho

- [`step1_macroscopic/`](step1_macroscopic/README.md): bước 1, mô phỏng vĩ mô (PDD, LET_t/LET_d, phổ fluence theo độ sâu).
- [`step2_microdosimetry/`](step2_microdosimetry/README.md): bước 2, mô phỏng vi mô Geant4-DNA (f(y), d(y), ȳ_F, ȳ_D, y* theo độ sâu).

# Thành phần Geant4

Ví dụ xuất phát: TestEm12, microyz, spower, range.
Kỹ thuật: hình học hai tầng (vĩ mô và vi mô) hoặc chia thành hai lần chạy riêng; G4PSEnergyDeposit với scorer tùy biến; xuất kết quả qua ROOT hoặc CSV.

# Kết quả đầu ra

Đồ thị PDD kèm LET_d(z), $\bar{y}_D(z)$, $y^*(z)$, α(z), α/β(z) và RBE(z).
Bảng so sánh RBE với 1.1 và với các mô hình thực nghiệm.

# Kiểm chứng

Range và PDD so với NIST PSTAR và dữ liệu đo.
Phổ vi liều so với đo bằng TEPC hoặc microdosimeter silicon đã công bố.
RBE so với dữ liệu tế bào in vitro trong PIDE (Particle Irradiation Data Ensemble).

# Rủi ro và lưu ý

Bộ tham số MKM phụ thuộc dòng tế bào, nên kết quả mang tính tương đối.
Thống kê ở vùng distal rất kém, cần kỹ thuật giảm phương sai hoặc tăng số sự kiện.
