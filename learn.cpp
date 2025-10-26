#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <string>

int main() {
    // Membuat window dengan ukuran 800x600 pixel
    sf::RenderWindow window(sf::VideoMode(800, 600), "Grafik Fungsi y = sin(x)");
    
    // Koordinat pusat layar (origin untuk sistem koordinat)
    float centerX = 400.0f;  // Tengah horizontal
    float centerY = 300.0f;  // Tengah vertikal
    
    // Skala untuk grafik (mengatur zoom)
    float scaleX = 40.0f;  // 40 pixel per unit di sumbu X
    float scaleY = 30.0f;  // 30 pixel per unit di sumbu Y
    
    // ===== LOAD FONT UNTUK TEXT =====
    // Font diperlukan untuk menampilkan angka
    sf::Font font;
    // Coba load font system (ganti path sesuai sistem operasi Anda)
    // Linux: /usr/share/fonts/truetype/dejavu/DejaVuSans.ttf
    // Windows: C:/Windows/Fonts/arial.ttf
    // Mac: /Library/Fonts/Arial.ttf
    if (!font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) {
        // Jika gagal load font, program tetap jalan tapi tanpa text
        return -1;
    }
    
    // ===== MEMBUAT GRID (KOTAK-KOTAK) =====
    std::vector<sf::RectangleShape> gridLinesVertical;    // Garis vertikal grid
    std::vector<sf::RectangleShape> gridLinesHorizontal;  // Garis horizontal grid
    
    // Membuat garis vertikal untuk grid
    // Dari x = -10 sampai x = 10
    for (int i = -10; i <= 10; i++) {
        float xPos = centerX + (i * scaleX);  // Posisi X di layar
        
        // Buat garis vertikal
        sf::RectangleShape line(sf::Vector2f(1, 600));  // Lebar 1px, tinggi 600px
        line.setPosition(xPos, 0);  // Posisi dari atas ke bawah
        line.setFillColor(sf::Color(50, 50, 50));  // Warna abu-abu gelap
        gridLinesVertical.push_back(line);
    }
    
    // Membuat garis horizontal untuk grid
    // Dari y = -10 sampai y = 10
    for (int i = -10; i <= 10; i++) {
        float yPos = centerY - (i * scaleY);  // Posisi Y di layar
        
        // Buat garis horizontal
        sf::RectangleShape line(sf::Vector2f(800, 1));  // Panjang 800px, tinggi 1px
        line.setPosition(0, yPos);  // Posisi dari kiri ke kanan
        line.setFillColor(sf::Color(50, 50, 50));  // Warna abu-abu gelap
        gridLinesHorizontal.push_back(line);
    }
    
    // ===== MEMBUAT SUMBU X =====
    // Garis horizontal untuk sumbu X
    sf::RectangleShape axisX(sf::Vector2f(800, 2));  // Panjang 800px, tebal 2px
    axisX.setPosition(0, centerY);  // Posisi di tengah vertikal
    axisX.setFillColor(sf::Color::White);  // Warna putih
    
    // ===== MEMBUAT SUMBU Y =====
    // Garis vertikal untuk sumbu Y
    sf::RectangleShape axisY(sf::Vector2f(2, 600));  // Lebar 2px, tinggi 600px
    axisY.setPosition(centerX, 0);  // Posisi di tengah horizontal
    axisY.setFillColor(sf::Color::White);  // Warna putih
    
    // ===== MEMBUAT LABEL ANGKA UNTUK SUMBU X =====
    std::vector<sf::Text> labelsX;  // Vector untuk menyimpan label sumbu X
    std::vector<sf::RectangleShape> ticksX;  // Vector untuk tanda kecil di sumbu X
    
    // Loop untuk membuat angka dari -10 sampai 10 di sumbu X
    for (int i = -10; i <= 10; i++) {
        if (i == 0) continue;  // Skip angka 0 agar tidak bertumpuk di tengah
        
        // Buat text untuk angka
        sf::Text label;
        label.setFont(font);  // Set font
        label.setString(std::to_string(i));  // Konversi angka ke string
        label.setCharacterSize(14);  // Ukuran font 14px
        label.setFillColor(sf::Color::White);  // Warna putih
        
        // Hitung posisi label di layar
        float xPos = centerX + (i * scaleX);
        label.setPosition(xPos - 8, centerY + 10);  // Posisi di bawah sumbu X
        labelsX.push_back(label);  // Simpan ke vector
        
        // Buat tanda kecil (tick mark) di sumbu X
        sf::RectangleShape tick(sf::Vector2f(2, 10));  // Lebar 2px, tinggi 10px
        tick.setPosition(xPos, centerY - 5);  // Posisi di sumbu X
        tick.setFillColor(sf::Color::White);
        ticksX.push_back(tick);  // Simpan ke vector
    }
    
    // ===== MEMBUAT LABEL ANGKA UNTUK SUMBU Y =====
    std::vector<sf::Text> labelsY;  // Vector untuk menyimpan label sumbu Y
    std::vector<sf::RectangleShape> ticksY;  // Vector untuk tanda kecil di sumbu Y
    
    // Loop untuk membuat angka dari -10 sampai 10 di sumbu Y
    for (int i = -10; i <= 10; i++) {
        if (i == 0) continue;  // Skip angka 0
        
        // Buat text untuk angka
        sf::Text label;
        label.setFont(font);  // Set font
        label.setString(std::to_string(i));  // Konversi angka ke string
        label.setCharacterSize(14);  // Ukuran font 14px
        label.setFillColor(sf::Color::White);  // Warna putih
        
        // Hitung posisi label di layar
        float yPos = centerY - (i * scaleY);
        label.setPosition(centerX + 10, yPos - 10);  // Posisi di sebelah kanan sumbu Y
        labelsY.push_back(label);  // Simpan ke vector
        
        // Buat tanda kecil (tick mark) di sumbu Y
        sf::RectangleShape tick(sf::Vector2f(10, 2));  // Lebar 10px, tinggi 2px
        tick.setPosition(centerX - 5, yPos);  // Posisi di sumbu Y
        tick.setFillColor(sf::Color::White);
        ticksY.push_back(tick);  // Simpan ke vector
    }
    
    // Label untuk titik origin (0,0)
    sf::Text originLabel;
    originLabel.setFont(font);
    originLabel.setString("0");
    originLabel.setCharacterSize(14);
    originLabel.setFillColor(sf::Color::White);
    originLabel.setPosition(centerX + 10, centerY + 10);  // Posisi di pojok kanan bawah origin
    
    // ===== MEMBUAT TITIK-TITIK GRAFIK FUNGSI =====
    std::vector<sf::Vertex> graphPoints;  // Vector untuk menyimpan titik-titik grafik
    
    // Loop untuk menghitung titik-titik grafik
    // Dari x = -10 sampai x = 10 dengan step kecil
    for (float x = -10.0f; x <= 10.0f; x += 0.01f) {
        // Hitung nilai y dari fungsi (dalam hal ini y = sin(x))
        float y = std::sin(x);
        
        // Konversi dari koordinat matematika ke koordinat layar
        float screenX = centerX + (x * scaleX);  // Koordinat X di layar
        float screenY = centerY - (y * scaleY);  // Koordinat Y di layar (minus karena Y layar terbalik)
        
        // Tambahkan vertex (titik) ke vector dengan warna merah
        graphPoints.push_back(sf::Vertex(sf::Vector2f(screenX, screenY), sf::Color::Red));
    }
    
    // Loop utama program
    while (window.isOpen()) {
        // Event handling - menangani input dari user
        sf::Event event;
        while (window.pollEvent(event)) {
            // Jika window ditutup, keluar dari program
            if (event.type == sf::Event::Closed)
                window.close();
        }
        
        // Clear window dengan warna hitam
        window.clear(sf::Color::Black);
        
        // Gambar grid vertikal (garis vertikal)
        for (auto& line : gridLinesVertical) {
            window.draw(line);
        }
        
        // Gambar grid horizontal (garis horizontal)
        for (auto& line : gridLinesHorizontal) {
            window.draw(line);
        }
        
        // Gambar sumbu X
        window.draw(axisX);
        
        // Gambar sumbu Y
        window.draw(axisY);
        
        // Gambar semua tick marks di sumbu X
        for (auto& tick : ticksX) {
            window.draw(tick);
        }
        
        // Gambar semua tick marks di sumbu Y
        for (auto& tick : ticksY) {
            window.draw(tick);
        }
        
        // Gambar semua label angka di sumbu X
        for (auto& label : labelsX) {
            window.draw(label);
        }
        
        // Gambar semua label angka di sumbu Y
        for (auto& label : labelsY) {
            window.draw(label);
        }
        
        // Gambar label origin (0,0)
        window.draw(originLabel);
        
        // Gambar grafik fungsi sebagai garis (LineStrip menghubungkan semua titik)
        window.draw(&graphPoints[0], graphPoints.size(), sf::LineStrip);
        
        // Tampilkan semua yang sudah digambar ke layar
        window.display();
    }
    
    return 0;
}