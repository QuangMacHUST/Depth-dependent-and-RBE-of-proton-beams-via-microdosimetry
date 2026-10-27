# Bước 1 – Mô phỏng vĩ mô (Geant4 tiêu chuẩn)

Ứng dụng `protonDepth` mô phỏng chùm proton (pristine hoặc SOBP) trong phantom nước và ghi theo độ sâu:

- liều tích phân theo lớp (IDD) kèm độ bất định thống kê, tách theo loại hạt;
- LET_t và LET_d (keV/µm) cho proton sơ cấp, toàn bộ proton và toàn bộ hạt mang điện nặng;
- phổ fluence dΦ/dE theo độ sâu cho proton (sơ cấp, thứ cấp), deuteron, triton, ³He, alpha và các ion nặng hơn. Đây là đầu vào nguồn hạt cho bước 2 (Geant4-DNA).

> **Trạng thái:** mã đã được kiểm tra cú pháp (`g++ -fsyntax-only -Wall -Wextra -Wshadow`) với header Geant4 11.2.2, và phần chia bin đã có test đơn vị. Mã **chưa được biên dịch, liên kết và chạy với một bản cài Geant4 thật**, nên lần build đầu tiên cần được kiểm tra.

## Cấu trúc

```
step1_macroscopic/
├── CMakeLists.txt
├── protonDepth.cc              # main: chọn physics list, số luồng, macro
├── include/, src/
│   ├── DetectorConstruction    # phantom nước, mặt trước tại z = 0
│   ├── PrimaryGeneratorAction  # G4GeneralParticleSource (/gps/...)
│   ├── SteppingAction          # thu liều, LET, track length
│   ├── EventAction             # đệm liều theo sự kiện (tính độ bất định)
│   ├── Run                     # bộ tích lũy, gộp đa luồng, ghi CSV
│   ├── StoppingPowerTable      # bảng S_el(E) từ G4EmCalculator
│   ├── Species                 # phân loại hạt
│   ├── Axes.hh                 # trục bin tuyến tính / log
│   └── ScoringConfig           # lệnh /rbe/score/..., /rbe/output/...
├── macros/
│   ├── pristine.mac            # một năng lượng (mặc định 150 MeV)
│   ├── pristine_scan.mac       # quét nhiều năng lượng (+ pristine_one.mac)
│   ├── sobp_example.mac        # ví dụ cú pháp SOBP nhiều nguồn GPS
│   └── vis.mac                 # chế độ tương tác
└── analysis/
    ├── step1_io.py             # đọc CSV, tính R80/R90
    ├── plot_depth.py           # vẽ PDD + LET
    └── sobp_weights.py         # tối ưu trọng số SOBP (NNLS) → macro GPS
```

## Biên dịch

Yêu cầu Geant4 ≥ 11.0, có cài datasets. Physics list `_HP` cần thêm G4NDL.

```bash
cd step1_macroscopic
mkdir build && cd build
cmake .. -DGeant4_DIR=/đường/dẫn/lib/cmake/Geant4
make -j$(nproc)
```

## Chạy

```bash
./protonDepth -m pristine.mac -t 8                     # QGSP_BIC_EMZ (mặc định)
./protonDepth -m pristine.mac -t 8 -p QGSP_BIC_HP_EMZ  # có neutron HP
./protonDepth                                          # tương tác + hiển thị
```

Kết quả được ghi vào `output/` (theo `/rbe/output/prefix`).

### Lệnh macro riêng

| Lệnh | Mặc định | Ý nghĩa |
|---|---|---|
| `/rbe/det/depth` | 40 cm | độ dày phantom theo trục chùm (PreInit) |
| `/rbe/det/width` | 40 cm | kích thước ngang phantom (PreInit) |
| `/rbe/det/maxStep` | 0.5 mm | bước tối đa của hạt mang điện trong phantom (PreInit) |
| `/rbe/det/material` | G4_WATER | vật liệu NIST (PreInit) |
| `/rbe/score/depthBins` | 800 | số bin độ sâu cho liều/LET |
| `/rbe/score/depthMax` | 0 (= độ dày phantom) | độ sâu tối đa được ghi |
| `/rbe/score/spectrumDepthBins` | 200 | số bin độ sâu cho phổ |
| `/rbe/score/eMin`, `eMax`, `eBins` | 1 keV, 1 GeV, 180 | trục năng lượng log của phổ |
| `/rbe/score/radius` | 0 (cả lớp) | chỉ ghi trong bán kính này quanh trục chùm |
| `/rbe/output/prefix` | output/run | tiền tố file kết quả |

Lệnh `/gps/...` chỉ dùng được **sau** `/run/initialize` ở chế độ đa luồng.

## File kết quả

| File | Nội dung |
|---|---|
| `<prefix>_depth.csv` | `z_lo_mm, z_hi_mm, z_mid_mm, dose_Gy, dose_rel_unc`, liều theo từng loại hạt `dose_<loại>_Gy`, và `LETt_/LETd_` cho `proton_primary`, `proton_all`, `charged_all` (keV/µm). Liều tính theo Gy trên một hạt sơ cấp. |
| `<prefix>_spectrum_<loại>.csv` | dΦ/dE (cm⁻² MeV⁻¹ trên một hạt sơ cấp); mỗi hàng là một bin độ sâu, mỗi cột là một bin động năng **toàn phần** (log). Dòng `# E_edges_MeV` chứa các mép bin. Có thêm file `proton_all` là tổng proton sơ cấp và thứ cấp. |
| `<prefix>_summary.txt` | phiên bản Geant4, physics list, số sự kiện, hình học, binning, tỷ lệ năng lượng lắng đọng theo loại hạt |

## Phương pháp

- **Liều:** năng lượng lắng đọng của mỗi bước được gán cho một điểm ngẫu nhiên trên bước (giống TestEm11/12). Liều là năng lượng chia cho khối lượng lớp (toàn bộ tiết diện phantom, hoặc πR² nếu đặt `radius`). Độ bất định tính theo từng lịch sử (history-by-history): tổng và tổng bình phương theo sự kiện.
- **LET:** dùng phương pháp track-length với stopping power điện tử không giới hạn (unrestricted) S_el(E) từ `G4EmCalculator::ComputeElectronicDEDX`. Giá trị được lập bảng log (100 điểm/decade) cho từng cặp hạt/vật liệu, rồi tra ở động năng trung bình của bước:
  LET_t = Σ S·l / Σ l,  LET_d = Σ S²·l / Σ S·l
  (xem Cortés-Giraldo & Carabe, *Phys. Med. Biol.* 60 (2015) 2645). Electron không tính vào LET. Nhóm `charged_all` gồm mọi hadron và ion mang điện.
- **Fluence:** dùng ước lượng track-length Φ = Σ l / V. Mỗi bước được chia **chính xác** theo phần chiều dài nằm trong từng bin độ sâu. Đây là cách lấy phổ đúng để làm nguồn cho bước 2, khác với việc đếm hạt đi qua mặt phẳng.
- **SOBP:** dùng nhiều nguồn GPS có trọng số (`/gps/source/add`). Quy trình:
  1. Chạy `pristine_scan.mac` với dãy năng lượng đủ dày, ví dụ bước 1–2 MeV, phủ vùng SOBP.
  2. Chạy `python3 analysis/sobp_weights.py output/pristine_*MeV_depth.csv --proximal 100 --distal 150 --output sobp.mac`.
  3. Chạy `./protonDepth -m sobp.mac`.

  Script chỉ dùng các đỉnh Bragg nằm trong vùng SOBP và in ra độ phẳng đạt được.

## Lưu ý và hạn chế

- Phantom rộng 40 cm nên `dose_Gy` là liều tích phân theo lớp (IDD), so được với đo bằng buồng ion hóa phẳng lớn (Bragg peak chamber). Muốn lấy giá trị trên trục trung tâm thì dùng `/rbe/score/radius`.
- `maxStep` 0.5 mm và bin 0.5 mm là cấu hình khởi đầu. Với vùng distal fall-off, nơi LET tăng nhanh, nên thử 0.1 mm để đánh giá độ hội tụ của LET_d.
- Phổ electron (delta) và neutron chưa được ghi. Ở bước 2, electron delta sẽ được Geant4-DNA tạo lại từ proton trong vùng nhạy. Đóng góp của electron đến từ thượng nguồn cần được đánh giá riêng.
- Phổ ion ghi theo động năng toàn phần, không chia theo nucleon.
- `G4_WATER` dùng I = 78 eV (mặc định của Geant4). Giá trị này ảnh hưởng trực tiếp đến range khi so với NIST PSTAR.

## Kiểm chứng (việc tiếp theo)

- So R80 (`analysis/plot_depth.py`) với CSDA range trong NIST PSTAR cho 70–230 MeV.
- So hai physics list `QGSP_BIC_EMZ` và `QGSP_BIC_HP_EMZ`.
- Kiểm tra độ hội tụ của LET_d theo `maxStep` và bề rộng bin.
