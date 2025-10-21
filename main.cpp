#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <sstream>
#include <iomanip>

class GraphPlotter
{
private:
  sf::RenderWindow &window;
  float width, height;
  float xMin, xMax, yMin, yMax;
  sf::Font font;
  bool fontLoaded;

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

  // Konversi koordinat matematika ke koordinat layar
  sf::Vector2f mathToScreen(float x, float y)
  {
    float screenX = (x - xMin) / (xMax - xMin) * width;
    float screenY = height - (y - yMin) / (yMax - yMin) * height;
    return sf::Vector2f(screenX, screenY);
  }

  // Gambar sumbu koordinat dengan label
  void drawAxes()
  {
    // Gambar sumbu X (horizontal)
    sf::Vertex xAxis[] = {
        sf::Vertex(mathToScreen(xMin, 0), sf::Color(255, 255, 255)),
        sf::Vertex(mathToScreen(xMax, 0), sf::Color(255, 255, 255))};
    window.draw(xAxis, 2, sf::Lines);

    // Gambar sumbu Y (vertikal)
    sf::Vertex yAxis[] = {
        sf::Vertex(mathToScreen(0, yMin), sf::Color(255, 255, 255)),
        sf::Vertex(mathToScreen(0, yMax), sf::Color(255, 255, 255))};
    window.draw(yAxis, 2, sf::Lines);

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

    for (int i = (int)yMin; i <= (int)yMax; i++)
    {
      if (i == 0)
        continue; // Skip sumbu utama
      sf::Vertex line[] = {
          sf::Vertex(mathToScreen(xMin, i), sf::Color(40, 40, 40)),
          sf::Vertex(mathToScreen(xMax, i), sf::Color(40, 40, 40))};
      window.draw(line, 2, sf::Lines);
    }

    // Gambar tanda tick dan label pada sumbu
    if (fontLoaded)
    {
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

      sf::Text yLabel;
      yLabel.setFont(font);
      yLabel.setString("Y");
      yLabel.setCharacterSize(16);
      yLabel.setFillColor(sf::Color::White);
      yLabel.setStyle(sf::Text::Bold);
      sf::Vector2f yAxisEnd = mathToScreen(0, yMax);
      yLabel.setPosition(yAxisEnd.x + 10, yAxisEnd.y + 5);
      window.draw(yLabel);

      // Label origin (0,0)
      sf::Text origin;
      origin.setFont(font);
      origin.setString("0");
      origin.setCharacterSize(12);
      origin.setFillColor(sf::Color::White);
      sf::Vector2f originPos = mathToScreen(0, 0);
      origin.setPosition(originPos.x + 5, originPos.y + 5);
      window.draw(origin);
    }
  }

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

  void setRange(float xmin, float xmax, float ymin, float ymax)
  {
    xMin = xmin;
    xMax = xmax;
    yMin = ymin;
    yMax = ymax;
  }
};

// Definisi fungsi-fungsi matematika
float sineFunction(float x)
{
  return 3.0f * std::sin(x);
}

float cosineFunction(float x)
{
  return 2.0f * std::cos(x);
}

float quadraticFunction(float x)
{
  return 0.1f * x * x - 2.0f;
}

float cubicFunction(float x)
{
  return 0.02f * x * x * x;
}

int main()
{
  // Buat window
  sf::RenderWindow window(sf::VideoMode(1000, 700), "SFML Graph Plotter - Sumbu X dan Y");
  window.setFramerateLimit(60);

  // Buat graph plotter
  GraphPlotter plotter(window, 1000, 700);

  // Set range
  plotter.setRange(-10, 10, -10, 10);

  // Daftar fungsi untuk legenda
  std::vector<std::pair<std::string, sf::Color>> functions = {
      {"y = 3sin(x)", sf::Color::Red},
      {"y = 2cos(x)", sf::Color::Green},
      {"y = 0.1x^2 - 2", sf::Color::Blue},
      {"y = 0.02x^3", sf::Color::Yellow}};

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

  return 0;
}