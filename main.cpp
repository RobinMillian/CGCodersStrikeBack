#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>

using namespace std;

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
    int lastx = 0;
    int lasty = 0;
    bool usedBoost = false;
    // game loop
    while (1) {
        int x; // x position of your pod
        int y; // y position of your pod
        int nextCheckpointX; // x position of the next check point
        int nextCheckpointY; // y position of the next check point
        int nextCheckpointAngle;
        int nextCheckpointDist;
        int thurst = 100;
        int opponentX;
        int opponentY;
        cin >> x >> y >> nextCheckpointX >> nextCheckpointY >> nextCheckpointDist >> nextCheckpointAngle; cin.ignore();
        cin >> opponentX >> opponentY; cin.ignore();
        // Write an action using cout. DON'T FORGET THE "<< endl"
        // To debug: cerr << "Debug messages..." << endl;
        if ((nextCheckpointDist <= dist(x, y, lastx, lasty) * 3)) {
             thurst = 0;
        }
        
        if (!usedBoost && (nextCheckpointDist >= dist(x, y, lastx, lasty) * 10) && (nextCheckpointAngle > -20 && nextCheckpointAngle < 20)) {
            cout << nextCheckpointX << " " << nextCheckpointY << " BOOST" << endl;
            usedBoost = !usedBoost;
        } else {
            cout << nextCheckpointX << " " << nextCheckpointY << " " << thurst << endl;
        }

        // Edit this line to output the target position
        // and thrust (0 <= thrust <= 100)
        // i.e.: "x y thrust"
        lastx = x;
        lasty = y;
    }
}
