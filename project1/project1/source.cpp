/*
Kevin Grajeda
Project 1: 15-Puzzle Problem
*/

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
using namespace std;
using State = vector<vector<int>>;

//Node struct, children are represented as a vector of node pointers
struct Node {
	Node(State st, Node* pa)
		: state(st), parent(pa) {}
	State state;
	Node* parent;
	vector<Node*>children;
};

//Open the text file, retries until file name in source path is entered
void openStream(ifstream& stream) {
	string filename;
	cout << "Enter file name: ";
	cin >> filename;
	stream.open(filename);
	while (!stream) {
		stream.clear();
		cout << "Invalid filename, re-enter file name: ";
		cin >> filename;
		stream.open(filename);
	}
}

//Reads in the contents of the file, puts all of it into one vector
//Then separates the values into initial and goal states 
void readStream(ifstream& stream, State& initial, State& goal) {
	int num;
	vector<int> input;
	//Takes the input
	while(stream >> num) {
		input.push_back(num);
	}
	//Creates initial state
	int inputCount = 0;
	for (size_t j = 0; j < 4; ++j) {
		for (size_t k = 0; k < 4; ++k) {
			initial[j][k] = input[inputCount];
			inputCount++;
		}
	}
	//Creates goal state
	for (size_t j = 0; j < 4; ++j) {
		for (size_t k = 0; k < 4; ++k) {
			goal[j][k] = input[inputCount];
			inputCount++;
		}
	}
	stream.close();
}

//Outputs the results to an output file called output
void writeOutput(ofstream& outputFile, const State& initialState, const State& goalState,
				const vector<int> fnValues, const vector<char> movements, int depth, int nodesProduced) {
	outputFile.open("output.txt");
	//Outputs initial state
	for (size_t i = 0; i < initialState.size(); ++i) {
		for (size_t j = 0; j < initialState[i].size(); ++j) {
			outputFile << initialState[i][j] << " ";
		}
		outputFile << endl;
	}
	outputFile << endl;
	//Outputs goal state
	for (size_t i = 0; i < goalState.size(); ++i) {
		for (size_t j = 0; j < goalState[i].size(); ++j) {
			outputFile << goalState[i][j] << " ";
		}
		outputFile << endl;
	}
	//Outputs depth and number of nodes produced
	outputFile << endl << depth << endl;
	outputFile << nodesProduced << endl;
	//Outputs list of movements
	for (size_t i = 0; i < movements.size(); ++i) {
		outputFile << movements[i] << " ";
	}
	outputFile << endl;
	//Outputs list of f(n) values
	for (size_t i = 0; i < fnValues.size(); ++i) {
		outputFile << fnValues[i] << " ";
	}
}

//Find a specific position number in a state, returns -1, -1 if not found
pair<int, int> findPos(const State& state, int piece) {
	for (size_t i = 0; i < state.size(); ++i) {
		for (size_t j = 0; j < state[i].size(); ++j) {
			if (state[i][j] == piece) {
				return make_pair(i, j);
			}
		}
	}
	return make_pair(-1, -1);
}

//Moves the given piece number to an empty slot, returns new state
//If swap does not happen, state remains the same but as a new state
State move(const State& state, int piece) {
	State newState = state;
	pair<int, int> position = findPos(state, piece);
	pair<int, int> zeroPos = findPos(state, 0);
	if (position.first == -1 && position.second == -1) {
		return newState;
	}
	for (size_t i = 0; i < state.size(); ++i) {
		for (size_t j = 0; j < state[i].size(); ++j) {
			//Makes the movement, swap 0 and the given position
			if (newState[i][j] == 0) {
				newState[i][j] = newState[position.first][position.second];
				newState[position.first][position.second] = 0;
				return newState;
			}
		}
	}
	return newState;
}

//Calculates Manhattan distance of a given state's position
int manhattanDistance(const State& state, int position, const State& goal) {
	pair<int, int> statePos = findPos(state, position);
	pair<int, int> goalPos = findPos(goal, position);
	//Ignores 0 since it represents an empty space
	if (position == 0) {
		return 0;
	}
	return abs(goalPos.first - statePos.first) + abs(goalPos.second - statePos.second);
}

//Calculates sum of Manhattan distances for a given state 
int manhattanDistanceSum(const State& state, const State& goal) {
	int sum = 0;
	for (size_t i = 0; i < state.size(); ++i) {
		for (size_t j = 0; j < state[i].size(); ++j) {
			sum += manhattanDistance(state, state[i][j], goal);
		}
	}
	return sum;
}

//Finds the possible moves for a state and returns the movable piece numbers as a vector
//Checks that index won't go out of bounds before adding move
//Order or possible moves is always: D, R, L, U
//If one of the movements is not possible, it is replaced with -1
vector<int> possibleMoves(const State& state) {
	vector<int> moveList;
	pair<int, int> zeroPos = findPos(state, 0);
	//Piece above empty space (Moves down)
	if (zeroPos.first - 1 >= 0) {
		moveList.push_back(state[zeroPos.first - 1][zeroPos.second]);
	}
	else {
		moveList.push_back(-1);
	}
	//Piece to the left of empty space (Moves right)
	if (zeroPos.second - 1 >= 0) {
		moveList.push_back(state[zeroPos.first][zeroPos.second - 1]);
	}
	else {
		moveList.push_back(-1);
	}
	//Piece to the right of empty space (Moves left)
	if (zeroPos.second + 1 < 4) {
		moveList.push_back(state[zeroPos.first][zeroPos.second + 1]);
	}
	else {
		moveList.push_back(-1);
	}
	//Piece below empty space (Moves up)
	if (zeroPos.first + 1 < 4) {
		moveList.push_back(state[zeroPos.first + 1][zeroPos.second]);
	}
	else {
		moveList.push_back(-1);
	}
	return moveList;
}

//Determines the value of the evaluation function, h(n) = sum of Manhattan distances
int evaluationFunction(const State& initial, const State& current, const State& goal) {
	//f(n) = g(n) + h(n)
	return manhattanDistanceSum(initial, current) + manhattanDistanceSum(current, goal);
}

//Find the lowest f(n) value in a list of nodes 
int lowestFVal(const vector<Node*>& list, const State& initial, const State& goal) {
	int resultInd = 0;
	for (size_t i = 0; i < list.size(); ++i) {
		if (evaluationFunction(initial, list[i]->state, goal) 
			< evaluationFunction(initial, list[resultInd]->state, goal)) {
			resultInd = i;
		}
	}
	return resultInd;
}

//Creates the possible successors of a given node
//Assigns the node's children and assigns each child's parent as the given node
//Returns the amount of nodes generated
int generateSuccessors(Node*& node) {
	vector<int> moves = possibleMoves(node->state);
	for (size_t i = 0; i < moves.size(); ++i) {
		//-1 is an ivalid movement position
		if (moves[i] != -1) {
			Node* child = new Node(move(node->state, moves[i]), node);
			node->children.push_back(child);
		}
	}
	return moves.size();
}

//Checks if a node is in a list of nodes
bool isInNodeList(const vector<Node*>& list, const State& state) {
	for (size_t i = 0; i < list.size(); ++i) {
		if (list[i]->state == state) {
			return true;
		}
	}
	return false;
}

//Performs the A* algorithm, returns a pair the contains a vector of 
//the path and the number of nodes produced
pair<vector<Node*>, int> aStar(const State& initial, const State& goal) {
	int nodesProduced = 0;
	vector<Node*> openList;
	vector<Node*> closedList;
	Node* start = new Node(initial, nullptr);
	openList.push_back(start);
	while (openList.size() != 0) {
		int lowestFInd = lowestFVal(openList, initial, goal);
		Node* lowestFNode = openList[lowestFInd];
		//Get the node with lowest evaluation function in the open list
		for (size_t i = 0; i < openList.size(); ++i) {
			if (evaluationFunction(initial, openList[i]->state, goal) 
				< evaluationFunction(initial, lowestFNode->state, goal)) {
				lowestFInd = i;
				lowestFNode = openList[i];
			}
		}
		openList.erase(openList.begin() + lowestFInd);
		closedList.push_back(lowestFNode);
		//If the node is the goal, backtracks
		//Returns list of nodes and the amount of nodes produced
		if (lowestFNode->state == goal) {
			vector<Node*> path;
			Node* current = lowestFNode;
			while (current != nullptr) {
				path.push_back(current);
				current = current->parent;
			}
			return make_pair(path, nodesProduced);
		}
		//Generates successors and adds to the amount of nodes produced
		nodesProduced += generateSuccessors(lowestFNode);
		//Iterates through children of the current node (node with lowest f)
		for (size_t i = 0; i < lowestFNode->children.size(); ++i) {
			//Child is on closed list
			if (isInNodeList(closedList, lowestFNode->children[i]->state)) {
				continue;
			}
			//Child is in open list
			if (isInNodeList(openList, lowestFNode->children[i]->state)) {
				int gVal = evaluationFunction(initial, lowestFNode->children[i]->state, goal) 
							- manhattanDistanceSum(lowestFNode->children[i]->state, goal);
				//Checks if evaluation function of current node is greater than the one of any node in the open list
				for (size_t j = 0; j < openList.size(); ++j) {
					if (lowestFNode->children[i]->state == openList[j]->state && gVal 
						> evaluationFunction(initial, openList[j]->state, goal) 
						- manhattanDistanceSum(openList[j]->state, goal) ) {
						continue;
					}
				}
			}
			openList.push_back(lowestFNode->children[i]);
		}

	}
}

//Determines if a move was L, R, U, or Down
//moves[0] will always be an up movement
//moves[1] will always be a left movement
//moves[2] will always be a right movement
//moves[3] will always be a down movement
//If a movement in a direction is not possible, it will be skipped by checking if theres a -1 in the moves vector
//Reverses the move and matches it with the parent node to see if which direction the movement was from
vector<char> getCharMovements(const vector<Node*>& nodes) {
	vector<char> movements;
	for (size_t i = 1; i < nodes.size(); ++i) {
		vector<int> moves = possibleMoves(nodes[i]->state);
		Node* previous = nodes[i]->parent;
		if (move(nodes[i]->state, moves[0]) == previous->state) {
			movements.push_back('U');
		}
		else if(move(nodes[i]->state, moves[1]) == previous->state){
			movements.push_back('L');
		}
		else if (move(nodes[i]->state, moves[2]) == previous->state) {
			movements.push_back('R');
		}
		else { //move(nodes[i]->state, moves[3]) == previous->state
			movements.push_back('D');
		}
	}
	return movements;
}

//prints state in 4x4 grid
void printState(const State& state) {
	for (size_t i = 0; i < state.size(); ++i) {
		for (size_t j = 0; j < state[i].size(); ++j) {
			cout << state[i][j] << " ";
		}
		cout << endl;
	}
	cout << endl;
}

int main() {
	State initialState(4, vector<int>(4));
	State goalState(4, vector<int>(4));
	vector<int> fnValues;
	ifstream inputFile;
	ofstream outputFile;

	openStream(inputFile);
	readStream(inputFile, initialState, goalState);
	pair<vector<Node*>, int> result = aStar(initialState, goalState);
	reverse((result.first).begin(), (result.first).end());

	vector<Node*> solutionPath = result.first;
	for (size_t i = 0; i < solutionPath.size(); ++i) {
		printState(solutionPath[i]->state);
	}
	for (size_t i = 0; i < solutionPath.size(); ++i) {
		fnValues.push_back(evaluationFunction(initialState, solutionPath[i]->state, goalState));
	}
	vector<char> movements = getCharMovements(solutionPath);
	int nodesProduced = result.second;
	int depth = (solutionPath).size();

	writeOutput(outputFile, initialState, goalState, fnValues, movements, depth, nodesProduced);
}