# Kalkulus - Grafis Fungsi dengan SFML

Proyek kecil untuk menggambar grafik fungsi y = sin(x) menggunakan SFML.

Instruksi build dan run (Linux)

1. Pastikan SFML terpasang. Contoh (Debian/Ubuntu):

```bash
sudo apt update
sudo apt install libsfml-dev
```

2. Kompilasi menggunakan pkg-config (jika tersedia):

```bash
g++ main.cpp -o main $(pkg-config --cflags --libs sfml-all)
```

3. Atau kompilasi dengan flag SFML eksplisit:

```bash
g++ main.cpp -o main -lsfml-graphics -lsfml-window -lsfml-system
```

4. Jalankan program:

```bash
./main
```

Catatan

- Program membuka jendela grafis (membutuhkan X/Wayland). Jika berjalan di environment headless, gunakan X forwarding atau virtual display.
- Program akan mencoba memuat font dari jalur sistem umum (mis. DejaVuSans). Jika font tidak ditemukan, tick akan tetap digambar tetapi label numerik tidak akan muncul.
- Jika ingin selalu menampilkan label, salin file TTF (mis. `DejaVuSans.ttf`) ke folder proyek dan beri nama `Sansation.ttf` atau sesuaikan path di `main.cpp`.

Jika Anda mau, saya dapat menambahkan file font ke proyek agar label selalu muncul. Kalau tidak perlu, perubahan sudah selesai.

5. Compile program terbaru (2D & 3D):
# Compile 2D
g++ -std=c++17 grapher2d.cpp -o grapher2d -lsfml-graphics -lsfml-window -lsfml-system

# Compile 3D
g++ -std=c++17 grapher3d.cpp -o grapher3d -lsfml-graphics -lsfml-window -lsfml-system

6. Cara run program terbaru (2D & 3D):
# Run 2D
./grapher2d

# Run 3D
./grapher3d
