#include <array>
#include <vector>
#include <stack>
#include <string>

enum class Symbol {
    F, PLUS, MINUS, LBRACKET, RBRACKET, X, EMPTY
};

struct Rule {
    Symbol predecessor;
    std::array<Symbol, 9> successor;
    size_t successor_length;
};

class Sprouts : public HemisphereApplet {
public:
    const char* applet_name() { 
        return "Sprouts";
    }

    void Start() {
        cursor = AXIOM_0;
        num_iterations = 3;
        
        axiom = { Symbol::F };
        rules[0].predecessor = { Symbol::F };
        rules[0].successor = { Symbol::F, Symbol::LBRACKET, Symbol::MINUS, Symbol::F, Symbol::RBRACKET, Symbol::LBRACKET, Symbol::PLUS, Symbol::F, Symbol::RBRACKET };
        
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
        if (drawTree) {
            DrawLSystem(32, 36, 90, 6);
            gfxPrint(1, 52, "Tree");
            cursor = DRAWMODE;
            if (cursor == DRAWMODE) gfxCursor(1, 60, 6);        
        } else {
            DrawUI();
            gfxPrint(1, 52, "Rules");
        }
    }

    void OnEncoderMove(int direction) {
        if (!EditMode()) {
            MoveCursor(direction);
        } else {
            EditSymbol(direction);
        }
    }

    uint64_t OnDataRequest() {
        uint64_t data = 0;
        return data;
    }

    void OnDataReceive(uint64_t data) {
    }

protected:
    void SetHelp() {
        help[HEMISPHERE_HELP_DIGITALS] = "1=Clock";
        help[HEMISPHERE_HELP_CVS]      = "Unused";
        help[HEMISPHERE_HELP_OUTS]     = "A=Pitch B=Gate";
        help[HEMISPHERE_HELP_ENCODER]  = "Select/Edit";
    }
    
private:
    enum SproutsCursor {
        AXIOM_0, AXIOM_1, AXIOM_2, AXIOM_3, AXIOM_4, AXIOM_5, AXIOM_6, AXIOM_7,
        RULE1_PRED, RULE1_SUCC_0, RULE1_SUCC_1, RULE1_SUCC_2, RULE1_SUCC_3, RULE1_SUCC_4, RULE1_SUCC_5, RULE1_SUCC_6, RULE1_SUCC_7,
        RULE2_PRED, RULE2_SUCC_0, RULE2_SUCC_1, RULE2_SUCC_2, RULE2_SUCC_3, RULE2_SUCC_4, RULE2_SUCC_5, RULE2_SUCC_6, RULE2_SUCC_7,
        RULE3_PRED, RULE3_SUCC_0, RULE3_SUCC_1, RULE3_SUCC_2, RULE3_SUCC_3, RULE3_SUCC_4, RULE3_SUCC_5, RULE3_SUCC_6, RULE3_SUCC_7,
        ITERATIONS,
        DRAWMODE,
        NUM_CURSORS
    };

    SproutsCursor cursor;
    std::array<Symbol, 8> axiom;
    size_t axiom_length;
    std::array<Rule, 3> rules;
    std::vector<Symbol> current_state;
    int current_step;
    int num_iterations;
    int drawTree = 0;

    void MoveCursor(int direction) {
        do {
            cursor = static_cast<SproutsCursor>((static_cast<int>(cursor) + direction + NUM_CURSORS) % NUM_CURSORS);
        } while (!IsValidCursorPosition());
    }

    bool IsValidCursorPosition() {
        if (cursor == ITERATIONS || cursor == DRAWMODE) return true;
        if (IsAxiomCursor()) return CursorPositionInAxiom() < axiom_length || CursorPositionInAxiom() == axiom_length;
        if (IsRulePredecessorCursor()) return true;
        if (IsRuleSuccessorCursor()) {
            Rule& rule = rules[CurrentRule()];
            return CursorPositionInRule() < rule.successor_length || CursorPositionInRule() == rule.successor_length;
        }
        return false;
    }

    void EditSymbol(int direction) {
        if (cursor == ITERATIONS) {
            num_iterations = constrain(num_iterations + direction, 1, 6);
            GenerateLSystem(num_iterations);
        } else if (cursor == DRAWMODE) {
            drawTree = constrain(drawTree + direction, 0, 1);
        } else {
            Symbol& symbol = GetCurrentSymbol();
            int symbolIndex = static_cast<int>(symbol);
            symbolIndex = (symbolIndex + direction + 7) % 7;  // 7 is the number of symbols including EMPTY
            symbol = static_cast<Symbol>(symbolIndex);

            // Handle adding/removing symbols
            if (IsAxiomCursor() && CursorPositionInAxiom() == axiom_length && symbol != Symbol::EMPTY) {
                axiom_length++;
            } else if (IsAxiomCursor() && CursorPositionInAxiom() == axiom_length - 1 && symbol == Symbol::EMPTY) {
                axiom_length--;
            } else if (IsRuleSuccessorCursor()) {
                Rule& rule = rules[CurrentRule()];
                if (CursorPositionInRule() == rule.successor_length && symbol != Symbol::EMPTY) {
                    rule.successor_length++;
                } else if (CursorPositionInRule() == rule.successor_length - 1 && symbol == Symbol::EMPTY) {
                    rule.successor_length--;
                }
            }

            GenerateLSystem(num_iterations);
        }
    }

    Symbol& GetCurrentSymbol() {
        if (IsAxiomCursor()) {
            return axiom[CursorPositionInAxiom()];
        } else if (IsRulePredecessorCursor()) {
            return rules[CurrentRule()].predecessor;
        } else if (IsRuleSuccessorCursor()) {
            return rules[CurrentRule()].successor[CursorPositionInRule()];
        }
        // This should never happen, but we need to return something
        static Symbol dummy = Symbol::EMPTY;
        return dummy;
    }

    bool IsAxiomCursor() const {
        return cursor >= AXIOM_0 && cursor <= AXIOM_7;
    }

    bool IsRuleCursor() const {
        return cursor >= RULE1_PRED && cursor <= RULE3_SUCC_7;
    }

    bool IsRulePredecessorCursor() const {
        return cursor == RULE1_PRED || cursor == RULE2_PRED || cursor == RULE3_PRED;
    }

    bool IsRuleSuccessorCursor() const {
        return IsRuleCursor() && !IsRulePredecessorCursor();
    }

    size_t CursorPositionInAxiom() const {
        return cursor - AXIOM_0;
    }

    size_t CurrentRule() const {
        return (cursor - RULE1_PRED) / 9;
    }

    size_t CursorPositionInRule() const {
        return (cursor - RULE1_PRED) % 9 - 1;  // -1 because PRED is at position -1
    }

    void DrawUI() {
        gfxPrint(1, 12, "Axiom:");
        DrawAxiom(axiom, axiom_length, 1 + 6 * 6, 12);

        for (int i = 0; i < 3; i++) {
            //gfxPrint(1, 20 + i * 8, i+1);
            //gfxPrint(":");
            //DrawRule(rules[i], 1 + 2 * 6, 20 + i * 8);
            DrawRule(rules[i], 1, 20 + i * 8);
        }
        gfxPrint(1, 44, "Iter: ");
        gfxPrint(num_iterations);

        // Highlight the currently selected parameter and symbol
        int x, y;
        if (IsAxiomCursor()) {
            x = 1 + 6 * 6 + CursorPositionInAxiom() * 6;
            y = 20;
        } else if (IsRuleCursor()) {
            int ruleIndex = CurrentRule();
            y = 28 + ruleIndex * 8;
            if (IsRulePredecessorCursor()) {
                x = 1;
            } else {
                x = 1 + 2 * 6 + CursorPositionInRule() * 6;
            }
        } else if (cursor == ITERATIONS) {
            x = 1 + 6 * 6;
            y = 52;
        } else if (cursor == DRAWMODE) {
            x = 1;
            y = 60;
        }
        gfxCursor(x, y, 6);
    }

    void DrawAxiom(const std::array<Symbol, 8>& symbols, size_t length, int x, int y) {
        for (size_t i = 0; i < length; i++) {
            char axiomStr[2] = { SymbolToChar(symbols[i]), '\0' };
            gfxPrint(x + i * 6, y, axiomStr);
            
        }
    }

    void DrawRule(const Rule& rule, int x, int y) {
        char predStr[2] = { SymbolToChar(rule.predecessor), '\0' };

        gfxPrint(x, y, predStr);    // Print the predecessor 
        gfxPrint(x + 6, y, ">");   // Print the rule arrow 
        
        // Print each symbol in the successor array
        for (int i = 0; i < rule.successor_length; ++i) {
            char succChar[2] = { SymbolToChar(rule.successor[i]), '\0' };
            gfxPrint(x + 12 + i * 6, y, succChar); // Adjust x for symbol positioning
        }
    }

    char SymbolToChar(Symbol symbol) {
        switch(symbol) {
            case Symbol::F: return 'F';
            case Symbol::PLUS: return '+';
            case Symbol::MINUS: return '-';
            case Symbol::LBRACKET: return '[';
            case Symbol::RBRACKET: return ']';
            case Symbol::X: return 'X';
            case Symbol::EMPTY: return ' ';
            default: return '?';  // Default case for unknown symbols
        }
    }

    void GenerateLSystem(int iterations) {
        std::vector<Symbol> current = std::vector<Symbol>(axiom.begin(), axiom.begin() + axiom_length);
        std::vector<Symbol> next;

        for (int i = 0; i < iterations; i++) {
            for (Symbol symbol : current) {
                bool ruleApplied = false;
                for (const Rule& rule : rules) {
                    if (rule.predecessor == symbol) {
                        next.insert(next.end(), rule.successor.begin(), rule.successor.begin() + rule.successor_length);
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
            case Symbol::EMPTY:
                gate = false;
                Out(1, 0);
                break;
        }
    }

    void DrawLSystem(int start_x, int start_y, double start_angle, int start_length) {
        int x = start_x;
        int y = start_y; 
        int angle = start_angle;    //in deg
        int length = start_length;
        const int rotation_amount = 30;
        std::stack<std::pair<int, int>> positionStack;

        for (Symbol symbol : current_state) {
            switch(symbol) {
                case Symbol::F: {
                    double radians = angle * M_PI / 180.0;
                    int x1 = x + length * cos(radians);
                    int y1 = y - length * sin(radians);
                    
                    x = constrain(x, 0, 63);
                    y = constrain(y, 0, 47);
                    x1 = constrain(x1, 0, 63);
                    y1 = constrain(y1, 0, 47);
                    gfxLine(x, y, x1, y1);
                    x = x1;
                    y = y1;
                    break;
                }
                case Symbol::PLUS:
                    angle += rotation_amount;
                    if (angle >=360) angle -= 360;
                    if (angle <= 360) angle += 360;
                    break;
                case Symbol::MINUS:
                    angle -= rotation_amount;
                    if (angle >=360) angle -= 360;
                    if (angle <= 360) angle += 360;
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
                case Symbol::EMPTY:
                    // Do nothing for X and EMPTY
                    break;
            }
        }
    }
};