#include <algorithm>
#include <iomanip>
#include <iostream>
#include <unordered_set>
#include <vector>

using namespace std;

constexpr int ROWS = 5;
constexpr int COLS = 4;
constexpr int NODES = ROWS * COLS;
constexpr int MAX_DEPTH = 29;
constexpr int MAX_CAR_SIZE = 4;

// constexpr int ROWS = 3;
// constexpr int COLS = 3;
// constexpr int NODES = ROWS * COLS;
// constexpr int MAX_DEPTH = 8;
// constexpr int MAX_CAR_SIZE = 4;

enum class Type { ANIMAL, HOME, EMPTY };
ostream& operator<<(ostream& ostr, Type t) {
  switch (t) {
    case Type::ANIMAL:
      ostr << "A";
      break;
    case Type::HOME:
      ostr << "H";
      break;
    case Type::EMPTY:
      ostr << " ";
      break;
  }
  return ostr;
}

struct Node {
  // id of node in the graph
  int id = -1;
  // id of matching animal or home
  string animal;
  Type type = Type::EMPTY;
  // whether we've picked up the animal or populated the home
  bool satisfied = false;

  void set(Type type, string animal) {
    this->type = type;
    this->animal = animal;
  }
};

typedef pair<int, int> Point;
typedef pair<Point, Point> Transition;

typedef bool Transitions[NODES][NODES];
typedef Node Nodes[ROWS][COLS];

typedef pair<int, int> Direction;

Direction directions[4] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

struct State {
  vector<Point> path;
  unordered_set<string> inCar;
};

vector<Point> children(const Point& p) {
  int x = p.first;
  int y = p.second;
  vector<Point> result;
  for (auto dir : directions) {
    int dx = dir.first;
    int dy = dir.second;
    if (x + dx >= 0 && x + dx < ROWS && y + dy >= 0 && y + dy < COLS) {
      result.push_back({x + dx, y + dy});
    }
  }
  return result;
}

int getId(const Point& p) { return COLS * p.first + p.second; }

void removeTransition(Transitions transitions, Transition& t) {
  int fromId = getId(t.first);
  int toId = getId(t.second);
  transitions[fromId][toId] = false;
  transitions[toId][fromId] = false;
}

bool isSatisfied(Nodes nodes) {
  bool result = true;
  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < COLS; j++) {
      if (nodes[i][j].type == Type::HOME) {
        result = result && nodes[i][j].satisfied;
        // if (!nodes[i][j].satisfied) {
        //   cout << "ns(" << i << "," << j << ") ";
        // }
      }
    }
  }
  // cout << endl;
  return result;
}

bool traverse(Point currentPoint, State& state, Nodes& nodes,
              Transitions& transitions, bool allowBacktrack) {
  Node& current = nodes[currentPoint.first][currentPoint.second];
  if (state.path.size() == MAX_DEPTH + 1) {
    auto satisfied = isSatisfied(nodes);
    if (satisfied) {
      cout << "Found result: ";
      for_each(state.path.begin(), state.path.end(), [](Point& p) {
        cout << "(" << p.first << "," << p.second << ") ";
      });
      cout << endl;
    }
    return satisfied;
  }

  if (state.path.size() > MAX_DEPTH + 1) {
    cerr << "Reached more than MAX_DEPTH :(" << endl;
    return false;
  }

  for (auto child : children(currentPoint)) {
    if (!transitions[getId(currentPoint)][getId(child)]) {
      continue;
    }
    if (!allowBacktrack && state.path.size() > 1) {
      Point& previous = state.path[state.path.size() - 2];
      if (previous.first == child.first && previous.second == child.second) {
        // skip it, no point in going back
        continue;
      }
    }
    Node& next = nodes[child.first][child.second];
    state.path.push_back(child);

    switch (next.type) {
      case Type::EMPTY:
        if (traverse(child, state, nodes, transitions, false)) {
          // cout << "E(" << child.first << "," << child.second << ") ";
          return true;
        }
        break;

      case Type::HOME:
        // cout << "At the home of " << next.animal << ", in car: ";
        // for (auto a : state.inCar) {
        //   cout << a << ", ";
        // }
        // cout << endl;
        if (next.satisfied ||
            state.inCar.find(next.animal) == state.inCar.end()) {
          if (traverse(child, state, nodes, transitions, false)) {
            // cout << "(" << child.first << "," << child.second << ") ";
            return true;
          }
        } else {
          state.inCar.erase(next.animal);
          next.satisfied = true;
          if (traverse(child, state, nodes, transitions, true)) {
            cout << "Satisfied home (" << child.first << "," << child.second << ") " << endl;
            return true;
          }
          state.inCar.insert(next.animal);
          next.satisfied = false;
        }

        break;

      case Type::ANIMAL:
        if (next.satisfied || state.inCar.size() == MAX_CAR_SIZE) {
          if (traverse(child, state, nodes, transitions, false)) {
            // cout << "(" << child.first << "," << child.second << ") ";
            return true;
          }
        } else {
          state.inCar.insert(next.animal);
          next.satisfied = true;
          // cout << "Picked up animal from (" << child.first << ", " <<
          // child.second << "), in car size: " << state.// inCar.size() <<
          // endl;
          if (traverse(child, state, nodes, transitions, true)) {
            cout << "Picked up animal (" << child.first << "," << child.second << ") " << endl;
            return true;
          }
          // cout << "Pick up from (" << child.first << ", " << child.second <<
          // ") did'nt work out, trying without" << endl;
          next.satisfied = false;
          state.inCar.erase(next.animal);
          if (traverse(child, state, nodes, transitions, false)) {
            cout << "Skipped animal (" << child.first << "," << child.second << ") " << endl;
            return true;
          }
        }
        break;
    }
    state.path.pop_back();
  }
  return false;
}

void populateNodes(Nodes& nodes) {
  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < COLS; j++) {
      nodes[i][j].id = getId({i, j});
    }
  }

  nodes[0][0].set(Type::ANIMAL, "hedgehog");
  nodes[0][1].set(Type::ANIMAL, "chameleon");
  nodes[0][2].set(Type::HOME, "cat");
  nodes[0][3].set(Type::ANIMAL, "mink");

  nodes[1][0].set(Type::ANIMAL, "parrot");
  nodes[1][1].set(Type::HOME, "turtle");
  nodes[1][2].set(Type::HOME, "dog");
  nodes[1][3].set(Type::ANIMAL, "siamese");

  nodes[2][0].set(Type::HOME, "dakel");
  nodes[2][1].set(Type::EMPTY, "");
  nodes[2][2].set(Type::HOME, "mink");
  nodes[2][3].set(Type::ANIMAL, "cat");

  nodes[3][0].set(Type::EMPTY, "");
  nodes[3][1].set(Type::HOME, "chameleon");
  nodes[3][2].set(Type::ANIMAL, "turtle");
  nodes[3][3].set(Type::ANIMAL, "dog");

  nodes[4][0].set(Type::ANIMAL, "dakel");
  nodes[4][1].set(Type::HOME, "hedgehog");
  nodes[4][2].set(Type::HOME, "parrot");
  nodes[4][3].set(Type::HOME, "siamese");


  // nodes[0][0].set(Type::EMPTY, "");
  // nodes[0][1].set(Type::ANIMAL, "cat");
  // nodes[0][2].set(Type::EMPTY, "");

  // nodes[1][0].set(Type::ANIMAL, "dog");
  // nodes[1][1].set(Type::EMPTY, "");
  // nodes[1][2].set(Type::EMPTY, "");

  // nodes[2][0].set(Type::HOME, "cat");
  // nodes[2][1].set(Type::EMPTY, "");
  // nodes[2][2].set(Type::HOME, "dog");

  // nodes[0][0].set(Type::ANIMAL, "cat");
  // nodes[0][1].set(Type::EMPTY, "");

  // nodes[1][0].set(Type::EMPTY, "");
  // nodes[1][1].set(Type::HOME, "cat");
}

void populateTransitions(Transitions& transitions) {
  // no transitions by default
  for (int i = 0; i < NODES; i++) {
    for (int j = 0; j < NODES; j++) {
      transitions[i][j] = false;
    }
  }

  // add transitions in the rectangular grid
  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < COLS; j++) {
      for (auto child : children({i, j})) {
        transitions[getId({i, j})][getId(child)] = true;
        transitions[getId(child)][getId({i, j})] = true;
      }
    }
  }

  // remove transitions according to current labirynth
  // vector<Transition> missingEdges = {{{1, 0}, {1, 1}}, {{2, 1}, {2, 2}}};
  vector<Transition> missingEdges = {
    {{1, 0}, {1, 1}},
    {{1, 2}, {2, 2}},
    {{2, 3}, {3, 3}},
    {{2, 1}, {2, 2}},
    {{4, 0}, {4, 1}},
    {{4, 2}, {4, 3}}
  };

  for (auto edge : missingEdges) {
    removeTransition(transitions, edge);
  }
}

int main() {
  Nodes nodes;
  Transitions transitions;

  populateNodes(nodes);
  populateTransitions(transitions);

  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < COLS; j++) {
      cout << setw(1) << nodes[i][j].type << "(" << setw(9)
           << nodes[i][j].animal << ")";
      string tr = "   ";
      if (j + 1 < COLS && transitions[getId({i, j})][getId({i, j + 1})]) {
        tr = "---";
      }
      cout << tr;
    }
    cout << endl;
    for (int j = 0; j < COLS && i != ROWS - 1; j++) {
      char tr = ' ';
      if (transitions[getId({i, j})][getId({i + 1, j})]) {
        tr = '|';
      }
      cout << setw(6) << " ";
      cout << setw(1) << tr;
      cout << setw(5) << " ";
      cout << setw(3) << " ";
    }
    cout << endl;
  }

  State state;
  // Point root = {1, 1};
  Point root = { 2, 1 };
  state.path.push_back(root);
  if (!traverse(root, state, nodes, transitions, false)) {
    cout << "No path found :(" << endl;
  }
}
