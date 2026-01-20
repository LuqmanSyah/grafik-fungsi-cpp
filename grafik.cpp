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
    enum Type { NUMBER, VAR_X, OP, FUNC, LPAREN, RPAREN } type;
    double value{};
    std::string text;
    int precedence{};
    bool rightAssoc{};
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

    // Tokenizer: Memecah string menjadi token
    std::vector<Token> parse(const std::string& s, std::string& err) {
        std::vector<Token> toks;
        err.clear();
        
        for (size_t i = 0; i < s.size();) {
            char c = s[i];
            
            // Skip whitespace
            if (isspace(c)) { i++; continue; }
            
            // Parse number
            if (isdigit(c) || c == '.') {
                size_t j = i;
                while (j < s.size() && (isdigit(s[j]) || s[j] == '.' || s[j] == 'e' || s[j] == 'E'))
                    j++;
                toks.push_back({Token::NUMBER, strtod(s.c_str() + i, 0)});
                i = j;
                continue;
            }
            
            // Parse identifier (variable or function)
            if (isalpha(c)) {
                size_t j = i;
                while (j < s.size() && isalnum(s[j])) j++;
                std::string id = s.substr(i, j - i);
                
                if (id == "x") toks.push_back({Token::VAR_X});
                else toks.push_back({Token::FUNC, 0, id});
                
                i = j;
                continue;
            }
            
            // Parse parentheses
            if (c == '(') { toks.push_back({Token::LPAREN}); i++; continue; }
            if (c == ')') { toks.push_back({Token::RPAREN}); i++; continue; }
            
            // Parse operators
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

    // Convert to Reverse Polish Notation using Shunting-yard algorithm
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
            out.push_back(st.back());
            st.pop_back();
        }
        return out;
    }

    // Evaluate RPN expression with given x value
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
// MAIN PROGRAM
// ============================================================================

int main() {
    sf::RenderWindow win(sf::VideoMode(1200, 750), "Grapher 2D - Fungsi 1 Variabel - Kalkulus 2");
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
    std::string expr = "sin(x)", err;
    
    // View state
    sf::Vector2f origin = {600, 375};
    float scale = 60;
    
    // Interaction state
    bool dragging = false;
    sf::Vector2f dragStart, originStart;
    
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
                scale = 60;
            }
            
            // Zoom with mouse wheel
            if (e.type == sf::Event::MouseWheelScrolled) {
                sf::Vector2f mouse(e.mouseWheelScroll.x, e.mouseWheelScroll.y);
                sf::Vector2f worldBefore = {(mouse.x - origin.x) / scale, (origin.y - mouse.y) / scale};
                scale *= e.mouseWheelScroll.delta > 0 ? 1.15f : 0.87f;
                scale = std::min(std::max(scale, 10.f), 300.f);
                sf::Vector2f screenAfter = {origin.x + worldBefore.x * scale, origin.y - worldBefore.y * scale};
                origin += mouse - screenAfter;
            }
            
            // Pan with drag
            if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.y > 60) {
                dragging = true;
                dragStart = {float(e.mouseButton.x), float(e.mouseButton.y)};
                originStart = origin;
            }
            if (e.type == sf::Event::MouseButtonReleased) dragging = false;
            if (e.type == sf::Event::MouseMoved && dragging) {
                origin = originStart + (sf::Vector2f(e.mouseMove.x, e.mouseMove.y) - dragStart);
            }
        }

        // ===== RENDERING =====
        win.clear(sf::Color::White);
        
        // Top bar
        sf::RectangleShape bar({1200, 60});
        bar.setFillColor({250, 250, 250});
        win.draw(bar);
        
        sf::Text txt;
        txt.setFont(font);
        txt.setCharacterSize(16);
        txt.setFillColor(sf::Color::Black);
        txt.setString("Fungsi f(x): " + expr + "\nTekan Enter untuk plot | R untuk reset | Scroll untuk zoom | Drag untuk pan");
        txt.setPosition(10, 10);
        win.draw(txt);
        
        // Draw grid
        sf::VertexArray grid(sf::Lines);
        for (int i = -30; i < 30; i++) {
            float x = origin.x + i * scale * 0.5f;
            grid.append({{x, 60}, {220, 220, 220}});
            grid.append({{x, 750}, {220, 220, 220}});
            
            float y = origin.y + i * scale * 0.5f;
            grid.append({{0, y}, {220, 220, 220}});
            grid.append({{1200, y}, {220, 220, 220}});
        }
        // Draw axes
        grid.append({{origin.x, 60}, {200, 80, 80}});
        grid.append({{origin.x, 750}, {200, 80, 80}});
        grid.append({{0, origin.y}, {200, 80, 80}});
        grid.append({{1200, origin.y}, {200, 80, 80}});
        win.draw(grid);
        
        // Draw function curve
        if (!rpn.empty()) {
            sf::VertexArray curve(sf::LineStrip);
            double prevY = 0;
            bool havePrev = false;
            
            for (int px = 0; px < 1200; px++) {
                bool ok;
                double x = (px - origin.x) / scale;
                double y = parser.eval(rpn, x, ok);
                
                if (ok && !std::isnan(y) && !std::isinf(y) && fabs(y) < 1000) {
                    // Detect discontinuities
                    if (havePrev && fabs(y - prevY) * scale > 3000) {
                        win.draw(curve);
                        curve.clear();
                    }
                    
                    curve.append({{float(px), origin.y - float(y) * scale}, {50, 90, 200}});
                    prevY = y;
                    havePrev = true;
                } else {
                    if (curve.getVertexCount() > 1) win.draw(curve);
                    curve.clear();
                    havePrev = false;
                }
            }
            if (curve.getVertexCount() > 1) win.draw(curve);
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