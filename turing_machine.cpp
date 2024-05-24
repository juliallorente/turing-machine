#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <set>
#include <sstream>
#include <algorithm>

using namespace std;

vector<string> read_lines(const string& file_path) {
    ifstream file(file_path);
    vector<string> lines;
    string line;
    while (getline(file, line)) {
        lines.push_back(line);
    }
    return lines;
}

pair<string, vector<string>> process_input(const string& file_path) {
    vector<string> lines = read_lines(file_path);
    string model_type = lines[0];
    vector<string> transitions;
    for (size_t i = 1; i < lines.size(); ++i) {
        if (!lines[i].empty() && lines[i][0] != ';') {
            transitions.push_back(lines[i]);
        }
    }
    return {model_type, transitions};
}

pair<vector<string>, set<string>> process_transitions(const vector<string>& transitions) {
    vector<string> processed_transitions;
    set<string> states;
    set<string> alphabet = {"M"};

    for (const string& line : transitions) {
        istringstream iss(line);
        string current_state, current_symbol, new_symbol, direction, next_state;
        iss >> current_state >> current_symbol >> new_symbol >> direction >> next_state;

        try {
            current_state = to_string(stoi(current_state) + 1);
        } catch (invalid_argument&) {}

        try {
            next_state = to_string(stoi(next_state) + 1);
        } catch (invalid_argument&) {}

        states.insert(current_state);
        states.insert(next_state);
        alphabet.insert(current_symbol);
        alphabet.insert(new_symbol);
        processed_transitions.push_back(current_state + " " + current_symbol + " " + new_symbol + " " + direction + " " + next_state);
    }

    return {processed_transitions, states};
}

vector<string> add_left_space_transitions(const string& state) {
    return {
        state + " % % r LEFT_" + state,
        "LEFT_" + state + " * * r LEFT_" + state,
        "LEFT_" + state + " | # r MARK_" + state,
        "MARK_" + state + " _ | l BACK_" + state,
        "BACK_" + state + " # # l BACK_" + state
    };
}

vector<string> add_shifting_transitions(const string& state, const set<string>& alphabet) {
    vector<string> transitions;
    for (const string& symbol : alphabet) {
        transitions.push_back("BACK_" + state + " " + symbol + " # r SHIFT_" + state + "%" + symbol);
        transitions.push_back("SHIFT_" + state + "%" + symbol + " # " + symbol + " l BACK_" + state);
    }
    return transitions;
}

vector<string> add_right_space_transitions(const string& state) {
    return {
        "BACK_" + state + " % % r RIGHT_" + state,
        "RIGHT_" + state + " # _ l RIGHT_" + state,
        "RIGHT_" + state + " % % r " + state,
        state + " | _ r END_" + state,
        "END_" + state + " _ | l " + state
    };
}

vector<string> add_initial_and_final_transitions(const set<string>& alphabet) {
    vector<string> transitions;
    for (const string& symbol : alphabet) {
        transitions.push_back("0 " + symbol + " % r INIT_" + symbol);
        transitions.push_back("INIT_" + symbol + " * * r INIT_" + symbol);
        transitions.push_back("INIT_" + symbol + " _ # r FINISH_" + symbol);
        transitions.push_back("FINISH_" + symbol + " _ | l %" + symbol);
        transitions.push_back("%" + symbol + " # # l %" + symbol);

        for (const string& s : alphabet) {
            transitions.push_back("%" + symbol + " " + s + " # r SHIFT_" + symbol + "$" + s);
            transitions.push_back("SHIFT_" + symbol + "$" + s + " # " + s + " l %" + symbol);
        }

        transitions.push_back("%" + symbol + " % % r END_" + symbol);
        transitions.push_back("END_" + symbol + " # " + symbol + " l END_" + symbol);
        transitions.push_back("END_" + symbol + " % % r 1");
    }
    return transitions;
}

vector<string> convert_to_sipser_tape(const vector<string>& transitions, const set<string>& states, const set<string>& alphabet) {
    vector<string> additional_transitions;
    for (const string& state : states) {
        vector<string> left_transitions = add_left_space_transitions(state);
        additional_transitions.insert(additional_transitions.end(), left_transitions.begin(), left_transitions.end());

        vector<string> shift_transitions = add_shifting_transitions(state, alphabet);
        additional_transitions.insert(additional_transitions.end(), shift_transitions.begin(), shift_transitions.end());

        vector<string> right_transitions = add_right_space_transitions(state);
        additional_transitions.insert(additional_transitions.end(), right_transitions.begin(), right_transitions.end());
    }
    vector<string> init_transitions = add_initial_and_final_transitions(alphabet);
    additional_transitions.insert(additional_transitions.end(), init_transitions.begin(), init_transitions.end());
    return additional_transitions;
}

vector<string> convert_to_infinite_tape(const vector<string>& transitions, const set<string>& states) {
    vector<string> additional_transitions = {
        "0 * * l %",
        "% _ # r 1"
    };
    for (const string& state : states) {
        additional_transitions.push_back(state + " # # r " + state);
    }
    return additional_transitions;
}

void write_output(const string& file_path, const string& model_type, const vector<string>& transitions) {
    ofstream file(file_path);
    file << model_type << endl;
    for (const string& transition : transitions) {
        file << transition << endl;
    }
}

int main() {
    string input_file = "sameamount10.in";
    string output_file = "sameamount10.out";

    auto [model_type, transitions] = process_input(input_file);
    auto [processed_transitions, states] = process_transitions(transitions);
    set<string> alphabet = {"M"};  // Assuming "M" is part of the alphabet by default

    for (const string& transition : processed_transitions) {
        istringstream iss(transition);
        string current_state, current_symbol, new_symbol, direction, next_state;
        iss >> current_state >> current_symbol >> new_symbol >> direction >> next_state;
        alphabet.insert(current_symbol);
        alphabet.insert(new_symbol);
    }

    vector<string> additional_transitions;
    string output_model_type;

    if (model_type == ";S") {
        additional_transitions = convert_to_infinite_tape(processed_transitions, states);
        output_model_type = ";I";
    } else if (model_type == ";I") {
        additional_transitions = convert_to_sipser_tape(processed_transitions, states, alphabet);
        output_model_type = ";S";
    } else {
        cerr << "Modelo inválido" << endl;
        return 1;
    }

    processed_transitions.insert(processed_transitions.end(), additional_transitions.begin(), additional_transitions.end());
    write_output(output_file, output_model_type, processed_transitions);

    return 0;
}
