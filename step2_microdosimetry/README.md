# Bước 2 – Mô phỏng vi mô (Geant4-DNA)

Ứng dụng `protonMicro` tính phổ năng lượng tuyến f(y), d(y) và các trung bình ȳ_F, ȳ_D, y* trong vùng nhạy hình cầu bằng nước, dùng mô hình track-structure của Geant4-DNA. Có hai chế độ nguồn:

- **gun:** một loại hạt đơn năng (`/gun/particle`, `/gun/energy`), dùng để kiểm chứng và lấy phổ tham chiếu;
- **spectrum:** trường hỗn hợp tại một độ sâu, lấy mẫu từ phổ fluence mà bước 1 ghi ra (`<prefix>_spectrum_*.csv`).

Đầu ra `y*` theo độ sâu (`analysis/collect_depth.py`) là đầu vào cho MKM ở bước 3.

> **Trạng thái:**
> - Phần lấy mẫu vùng nhạy (`SiteSampler`) không phụ thuộc Geant4 và đã có test đơn vị so với lời giải giải tích: vết thẳng LET không đổi cho ȳ_F = L và ȳ_D/ȳ_F = 9/8.
> - Phần Geant4 đã được kiểm tra cú pháp với header 11.2.2, nhưng **chưa được biên dịch và chạy với một bản cài Geant4 thật**.

## Phương pháp

### Đoạn vết (track segment)
Mỗi sự kiện là một hạt sơ cấp bắt đầu tại z = −buildup và đi theo +z trong một khối nước:

```
 z:  -buildup        0            zone      zone+exit
      |--- buildup ---|--- zone ---|-- exit --|
      ^ hạt sơ cấp     tâm vùng nhạy nằm ở đây
```

- Đoạn **buildup** (mặc định 3 µm) cho electron delta có năng lượng thấp đạt cân bằng trước khi tới vùng ghi.
- Tâm vùng nhạy chỉ được đặt trong **zone** (mặc định 2 µm).
- Mọi điểm truyền năng lượng (transfer point) trong khoảng [−r, zone + r] đều được lưu lại.

### Đặt vùng nhạy ngẫu nhiên
Đây là cách lấy mẫu theo trọng số năng lượng (weighted sampling), tương tự ví dụ `microyz`:

1. Chọn một điểm truyền năng lượng i với xác suất ε_i / E.
2. Đặt tâm c của hình cầu bán kính r phân bố đều trong quả cầu bán kính r quanh điểm i.
3. Loại mẫu nếu c nằm ngoài zone.
4. ε(c) là tổng năng lượng của các điểm nằm trong hình cầu.

Mật độ xác suất của c tỷ lệ với ε(c), nên ε lấy mẫu được phân bố theo **d(ε)**. Gán trọng số 1/ε cho mỗi mẫu thì được **f(ε)**. Với N lần thử mỗi sự kiện, mọi mẫu được chấp nhận mang cùng trọng số E/N, nên các vết khác nhau được gộp với trọng số đúng.

Năng lượng tuyến tính bằng y = ε / l̄, với l̄ = 2d/3 (dây cung trung bình của hình cầu).

### Các trung bình
- ȳ_F = Σw / Σ(w/y)
- ȳ_D = Σwy / Σw
- y* = y₀² Σ(w/y)(1 − e^(−y²/y₀²)) / Σw, với y₀ = 150 keV/µm (Kase et al. 2006), đổi được bằng `/micro/score/y0`.

Độ bất định tính theo dao động giữa các sự kiện (ratio estimator), nên tính đúng cả tương quan giữa các mẫu trong cùng một vết.

### Nguồn phổ từ bước 1
- Bin (loại hạt, năng lượng) được chọn với xác suất tỷ lệ Φ(E)·ΔE, tức theo track length. Năng lượng được lấy log-đều trong bin.
- Mỗi hạt sơ cấp có cùng chiều dài zone, nên trọng số track length của trường được giữ đúng.
- `/micro/source/bias` cho phép lấy mẫu dày hơn các hạt hiếm (alpha, d, t, ³He). Trọng số được tự động bù lại.

### Hạt mà Geant4-DNA không hỗ trợ
Geant4-DNA 11.2 không có quá trình EM cho deuteron, triton và ³He. Nếu bắn trực tiếp, chúng sẽ bay qua mà không tương tác. Vì vậy chúng được thay bằng hạt **cùng vận tốc**, E' = E·m'/m:

| Hạt (bước 1) | Hạt mô phỏng |
|---|---|
| d, t | proton |
| ³He | alpha |
| `ion_other` (Z > 2) | bỏ qua (mặc định), hoặc một ion đại diện qua `/micro/source/otherIon Z A` |

DNA có mô hình cho Li, Be, B, C, N, O, Si, Fe.

### Lưu điểm truyền năng lượng
- Với quá trình DNA (rời rạc), năng lượng được đặt tại điểm tương tác (post-step).
- Với mô hình condensed-history, dùng khi năng lượng vượt giới hạn DNA (ví dụ electron > 1 MeV), năng lượng mất liên tục được rải đều dọc bước, mỗi điểm cách nhau tối đa `pointSpacing` (mặc định 10 nm).

### Physics
Chỉ dùng EM Geant4-DNA, chọn bằng `-p`:

| Tùy chọn | Constructor | Ghi chú |
|---|---|---|
| `dna_opt0` | `G4EmDNAPhysics` | |
| `dna_opt2` | `G4EmDNAPhysics_option2` | |
| `dna_opt4` | `G4EmDNAPhysics_option4` | mặc định |
| `dna_opt6` | `G4EmDNAPhysics_option6` | CPA100 |

Tương tác hạt nhân bị bỏ qua vì không đáng kể trên vài µm. Ở option4, proton dùng mô hình DNA tới 300 MeV.

## Cấu trúc

```
step2_microdosimetry/
├── CMakeLists.txt              # BUILD_SIMULATION (cần Geant4), BUILD_TESTING (không cần)
├── protonMicro.cc
├── include/, src/
│   ├── SiteSampler             # đặt vùng nhạy + lưới không gian (không phụ thuộc Geant4)
│   ├── SpectrumSource          # đọc phổ bước 1, lấy mẫu (loại hạt, E)
│   ├── PrimaryGeneratorAction  # gun / spectrum, quy đổi d, t, ³He
│   ├── SteppingAction, EventAction
│   ├── Run                     # f(y), d(y), ȳ_F, ȳ_D, y* ± σ, theo loại hạt
│   ├── PhysicsList, DetectorConstruction
│   └── MicroConfig, SourceSpecies
├── macros/  mono.mac, mono_scan.mac, spectrum.mac, depth_scan.mac (+ *_one.mac)
├── analysis/ micro_io.py, plot_ydist.py, collect_depth.py
└── tests/   test_site_sampler.cc
```

## Biên dịch và chạy

```bash
cd step2_microdosimetry && mkdir build && cd build
cmake .. -DGeant4_DIR=/đường/dẫn/lib/cmake/Geant4
make -j$(nproc) && ctest              # ctest chạy test SiteSampler

./protonMicro -m mono.mac -t 8                    # proton 10 MeV, dna_opt4
./protonMicro -m mono_scan.mac -t 8 -p dna_opt6   # quét năng lượng proton
./protonMicro -m spectrum.mac -t 8                # trường hỗn hợp tại 150 mm
./protonMicro -m depth_scan.mac -t 8              # nhiều độ sâu

python3 ../analysis/plot_ydist.py output/depth_*mm_ydist.csv
python3 ../analysis/collect_depth.py output/depth_*mm_summary.txt --output micro_vs_depth.csv
```

Nếu không có Geant4, vẫn chạy được test bằng `cmake .. -DBUILD_SIMULATION=OFF`.

### Lệnh macro

| Lệnh | Mặc định | Ý nghĩa |
|---|---|---|
| `/micro/det/buildup`, `zone`, `exit` | 3, 2, 1 µm | các đoạn của vết (PreInit) |
| `/micro/det/halfWidth` | 5 µm | nửa bề rộng ngang của khối nước (PreInit) |
| `/micro/score/siteDiameter` | 1 µm | đường kính vùng nhạy |
| `/micro/score/samples` | 200 | số lần thử đặt vùng nhạy mỗi sự kiện |
| `/micro/score/pointSpacing` | 10 nm | khoảng cách điểm khi rải năng lượng condensed-history |
| `/micro/score/yMin`, `yMax`, `binsPerDecade` | 0.001, 10⁴ keV/µm, 50 | trục y (log) |
| `/micro/score/y0` | 150 keV/µm | tham số bão hòa của y* |
| `/micro/source/mode` | gun | `gun` hoặc `spectrum` |
| `/micro/source/spectrumPrefix` | – | tiền tố đầu ra của bước 1 |
| `/micro/source/depth` | – | độ sâu lấy phổ |
| `/micro/source/otherIon` | `0 0` (bỏ qua) | ion đại diện cho `ion_other`, dạng `Z A` |
| `/micro/source/bias` | 1 | ví dụ `alpha 20` |
| `/micro/output/prefix` | output/micro | tiền tố file kết quả |

## File kết quả

| File | Nội dung |
|---|---|
| `<prefix>_ydist.csv` | `y_lo, y_hi, y_mid, f_y, d_y, yf_y, yd_y` và `d_y_<loại hạt>`. Các mật độ được chuẩn hóa: ∫f dy = ∫d dy = 1, và y·d(y) có diện tích 1 trên trục ln y. |
| `<prefix>_summary.txt` | ȳ_F, ȳ_D, y* ± σ; từng loại hạt: số sự kiện, tỷ lệ liều, ȳ_F, ȳ_D, y*; toàn bộ thiết lập |

## Hạn chế cần biết

- **Cân bằng electron delta:** chỉ có delta sinh ra trong đoạn buildup là đến được zone. Delta năng lượng cao sinh ra xa hơn ở thượng nguồn bị thiếu, và electron thoát ra ngoài khối nước bị mất. Nên khảo sát độ nhạy của ȳ_D theo `buildup` và `halfWidth`; đây cũng là một mục của bước 4.
- **Hạt năng lượng thấp:** hạt có quãng chạy còn lại cỡ chiều dài zone (proton dưới khoảng 0.5 MeV) sẽ dừng ngay trong zone. Khi đó giả thiết "đoạn vết ở năng lượng E" không còn đúng. Ảnh hưởng chủ yếu ở vùng distal fall-off.
- **Ion nặng:** bước 1 gộp mọi ion Z > 2 vào `ion_other`. Muốn tính đóng góp của hạt giật lùi chính xác hơn, nên tách phổ theo Z ở bước 1.
- Hướng hạt ở bước 1 không được dùng. Với vùng nhạy hình cầu trong môi trường đồng nhất, điều này không làm thay đổi phân bố y.
