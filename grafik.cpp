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
// STRUKTUR DATA
// ============================================================================

struct Token {
    enum Type { NUMBER, VAR_X, OP, FUNC, LPAREN, RPAREN } type;
    double value{};
    std::string text;
    int precedence{};
    bool rightAssoc{};
};

struct Function {
    std::string expr;
    std::vector<Token> rpn;
    sf::Color color;
    bool visible = true;
    bool showDerivative = false;
};

// ============================================================================
// PARSER - Parsing dan Evaluasi Ekspresi
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
            
            err = "Karakter tidak dikenal: " + std::string(1, c);
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
            if (t.type == Token::NUMBER || t.type == Token::VAR_X)
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

    double eval(std::vector<Token>& rpn, double x, bool& ok) {
        ok = true;
        std::vector<double> st;
        
        for (auto& t : rpn) {
            if (t.type == Token::NUMBER)
                st.push_back(t.value);
            else if (t.type == Token::VAR_X)
                st.push_back(x);
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
                auto it = funcs.find(t.text);
                if (it != funcs.end())
                    st.push_back(it->second(a));
                else { ok = false; return 0; }
            }
        }
        return st.size() == 1 ? st[0] : (ok = false, 0);
    }

    // Numerical derivative
    double derivative(std::vector<Token>& rpn, double x, bool& ok) {
        const double h = 1e-6;
        double y1 = eval(rpn, x + h, ok);
        if (!ok) return 0;
        double y2 = eval(rpn, x - h, ok);
        if (!ok) return 0;
        return (y1 - y2) / (2 * h);
    }
};

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

std::string formatNumber(double val) {
    std::ostringstream oss;
    if (fabs(val) < 0.001 || fabs(val) > 10000)
        oss << std::scientific << std::setprecision(2) << val;
    else
        oss << std::fixed << std::setprecision(2) << val;
    return oss.str();
}

// ============================================================================
// MAIN PROGRAM
// ============================================================================

int main() {
    const float TOP_BAR_HEIGHT = 90;
    const float BOTTOM_BAR_HEIGHT = 35;
    const float RIGHT_PANEL_WIDTH = 250;
    const float WINDOW_WIDTH = 1400;
    const float WINDOW_HEIGHT = 800;
    const float GRAPH_TOP = TOP_BAR_HEIGHT;
    const float GRAPH_BOTTOM = WINDOW_HEIGHT - BOTTOM_BAR_HEIGHT;
    const float GRAPH_RIGHT = WINDOW_WIDTH - RIGHT_PANEL_WIDTH;
    const float GRAPH_HEIGHT = GRAPH_BOTTOM - GRAPH_TOP;
    
    sf::RenderWindow win(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Grapher 2D - Kalkulus 2 (Revised)");
    win.setFramerateLimit(60);
    
    sf::Font font;
    std::vector<std::string> fonts = {
        "C:\\Windows\\Fonts\\Arial.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/System/Library/Fonts/Supplemental/Arial Unicode.ttf"
    };
    for (auto& f : fonts) if (font.loadFromFile(f)) break;

    Parser parser;
    std::vector<Function> functions;
    std::string currentExpr = "sin(x)", err;
    int selectedFunc = -1;
    
    sf::Vector2f origin = {GRAPH_RIGHT / 2, GRAPH_TOP + GRAPH_HEIGHT / 2};
    float scale = 60;
    
    bool dragging = false;
    sf::Vector2f dragStart, originStart;
    sf::Vector2i mousePos;
    
    bool showGrid = true;
    bool showAxesNumbers = true;
    bool showCrosshair = true;

    auto compile = [&]() {
        auto t = parser.parse(currentExpr, err);
        if (err.empty()) {
            auto rpn = parser.toRPN(t, err);
            if (err.empty() && !rpn.empty()) {
                Function f;
                f.expr = currentExpr;
                f.rpn = rpn;
                static int colorIdx = 0;
                sf::Color colors[] = {{50,90,200}, {200,50,90}, {50,200,90}, {200,150,50}, {150,50,200}};
                f.color = colors[colorIdx++ % 5];
                functions.push_back(f);
                currentExpr.clear();
            }
        }
    };

    while (win.isOpen()) {
        sf::Event e;
        while (win.pollEvent(e)) {
            if (e.type == sf::Event::Closed) win.close();
            
            if (e.type == sf::Event::TextEntered) {
                if (e.text.unicode == '\r') compile();
                else if (e.text.unicode >= 32 && e.text.unicode < 127)
                    currentExpr += static_cast<char>(e.text.unicode);
                else if (e.text.unicode == 8 && !currentExpr.empty())
                    currentExpr.pop_back();
            }
            
            if (e.type == sf::Event::KeyPressed) {
                if (e.key.code == sf::Keyboard::R) {
                    origin = {GRAPH_RIGHT / 2, GRAPH_TOP + GRAPH_HEIGHT / 2};
                    scale = 60;
                }
                if (e.key.code == sf::Keyboard::G) showGrid = !showGrid;
                if (e.key.code == sf::Keyboard::N) showAxesNumbers = !showAxesNumbers;
                if (e.key.code == sf::Keyboard::C) showCrosshair = !showCrosshair;
                if (e.key.code == sf::Keyboard::Delete && selectedFunc >= 0 && selectedFunc < functions.size()) {
                    functions.erase(functions.begin() + selectedFunc);
                    selectedFunc = -1;
                }
            }
            
            if (e.type == sf::Event::MouseWheelScrolled) {
                float mouseX = e.mouseWheelScroll.x;
                float mouseY = e.mouseWheelScroll.y;
                if (mouseX < GRAPH_RIGHT && mouseY >= GRAPH_TOP && mouseY <= GRAPH_BOTTOM) {
                    sf::Vector2f mouse(mouseX, mouseY);
                    sf::Vector2f worldBefore = {(mouse.x - origin.x) / scale, (origin.y - mouse.y) / scale};
                    scale *= e.mouseWheelScroll.delta > 0 ? 1.15f : 0.87f;
                    scale = std::min(std::max(scale, 10.f), 500.f);
                    sf::Vector2f screenAfter = {origin.x + worldBefore.x * scale, origin.y - worldBefore.y * scale};
                    origin += mouse - screenAfter;
                }
            }
            
            if (e.type == sf::Event::MouseButtonPressed) {
                float mouseX = e.mouseButton.x;
                float mouseY = e.mouseButton.y;
                
                // Check if clicking on function list
                if (mouseX >= GRAPH_RIGHT && mouseY >= GRAPH_TOP) {
                    int idx = (mouseY - GRAPH_TOP - 10) / 25;
                    if (idx >= 0 && idx < functions.size()) {
                        selectedFunc = idx;
                    }
                } else if (mouseX < GRAPH_RIGHT && mouseY >= GRAPH_TOP && mouseY <= GRAPH_BOTTOM) {
                    dragging = true;
                    dragStart = {float(mouseX), float(mouseY)};
                    originStart = origin;
                }
            }
            
            if (e.type == sf::Event::MouseButtonReleased) dragging = false;
            if (e.type == sf::Event::MouseMoved) {
                mousePos = {e.mouseMove.x, e.mouseMove.y};
                if (dragging) {
                    origin = originStart + (sf::Vector2f(e.mouseMove.x, e.mouseMove.y) - dragStart);
                }
            }
        }

        // ===== RENDERING =====
        win.clear(sf::Color::White);
        
        // Draw grid
        if (showGrid) {
            sf::VertexArray grid(sf::Lines);
            for (int i = -50; i < 50; i++) {
                float x = origin.x + i * scale * 0.5f;
                if (x >= 0 && x <= GRAPH_RIGHT) {
                    grid.append({{x, GRAPH_TOP}, {230, 230, 230}});
                    grid.append({{x, GRAPH_BOTTOM}, {230, 230, 230}});
                }
                
                float y = origin.y + i * scale * 0.5f;
                if (y >= GRAPH_TOP && y <= GRAPH_BOTTOM) {
                    grid.append({{0, y}, {230, 230, 230}});
                    grid.append({{GRAPH_RIGHT, y}, {230, 230, 230}});
                }
            }
            win.draw(grid);
        }
        
        // Draw axes
        sf::VertexArray axes(sf::Lines);
        if (origin.x >= 0 && origin.x <= GRAPH_RIGHT) {
            axes.append({{origin.x, GRAPH_TOP}, {180, 80, 80}});
            axes.append({{origin.x, GRAPH_BOTTOM}, {180, 80, 80}});
        }
        if (origin.y >= GRAPH_TOP && origin.y <= GRAPH_BOTTOM) {
            axes.append({{0, origin.y}, {180, 80, 80}});
            axes.append({{GRAPH_RIGHT, origin.y}, {180, 80, 80}});
        }
        win.draw(axes);
        
        // Draw axis numbers
        if (showAxesNumbers) {
            sf::Text numText;
            numText.setFont(font);
            numText.setCharacterSize(11);
            numText.setFillColor({100, 100, 100});
            
            for (int i = -30; i <= 30; i++) {
                if (i == 0) continue;
                
                float x = origin.x + i * scale;
                if (x >= 0 && x <= GRAPH_RIGHT && origin.y >= GRAPH_TOP && origin.y <= GRAPH_BOTTOM) {
                    numText.setString(std::to_string(i));
                    numText.setPosition(x - 8, origin.y + 5);
                    win.draw(numText);
                }
                
                float y = origin.y - i * scale;
                if (y >= GRAPH_TOP && y <= GRAPH_BOTTOM && origin.x >= 0 && origin.x <= GRAPH_RIGHT) {
                    numText.setString(std::to_string(i));
                    numText.setPosition(origin.x + 5, y - 8);
                    win.draw(numText);
                }
            }
        }
        
        // Draw functions
        for (auto& func : functions) {
            if (!func.visible) continue;
            
            // Draw main function
            sf::VertexArray curve(sf::LineStrip);
            double prevY = 0;
            bool havePrev = false;
            
            for (int px = 0; px < GRAPH_RIGHT; px++) {
                bool ok;
                double x = (px - origin.x) / scale;
                double y = parser.eval(func.rpn, x, ok);
                
                if (ok && !std::isnan(y) && !std::isinf(y) && fabs(y) < 1e6) {
                    float screenY = origin.y - float(y) * scale;
                    
                    if (screenY >= GRAPH_TOP && screenY <= GRAPH_BOTTOM) {
                        if (havePrev && fabs(y - prevY) * scale > 3000) {
                            win.draw(curve);
                            curve.clear();
                        }
                        
                        curve.append({{float(px), screenY}, func.color});
                        prevY = y;
                        havePrev = true;
                    } else {
                        if (curve.getVertexCount() > 1) win.draw(curve);
                        curve.clear();
                        havePrev = false;
                    }
                } else {
                    if (curve.getVertexCount() > 1) win.draw(curve);
                    curve.clear();
                    havePrev = false;
                }
            }
            if (curve.getVertexCount() > 1) win.draw(curve);
            
            // Draw derivative if enabled
            if (func.showDerivative) {
                sf::VertexArray deriv(sf::LineStrip);
                for (int px = 0; px < GRAPH_RIGHT; px++) {
                    bool ok;
                    double x = (px - origin.x) / scale;
                    double dy = parser.derivative(func.rpn, x, ok);
                    
                    if (ok && !std::isnan(dy) && !std::isinf(dy) && fabs(dy) < 1e6) {
                        float screenY = origin.y - float(dy) * scale;
                        if (screenY >= GRAPH_TOP && screenY <= GRAPH_BOTTOM) {
                            sf::Color derivColor = func.color;
                            derivColor.a = 120;
                            deriv.append({{float(px), screenY}, derivColor});
                        }
                    }
                }
                if (deriv.getVertexCount() > 1) win.draw(deriv);
            }
        }
        
        // Draw crosshair
        if (showCrosshair && mousePos.x < GRAPH_RIGHT && mousePos.y >= GRAPH_TOP && mousePos.y <= GRAPH_BOTTOM) {
            sf::VertexArray cross(sf::Lines);
            cross.append({{float(mousePos.x), GRAPH_TOP}, {150, 150, 150, 100}});
            cross.append({{float(mousePos.x), GRAPH_BOTTOM}, {150, 150, 150, 100}});
            cross.append({{0, float(mousePos.y)}, {150, 150, 150, 100}});
            cross.append({{GRAPH_RIGHT, float(mousePos.y)}, {150, 150, 150, 100}});
            win.draw(cross);
            
            double worldX = (mousePos.x - origin.x) / scale;
            double worldY = (origin.y - mousePos.y) / scale;
            
            sf::Text coordText;
            coordText.setFont(font);
            coordText.setCharacterSize(12);
            coordText.setFillColor(sf::Color::Black);
            coordText.setString("(" + formatNumber(worldX) + ", " + formatNumber(worldY) + ")");
            coordText.setPosition(mousePos.x + 10, mousePos.y - 20);
            win.draw(coordText);
        }
        
        // Top bar
        sf::RectangleShape bar({WINDOW_WIDTH, TOP_BAR_HEIGHT});
        bar.setFillColor({245, 245, 245});
        win.draw(bar);
        
        sf::RectangleShape barBorder({WINDOW_WIDTH, 2});
        barBorder.setPosition(0, TOP_BAR_HEIGHT - 2);
        barBorder.setFillColor({200, 200, 200});
        win.draw(barBorder);
        
        sf::Text txt;
        txt.setFont(font);
        txt.setCharacterSize(16);
        txt.setFillColor(sf::Color::Black);
        txt.setString("Fungsi f(x): " + currentExpr);
        txt.setPosition(10, 10);
        win.draw(txt);
        
        sf::Text helpTxt;
        helpTxt.setFont(font);
        helpTxt.setCharacterSize(12);
        helpTxt.setFillColor({80, 80, 80});
        helpTxt.setString("Enter: add | R: reset | G: grid | N: numbers | C: crosshair | Del: hapus fungsi");
        helpTxt.setPosition(10, 38);
        win.draw(helpTxt);
        
        sf::Text consTxt;
        consTxt.setFont(font);
        consTxt.setCharacterSize(11);
        consTxt.setFillColor({100, 100, 100});
        consTxt.setString("Konstanta: pi, e | Fungsi: sin, cos, tan, exp, ln, sqrt, abs, dll");
        consTxt.setPosition(10, 62);
        win.draw(consTxt);
        
        // Right panel
        sf::RectangleShape rightPanel({RIGHT_PANEL_WIDTH, WINDOW_HEIGHT});
        rightPanel.setPosition(GRAPH_RIGHT, 0);
        rightPanel.setFillColor({250, 250, 250});
        win.draw(rightPanel);
        
        sf::RectangleShape panelBorder({2, WINDOW_HEIGHT});
        panelBorder.setPosition(GRAPH_RIGHT - 2, 0);
        panelBorder.setFillColor({200, 200, 200});
        win.draw(panelBorder);
        
        sf::Text panelTitle;
        panelTitle.setFont(font);
        panelTitle.setCharacterSize(14);
        panelTitle.setFillColor(sf::Color::Black);
        panelTitle.setString("Daftar Fungsi (" + std::to_string(functions.size()) + ")");
        panelTitle.setPosition(GRAPH_RIGHT + 10, GRAPH_TOP);
        win.draw(panelTitle);
        
        for (size_t i = 0; i < functions.size(); i++) {
            float y = GRAPH_TOP + 30 + i * 25;
            
            sf::RectangleShape funcBg({RIGHT_PANEL_WIDTH - 20, 22});
            funcBg.setPosition(GRAPH_RIGHT + 10, y);
            funcBg.setFillColor(i == selectedFunc ? sf::Color(220, 230, 255) : sf::Color(255, 255, 255));
            funcBg.setOutlineThickness(1);
            funcBg.setOutlineColor({200, 200, 200});
            win.draw(funcBg);
            
            sf::CircleShape colorDot(5);
            colorDot.setPosition(GRAPH_RIGHT + 15, y + 6);
            colorDot.setFillColor(functions[i].color);
            win.draw(colorDot);
            
            sf::Text funcText;
            funcText.setFont(font);
            funcText.setCharacterSize(11);
            funcText.setFillColor(sf::Color::Black);
            std::string label = functions[i].expr;
            if (label.length() > 25) label = label.substr(0, 22) + "...";
            funcText.setString(label);
            funcText.setPosition(GRAPH_RIGHT + 30, y + 4);
            win.draw(funcText);
        }
        
        // Bottom bar
        sf::RectangleShape bottomBar({WINDOW_WIDTH, BOTTOM_BAR_HEIGHT});
        bottomBar.setPosition(0, GRAPH_BOTTOM);
        bottomBar.setFillColor({245, 245, 245});
        win.draw(bottomBar);
        
        sf::RectangleShape bottomBorder({WINDOW_WIDTH, 2});
        bottomBorder.setPosition(0, GRAPH_BOTTOM);
        bottomBorder.setFillColor({200, 200, 200});
        win.draw(bottomBorder);
        
        if (!err.empty()) {
            sf::Text etxt;
            etxt.setFont(font);
            etxt.setCharacterSize(13);
            etxt.setString("Error: " + err);
            etxt.setPosition(10, GRAPH_BOTTOM + 8);
            etxt.setFillColor({200, 0, 0});
            win.draw(etxt);
        } else {
            sf::Text statusTxt;
            statusTxt.setFont(font);
            statusTxt.setCharacterSize(12);
            statusTxt.setFillColor({80, 80, 80});
            statusTxt.setString("Scale: " + std::to_string(int(scale)) + "px/unit | Functions: " + 
                              std::to_string(functions.size()));
            statusTxt.setPosition(10, GRAPH_BOTTOM + 8);
            win.draw(statusTxt);
        }

        win.display();
    }
    return 0;
}