
#include <SFML/Graphics.hpp>
#include <cmath>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <algorithm>

// ============================================================================
// STRUKTUR DATA
// ============================================================================

struct Token {
    enum Type { NUMBER, VAR_X, VAR_Y, OP, FUNC, LPAREN, RPAREN } type;
    double value{};
    std::string text;
    int precedence{};
    bool rightAssoc{};
};

struct Point3D {
    float x, y, z;
    Point3D(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
};

// ============================================================================
// PARSER - Parsing dan Evaluasi Ekspresi dengan 2 Variabel
// ============================================================================

class Parser {
    std::map<std::string, std::function<double(double)>> funcs;
    
public:
    Parser() {
        funcs = {
            {"sin", [](double x) { return sin(x); }},
            {"cos", [](double x) { return cos(x); }},
            {"tan", [](double x) { return tan(x); }},
            {"asin", [](double x) { return asin(x); }},
            {"acos", [](double x) { return acos(x); }},
            {"atan", [](double x) { return atan(x); }},
            {"sinh", [](double x) { return sinh(x); }},
            {"cosh", [](double x) { return cosh(x); }},
            {"tanh", [](double x) { return tanh(x); }},
            {"exp", [](double x) { return exp(x); }},
            {"ln", [](double x) { return log(x); }},
            {"log", [](double x) { return log10(x); }},
            {"sqrt", [](double x) { return sqrt(x); }},
            {"abs", [](double x) { return fabs(x); }},
            {"floor", [](double x) { return floor(x); }},
            {"ceil", [](double x) { return ceil(x); }}
        };
    }

    // Tokenizer: Memecah string menjadi token (mendukung x dan y)
    std::vector<Token> parse(const std::string& s, std::string& err) {
        std::vector<Token> toks;
        err.clear();
        
        for (size_t i = 0; i < s.size();) {
            char c = s[i];
            
            if (isspace(c)) { i++; continue; }
            
            if (isdigit(c) || c == '.') {
                size_t j = i;
                while (j < s.size() && (isdigit(s[j]) || s[j] == '.' || s[j] == 'e' || s[j] == 'E'))
                    j++;
                toks.push_back({Token::NUMBER, strtod(s.c_str() + i, 0)});
                i = j;
                continue;
            }
            
            if (isalpha(c)) {
                size_t j = i;
                while (j < s.size() && isalnum(s[j])) j++;
                std::string id = s.substr(i, j - i);
                
                if (id == "x") toks.push_back({Token::VAR_X});
                else if (id == "y") toks.push_back({Token::VAR_Y});
                else toks.push_back({Token::FUNC, 0, id});
                
                i = j;
                continue;
            }
            
            if (c == '(') { toks.push_back({Token::LPAREN}); i++; continue; }
            if (c == ')') { toks.push_back({Token::RPAREN}); i++; continue; }
            
            if (std::string("+-*/^").find(c) != std::string::npos) {
                int p = (c == '+' || c == '-') ? 1 : (c == '*' || c == '/') ? 2 : 3;
                toks.push_back({Token::OP, 0, std::string(1, c), p, c == '^'});
                i++;
                continue;
            }
            
            err = "Karakter tidak dikenal";
            return {};
        }
        
        // Handle unary minus
        std::vector<Token> out;
        for (size_t i = 0; i < toks.size(); i++) {
            if (toks[i].type == Token::OP && toks[i].text == "-" &&
                (i == 0 || toks[i-1].type == Token::OP || toks[i-1].type == Token::LPAREN))
                out.push_back({Token::NUMBER, 0});
            out.push_back(toks[i]);
        }
        return out;
    }

    // Convert to RPN
    std::vector<Token> toRPN(std::vector<Token> toks, std::string& err) {
        std::vector<Token> out, st;
        
        for (auto& t : toks) {
            if (t.type == Token::NUMBER || t.type == Token::VAR_X || t.type == Token::VAR_Y)
                out.push_back(t);
            else if (t.type == Token::FUNC)
                st.push_back(t);
            else if (t.type == Token::OP) {
                while (!st.empty() && st.back().type == Token::OP &&
                       ((!t.rightAssoc && t.precedence <= st.back().precedence) ||
                        (t.rightAssoc && t.precedence < st.back().precedence))) {
                    out.push_back(st.back());
                    st.pop_back();
                }
                st.push_back(t);
            } else if (t.type == Token::LPAREN)
                st.push_back(t);
            else if (t.type == Token::RPAREN) {
                while (!st.empty() && st.back().type != Token::LPAREN) {
                    out.push_back(st.back());
                    st.pop_back();
                }
                if (st.empty()) { err = "Kurung tidak seimbang"; return {}; }
                st.pop_back();
                if (!st.empty() && st.back().type == Token::FUNC) {
                    out.push_back(st.back());
                    st.pop_back();
                }
            }
        }
        
        while (!st.empty()) {
            out.push_back(st.back());
            st.pop_back();
        }
        return out;
    }

    // Evaluate RPN expression with x and y values
    double eval(std::vector<Token>& rpn, double x, double y, bool& ok) {
        ok = true;
        std::vector<double> st;
        
        for (auto& t : rpn) {
            if (t.type == Token::NUMBER)
                st.push_back(t.value);
            else if (t.type == Token::VAR_X)
                st.push_back(x);
            else if (t.type == Token::VAR_Y)
                st.push_back(y);
            else if (t.type == Token::OP) {
                if (st.size() < 2) { ok = false; return 0; }
                double b = st.back(); st.pop_back();
                double a = st.back(); st.pop_back();
                double r = 0;
                if (t.text == "+") r = a + b;
                else if (t.text == "-") r = a - b;
                else if (t.text == "*") r = a * b;
                else if (t.text == "/") r = a / b;
                else if (t.text == "^") r = pow(a, b);
                st.push_back(r);
            } else if (t.type == Token::FUNC) {
                if (st.empty()) { ok = false; return 0; }
                double a = st.back(); st.pop_back();
                st.push_back(funcs[t.text](a));
            }
        }
        return st.size() == 1 ? st[0] : (ok = false, 0);
    }
};

// ============================================================================
// 3D PROJECTION
// ============================================================================

// Proyeksi perspektif 3D ke 2D
sf::Vector2f project3D(Point3D p, float rotX, float rotY, float scale, sf::Vector2f origin) {
    // Rotation around Y-axis (yaw)
    float cosY = cos(rotY), sinY = sin(rotY);
    float x1 = p.x * cosY - p.z * sinY;
    float z1 = p.x * sinY + p.z * cosY;
    
    // Rotation around X-axis (pitch)
    float cosX = cos(rotX), sinX = sin(rotX);
    float y2 = p.y * cosX - z1 * sinX;
    float z2 = p.y * sinX + z1 * cosX;
    
    // Perspective projection
    float distance = 15.0f;
    float factor = distance / (distance + z2);
    
    return {origin.x + x1 * factor * scale, origin.y - y2 * factor * scale};
}

// ============================================================================
// MAIN PROGRAM
// ============================================================================

int main() {
    sf::RenderWindow win(sf::VideoMode(1200, 750), "Grapher 3D - Fungsi 2 Variabel - Kalkulus 2");
    win.setFramerateLimit(60);
    
    // Load font
    sf::Font font;
    std::vector<std::string> fonts = {
        "C:\\Windows\\Fonts\\Arial.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/System/Library/Fonts/Supplemental/Arial Unicode.ttf"
    };
    for (auto& f : fonts) if (font.loadFromFile(f)) break;

    // Initialize parser
    Parser parser;
    std::vector<Token> rpn;
    std::string expr = "sin(x)*cos(y)", err;
    
    // 3D View state
    sf::Vector2f origin = {600, 375};
    float scale = 50;
    float rotX = -0.5f;  // Pitch
    float rotY = 0.3f;   // Yaw
    
    // Interaction state
    bool dragging = false;
    sf::Vector2f dragStart;
    float dragRotX, dragRotY;
    
    // Compile initial expression
    auto compile = [&]() {
        auto t = parser.parse(expr, err);
        if (err.empty()) rpn = parser.toRPN(t, err);
        else rpn.clear();
    };
    compile();

    while (win.isOpen()) {
        sf::Event e;
        while (win.pollEvent(e)) {
            if (e.type == sf::Event::Closed) win.close();
            
            // Text input
            if (e.type == sf::Event::TextEntered) {
                if (e.text.unicode == '\r') compile();
                else if (e.text.unicode >= 32 && e.text.unicode < 127)
                    expr += static_cast<char>(e.text.unicode);
                else if (e.text.unicode == 8 && !expr.empty())
                    expr.pop_back();
            }
            
            // Reset view
            if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::R) {
                origin = {600, 375};
                scale = 50;
                rotX = -0.5f;
                rotY = 0.3f;
            }
            
            // Zoom with mouse wheel
            if (e.type == sf::Event::MouseWheelScrolled) {
                scale *= e.mouseWheelScroll.delta > 0 ? 1.15f : 0.87f;
                scale = std::min(std::max(scale, 10.f), 200.f);
            }
            
            // Rotate with drag
            if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.y > 60) {
                dragging = true;
                dragStart = {float(e.mouseButton.x), float(e.mouseButton.y)};
                dragRotX = rotX;
                dragRotY = rotY;
            }
            if (e.type == sf::Event::MouseButtonReleased) dragging = false;
            if (e.type == sf::Event::MouseMoved && dragging) {
                sf::Vector2f delta = sf::Vector2f(e.mouseMove.x, e.mouseMove.y) - dragStart;
                rotY = dragRotY + delta.x * 0.01f;
                rotX = dragRotX + delta.y * 0.01f;
                rotX = std::min(std::max(rotX, -1.5f), 1.5f);
            }
        }

        // ===== RENDERING =====
        win.clear({245, 245, 250});
        
        // Top bar
        sf::RectangleShape bar({1200, 60});
        bar.setFillColor({250, 250, 250});
        win.draw(bar);
        
        sf::Text txt;
        txt.setFont(font);
        txt.setCharacterSize(16);
        txt.setFillColor(sf::Color::Black);
        txt.setString("Fungsi f(x,y): " + expr + "\nEnter=plot | R=reset | Scroll=zoom | Drag=rotate");
        txt.setPosition(10, 10);
        win.draw(txt);
        
        // Render 3D surface
        if (!rpn.empty()) {
            const int gridSize = 35;
            const float range = 3.5f;
            const float step = (2 * range) / gridSize;
            
            std::vector<sf::Vertex> lines;
            
            // Generate mesh
            for (int i = 0; i < gridSize; i++) {
                for (int j = 0; j < gridSize; j++) {
                    float x = -range + i * step;
                    float y = -range + j * step;
                    
                    bool ok;
                    float z = parser.eval(rpn, x, y, ok);
                    
                    if (!ok || std::isnan(z) || std::isinf(z) || fabs(z) > 10) continue;
                    
                    auto p = project3D({x, y, z}, rotX, rotY, scale, origin);
                    
                    // Color gradient based on height
                    float h = std::min(std::max((z + 2) / 4, 0.f), 1.f);
                    sf::Color color(50 + h * 150, 90 + h * 100, 200 - h * 50);
                    
                    // Draw grid lines
                    if (i < gridSize - 1) {
                        float x2 = -range + (i + 1) * step;
                        float z2 = parser.eval(rpn, x2, y, ok);
                        if (ok && !std::isnan(z2) && !std::isinf(z2) && fabs(z2) < 10) {
                            auto p2 = project3D({x2, y, z2}, rotX, rotY, scale, origin);
                            lines.push_back({p, color});
                            lines.push_back({p2, color});
                        }
                    }
                    
                    if (j < gridSize - 1) {
                        float y2 = -range + (j + 1) * step;
                        float z2 = parser.eval(rpn, x, y2, ok);
                        if (ok && !std::isnan(z2) && !std::isinf(z2) && fabs(z2) < 10) {
                            auto p2 = project3D({x, y2, z2}, rotX, rotY, scale, origin);
                            lines.push_back({p, color});
                            lines.push_back({p2, color});
                        }
                    }
                }
            }
            
            win.draw(&lines[0], lines.size(), sf::Lines);
            
            // Draw 3D axes
            auto axisX1 = project3D({-range, 0, 0}, rotX, rotY, scale, origin);
            auto axisX2 = project3D({range, 0, 0}, rotX, rotY, scale, origin);
            auto axisY1 = project3D({0, -range, 0}, rotX, rotY, scale, origin);
            auto axisY2 = project3D({0, range, 0}, rotX, rotY, scale, origin);
            auto axisZ1 = project3D({0, 0, -range}, rotX, rotY, scale, origin);
            auto axisZ2 = project3D({0, 0, range}, rotX, rotY, scale, origin);
            
            sf::Vertex axes[] = {
                {axisX1, {255, 0, 0}}, {axisX2, {255, 0, 0}},    // X-axis (red)
                {axisY1, {0, 255, 0}}, {axisY2, {0, 255, 0}},    // Y-axis (green)
                {axisZ1, {0, 0, 255}}, {axisZ2, {0, 0, 255}}     // Z-axis (blue)
            };
            win.draw(axes, 6, sf::Lines);
        }
        
        // Error message
        if (!err.empty()) {
            sf::RectangleShape errBox({1180, 25});
            errBox.setPosition(10, 715);
            errBox.setFillColor({255, 230, 230});
            win.draw(errBox);
            
            sf::Text etxt;
            etxt.setFont(font);
            etxt.setCharacterSize(14);
            etxt.setString("Error: " + err);
            etxt.setPosition(15, 718);
            etxt.setFillColor({200, 0, 0});
            win.draw(etxt);
        }

        win.display();
    }
    return 0;
}