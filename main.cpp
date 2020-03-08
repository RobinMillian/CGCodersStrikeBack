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

class Pods : public Position {
    public:
        float vx;
        float vy;
        float angle;
        int nextCheckpointID;
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
    int laps;
    int checkpointsLength;
    Pods pods[2];
    Pods opponnent[2];
    Position last;
    int currentCheckpointIndex = 0;
    Position previousCheckpoint;
    float angleNextCheckpoint;


    cin >> laps; cin.ignore();
    cin >> checkpointsLength; cin.ignore();
    Position checkpoints[checkpointsLength];
    for (int i = 0; i < checkpointsLength; i++) {
        int x;
        int y;
        cin >> checkpoints[i].x >> checkpoints[i].y; cin.ignore();
    }
    // game loop
    while (1) {
        int nextCheckpointAngle;
        int nextCheckpointDist;
        int thurst = 100;
        vector<Position>::iterator itr;
        Position opponent;
        Position nextCheckpoint;
        cin >> pods[0].x >> pods[0].y >> pods[0].vx >> pods[0].vy >> pods[0].angle >> pods[0].nextCheckpointID; cin.ignore();
        cin >> pods[1].x >> pods[1].y >> pods[1].vx >> pods[1].vy >> pods[1].angle >> pods[1].nextCheckpointID; cin.ignore();
 
        cin >> opponnent[0].x >> opponnent[0].y >> opponnent[0].vx >> opponnent[0].vy >> opponnent[0].angle >> opponnent[0].nextCheckpointID; cin.ignore();
        cin >> opponnent[1].x >> opponnent[1].y >> opponnent[1].vx >> opponnent[1].vy >> opponnent[1].angle >> opponnent[1].nextCheckpointID; cin.ignore();
        // Write an action using cout. DON'T FORGET THE "<< endl"
        // To debug: cerr << "Debug messages..." << endl;

        cout << checkpoints[pods[0].nextCheckpointID].x << " " << checkpoints[pods[0].nextCheckpointID].y << " " << thurst << endl;
        cout << checkpoints[pods[1].nextCheckpointID].x << " " << checkpoints[pods[1].nextCheckpointID].y << " " << thurst << endl;

    }
}