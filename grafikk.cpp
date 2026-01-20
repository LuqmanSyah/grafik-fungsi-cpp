#include <SFML/Graphics.hpp>
#include <cmath>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <algorithm>

// ============================================================================
// KONSTANTA KONFIGURASI
// ============================================================================

const float INPUT_BOX_WIDTH = 520.f;
const float INPUT_BOX_HEIGHT = 40.f;
const float BUTTON_WIDTH = 100.f;
const float BUTTON_HEIGHT = 40.f;
const float TOP_BAR_HEIGHT = 100.f;

const int GRID_SIZE = 40; // Balanced between quality and performance
const float GRID_RANGE = 3.5f;

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

struct Triangle {
    sf::Vector2f p[3];
    float depth;
    sf::Color color;
};

struct InputBox {
    sf::RectangleShape box;
    sf::Text text;
    sf::RectangleShape cursor;
    bool focused = false;
    std::string content;
    float cursorTimer = 0.f;
    bool cursorVisible = true;
    sf::Vector2f position;
    
    void update(float dt) {
        cursorTimer += dt;
        if (cursorTimer >= 0.5f) {
            cursorVisible = !cursorVisible;
            cursorTimer = 0.f;
        }
    }
    
    bool contains(sf::Vector2f point) {
        return box.getGlobalBounds().contains(point);
    }
};

struct Button {
    sf::RectangleShape shape;
    sf::Text label;
    bool hovered = false;
    bool pressed = false;
    
    bool contains(sf::Vector2f point) {
        return shape.getGlobalBounds().contains(point);
    }
    
    void update(sf::Vector2f mousePos) {
        hovered = contains(mousePos);
    }
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
            
            err = "Karakter tidak dikenal: '" + std::string(1, c) + "'";
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
                if (funcs.find(t.text) == funcs.end()) { ok = false; return 0; }
                st.push_back(funcs[t.text](a));
            }
        }
        return st.size() == 1 ? st[0] : (ok = false, 0);
    }
};

// ============================================================================
// 3D PROJECTION
// ============================================================================

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

float getDepth(Point3D p, float rotX, float rotY) {
    float cosY = cos(rotY), sinY = sin(rotY);
    float x1 = p.x * cosY - p.z * sinY;
    float z1 = p.x * sinY + p.z * cosY;
    
    float cosX = cos(rotX), sinX = sin(rotX);
    float z2 = p.y * sinX + z1 * cosX;
    
    return z2;
}

// ============================================================================
// MAIN PROGRAM
// ============================================================================

int main() {
    // Enable anti-aliasing
    sf::ContextSettings settings;
    settings.antialiasingLevel = 4;
    
    sf::RenderWindow win(sf::VideoMode(1200, 800), "Grapher 3D - Fungsi 2 Variabel", sf::Style::Default, settings);
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
    std::string err;
    
    // 3D View state
    sf::Vector2f origin = {600, 450};
    float scale = 50;
    float rotX = -0.5f;
    float rotY = 0.3f;
    
    // Interaction state
    bool dragging = false;
    sf::Vector2f dragStart;
    float dragRotX, dragRotY;
    
    // Clock for cursor animation
    sf::Clock clock;
    
    // Input Box
    InputBox inputBox;
    inputBox.content = "sin(x)*cos(y)";
    inputBox.position = {20, 25};
    inputBox.box.setSize({INPUT_BOX_WIDTH, INPUT_BOX_HEIGHT});
    inputBox.box.setPosition(inputBox.position);
    inputBox.box.setFillColor({255, 255, 255});
    inputBox.box.setOutlineThickness(2);
    inputBox.box.setOutlineColor({180, 180, 180});
    
    inputBox.text.setFont(font);
    inputBox.text.setCharacterSize(18);
    inputBox.text.setFillColor({30, 30, 30});
    inputBox.text.setPosition(inputBox.position.x + 10, inputBox.position.y + 8);
    
    inputBox.cursor.setSize({2, 28});
    inputBox.cursor.setFillColor({50, 50, 50});
    
    // Plot Button
    Button plotButton;
    plotButton.shape.setSize({BUTTON_WIDTH, BUTTON_HEIGHT});
    plotButton.shape.setPosition(inputBox.position.x + INPUT_BOX_WIDTH + 15, inputBox.position.y);
    plotButton.shape.setFillColor({70, 130, 200});
    
    plotButton.label.setFont(font);
    plotButton.label.setString("PLOT");
    plotButton.label.setCharacterSize(16);
    plotButton.label.setFillColor({255, 255, 255});
    plotButton.label.setStyle(sf::Text::Bold);
    sf::FloatRect textBounds = plotButton.label.getLocalBounds();
    plotButton.label.setOrigin(textBounds.left + textBounds.width/2.0f, textBounds.top + textBounds.height/2.0f);
    plotButton.label.setPosition(
        plotButton.shape.getPosition().x + BUTTON_WIDTH/2.0f,
        plotButton.shape.getPosition().y + BUTTON_HEIGHT/2.0f
    );
    
    // Compile initial expression
    auto compile = [&]() {
        auto t = parser.parse(inputBox.content, err);
        if (err.empty()) rpn = parser.toRPN(t, err);
        else rpn.clear();
    };
    compile();

    while (win.isOpen()) {
        sf::Event e;
        sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition(win));
        
        while (win.pollEvent(e)) {
            if (e.type == sf::Event::Closed) win.close();
            
            // Paste functionality with Ctrl+V
            if (inputBox.focused && e.type == sf::Event::KeyPressed) {
                if (e.key.code == sf::Keyboard::V && (e.key.control || e.key.system)) {
                    std::string clipboard = sf::Clipboard::getString();
                    inputBox.content += clipboard;
                }
                // Select all with Ctrl+A
                else if (e.key.code == sf::Keyboard::A && (e.key.control || e.key.system)) {
                    // For now, just ignore - could implement text selection later
                }
            }
            
            // Mouse click events
            if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f clickPos(e.mouseButton.x, e.mouseButton.y);
                
                // Click on input box
                if (inputBox.contains(clickPos)) {
                    inputBox.focused = true;
                }
                // Click on plot button
                else if (plotButton.contains(clickPos)) {
                    plotButton.pressed = true;
                    compile();
                }
                // Click on graph area - start dragging
                else if (e.mouseButton.y > TOP_BAR_HEIGHT) {
                    inputBox.focused = false;
                    dragging = true;
                    dragStart = clickPos;
                    dragRotX = rotX;
                    dragRotY = rotY;
                }
                // Click elsewhere - unfocus
                else {
                    inputBox.focused = false;
                }
            }
            
            if (e.type == sf::Event::MouseButtonReleased) {
                dragging = false;
                plotButton.pressed = false;
            }
            
            // Text input when focused
            if (inputBox.focused && e.type == sf::Event::TextEntered) {
                if (e.text.unicode == '\r' || e.text.unicode == '\n') {
                    compile();
                    inputBox.focused = false;
                }
                else if (e.text.unicode == 27) { // Escape
                    inputBox.focused = false;
                }
                else if (e.text.unicode == 8) { // Backspace
                    if (!inputBox.content.empty())
                        inputBox.content.pop_back();
                }
                else if (e.text.unicode >= 32 && e.text.unicode < 127) {
                    inputBox.content += static_cast<char>(e.text.unicode);
                }
            }
            
            // Keyboard shortcuts (when not typing in input box)
            if (e.type == sf::Event::KeyPressed && !inputBox.focused) {
                if (e.key.code == sf::Keyboard::R) {
                    origin = {600, 450};
                    scale = 50;
                    rotX = -0.5f;
                    rotY = 0.3f;
                }
            }
            
            // Zoom
            if (e.type == sf::Event::MouseWheelScrolled && mousePos.y > TOP_BAR_HEIGHT) {
                scale *= e.mouseWheelScroll.delta > 0 ? 1.15f : 0.87f;
                scale = std::min(std::max(scale, 10.f), 300.f);
            }
            
            // Drag rotation
            if (e.type == sf::Event::MouseMoved && dragging) {
                sf::Vector2f delta = sf::Vector2f(e.mouseMove.x, e.mouseMove.y) - dragStart;
                rotY = dragRotY + delta.x * 0.01f;
                rotX = dragRotX + delta.y * 0.01f;
                rotX = std::min(std::max(rotX, -1.5f), 1.5f);
            }
        }
        
        // Update animations
        float dt = clock.restart().asSeconds();
        inputBox.update(dt);
        plotButton.update(mousePos);

        // ===== RENDERING =====
        win.clear({240, 242, 245});
        
        // Top bar background
        sf::RectangleShape topBar({1200, TOP_BAR_HEIGHT});
        topBar.setFillColor({250, 251, 252});
        win.draw(topBar);
        
        sf::RectangleShape topBarLine({1200, 2});
        topBarLine.setPosition(0, TOP_BAR_HEIGHT);
        topBarLine.setFillColor({220, 220, 220});
        win.draw(topBarLine);
        
        // Label
        sf::Text label;
        label.setFont(font);
        label.setCharacterSize(16);
        label.setFillColor({80, 80, 80});
        label.setString("f(x,y) =");
        label.setPosition(inputBox.position.x, inputBox.position.y - 22);
        win.draw(label);
        
        // Input box
        inputBox.text.setString(inputBox.content);
        
        if (inputBox.focused) {
            inputBox.box.setOutlineColor({70, 130, 200});
            inputBox.box.setOutlineThickness(3);
        } else {
            inputBox.box.setOutlineColor({180, 180, 180});
            inputBox.box.setOutlineThickness(2);
        }
        
        win.draw(inputBox.box);
        win.draw(inputBox.text);
        
        // Cursor
        if (inputBox.focused && inputBox.cursorVisible) {
            sf::FloatRect textBounds = inputBox.text.getGlobalBounds();
            inputBox.cursor.setPosition(textBounds.left + textBounds.width + 3, inputBox.position.y + 6);
            win.draw(inputBox.cursor);
        }
        
        // Plot button
        if (plotButton.pressed) {
            plotButton.shape.setFillColor({50, 100, 160});
        } else if (plotButton.hovered) {
            plotButton.shape.setFillColor({90, 150, 220});
        } else {
            plotButton.shape.setFillColor({70, 130, 200});
        }
        win.draw(plotButton.shape);
        win.draw(plotButton.label);
        
        // Help text
        sf::Text helpText;
        helpText.setFont(font);
        helpText.setCharacterSize(13);
        helpText.setFillColor({120, 120, 120});
        helpText.setString("R = Reset View  |  Scroll = Zoom  |  Drag = Rotate  |  Enter = Plot");
        helpText.setPosition(20, 72);
        win.draw(helpText);
        
        // Render 3D surface as wireframe grid
        if (!rpn.empty()) {
            const float step = (2 * GRID_RANGE) / GRID_SIZE;
            
            std::vector<sf::Vertex> lines;
            
            // Generate wireframe mesh
            for (int i = 0; i <= GRID_SIZE; i++) {
                for (int j = 0; j <= GRID_SIZE; j++) {
                    float x = -GRID_RANGE + i * step;
                    float y = -GRID_RANGE + j * step;
                    
                    bool ok;
                    float z = parser.eval(rpn, x, y, ok);
                    
                    if (!ok || std::isnan(z) || std::isinf(z) || fabs(z) > 10) continue;
                    
                    auto p = project3D({x, y, z}, rotX, rotY, scale, origin);
                    
                    // Color gradient based on height
                    float h = std::min(std::max((z + 2) / 4.0f, 0.f), 1.f);
                    sf::Color color(
                        static_cast<sf::Uint8>(50 + h * 150),
                        static_cast<sf::Uint8>(90 + h * 120),
                        static_cast<sf::Uint8>(220 - h * 80)
                    );
                    
                    // Draw horizontal lines
                    if (i < GRID_SIZE) {
                        float x2 = -GRID_RANGE + (i + 1) * step;
                        bool ok2;
                        float z2 = parser.eval(rpn, x2, y, ok2);
                        if (ok2 && !std::isnan(z2) && !std::isinf(z2) && fabs(z2) < 10) {
                            auto p2 = project3D({x2, y, z2}, rotX, rotY, scale, origin);
                            float h2 = std::min(std::max((z2 + 2) / 4.0f, 0.f), 1.f);
                            sf::Color color2(
                                static_cast<sf::Uint8>(50 + h2 * 150),
                                static_cast<sf::Uint8>(90 + h2 * 120),
                                static_cast<sf::Uint8>(220 - h2 * 80)
                            );
                            lines.push_back({p, color});
                            lines.push_back({p2, color2});
                        }
                    }
                    
                    // Draw vertical lines
                    if (j < GRID_SIZE) {
                        float y2 = -GRID_RANGE + (j + 1) * step;
                        bool ok2;
                        float z2 = parser.eval(rpn, x, y2, ok2);
                        if (ok2 && !std::isnan(z2) && !std::isinf(z2) && fabs(z2) < 10) {
                            auto p2 = project3D({x, y2, z2}, rotX, rotY, scale, origin);
                            float h2 = std::min(std::max((z2 + 2) / 4.0f, 0.f), 1.f);
                            sf::Color color2(
                                static_cast<sf::Uint8>(50 + h2 * 150),
                                static_cast<sf::Uint8>(90 + h2 * 120),
                                static_cast<sf::Uint8>(220 - h2 * 80)
                            );
                            lines.push_back({p, color});
                            lines.push_back({p2, color2});
                        }
                    }
                }
            }
            
            // Draw all grid lines
            if (!lines.empty()) {
                win.draw(&lines[0], lines.size(), sf::Lines);
            }
            
            // Draw 3D axes
            auto axisX1 = project3D({-GRID_RANGE, 0, 0}, rotX, rotY, scale, origin);
            auto axisX2 = project3D({GRID_RANGE, 0, 0}, rotX, rotY, scale, origin);
            auto axisY1 = project3D({0, -GRID_RANGE, 0}, rotX, rotY, scale, origin);
            auto axisY2 = project3D({0, GRID_RANGE, 0}, rotX, rotY, scale, origin);
            auto axisZ1 = project3D({0, 0, -GRID_RANGE}, rotX, rotY, scale, origin);
            auto axisZ2 = project3D({0, 0, GRID_RANGE}, rotX, rotY, scale, origin);
            
            sf::Vertex axes[] = {
                {axisX1, {255, 80, 80}}, {axisX2, {255, 80, 80}},
                {axisY1, {80, 255, 80}}, {axisY2, {80, 255, 80}},
                {axisZ1, {80, 80, 255}}, {axisZ2, {80, 80, 255}}
            };
            win.draw(axes, 6, sf::Lines);
            
            // Axis labels
            sf::Text xLabel, yLabel, zLabel;
            xLabel.setFont(font); yLabel.setFont(font); zLabel.setFont(font);
            xLabel.setCharacterSize(14); yLabel.setCharacterSize(14); zLabel.setCharacterSize(14);
            xLabel.setFillColor({200, 50, 50}); yLabel.setFillColor({50, 200, 50}); zLabel.setFillColor({50, 50, 200});
            xLabel.setString("X"); yLabel.setString("Y"); zLabel.setString("Z");
            xLabel.setPosition(axisX2.x + 5, axisX2.y - 5);
            yLabel.setPosition(axisY2.x + 5, axisY2.y - 5);
            zLabel.setPosition(axisZ2.x + 5, axisZ2.y - 5);
            win.draw(xLabel); win.draw(yLabel); win.draw(zLabel);
        }
        
        // Error message
        if (!err.empty()) {
            sf::RectangleShape errBox({1180, 30});
            errBox.setPosition(10, 760);
            errBox.setFillColor({255, 235, 235});
            errBox.setOutlineThickness(1);
            errBox.setOutlineColor({220, 180, 180});
            win.draw(errBox);
            
            sf::Text etxt;
            etxt.setFont(font);
            etxt.setCharacterSize(14);
            etxt.setString("Error: " + err);
            etxt.setPosition(20, 766);
            etxt.setFillColor({200, 50, 50});
            win.draw(etxt);
        }

        win.display();
    }
    return 0;
}