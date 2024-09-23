#ifndef SPROUTS_H
#define SPROUTS_H

//#include "../HemisphereApplet.h"
#include <array>
#include <vector>
#include <stack>
#include <string>

enum class Symbol {
    F, PLUS, MINUS, LBRACKET, RBRACKET, X
};

struct Rule {
    Symbol predecessor;
    std::vector<Symbol> successor;
};

class Sprouts : public HemisphereApplet {
public:
    const char* applet_name() { 
        return "Sprouts";
    }

    void Start() {
        cursor = 0;
        num_iterations = 3;
        ParseAxiom("F");
        ParseRule("F->F[+F]F[-F]F", 0);
        ParseRule("", 1);
        ParseRule("", 2);
        GenerateLSystem(num_iterations);
    }

    void Controller() {
        if (Clock(0)) {
            if (current_step < current_state.size()) {
                ProcessSymbol(current_state[current_step]);
                current_step++;
            } else {
                current_step = 0;
            }
        }
    }

    void View() {
        switch(drawTree){
        case 0: 
            DrawUI(); 
            gfxPrint(1, 52, "Rules");
            if (cursor == DRAWMODE) gfxCursor(1, 60, 32); 
            break;
        case 1: 
            DrawLSystem(32, 63, 90, 2); 
            gfxPrint(1, 52, "Tree");
            if (cursor == DRAWMODE) gfxCursor(1, 60, 22); 
            break;
        }
    }

    

    void OnEncoderMove(int direction) {
        if (!EditMode()) { // move cursor
        MoveCursor(cursor, direction, LAST_SETTING);
        return;
        }

        switch(cursor) {
            case AXIOM:
                EditSymbols(axiom, direction);
                break;
            case RULE1:
            case RULE2:
            case RULE3:
                EditRule(rules[cursor - RULE1], direction);
                break;
            case ITERATIONS:
                num_iterations = constrain(num_iterations + direction, 1, 6);
                GenerateLSystem(num_iterations);
                break;
            case DRAWMODE:
                drawTree = constrain(drawTree + direction, 0, 1);
                break;
        }
    }
        
    uint64_t OnDataRequest() {
        uint64_t data = 0;
        return data;
    }

    void OnDataReceive(uint64_t data) {
    }

protected:
    void SetHelp() override {
        help[HEMISPHERE_HELP_DIGITALS] = "1=Clock";
        help[HEMISPHERE_HELP_CVS]      = "Unused";
        help[HEMISPHERE_HELP_OUTS]     = "A=Pitch B=Gate";
        help[HEMISPHERE_HELP_ENCODER]  = "Select/Edit";
    }
    
private:
    enum SproutsCursor {
        AXIOM, RULE1, RULE2, RULE3, ITERATIONS, DRAWMODE, LAST_SETTING=DRAWMODE
    };

    int cursor;
    std::vector<Symbol> axiom;
    std::array<Rule, 3> rules;
    std::vector<Symbol> current_state;
    int current_step;
    int num_iterations;
    int drawTree = 0;

    void GenerateLSystem(int iterations) {
        std::vector<Symbol> current = axiom;
        std::vector<Symbol> next;

        for (int i = 0; i < iterations; i++) {
            for (Symbol symbol : current) {
                bool ruleApplied = false;
                for (const Rule& rule : rules) {
                    if (rule.predecessor == symbol) {
                        next.insert(next.end(), rule.successor.begin(), rule.successor.end());
                        ruleApplied = true;
                        break;
                    }
                }
                if (!ruleApplied) {
                    next.push_back(symbol);
                }
            }
            current = next;
            next.clear();
        }

        current_state = current;
        current_step = 0;
    }

    void ProcessSymbol(Symbol symbol) {
        static int pitch = 0;
        static bool gate = false;

        switch(symbol) {
            case Symbol::F:
                gate = true;
                Out(0, pitch);
                Out(1, gate ? HEMISPHERE_MAX_CV : 0);
                break;
            case Symbol::PLUS:
                pitch = constrain(pitch + 1, 0, 60);
                break;
            case Symbol::MINUS:
                pitch = constrain(pitch - 1, 0, 60);
                break;
            case Symbol::LBRACKET:
            case Symbol::RBRACKET:
            case Symbol::X:
                gate = false;
                Out(1, 0);
                break;
        }
    }

    void DrawLSystem(int start_x, int start_y, double start_angle, int start_length) {
        int x = start_x;
        int y = start_y; 
        int angle = start_angle;
        int length = start_length;
        const int rotation_amount = 30;
        std::stack<std::pair<int, int>> positionStack;

        for (Symbol symbol : current_state) {
            switch(symbol) {
                case Symbol::F: {
                    //calculate line end
                    double radians = angle * M_PI / 180.0;
                    int x1 = x + length * cos(radians);
                    int y1 = y - length * sin(radians); // Subtracting because y-coordinates increase downward
                    
                    constrain(x, 0, 63);
                    constrain(y, 0, 47);
                    constrain(x1, 0, 63);
                    constrain(y1, 0, 47);
                    gfxLine(x, y, x1, y1);
                    x = x1;
                    y = y1;
                    break;
                }
                case Symbol::PLUS:
                    angle += rotation_amount;
                    break;
                case Symbol::MINUS:
                    angle -= rotation_amount;
                    break;
                case Symbol::LBRACKET:
                    positionStack.push({x, y});
                    break;
                case Symbol::RBRACKET:
                    if (!positionStack.empty()) {
                        auto [prevX, prevY] = positionStack.top();
                        positionStack.pop();
                        x = prevX;
                        y = prevY;
                    }
                    break;
                case Symbol::X:
                    // Do nothing for X
                    break;
            }
        }
    }

    void DrawUI(){
        gfxPrint(1, 12, "Axiom: ");
        gfxPrint(SymbolsToString(axiom).c_str());

        for (int i = 0; i < 3; i++) {
            gfxPrint(1, 20 + i * 8, i+1);
            gfxPrint(": ");
            gfxPrint(RuleToString(rules[i]).c_str());
        }
        gfxPrint(1, 44, "Iter: ");
        gfxPrint(num_iterations);

        // Highlight the currently selected parameter
        switch (cursor){
        case AXIOM: gfxCursor(1, 20, 8); break;
        case RULE1: gfxCursor(1, 28, 8); break;
        case RULE2: gfxCursor(1, 36, 8); break;
        case RULE3: gfxCursor(1, 44, 8); break;
        case ITERATIONS: gfxCursor(1, 52, 8); break;
        }
    }

    Symbol ParseSymbol(char c) {
        switch(c) {
            case 'F': return Symbol::F;
            case '+': return Symbol::PLUS;
            case '-': return Symbol::MINUS;
            case '[': return Symbol::LBRACKET;
            case ']': return Symbol::RBRACKET;
            case 'X': return Symbol::X;
            default: return Symbol::X; // Default to X for invalid input
        }
    }

    void ParseAxiom(const std::string& input) {
        axiom.clear();
        for (char c : input) {
            axiom.push_back(ParseSymbol(c));
        }
    }

    void ParseRule(const std::string& input, int ruleIndex) {
        if (ruleIndex < 0 || ruleIndex >= 3) return;

        size_t arrowPos = input.find("->");
        if (arrowPos == std::string::npos || arrowPos == 0 || arrowPos == input.length() - 2) {
            rules[ruleIndex] = {Symbol::X, {}};
            return;
        }

        Symbol predecessor = ParseSymbol(input[0]);
        std::vector<Symbol> successor;
        for (size_t i = arrowPos + 2; i < input.length(); i++) {
            successor.push_back(ParseSymbol(input[i]));
        }

        rules[ruleIndex] = {predecessor, successor};
    }

    std::string SymbolsToString(const std::vector<Symbol>& symbols) {
        std::string result;
        for (Symbol s : symbols) {
            switch(s) {
                case Symbol::F: result += 'F'; break;
                case Symbol::PLUS: result += '+'; break;
                case Symbol::MINUS: result += '-'; break;
                case Symbol::LBRACKET: result += '['; break;
                case Symbol::RBRACKET: result += ']'; break;
                case Symbol::X: result += 'X'; break;
            }
        }
        return result;
    }

    std::string RuleToString(const Rule& rule) {
        if (rule.successor.empty()) return "";
        return std::string(1, SymbolsToString({rule.predecessor})[0]) + "->" + SymbolsToString(rule.successor);
    }

    void EditSymbols(std::vector<Symbol>& symbols, int direction) {
        if (symbols.empty()) {
            symbols.push_back(Symbol::F);
        } else {
            int index = (static_cast<int>(symbols.back()) + direction + 6) % 6;
            symbols.back() = static_cast<Symbol>(index);
        }
    }

    void EditRule(Rule& rule, int direction) {
        if (rule.successor.empty()) {
            rule.predecessor = Symbol::F;
            rule.successor.push_back(Symbol::F);
        } else {
            EditSymbols(rule.successor, direction);
        }
    }
};

#endif // SPROUTS_H