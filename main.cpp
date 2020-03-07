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
        inline bool operator!=(const Position &rhs) const {return x!= rhs.x && y != rhs.y;};
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
    int turn = 1;
    Position previousCheckpoint;
    float angleNextCheckpoint;
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
        if (!checkpoints.empty() && nextCheckpoint == checkpoints[1] && checkpoints.size() > 2 && previousCheckpoint != nextCheckpoint)
            turn++;
        itr = find(checkpoints.begin(), checkpoints.end(), nextCheckpoint);
        cerr << turn << endl;
        if (!turnCompleted) {
            if (itr == checkpoints.end()) {
                checkpoints.push_back(nextCheckpoint);
            }
            if (!checkpoints.empty() && nextCheckpoint == checkpoints[0] && checkpoints.size() > 1) {
                turnCompleted = !turnCompleted;
            }
        }
        if (turn == 3 && nextCheckpoint == checkpoints[checkpoints.size() - 1]) {
            Position dirPod;
            Position dirNextCheckpoint;
            dirPod.x = last.x - current.x;
            dirPod.y = last.y - current.y;
            dirNextCheckpoint.x = last.x - nextCheckpoint.x;
            dirNextCheckpoint.y = last.y - nextCheckpoint.y;
            angleNextCheckpoint = (dirPod.x * dirNextCheckpoint.x + dirPod.y * dirNextCheckpoint.y) / (dist(current.x, current.y, last.x, last.y) * dist(last.x, last.y, nextCheckpoint.x, nextCheckpoint.y));
            angleNextCheckpoint = acos(angleNextCheckpoint)  * 180 / 3.14;
            thurst = 100;
        if (!usedBoost && (angleNextCheckpoint >= -15 && angleNextCheckpoint <= 15)) {
                cout << nextCheckpoint.x << " " << nextCheckpoint.y << " BOOST" << endl;
                usedBoost = !usedBoost;
        } else if (nextCheckpointAngle > 45 || nextCheckpointAngle < -45) {
                cout << nextCheckpoint.x << " " << nextCheckpoint.y << " " << 0 << endl;
        } else {
            cout << nextCheckpoint.x << " " << nextCheckpoint.y << " " << thurst << endl;
            }
        }
        else if (turnCompleted && (nextCheckpointDist <= dist(current.x, current.y, last.x, last.y) * 4)) {
            currentCheckpointIndex = distance(checkpoints.begin(), itr);
            if (turnCompleted && nextCheckpointDist <= dist(current.x, current.y, last.x, last.y)) {
                thurst = 100;
            } else if (angleNextCheckpoint <= 20 && angleNextCheckpoint >= -20 && nextCheckpointAngle <= 20 && nextCheckpointAngle >= -20) {
                thurst = 100;
            } else {
                 thurst = 0;
                 //cout << nextCheckpoint.x << " " << nextCheckpoint.y << " " << thurst << endl;
            }
            if (nextCheckpointAngle > 60 || nextCheckpointAngle < -60)
                thurst = 0;
            cout << checkpoints[(currentCheckpointIndex + 1) % checkpoints.size()].x << " " <<  checkpoints[(currentCheckpointIndex + 1) % checkpoints.size()].y << " " << thurst << endl;  
        } else {
                if (nextCheckpointAngle > 60 || nextCheckpointAngle < -60)
                    thurst = 0;
                cout << nextCheckpoint.x << " " << nextCheckpoint.y << " " << thurst << endl;
        }
        // Edit this line to output the target position
        // and thrust (0 <= thrust <= 100)
        // i.e.: "x y thrust"
        last.x = current.x;
        last.y = current.y;
        previousCheckpoint = nextCheckpoint;
    }
}