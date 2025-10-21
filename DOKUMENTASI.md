# Dokumentasi Code SFML Graph Plotter

## Daftar Isi

1. [Overview](#overview)
2. [Include Libraries](#include-libraries)
3. [Class GraphPlotter](#class-graphplotter)
4. [Fungsi-Fungsi Matematika](#fungsi-fungsi-matematika)
5. [Fungsi Main](#fungsi-main)

---

## Overview

Program ini adalah aplikasi visualisasi grafik matematika menggunakan SFML (Simple and Fast Multimedia Library). Program menampilkan berbagai fungsi matematika (sinus, cosinus, kuadrat, dan kubik) pada sebuah sistem koordinat kartesian dengan grid, label, dan legenda.

---

## Include Libraries

### Baris 1-6: Header Files

```cpp
#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <sstream>
#include <iomanip>
```

| Include               | Fungsi                                                   |
| --------------------- | -------------------------------------------------------- |
| `<SFML/Graphics.hpp>` | Library SFML untuk rendering grafis 2D                   |
| `<cmath>`             | Fungsi matematika seperti sin(), cos(), isnan(), isinf() |
| `<vector>`            | Container dinamis untuk menyimpan kumpulan data          |
| `<sstream>`           | String stream untuk manipulasi string                    |
| `<iomanip>`           | Manipulator input/output untuk formatting                |

---

## Class GraphPlotter

### Baris 8-10: Deklarasi Class dan Access Specifier

```cpp
class GraphPlotter
{
private:
```

- Mendefinisikan class `GraphPlotter` dengan member private

### Baris 11-17: Private Member Variables

```cpp
  sf::RenderWindow &window;    // Referensi ke window rendering
  float width, height;          // Ukuran window (lebar dan tinggi)
  float xMin, xMax, yMin, yMax; // Range koordinat matematika
  sf::Font font;                // Font untuk menampilkan label/text
  bool fontLoaded;              // Flag status keberhasilan load font
```

| Variable                 | Tipe                | Fungsi                            |
| ------------------------ | ------------------- | --------------------------------- |
| `window`                 | `sf::RenderWindow&` | Referensi window untuk menggambar |
| `width, height`          | `float`             | Dimensi area gambar               |
| `xMin, xMax, yMin, yMax` | `float`             | Batas-batas koordinat matematika  |
| `font`                   | `sf::Font`          | Objek font untuk text rendering   |
| `fontLoaded`             | `bool`              | Indikator font berhasil dimuat    |

### Baris 19-33: Constructor GraphPlotter

```cpp
public:
  GraphPlotter(sf::RenderWindow &win, float w, float h)
      : window(win), width(w), height(h), fontLoaded(false)
  {
    xMin = -10.0f;
    xMax = 10.0f;
    yMin = -10.0f;
    yMax = 10.0f;

    // Coba load font default sistem
    fontLoaded = font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf") ||
                 font.loadFromFile("C:\\Windows\\Fonts\\Arial.ttf") ||
                 font.loadFromFile("/System/Library/Fonts/Helvetica.ttc");
  }
```

**Penjelasan Baris Demi Baris:**

| Baris | Kode                                                    | Fungsi                                                                                        |
| ----- | ------------------------------------------------------- | --------------------------------------------------------------------------------------------- |
| 19    | `public:`                                               | Mulai section public members                                                                  |
| 20    | `GraphPlotter(sf::RenderWindow &win, float w, float h)` | Konstruktor dengan 3 parameter                                                                |
| 21    | `: window(win), width(w), height(h), fontLoaded(false)` | Member initializer list, inisialisasi member variables                                        |
| 23    | `xMin = -10.0f;`                                        | Set range X minimum ke -10                                                                    |
| 24    | `xMax = 10.0f;`                                         | Set range X maksimum ke 10                                                                    |
| 25    | `yMin = -10.0f;`                                        | Set range Y minimum ke -10                                                                    |
| 26    | `yMax = 10.0f;`                                         | Set range Y maksimum ke 10                                                                    |
| 29-32 | `fontLoaded = font.loadFromFile(...)`                   | Coba load font dari 3 lokasi berbeda (Linux, Windows, macOS) menggunakan operator `\|\|` (OR) |

### Baris 35-42: Method mathToScreen()

```cpp
  // Konversi koordinat matematika ke koordinat layar
  sf::Vector2f mathToScreen(float x, float y)
  {
    float screenX = (x - xMin) / (xMax - xMin) * width;
    float screenY = height - (y - yMin) / (yMax - yMin) * height;
    return sf::Vector2f(screenX, screenY);
  }
```

**Fungsi:** Mengonversi titik (x, y) dari sistem koordinat matematika ke sistem koordinat layar (pixel).

| Baris | Rumus                                                    | Penjelasan                                                         |
| ----- | -------------------------------------------------------- | ------------------------------------------------------------------ |
| 38    | `screenX = (x - xMin) / (xMax - xMin) * width`           | Normalisasi x ke rentang [0, width]                                |
| 39    | `screenY = height - (y - yMin) / (yMax - yMin) * height` | Normalisasi y (dikurangi dari height karena layar y-axis terbalik) |
| 40    | `return sf::Vector2f(screenX, screenY)`                  | Return vektor 2D dengan koordinat layar                            |

### Baris 44-92: Method drawAxes()

```cpp
  // Gambar sumbu koordinat dengan label
  void drawAxes()
  {
```

**Baris 45-51: Gambar Sumbu X**

```cpp
    // Gambar sumbu X (horizontal)
    sf::Vertex xAxis[] = {
        sf::Vertex(mathToScreen(xMin, 0), sf::Color(255, 255, 255)),
        sf::Vertex(mathToScreen(xMax, 0), sf::Color(255, 255, 255))};
    window.draw(xAxis, 2, sf::Lines);
```

- Membuat 2 vertex (titik) untuk sumbu X
- Warna putih RGB(255, 255, 255)
- Menggambar sebagai line strip antara 2 titik

**Baris 53-59: Gambar Sumbu Y**

```cpp
    // Gambar sumbu Y (vertikal)
    sf::Vertex yAxis[] = {
        sf::Vertex(mathToScreen(0, yMin), sf::Color(255, 255, 255)),
        sf::Vertex(mathToScreen(0, yMax), sf::Color(255, 255, 255))};
    window.draw(yAxis, 2, sf::Lines);
```

- Sama seperti sumbu X, tapi untuk sumbu Y (vertikal)

**Baris 61-71: Gambar Grid Vertikal**

```cpp
    // Gambar grid dengan warna redup
    for (int i = (int)xMin; i <= (int)xMax; i++)
    {
      if (i == 0)
        continue; // Skip sumbu utama
      sf::Vertex line[] = {
          sf::Vertex(mathToScreen(i, yMin), sf::Color(40, 40, 40)),
          sf::Vertex(mathToScreen(i, yMax), sf::Color(40, 40, 40))};
      window.draw(line, 2, sf::Lines);
    }
```

- Loop dari xMin hingga xMax (melompat 1 per iterasi)
- Skip i==0 (karena sudah jadi sumbu X)
- Gambar garis vertikal dengan warna abu-abu gelap RGB(40, 40, 40)

**Baris 73-83: Gambar Grid Horizontal**

```cpp
    for (int i = (int)yMin; i <= (int)yMax; i++)
    {
      if (i == 0)
        continue; // Skip sumbu utama
      sf::Vertex line[] = {
          sf::Vertex(mathToScreen(xMin, i), sf::Color(40, 40, 40)),
          sf::Vertex(mathToScreen(xMax, i), sf::Color(40, 40, 40))};
      window.draw(line, 2, sf::Lines);
    }
```

- Sama seperti grid vertikal, tapi untuk sumbu Y

**Baris 85-92: Jika Font Berhasil Dimuat**

```cpp
    if (fontLoaded)
    {
      // Label pada sumbu X
      for (int i = (int)xMin; i <= (int)xMax; i++)
```

- Kondisi: jika font sudah berhasil dimuat, baru tampilkan label

**Baris 87-103: Label pada Sumbu X**

```cpp
      // Label pada sumbu X
      for (int i = (int)xMin; i <= (int)xMax; i++)
      {
        if (i == 0)
          continue;

        // Gambar tick mark
        sf::Vector2f pos = mathToScreen(i, 0);
        sf::Vertex tick[] = {
            sf::Vertex(sf::Vector2f(pos.x, pos.y - 5), sf::Color::White),
            sf::Vertex(sf::Vector2f(pos.x, pos.y + 5), sf::Color::White)};
        window.draw(tick, 2, sf::Lines);

        // Label angka
        sf::Text label;
        label.setFont(font);
        label.setString(std::to_string(i));
        label.setCharacterSize(12);
        label.setFillColor(sf::Color::White);
        sf::FloatRect bounds = label.getLocalBounds();
        label.setPosition(pos.x - bounds.width / 2, pos.y + 8);
        window.draw(label);
      }
```

| Bagian                               | Fungsi                                      |
| ------------------------------------ | ------------------------------------------- |
| `pos = mathToScreen(i, 0)`           | Hitung posisi layar untuk titik (i, 0)      |
| `sf::Vertex tick[]`                  | Buat 2 vertex untuk tick mark (garis kecil) |
| `label.setString(std::to_string(i))` | Ubah angka menjadi string                   |
| `label.setPosition()`                | Posisikan label di tengah tick mark         |
| `window.draw(label)`                 | Gambar label angka                          |

**Baris 105-121: Label pada Sumbu Y**

```cpp
      // Label pada sumbu Y
      for (int i = (int)yMin; i <= (int)yMax; i++)
      {
        if (i == 0)
          continue;

        // Gambar tick mark
        sf::Vector2f pos = mathToScreen(0, i);
        sf::Vertex tick[] = {
            sf::Vertex(sf::Vector2f(pos.x - 5, pos.y), sf::Color::White),
            sf::Vertex(sf::Vector2f(pos.x + 5, pos.y), sf::Color::White)};
        window.draw(tick, 2, sf::Lines);

        // Label angka
        sf::Text label;
        label.setFont(font);
        label.setString(std::to_string(i));
        label.setCharacterSize(12);
        label.setFillColor(sf::Color::White);
        sf::FloatRect bounds = label.getLocalBounds();
        label.setPosition(pos.x - bounds.width - 10, pos.y - bounds.height / 2 - 5);
        window.draw(label);
      }
```

- Mirip dengan label sumbu X, tapi tick mark horizontal (pos.x ± 5)
- Label diposisikan di sebelah kiri garis

**Baris 123-135: Label Sumbu X dan Y**

```cpp
      // Label "X" dan "Y" untuk sumbu
      sf::Text xLabel;
      xLabel.setFont(font);
      xLabel.setString("X");
      xLabel.setCharacterSize(16);
      xLabel.setFillColor(sf::Color::White);
      xLabel.setStyle(sf::Text::Bold);
      sf::Vector2f xAxisEnd = mathToScreen(xMax, 0);
      xLabel.setPosition(xAxisEnd.x - 20, xAxisEnd.y + 10);
      window.draw(xLabel);
```

- Buat teks "X" berukuran 16, bold
- Posisikan di ujung kanan sumbu X

**Baris 137-149: Label Sumbu Y (Serupa)**

```cpp
      sf::Text yLabel;
      yLabel.setFont(font);
      yLabel.setString("Y");
      yLabel.setCharacterSize(16);
      yLabel.setFillColor(sf::Color::White);
      yLabel.setStyle(sf::Text::Bold);
      sf::Vector2f yAxisEnd = mathToScreen(0, yMax);
      yLabel.setPosition(yAxisEnd.x + 10, yAxisEnd.y + 5);
      window.draw(yLabel);
```

- Sama seperti label X, tapi label "Y" di ujung atas sumbu Y

**Baris 151-160: Label Origin (0,0)**

```cpp
      // Label origin (0,0)
      sf::Text origin;
      origin.setFont(font);
      origin.setString("0");
      origin.setCharacterSize(12);
      origin.setFillColor(sf::Color::White);
      sf::Vector2f originPos = mathToScreen(0, 0);
      origin.setPosition(originPos.x + 5, originPos.y + 5);
      window.draw(origin);
```

- Tampilkan teks "0" di posisi origin (persimpangan sumbu)

### Baris 164-179: Method plotFunction()

```cpp
  // Gambar fungsi matematika
  void plotFunction(float (*func)(float), sf::Color color, const std::string &name)
  {
    std::vector<sf::Vertex> points;
    float step = (xMax - xMin) / (width * 2); // Lebih banyak titik untuk kurva halus

    for (float x = xMin; x <= xMax; x += step)
    {
      float y = func(x);

      // Pastikan y dalam range
      if (y >= yMin && y <= yMax && !std::isnan(y) && !std::isinf(y))
      {
        sf::Vector2f pos = mathToScreen(x, y);
        points.push_back(sf::Vertex(pos, color));
      }
    }

    if (points.size() > 1)
    {
      window.draw(&points[0], points.size(), sf::LineStrip);
    }
  }
```

| Baris   | Penjelasan                                              |
| ------- | ------------------------------------------------------- | --------------------------------------------------------------------- |
| 165     | `float (*func)(float)`                                  | Parameter: pointer ke fungsi yang menerima float, return float        |
| 166-167 | `sf::Color color, const std::string &name`              | Parameter: warna garis dan nama fungsi (name tidak digunakan di sini) |
| 168     | `std::vector<sf::Vertex> points`                        | Container untuk menyimpan semua titik kurva                           |
| 169     | `step = (xMax - xMin) / (width * 2)`                    | Hitung increment X untuk smooth curve (2 pixel per step)              |
| 171-174 | `for (float x = xMin; x <= xMax; x += step)`            | Loop dari xMin ke xMax dengan step tertentu                           |
| 175     | `float y = func(x)`                                     | Hitung nilai y dari fungsi                                            |
| 178     | `!std::isnan(y) && !std::isinf(y)`                      | Validasi y tidak NaN atau infinity                                    |
| 183     | `window.draw(&points[0], points.size(), sf::LineStrip)` | Gambar semua vertex sebagai line strip                                |

### Baris 182-198: Method drawLegend()

```cpp
  // Gambar legenda
  void drawLegend(const std::vector<std::pair<std::string, sf::Color>> &functions)
  {
    if (!fontLoaded)
      return;

    float startX = 10;
    float startY = 10;
    float lineHeight = 25;

    for (size_t i = 0; i < functions.size(); i++)
    {
      // Gambar kotak warna
      sf::RectangleShape colorBox(sf::Vector2f(15, 15));
      colorBox.setFillColor(functions[i].second);
      colorBox.setPosition(startX, startY + i * lineHeight);
      window.draw(colorBox);

      // Gambar nama fungsi
      sf::Text text;
      text.setFont(font);
      text.setString(functions[i].first);
      text.setCharacterSize(14);
      text.setFillColor(sf::Color::White);
      text.setPosition(startX + 25, startY + i * lineHeight);
      window.draw(text);
    }
  }
```

| Baris   | Fungsi                                          |
| ------- | ----------------------------------------------- | -------------------------------------------------- |
| 184     | `if (!fontLoaded) return;`                      | Jika font tidak dimuat, keluar                     |
| 186-188 | `startX, startY, lineHeight`                    | Variabel untuk posisi dan jarak antar item legenda |
| 190     | `for (size_t i = 0; i < functions.size(); i++)` | Loop untuk setiap fungsi                           |
| 192-197 | `sf::RectangleShape colorBox`                   | Buat kotak warna kecil 15x15 pixel                 |
| 199-206 | `sf::Text text`                                 | Buat teks untuk nama fungsi                        |

### Baris 208-211: Method setRange()

```cpp
  void setRange(float xmin, float xmax, float ymin, float ymax)
  {
    xMin = xmin;
    xMax = xmax;
    yMin = ymin;
    yMax = ymax;
  }
```

- Method untuk mengubah range koordinat matematika

---

## Fungsi-Fungsi Matematika

### Baris 215-217: sineFunction()

```cpp
float sineFunction(float x)
{
  return 3.0f * std::sin(x);
}
```

- Fungsi: y = 3 × sin(x)
- Amplitudo 3, periode 2π

### Baris 219-221: cosineFunction()

```cpp
float cosineFunction(float x)
{
  return 2.0f * std::cos(x);
}
```

- Fungsi: y = 2 × cos(x)
- Amplitudo 2, periode 2π

### Baris 223-225: quadraticFunction()

```cpp
float quadraticFunction(float x)
{
  return 0.1f * x * x - 2.0f;
}
```

- Fungsi: y = 0.1x² - 2
- Parabola yang terbuka ke atas

### Baris 227-229: cubicFunction()

```cpp
float cubicFunction(float x)
{
  return 0.02f * x * x * x;
}
```

- Fungsi: y = 0.02x³
- Kurva kubik

---

## Fungsi Main

### Baris 231-234: Inisialisasi Window

```cpp
int main()
{
  // Buat window
  sf::RenderWindow window(sf::VideoMode(1000, 700), "SFML Graph Plotter - Sumbu X dan Y");
  window.setFramerateLimit(60);
```

| Baris | Fungsi                         |
| ----- | ------------------------------ | --------------------------------- |
| 234   | `sf::VideoMode(1000, 700)`     | Buat window ukuran 1000×700 pixel |
| 234   | `"SFML Graph Plotter..."`      | Judul window                      |
| 235   | `window.setFramerateLimit(60)` | Batasi FPS ke 60 frame per detik  |

### Baris 237-240: Inisialisasi GraphPlotter

```cpp
  // Buat graph plotter
  GraphPlotter plotter(window, 1000, 700);

  // Set range
  plotter.setRange(-10, 10, -10, 10);
```

- Buat objek GraphPlotter dengan ukuran 1000×700
- Set range koordinat: X dari -10 ke 10, Y dari -10 ke 10

### Baris 242-247: Data Legenda

```cpp
  // Daftar fungsi untuk legenda
  std::vector<std::pair<std::string, sf::Color>> functions = {
      {"y = 3sin(x)", sf::Color::Red},
      {"y = 2cos(x)", sf::Color::Green},
      {"y = 0.1x^2 - 2", sf::Color::Blue},
      {"y = 0.02x^3", sf::Color::Yellow}};
```

- Vector of pairs: string (nama) dan Color (warna)
- Berisi 4 fungsi dengan warna berbeda

### Baris 249-269: Event Loop

```cpp
  while (window.isOpen())
  {
    sf::Event event;
    while (window.pollEvent(event))
    {
      if (event.type == sf::Event::Closed)
        window.close();
    }

    window.clear(sf::Color(20, 20, 30)); // Background gelap

    // Gambar sumbu dan grid dengan label
    plotter.drawAxes();

    // Gambar berbagai fungsi dengan warna berbeda
    plotter.plotFunction(sineFunction, sf::Color::Red, "sin");
    plotter.plotFunction(cosineFunction, sf::Color::Green, "cos");
    plotter.plotFunction(quadraticFunction, sf::Color::Blue, "quad");
    plotter.plotFunction(cubicFunction, sf::Color::Yellow, "cubic");

    // Gambar legenda
    plotter.drawLegend(functions);

    window.display();
  }
```

| Baris   | Fungsi                                |
| ------- | ------------------------------------- | ------------------------------------ |
| 249     | `while (window.isOpen())`             | Loop utama selama window terbuka     |
| 251-256 | `sf::Event event`                     | Handle event (close window)          |
| 258     | `window.clear(sf::Color(20, 20, 30))` | Clear window dengan background gelap |
| 261     | `plotter.drawAxes()`                  | Gambar sumbu dan grid                |
| 264-267 | `plotter.plotFunction(...)`           | Gambar 4 fungsi dengan warna berbeda |
| 270     | `plotter.drawLegend(functions)`       | Gambar legenda di pojok atas         |
| 272     | `window.display()`                    | Update/refresh display               |

### Baris 274-276: Return

```cpp
  }

  return 0;
}
```

- Loop berakhir ketika window ditutup
- Return 0 (sukses)

---

## Ringkasan Alur Program

1. **Inisialisasi**: Buat window 1000×700, GraphPlotter object, dan data legenda
2. **Loop Utama**: Selama window terbuka:
   - Handle event (tombol close)
   - Clear window dengan background gelap
   - Gambar sumbu X dan Y dengan grid dan label
   - Gambar 4 fungsi matematika dengan warna berbeda
   - Gambar legenda di pojok atas
   - Update display
3. **Selesai**: Loop berakhir ketika window ditutup

---

## Dependencies

- **SFML 2.x**: Library grafis untuk window, rendering, dan event handling
- **C++ Standard Library**: cmath, vector, sstream, iomanip

---

## Cara Compile dan Run

```bash
g++ main.cpp -o main -lsfml-graphics -lsfml-window -lsfml-system
./main
```

Atau gunakan task yang sudah tersedia:

```bash
./build and run SFML
```

---

**Dokumentasi dibuat: Oktober 21, 2025**
