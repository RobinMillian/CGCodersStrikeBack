#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>

using namespace std;


class Position {
    public:
    Position() { x = 0; y = 0;};
    Position(const Position &pos) {
        x = pos.x;
        y = pos.y;
    };
        int x;
        int y;
        inline bool operator==(const Position &rhs) const {return x == rhs.x && y == rhs.y;};
};

/**
 * This code automatically collects game data in an infinite loop.
 * It uses the standard input to place data into the game variables such as x and y.
 * YOU DO NOT NEED TO MODIFY THE INITIALIZATION OF THE GAME VARIABLES.
 **/

float dist(float x, float y, float lastx, float lasty) {
    return sqrt((lastx - x) * (lastx - x) + (lasty - y) * (lasty - y));
}

int main()
{
    Position last;
    bool usedBoost = false;
    vector<Position> checkpoints;
    bool turnCompleted = false;
    int currentCheckpointIndex = 0;

    // game loop
    while (1) {
        Position current;
        int nextCheckpointAngle;
        int nextCheckpointDist;
        int thurst = 100;
        vector<Position>::iterator itr;
        Position opponent;
        Position nextCheckpoint;
        cin >> current.x >> current.y >> nextCheckpoint.x >> nextCheckpoint.y >> nextCheckpointDist >> nextCheckpointAngle; cin.ignore();
        cin >> opponent.x >> opponent.y; cin.ignore();
        // Write an action using cout. DON'T FORGET THE "<< endl"
        // To debug: cerr << "Debug messages..." << endl;
        itr = find(checkpoints.begin(), checkpoints.end(), nextCheckpoint);
        if (!turnCompleted) {
            if (itr == checkpoints.end()) {
                checkpoints.push_back(nextCheckpoint);
            }
            if (!checkpoints.empty() && nextCheckpoint == checkpoints[0] && checkpoints.size() > 1) {
                turnCompleted = !turnCompleted;
            }
        }        
        if ((nextCheckpointDist <= dist(current.x, current.y, last.x, last.y) * 3)) {
             thurst = 0;
             if (turnCompleted) {
                 currentCheckpointIndex = distance(checkpoints.begin(), itr);
                 cout << checkpoints[currentCheckpointIndex + 1 % checkpoints.size()].x << " " <<  checkpoints[currentCheckpointIndex + 1 % checkpoints.size()].y << " " << thurst << endl;
                 
             } else {
                cout << nextCheckpoint.x << " " << nextCheckpoint.y << " " << thurst << endl;
            }
        } else {
            if (!usedBoost && (nextCheckpointDist >= dist(current.x, current.y, last.x, last.y) * 10) && (nextCheckpointAngle > -20 && nextCheckpointAngle < 20)) {
                cout << nextCheckpoint.x << " " << nextCheckpoint.y << " BOOST" << endl;
                usedBoost = !usedBoost;
            } else {
                cout << nextCheckpoint.x << " " << nextCheckpoint.y << " " << thurst << endl;
            }

        }


        // Edit this line to output the target position
        // and thrust (0 <= thrust <= 100)
        // i.e.: "x y thrust"
        last.x = current.x;
        last.y = current.y;
    }
}