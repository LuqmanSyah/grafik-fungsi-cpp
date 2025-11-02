// SFML Function Grapher — minimal "like Desmos" experience
// Features: type function in the top text box (e.g., sin(x), x^2+2*x+1, exp(-x^2)),
// pan with left-drag, zoom with mouse wheel, reset view button, grid & axes.
// Single-file, no extra deps beyond SFML. Expression parser supports + - * / ^,
// parentheses, unary minus, and functions: sin, cos, tan, asin, acos, atan,
// sinh, cosh, tanh, exp, ln (natural log), log (base 10), sqrt, abs, floor, ceil.

#include <SFML/Graphics.hpp>
#include <cmath>
#include <cctype>
#include <string>
#include <vector>
#include <stack>
#include <map>
#include <functional>
#include <sstream>
#include <iomanip>
#include <limits>
#include <algorithm>

// ---------------- Expression Parser (Shunting-yard to RPN) -----------------
struct Token {
    enum Type { NUMBER, VAR, OP, FUNC, LPAREN, RPAREN, COMMA } type;
    double value{};          // for numbers
    std::string text;        // for ops/func names
    int precedence{};        // for ops
    bool rightAssoc{};       // for ops
    int arity{1};            // for functions (currently all unary)
};

static bool isIdentStart(char c){ return std::isalpha((unsigned char)c) || c=='_'; }
static bool isIdentChar(char c){ return std::isalnum((unsigned char)c) || c=='_'; }

struct Parser {
    std::map<std::string, std::function<double(double)>> funcs1;
    std::map<std::string, std::function<double(double,double)>> funcs2; // reserved

    Parser(){
        using std::sin; using std::cos; using std::tan; using std::asin; using std::acos; using std::atan;
        using std::sinh; using std::cosh; using std::tanh; using std::exp; using std::log; using std::sqrt; using std::fabs; using std::floor; using std::ceil;
        funcs1 = {
            {"sin", [](double x){ return sin(x); }},
            {"cos", [](double x){ return cos(x); }},
            {"tan", [](double x){ return tan(x); }},
            {"asin", [](double x){ return asin(x); }},
            {"acos", [](double x){ return acos(x); }},
            {"atan", [](double x){ return atan(x); }},
            {"sinh", [](double x){ return sinh(x); }},
            {"cosh", [](double x){ return cosh(x); }},
            {"tanh", [](double x){ return tanh(x); }},
            {"exp", [](double x){ return exp(x); }},
            {"ln",  [](double x){ return log(x); }},
            {"log", [](double x){ return std::log10(x); }},
            {"sqrt",[](double x){ return sqrt(x); }},
            {"abs", [](double x){ return fabs(x); }},
            {"floor",[](double x){ return floor(x); }},
            {"ceil", [](double x){ return ceil(x); }}
        };
    }

    std::vector<Token> tokenize(const std::string& s, std::string& err){
        std::vector<Token> out; err.clear();
        for(size_t i=0;i<s.size();){
            char c = s[i];
            if(std::isspace((unsigned char)c)){ ++i; continue; }
            if(std::isdigit((unsigned char)c) || (c=='.' && i+1<s.size() && std::isdigit((unsigned char)s[i+1]))){
                size_t j=i; bool hasE=false; bool hasDot=false; if(c=='.') hasDot=true;
                while(j<s.size()){
                    char d=s[j];
                    if(std::isdigit((unsigned char)d)){ j++; }
                    else if(d=='.' && !hasDot){ hasDot=true; j++; }
                    else if((d=='e'||d=='E')&&!hasE){ hasE=true; j++; if(j<s.size() && (s[j]=='+'||s[j]=='-')) j++; }
                    else break;
                }
                double val = std::strtod(s.c_str()+i,nullptr);
                out.push_back({Token::NUMBER,val});
                i=j; continue;
            }
            if(isIdentStart(c)){
                size_t j=i+1; while(j<s.size() && isIdentChar(s[j])) j++;
                std::string id = s.substr(i,j-i);
                if(id=="x" || id=="X") out.push_back({Token::VAR});
                else out.push_back({Token::FUNC,0,id});
                i=j; continue;
            }
            if(c=='('){ out.push_back({Token::LPAREN}); i++; continue; }
            if(c==')'){ out.push_back({Token::RPAREN}); i++; continue; }
            if(c==','){ out.push_back({Token::COMMA}); i++; continue; }
            // operators
            if(std::string("+-*/^{}").find(c)!=std::string::npos){
                std::string op(1,c);
                int prec = (c=='+'||c=='-')?1:(c=='*'||c=='/')?2:(c=='^')?3:0;
                bool right = (c=='^');
                out.push_back({Token::OP,0,op,prec,right});
                i++; continue;
            }
            err = std::string("Unknown char: ") + c; return {};
        }
        // Handle unary minus by inserting 0 before leading '-' or '(-' situations
        std::vector<Token> fixed; fixed.reserve(out.size()*2);
        for(size_t i=0;i<out.size();++i){
            Token t = out[i];
            if(t.type==Token::OP && t.text=="-"){
                bool unary = (i==0) || (out[i-1].type==Token::OP || out[i-1].type==Token::LPAREN || out[i-1].type==Token::COMMA);
                if(unary){
                    fixed.push_back({Token::NUMBER,0.0});
                }
            }
            fixed.push_back(t);
        }
        return fixed;
    }

    std::vector<Token> toRPN(const std::vector<Token>& tokens, std::string& err){
        std::vector<Token> out; std::vector<Token> st; err.clear();
        for(size_t i=0;i<tokens.size();++i){
            const Token& t = tokens[i];
            switch(t.type){
                case Token::NUMBER: case Token::VAR: out.push_back(t); break;
                case Token::FUNC: st.push_back(t); break;
                case Token::COMMA:
                    while(!st.empty() && st.back().type!=Token::LPAREN){ out.push_back(st.back()); st.pop_back(); }
                    if(st.empty()){ err="Misplaced comma"; return {}; }
                    break;
                case Token::OP:
                    while(!st.empty() && st.back().type==Token::OP &&
                          ((t.rightAssoc==false && t.precedence<=st.back().precedence) ||
                           (t.rightAssoc==true && t.precedence< st.back().precedence))){
                        out.push_back(st.back()); st.pop_back();
                    }
                    st.push_back(t); break;
                case Token::LPAREN: st.push_back(t); break;
                case Token::RPAREN:
                    while(!st.empty() && st.back().type!=Token::LPAREN){ out.push_back(st.back()); st.pop_back(); }
                    if(st.empty()){ err="Mismatched parentheses"; return {}; }
                    st.pop_back(); // remove LPAREN
                    if(!st.empty() && st.back().type==Token::FUNC){ out.push_back(st.back()); st.pop_back(); }
                    break;
            }
        }
        while(!st.empty()){
            if(st.back().type==Token::LPAREN || st.back().type==Token::RPAREN){ err="Mismatched parentheses"; return {}; }
            out.push_back(st.back()); st.pop_back();
        }
        return out;
    }

    bool compile(const std::string& expr, std::vector<Token>& rpn, std::string& err){
        auto toks = tokenize(expr, err);
        if(!err.empty()) return false;
        rpn = toRPN(toks, err);
        return err.empty();
    }

    double eval(const std::vector<Token>& rpn, double x, bool& ok){
        ok = true; std::vector<double> st; st.reserve(32);
        for(const auto& t : rpn){
            switch(t.type){
                case Token::NUMBER: st.push_back(t.value); break;
                case Token::VAR: st.push_back(x); break;
                case Token::OP: {
                    if(st.size()<2){ ok=false; return NAN; }
                    double b=st.back(); st.pop_back();
                    double a=st.back(); st.pop_back();
                    double r = 0;
                    if(t.text=="+") r=a+b;
                    else if(t.text=="-") r=a-b;
                    else if(t.text=="*") r=a*b;
                    else if(t.text=="/") r=a/b;
                    else if(t.text=="^") r=std::pow(a,b);
                    else { ok=false; return NAN; }
                    st.push_back(r);
                } break;
                case Token::FUNC: {
                    // only unary for now
                    if(st.empty()){ ok=false; return NAN; }
                    double a=st.back(); st.pop_back();
                    auto it = funcs1.find(t.text);
                    if(it==funcs1.end()){ ok=false; return NAN; }
                    double r = it->second(a);
                    st.push_back(r);
                } break;
                default: ok=false; return NAN;
            }
        }
        if(st.size()!=1){ ok=false; return NAN; }
        return st.back();
    }
};

// ---------------- Graphing Utilities -----------------
struct ViewState {
    // World coordinates mapping: screen = origin + world * scale (y inverted)
    sf::Vector2f origin; // pixels of world (0,0)
    float scale;         // pixels per unit
};

static sf::Vector2f worldToScreen(const ViewState& v, sf::Vector2f w){
    return { v.origin.x + w.x * v.scale, v.origin.y - w.y * v.scale };
}
static sf::Vector2f screenToWorld(const ViewState& v, sf::Vector2f s){
    return { (s.x - v.origin.x)/v.scale, (v.origin.y - s.y)/v.scale };
}

static void drawGrid(sf::RenderWindow& win, const ViewState& view, sf::Vector2u size){
    sf::VertexArray lines(sf::Lines);
    auto addLine=[&](sf::Vector2f a, sf::Vector2f b, sf::Color c){
        sf::Vertex va(a,c), vb(b,c); lines.append(va); lines.append(vb);
    };

    // Choose grid spacing depending on zoom
    double step = 1.0; // world units
    double pxPerUnit = view.scale;
    if(pxPerUnit<15) step = 10;
    if(pxPerUnit<5) step = 50;
    if(pxPerUnit>60) step = 0.5;
    if(pxPerUnit>120) step = 0.25;
    if(pxPerUnit>240) step = 0.125;

    // screen bounds to world
    sf::Vector2f wMin = screenToWorld(view, {0.f, (float)size.y});
    sf::Vector2f wMax = screenToWorld(view, {(float)size.x, 0.f});

    // vertical grid lines
    double startX = std::floor(wMin.x/step)*step;
    for(double x=startX; x<=wMax.x+1e-6; x+=step){
        auto a = worldToScreen(view, {(float)x, wMin.y});
        auto b = worldToScreen(view, {(float)x, wMax.y});
        sf::Color c = (std::abs(x)<1e-9? sf::Color(180,60,60): sf::Color(210,210,210));
        addLine(a,b,c);
    }
    // horizontal grid lines
    double startY = std::floor(wMin.y/step)*step;
    for(double y=startY; y<=wMax.y+1e-6; y+=step){
        auto a = worldToScreen(view, {wMin.x, (float)y});
        auto b = worldToScreen(view, {wMax.x, (float)y});
        sf::Color c = (std::abs(y)<1e-9? sf::Color(180,60,60): sf::Color(210,210,210));
        addLine(a,b,c);
    }

    win.draw(lines);
}

static void drawAxesLabels(sf::RenderWindow& win, const ViewState& view, sf::Vector2u size, sf::Font& font){
    // Draw small tick labels near axes intersections
    double step = 1.0; double pxPerUnit = view.scale;
    if(pxPerUnit<15) step = 10; if(pxPerUnit<5) step = 50;
    if(pxPerUnit>60) step = 0.5; if(pxPerUnit>120) step = 0.25; if(pxPerUnit>240) step = 0.125;

    sf::Vector2f wMin = screenToWorld(view, {0.f, (float)size.y});
    sf::Vector2f wMax = screenToWorld(view, {(float)size.x, 0.f});

    // X labels along y=0
    double startX = std::floor(wMin.x/step)*step;
    for(double x=startX; x<=wMax.x; x+=step){
        auto p = worldToScreen(view, {(float)x, 0});
        if(p.y<0 || p.y>size.y) continue;
        sf::Text txt; txt.setFont(font); txt.setCharacterSize(12);
        std::ostringstream oss; oss<<std::fixed<<std::setprecision(step<1?2:0)<<x; txt.setString(oss.str());
        txt.setFillColor(sf::Color(120,120,120));
        txt.setPosition(p.x+2, p.y+2);
        win.draw(txt);
    }
    // Y labels along x=0
    double startY = std::floor(wMin.y/step)*step;
    for(double y=startY; y<=wMax.y; y+=step){
        auto p = worldToScreen(view, {0, (float)y});
        if(p.x<0 || p.x>size.x) continue;
        sf::Text txt; txt.setFont(font); txt.setCharacterSize(12);
        std::ostringstream oss; oss<<std::fixed<<std::setprecision(step<1?2:0)<<y; txt.setString(oss.str());
        txt.setFillColor(sf::Color(120,120,120));
        txt.setPosition(p.x+4, p.y-16);
        win.draw(txt);
    }
}

// Simple button & textbox UI elements built with SFML shapes
struct TextBox {
    sf::RectangleShape box; sf::Text text; bool focused=false; std::string content; std::string placeholder;
    void init(sf::Font& font, sf::Vector2f pos, sf::Vector2f size, const std::string& ph){
        box.setPosition(pos); box.setSize(size); box.setFillColor(sf::Color(245,245,245));
        box.setOutlineThickness(2); box.setOutlineColor(sf::Color(180,180,180));
        text.setFont(font); text.setCharacterSize(18); text.setFillColor(sf::Color::Black);
        text.setPosition(pos.x+10, pos.y+7);
        placeholder = ph;
    }
    void setContent(const std::string& s){ content=s; }
    bool handleEvent(const sf::Event& e){
        bool submitted=false;
        if(e.type==sf::Event::MouseButtonPressed){
            auto mp = sf::Vector2f((float)e.mouseButton.x,(float)e.mouseButton.y);
            focused = box.getGlobalBounds().contains(mp);
            box.setOutlineColor(focused? sf::Color(70,130,180): sf::Color(180,180,180));
        } else if(focused && e.type==sf::Event::TextEntered){
            sf::Uint32 u = e.text.unicode;
            if(u>=32 && u<127){ content.push_back((char)u); }
            else if(u==8){ if(!content.empty()) content.pop_back(); }
            else if(u=='\r' || u=='\n'){ submitted=true; }
        }
        return submitted;
    }
    void draw(sf::RenderWindow& win){
        win.draw(box);
        std::string show = content.empty()? placeholder : content;
        sf::Color col = content.empty()? sf::Color(140,140,140): sf::Color::Black;
        text.setString(show); text.setFillColor(col);
        win.draw(text);
    }
    sf::FloatRect bounds() const { return box.getGlobalBounds(); }
};

struct Button {
    sf::RectangleShape box; sf::Text label; std::function<void()> onClick;
    void init(sf::Font& font, const std::string& text, sf::Vector2f pos, sf::Vector2f size){
        box.setPosition(pos); box.setSize(size); box.setFillColor(sf::Color(235,235,235));
        box.setOutlineThickness(1); box.setOutlineColor(sf::Color(160,160,160));
        label.setFont(font); label.setString(text); label.setCharacterSize(16); label.setFillColor(sf::Color::Black);
        auto gb = box.getGlobalBounds(); auto lb = label.getLocalBounds();
        label.setPosition(gb.left + (gb.width - lb.width)/2.f - 2, gb.top + (gb.height - lb.height)/2.f - 8);
    }
    bool handleClick(sf::Vector2f mp){ if(box.getGlobalBounds().contains(mp)){ if(onClick) onClick(); return true; } return false; }
    void draw(sf::RenderWindow& win){ win.draw(box); win.draw(label); }
};

int main(){
    sf::ContextSettings settings; settings.antialiasingLevel=8;
    sf::RenderWindow window(sf::VideoMode(1100, 700), "SFML Function Grapher", sf::Style::Default, settings);
    window.setFramerateLimit(60);

    // Load a default font (Arial-like). On many systems, you may need to provide a font file.
    // We'll try to load from a few common paths and fallback to default sans if available.
    sf::Font font;
    std::vector<std::string> fontCandidates = {
        "./DejaVuSans.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
        "/System/Library/Fonts/Supplemental/Arial Unicode.ttf",
    };
    bool loaded=false; for(auto& p: fontCandidates){ if(font.loadFromFile(p)){ loaded=true; break; } }
    if(!loaded){
        // As a last resort, create a simple default font if available (on Windows, SFML may embed one)
        // If this fails, program will still run but text may not render.
    }

    // UI elements
    TextBox input; input.init(font, {15, 12}, {680, 36}, "Ketik fungsi di sini, contoh: sin(x) atau x^2 + 2*x + 1, Enter untuk plot");
    input.setContent("sin(x)");

    Button resetBtn; resetBtn.init(font, "Reset View", {710, 12}, {120, 36});

    Button helpBtn; helpBtn.init(font, "Bantuan", {840, 12}, {100, 36});

    // Parser & expression
    Parser parser; std::vector<Token> rpn; std::string parseErr;
    auto compileExpr = [&](const std::string& expr){
        parseErr.clear();
        if(expr.empty()){ rpn.clear(); return; }
        if(!parser.compile(expr, rpn, parseErr)){
            rpn.clear();
        }
    };
    compileExpr(input.content);

    // View state
    ViewState view; view.scale = 80.f; // 80 px per unit initially
    view.origin = {(float)window.getSize().x/2.f, (float)window.getSize().y/2.f + 20.f};

    resetBtn.onClick = [&](){ view.scale=80.f; view.origin={(float)window.getSize().x/2.f, (float)window.getSize().y/2.f + 20.f}; };

    bool showHelp=false;
    helpBtn.onClick = [&](){ showHelp = !showHelp; };

    bool dragging=false; sf::Vector2f dragStart, originStart;

    while(window.isOpen()){
        sf::Event e; while(window.pollEvent(e)){
            if(e.type==sf::Event::Closed) window.close();

            // Handle window resize
            if(e.type==sf::Event::Resized){
                sf::FloatRect visibleArea(0, 0, e.size.width, e.size.height);
                window.setView(sf::View(visibleArea));
            }

            // Textbox typing & submit
            bool submitted = input.handleEvent(e);
            if(submitted){ compileExpr(input.content); }

            // Mouse wheel zoom (toward cursor)
            if(e.type==sf::Event::MouseWheelScrolled){
                float delta = e.mouseWheelScroll.delta;
                sf::Vector2f mousePos((float)e.mouseWheelScroll.x, (float)e.mouseWheelScroll.y);
                sf::Vector2f worldBefore = screenToWorld(view, mousePos);
                float factor = (delta>0)? 1.15f : 0.87f;
                view.scale = std::clamp(view.scale * factor, 5.f, 2000.f);
                sf::Vector2f screenAfter = worldToScreen(view, worldBefore);
                sf::Vector2f diff = mousePos - screenAfter;
                view.origin += diff; // keep point under cursor fixed
            }

            // Start dragging if left-click not on UI
            if(e.type==sf::Event::MouseButtonPressed && e.mouseButton.button==sf::Mouse::Left){
                sf::Vector2f mp((float)e.mouseButton.x,(float)e.mouseButton.y);
                if(!input.bounds().contains(mp) && !resetBtn.box.getGlobalBounds().contains(mp) && !helpBtn.box.getGlobalBounds().contains(mp)){
                    dragging=true; dragStart=mp; originStart=view.origin;
                } else {
                    // pass click to buttons
                    if(resetBtn.handleClick(mp)){}
                    if(helpBtn.handleClick(mp)){}
                }
            }
            if(e.type==sf::Event::MouseButtonReleased && e.mouseButton.button==sf::Mouse::Left){ dragging=false; }
            if(e.type==sf::Event::MouseMoved && dragging){
                sf::Vector2f mp((float)e.mouseMove.x,(float)e.mouseMove.y);
                view.origin = originStart + (mp - dragStart);
            }
        }

        // ---- Rendering ----
        window.clear(sf::Color::White);

        // Top bar background
        sf::RectangleShape topBar({(float)window.getSize().x, 60}); topBar.setFillColor(sf::Color(250,250,250));
        window.draw(topBar);

        // Grid & axes
        drawGrid(window, view, window.getSize());

        // Plot function if compiled
        if(!rpn.empty()){
            const int W = (int)window.getSize().x;
            sf::VertexArray strip(sf::LineStrip);
            strip.resize(W);
            bool okPrev=false; double yPrev=0; bool havePrev=false;
            std::vector<sf::VertexArray> segments; segments.reserve(16);
            sf::VertexArray current(sf::LineStrip);

            for(int px=0; px<W; ++px){
                sf::Vector2f world = screenToWorld(view, {(float)px, view.origin.y});
                double x = world.x;
                bool ok=true; double y = parser.eval(rpn, x, ok);
                sf::Vector2f scr = worldToScreen(view, {(float)x, (float)y});

                if(!ok || std::isnan(y) || std::isinf(y)){
                    if(current.getVertexCount()>=2) segments.push_back(current);
                    current.clear(); havePrev=false; okPrev=false; continue;
                }
                if(havePrev){
                    double dy = std::abs(y - yPrev);
                    if(dy * view.scale > 3000) { // large jump => break segment
                        if(current.getVertexCount()>=2) segments.push_back(current);
                        current.clear(); havePrev=false; okPrev=false;
                    }
                }
                current.append(sf::Vertex(scr, sf::Color(50,90,200)));
                yPrev=y; havePrev=true; okPrev=ok;
            }
            if(current.getVertexCount()>=2) segments.push_back(current);
            for(auto& seg: segments) window.draw(seg);
        }

        // Draw axes labels (after curve for visibility)
        drawAxesLabels(window, view, window.getSize(), font);

        // UI draw on top
        input.draw(window);
        resetBtn.draw(window); helpBtn.draw(window);

        // Parse error display
        if(!parseErr.empty()){
            sf::RectangleShape errBox; errBox.setPosition(15, 54); errBox.setSize({(float)window.getSize().x-30, 28});
            errBox.setFillColor(sf::Color(255,235,235)); errBox.setOutlineThickness(1); errBox.setOutlineColor(sf::Color(200,80,80));
            sf::Text t; t.setFont(font); t.setCharacterSize(16); t.setFillColor(sf::Color(160,30,30));
            t.setString("Error: "+parseErr);
            t.setPosition(20, 58);
            window.draw(errBox); window.draw(t);
        }

        if(showHelp){
            sf::RectangleShape panel; panel.setPosition(window.getSize().x-320.f, 60);
            panel.setSize({300.f, 260.f}); panel.setFillColor(sf::Color(250,250,250));
            panel.setOutlineThickness(1); panel.setOutlineColor(sf::Color(180,180,180));
            sf::Text h; h.setFont(font); h.setCharacterSize(18); h.setFillColor(sf::Color::Black); h.setString("Bantuan");
            h.setPosition(panel.getPosition().x+12, panel.getPosition().y+8);

            sf::Text b; b.setFont(font); b.setCharacterSize(15); b.setFillColor(sf::Color(60,60,60));
            b.setPosition(panel.getPosition().x+12, panel.getPosition().y+36);
            b.setString(
                "Format fungsi:\n"
                "  - Operator: + - * / ^, kurung ()\n"
                "  - Variabel: x\n"
                "  - Fungsi: sin, cos, tan, asin, acos, atan,\n"
                "            sinh, cosh, tanh, exp, ln, log,\n"
                "            sqrt, abs, floor, ceil\n\n"
                "Kontrol:\n"
                "  - Ketik fungsi lalu tekan Enter untuk plot\n"
                "  - Zoom: Scroll mouse\n"
                "  - Pan: Drag kiri pada canvas\n"
                "  - Reset: Klik 'Reset View'\n");
            window.draw(panel); window.draw(h); window.draw(b);
        }

        // Outline around top bar
        sf::RectangleShape sep; sep.setPosition(0, 60); sep.setSize({(float)window.getSize().x, 1}); sep.setFillColor(sf::Color(200,200,200));
        window.draw(sep);

        window.display();
    }
    return 0;
}
