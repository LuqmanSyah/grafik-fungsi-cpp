#include <SFML/Graphics.hpp>
#include <cmath>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <algorithm>
#include <sstream>
#include <iomanip>

// ============================================================================
// KONSTANTA KONFIGURASI
// ============================================================================

const float INPUT_BOX_WIDTH = 420.f;
const float INPUT_BOX_HEIGHT = 40.f;
const float BUTTON_WIDTH = 100.f;
const float BUTTON_HEIGHT = 40.f;
const float TOP_BAR_HEIGHT = 130.f;
const float RIGHT_PANEL_WIDTH = 280.f;

const int GRID_SIZE = 50; // Increased for better quality
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

struct Function3D {
    std::string expr;
    std::vector<Token> rpn;
    sf::Color color;
    bool visible = true;
    bool showWireframe = true;
    bool showSurface = false;
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
                while (j < s.size() && (isdigit(s[j]) || s[j] == '.' || s[j] == 'e' || s[j] == 'E' || s[j] == '-'))
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
                else if (id == "pi") toks.push_back({Token::NUMBER, 3.14159265358979});
                else if (id == "e") toks.push_back({Token::NUMBER, 2.71828182845905});
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
            if (st.back().type == Token::LPAREN) {
                err = "Kurung tidak seimbang";
                return {};
            }
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
                else if (t.text == "/") r = (b != 0) ? a / b : (ok = false, 0);
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
    float cosY = cos(rotY), sinY = sin(rotY);
    float x1 = p.x * cosY - p.z * sinY;
    float z1 = p.x * sinY + p.z * cosY;
    
    float cosX = cos(rotX), sinX = sin(rotX);
    float y2 = p.y * cosX - z1 * sinX;
    float z2 = p.y * sinX + z1 * cosX;
    
    float distance = 15.0f;
    float factor = distance / (distance + z2);
    
    return {origin.x + x1 * factor * scale, origin.y - y2 * factor * scale};
}

std::string formatNumber(double val) {
    std::ostringstream oss;
    if (fabs(val) < 0.01 || fabs(val) > 1000)
        oss << std::scientific << std::setprecision(2) << val;
    else
        oss << std::fixed << std::setprecision(2) << val;
    return oss.str();
}

// ============================================================================
// MAIN PROGRAM
// ============================================================================

int main() {
    sf::ContextSettings settings;
    settings.antialiasingLevel = 4;
    
    sf::RenderWindow win(sf::VideoMode(1400, 900), "Grapher 3D - Kalkulus 2 (Revised)", sf::Style::Default, settings);
    win.setFramerateLimit(60);
    
    sf::Font font;
    std::vector<std::string> fonts = {
        "C:\\Windows\\Fonts\\Arial.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/System/Library/Fonts/Supplemental/Arial Unicode.ttf"
    };
    for (auto& f : fonts) if (font.loadFromFile(f)) break;

    Parser parser;
    std::vector<Function3D> functions;
    std::string err;
    int selectedFunc = -1;
    
    sf::Vector2f origin = {(1400 - RIGHT_PANEL_WIDTH) / 2.f, 500};
    float scale = 50;
    float rotX = -0.5f;
    float rotY = 0.3f;
    
    bool dragging = false;
    sf::Vector2f dragStart;
    float dragRotX, dragRotY;
    
    bool showAxes = true;
    bool showGrid = true;
    sf::Clock clock;
    
    // Input Box
    InputBox inputBox;
    inputBox.content = "sin(x)*cos(y)";
    inputBox.position = {20, 35};
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
    
    // Add Button
    Button addButton;
    addButton.shape.setSize({BUTTON_WIDTH, BUTTON_HEIGHT});
    addButton.shape.setPosition(inputBox.position.x + INPUT_BOX_WIDTH + 15, inputBox.position.y);
    addButton.shape.setFillColor({70, 130, 200});
    
    addButton.label.setFont(font);
    addButton.label.setString("ADD");
    addButton.label.setCharacterSize(16);
    addButton.label.setFillColor({255, 255, 255});
    addButton.label.setStyle(sf::Text::Bold);
    sf::FloatRect textBounds = addButton.label.getLocalBounds();
    addButton.label.setOrigin(textBounds.left + textBounds.width/2.0f, textBounds.top + textBounds.height/2.0f);
    addButton.label.setPosition(
        addButton.shape.getPosition().x + BUTTON_WIDTH/2.0f,
        addButton.shape.getPosition().y + BUTTON_HEIGHT/2.0f
    );
    
    auto compile = [&]() {
        auto t = parser.parse(inputBox.content, err);
        if (err.empty()) {
            auto rpn = parser.toRPN(t, err);
            if (err.empty() && !rpn.empty()) {
                Function3D f;
                f.expr = inputBox.content;
                f.rpn = rpn;
                static int colorIdx = 0;
                sf::Color colors[] = {
                    {70, 120, 220}, {220, 70, 120}, {70, 220, 120}, 
                    {220, 170, 70}, {170, 70, 220}, {70, 220, 220}
                };
                f.color = colors[colorIdx++ % 6];
                functions.push_back(f);
                inputBox.content.clear();
            }
        }
    };

    while (win.isOpen()) {
        sf::Event e;
        sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition(win));
        
        while (win.pollEvent(e)) {
            if (e.type == sf::Event::Closed) win.close();
            
            if (inputBox.focused && e.type == sf::Event::KeyPressed) {
                if (e.key.code == sf::Keyboard::V && (e.key.control || e.key.system)) {
                    std::string clipboard = sf::Clipboard::getString();
                    inputBox.content += clipboard;
                }
            }
            
            if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f clickPos(e.mouseButton.x, e.mouseButton.y);
                
                if (inputBox.contains(clickPos)) {
                    inputBox.focused = true;
                }
                else if (addButton.contains(clickPos)) {
                    addButton.pressed = true;
                    compile();
                }
                else if (clickPos.x >= 1400 - RIGHT_PANEL_WIDTH && clickPos.y >= TOP_BAR_HEIGHT) {
                    int idx = (clickPos.y - TOP_BAR_HEIGHT - 40) / 30;
                    if (idx >= 0 && idx < functions.size()) {
                        selectedFunc = idx;
                    }
                }
                else if (e.mouseButton.y > TOP_BAR_HEIGHT && clickPos.x < 1400 - RIGHT_PANEL_WIDTH) {
                    inputBox.focused = false;
                    dragging = true;
                    dragStart = clickPos;
                    dragRotX = rotX;
                    dragRotY = rotY;
                }
                else {
                    inputBox.focused = false;
                }
            }
            
            if (e.type == sf::Event::MouseButtonReleased) {
                dragging = false;
                addButton.pressed = false;
            }
            
            if (inputBox.focused && e.type == sf::Event::TextEntered) {
                if (e.text.unicode == '\r' || e.text.unicode == '\n') {
                    compile();
                    inputBox.focused = false;
                }
                else if (e.text.unicode == 27) {
                    inputBox.focused = false;
                }
                else if (e.text.unicode == 8) {
                    if (!inputBox.content.empty())
                        inputBox.content.pop_back();
                }
                else if (e.text.unicode >= 32 && e.text.unicode < 127) {
                    inputBox.content += static_cast<char>(e.text.unicode);
                }
            }
            
            if (e.type == sf::Event::KeyPressed && !inputBox.focused) {
                if (e.key.code == sf::Keyboard::R) {
                    origin = {(1400 - RIGHT_PANEL_WIDTH) / 2.f, 500};
                    scale = 50;
                    rotX = -0.5f;
                    rotY = 0.3f;
                }
                if (e.key.code == sf::Keyboard::A) showAxes = !showAxes;
                if (e.key.code == sf::Keyboard::G) showGrid = !showGrid;
                if (e.key.code == sf::Keyboard::Delete && selectedFunc >= 0 && selectedFunc < functions.size()) {
                    functions.erase(functions.begin() + selectedFunc);
                    selectedFunc = -1;
                }
            }
            
            if (e.type == sf::Event::MouseWheelScrolled && mousePos.y > TOP_BAR_HEIGHT && mousePos.x < 1400 - RIGHT_PANEL_WIDTH) {
                scale *= e.mouseWheelScroll.delta > 0 ? 1.15f : 0.87f;
                scale = std::min(std::max(scale, 10.f), 300.f);
            }
            
            if (e.type == sf::Event::MouseMoved && dragging) {
                sf::Vector2f delta = sf::Vector2f(e.mouseMove.x, e.mouseMove.y) - dragStart;
                rotY = dragRotY + delta.x * 0.01f;
                rotX = dragRotX + delta.y * 0.01f;
                rotX = std::min(std::max(rotX, -1.5f), 1.5f);
            }
        }
        
        float dt = clock.restart().asSeconds();
        inputBox.update(dt);
        addButton.update(mousePos);

        // ===== RENDERING =====
        win.clear({235, 238, 242});
        
        // Top bar
        sf::RectangleShape topBar({1400, TOP_BAR_HEIGHT});
        topBar.setFillColor({248, 249, 250});
        win.draw(topBar);
        
        sf::RectangleShape topBarLine({1400, 2});
        topBarLine.setPosition(0, TOP_BAR_HEIGHT);
        topBarLine.setFillColor({215, 218, 222});
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
        
        if (inputBox.focused && inputBox.cursorVisible) {
            sf::FloatRect textBounds = inputBox.text.getGlobalBounds();
            inputBox.cursor.setPosition(textBounds.left + textBounds.width + 3, inputBox.position.y + 6);
            win.draw(inputBox.cursor);
        }
        
        // Add button
        if (addButton.pressed) {
            addButton.shape.setFillColor({50, 100, 160});
        } else if (addButton.hovered) {
            addButton.shape.setFillColor({90, 150, 220});
        } else {
            addButton.shape.setFillColor({70, 130, 200});
        }
        win.draw(addButton.shape);
        win.draw(addButton.label);
        
        // Help text
        sf::Text helpText;
        helpText.setFont(font);
        helpText.setCharacterSize(12);
        helpText.setFillColor({110, 110, 110});
        helpText.setString("Enter = Add  |  R = Reset  |  G = Grid  |  A = Axes  |  Delete = Remove  |  Drag = Rotate  |  Scroll = Zoom");
        helpText.setPosition(20, 85);
        win.draw(helpText);
        
        sf::Text constText;
        constText.setFont(font);
        constText.setCharacterSize(11);
        constText.setFillColor({120, 120, 120});
        constText.setString("Konstanta: pi, e  |  Fungsi: sin, cos, tan, exp, ln, sqrt, abs, dll");
        constText.setPosition(20, 105);
        win.draw(constText);
        
        // Draw 3D surfaces
        for (auto& func : functions) {
            if (!func.visible) continue;
            
            const float step = (2 * GRID_RANGE) / GRID_SIZE;
            std::vector<sf::Vertex> lines;
            
            for (int i = 0; i <= GRID_SIZE; i++) {
                for (int j = 0; j <= GRID_SIZE; j++) {
                    float x = -GRID_RANGE + i * step;
                    float y = -GRID_RANGE + j * step;
                    
                    bool ok;
                    float z = parser.eval(func.rpn, x, y, ok);
                    
                    if (!ok || std::isnan(z) || std::isinf(z) || fabs(z) > 10) continue;
                    
                    auto p = project3D({x, y, z}, rotX, rotY, scale, origin);
                    
                    float h = std::min(std::max((z + 2) / 4.0f, 0.f), 1.f);
                    sf::Color baseColor = func.color;
                    sf::Color color(
                        static_cast<sf::Uint8>(baseColor.r * (0.4f + h * 0.6f)),
                        static_cast<sf::Uint8>(baseColor.g * (0.4f + h * 0.6f)),
                        static_cast<sf::Uint8>(baseColor.b * (0.4f + h * 0.6f))
                    );
                    
                    if (i < GRID_SIZE) {
                        float x2 = -GRID_RANGE + (i + 1) * step;
                        bool ok2;
                        float z2 = parser.eval(func.rpn, x2, y, ok2);
                        if (ok2 && !std::isnan(z2) && !std::isinf(z2) && fabs(z2) < 10) {
                            auto p2 = project3D({x2, y, z2}, rotX, rotY, scale, origin);
                            float h2 = std::min(std::max((z2 + 2) / 4.0f, 0.f), 1.f);
                            sf::Color color2(
                                static_cast<sf::Uint8>(baseColor.r * (0.4f + h2 * 0.6f)),
                                static_cast<sf::Uint8>(baseColor.g * (0.4f + h2 * 0.6f)),
                                static_cast<sf::Uint8>(baseColor.b * (0.4f + h2 * 0.6f))
                            );
                            lines.push_back({p, color});
                            lines.push_back({p2, color2});
                        }
                    }
                    
                    if (j < GRID_SIZE) {
                        float y2 = -GRID_RANGE + (j + 1) * step;
                        bool ok2;
                        float z2 = parser.eval(func.rpn, x, y2, ok2);
                        if (ok2 && !std::isnan(z2) && !std::isinf(z2) && fabs(z2) < 10) {
                            auto p2 = project3D({x, y2, z2}, rotX, rotY, scale, origin);
                            float h2 = std::min(std::max((z2 + 2) / 4.0f, 0.f), 1.f);
                            sf::Color color2(
                                static_cast<sf::Uint8>(baseColor.r * (0.4f + h2 * 0.6f)),
                                static_cast<sf::Uint8>(baseColor.g * (0.4f + h2 * 0.6f)),
                                static_cast<sf::Uint8>(baseColor.b * (0.4f + h2 * 0.6f))
                            );
                            lines.push_back({p, color});
                            lines.push_back({p2, color2});
                        }
                    }
                }
            }
            
            if (!lines.empty()) {
                win.draw(&lines[0], lines.size(), sf::Lines);
            }
        }
        
        // Draw 3D axes
        if (showAxes) {
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
            
            sf::Text xLabel, yLabel, zLabel;
            xLabel.setFont(font); yLabel.setFont(font); zLabel.setFont(font);
            xLabel.setCharacterSize(14); yLabel.setCharacterSize(14); zLabel.setCharacterSize(14);
            xLabel.setFillColor({220, 50, 50}); yLabel.setFillColor({50, 220, 50}); zLabel.setFillColor({50, 50, 220});
            xLabel.setString("X"); yLabel.setString("Y"); zLabel.setString("Z");
            xLabel.setPosition(axisX2.x + 5, axisX2.y - 5);
            yLabel.setPosition(axisY2.x + 5, axisY2.y - 5);
            zLabel.setPosition(axisZ2.x + 5, axisZ2.y - 5);
            win.draw(xLabel); win.draw(yLabel); win.draw(zLabel);
        }
        
        // Right panel
        sf::RectangleShape rightPanel({RIGHT_PANEL_WIDTH, 900});
        rightPanel.setPosition(1400 - RIGHT_PANEL_WIDTH, 0);
        rightPanel.setFillColor({248, 249, 250});
        win.draw(rightPanel);
        
        sf::RectangleShape panelBorder({2, 900});
        panelBorder.setPosition(1400 - RIGHT_PANEL_WIDTH - 2, 0);
        panelBorder.setFillColor({215, 218, 222});
        win.draw(panelBorder);
        
        sf::Text panelTitle;
        panelTitle.setFont(font);
        panelTitle.setCharacterSize(15);
        panelTitle.setFillColor({60, 60, 60});
        panelTitle.setStyle(sf::Text::Bold);
        panelTitle.setString("Daftar Fungsi (" + std::to_string(functions.size()) + ")");
        panelTitle.setPosition(1400 - RIGHT_PANEL_WIDTH + 15, TOP_BAR_HEIGHT + 10);
        win.draw(panelTitle);
        
        for (size_t i = 0; i < functions.size(); i++) {
            float y = TOP_BAR_HEIGHT + 40 + i * 30;
            
            sf::RectangleShape funcBg({RIGHT_PANEL_WIDTH - 30, 26});
            funcBg.setPosition(1400 - RIGHT_PANEL_WIDTH + 15, y);
            funcBg.setFillColor(i == selectedFunc ? sf::Color(220, 235, 255) : sf::Color(255, 255, 255));
            funcBg.setOutlineThickness(1);
            funcBg.setOutlineColor({210, 210, 210});
            win.draw(funcBg);
            
            sf::CircleShape colorDot(6);
            colorDot.setPosition(1400 - RIGHT_PANEL_WIDTH + 22, y + 7);
            colorDot.setFillColor(functions[i].color);
            win.draw(colorDot);
            
            sf::Text funcText;
            funcText.setFont(font);
            funcText.setCharacterSize(11);
            funcText.setFillColor({40, 40, 40});
            std::string label = functions[i].expr;
            if (label.length() > 28) label = label.substr(0, 25) + "...";
            funcText.setString(label);
            funcText.setPosition(1400 - RIGHT_PANEL_WIDTH + 40, y + 6);
            win.draw(funcText);
        }
        
        // Info panel
        sf::Text infoTitle;
        infoTitle.setFont(font);
        infoTitle.setCharacterSize(14);
        infoTitle.setFillColor({60, 60, 60});
        infoTitle.setStyle(sf::Text::Bold);
        infoTitle.setString("Info View");
        infoTitle.setPosition(1400 - RIGHT_PANEL_WIDTH + 15, TOP_BAR_HEIGHT + 300);
        win.draw(infoTitle);
        
        sf::Text rotInfo;
        rotInfo.setFont(font);
        rotInfo.setCharacterSize(11);
        rotInfo.setFillColor({90, 90, 90});
        rotInfo.setString(
            "Rotation X: " + formatNumber(rotX * 57.2958) + "\xB0\n"
            "Rotation Y: " + formatNumber(rotY * 57.2958) + "\xB0\n"
            "Scale: " + std::to_string(int(scale)) + " px/unit\n"
            "Grid: " + std::string(showGrid ? "ON" : "OFF") + "\n"
            "Axes: " + std::string(showAxes ? "ON" : "OFF")
        );
        rotInfo.setPosition(1400 - RIGHT_PANEL_WIDTH + 15, TOP_BAR_HEIGHT + 325);
        rotInfo.setLineSpacing(1.5f);
        win.draw(rotInfo);
        
        // Status bar
        sf::RectangleShape statusBar({1400, 35});
        statusBar.setPosition(0, 865);
        statusBar.setFillColor({248, 249, 250});
        win.draw(statusBar);
        
        sf::RectangleShape statusBorder({1400, 2});
        statusBorder.setPosition(0, 865);
        statusBorder.setFillColor({215, 218, 222});
        win.draw(statusBorder);
        
        if (!err.empty()) {
            sf::Text etxt;
            etxt.setFont(font);
            etxt.setCharacterSize(13);
            etxt.setString("Error: " + err);
            etxt.setPosition(15, 873);
            etxt.setFillColor({200, 50, 50});
            win.draw(etxt);
        } else {
            sf::Text statusTxt;
            statusTxt.setFont(font);
            statusTxt.setCharacterSize(12);
            statusTxt.setFillColor({90, 90, 90});
            statusTxt.setString("Functions: " + std::to_string(functions.size()) + 
                              " | Grid Resolution: " + std::to_string(GRID_SIZE) + "x" + std::to_string(GRID_SIZE));
            statusTxt.setPosition(15, 873);
            win.draw(statusTxt);
        }

        win.display();
    }
    return 0;
}