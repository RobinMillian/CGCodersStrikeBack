#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <array>
#include <chrono>


using namespace std;

#define MAX_POP 10000
#define MAX_TURN 6


class Point {
    public:
        Point() {x = 0; y = 0;};
        Point(float _x, float _y) {x = _x; y =_y;};
        float x;
        float y;
        float distSquare(const Point &p) const {
            return (x - p.x) * (x - p.x) + (y - p.y) * (y - p.y);
        };
        float dist(const Point &p) const {
            return sqrt(distSquare(p));
        };
        Point closest(const Point &a, const Point &b) const {
            float da = b.y - a.y;
            float db = a.x - b.x;
            float c1 = da * a.x + db * a.y;
            float c2 = -db * x + da * y;
            float det = da * da + db * db;
            float cx = 0;
            float cy = 0;

            if (det != 0) {
                cx = (da * c1 - db * c2) / det;
                cy = (da * c2 + db * c1) / det;
            } else {
                cx = x;
                cy = y;
            }

            return Point(cx, cy);
        };

};

class Coll;

class Unit : public Point {
    public:
        Unit() {vx = 0; vy = 0;};
        int id;
        float r;
        float vx;
        float vy;
        Unit &operator=(const Unit &a);
        Coll *collision(const Unit &u);
        void bounce(Unit &u);
};

class Coll {
    public:
        Coll();
        Coll(const Unit &_a, const Unit &_b, float _t)
        {
            a = _a;
            b = _b;
            t = _t;
        };
        Unit a;
        Unit b;
        float t;
};

Unit &Unit::operator=(const Unit &a) {
        id = a.id;
        r = a.r;
        vx = a.vx;
        vy = a.vy;
        x = a.x;
        y = a.y;
        return *this;
}

Coll *Unit::collision(const Unit &u) {
    float dist = distSquare(u);
    float sr = (r + u.r)*(r + u.r);

    if (dist < sr) {
        return new Coll(*this, u, 0.0);
    }

    // Cannot collide if object have same speed
    if (vx == u.vx && vy == u.vy) {
        return nullptr;
    }
    float _x = x - u.x;
    float _y = y - u.y;
    Point myp = Point(_x, _y);
    float _vx = vx - u.vx;
    float _vy = vy - u.vy;
    Point up(0, 0);

    Point p = up.closest(myp, Point(_x + _vx, _y + _vy));

    float pdist = up.distSquare(p);
    float mypdist = myp.distSquare(p);

    if (pdist < sr) {
        float length = sqrt(_vx*_vx + _vy*_vy);

        // find impact point
        float backdist = sqrt(sr - pdist);
        p.x = p.x - backdist * (_vx / length);
        p.y = p.y - backdist * (_vy / length);

        if (myp.distSquare(p) > mypdist) {
            return nullptr;
        }

        pdist = p.dist(myp);

        if (pdist > length) {
            return nullptr;
        }

        // Computed time before impact
        float t = pdist / length;

        return new Coll(*this, u, t);
    }
    return nullptr;
}

void Unit::bounce(Unit &u) {
        //mass can be change if we want to implement shield
        float m1 = 1;
        float m2 = 1;
        // If mass are equals the coef will be 2 otherwise it will be 11/10
        float mcoeff = (m1 + m2) / (m1 * m2);

        float nx = x - u.x;
        float ny = y - u.y;

        // Dist² between two pods
        float nxnysquare = nx*nx + ny*ny;

        float dvx = vx - u.vx;
        float dvy = vy - u.vy;

        // fx and fy are the impact vector
        float product = nx*dvx + ny*dvy;
        float fx = (nx * product) / (nxnysquare * mcoeff);
        float fy = (ny * product) / (nxnysquare * mcoeff);

        // Apply one time the impact vector of each pods depending on mass
        vx -= fx / m1;
        vy -= fy / m1;
        u.vx += fx / m2;
        u.vy += fy / m2;

        float impulse = sqrt(fx*fx + fy*fy);
        if (impulse < 120.0) {
            fx = fx * 120.0 / impulse;
            fy = fy * 120.0 / impulse;
        }

        // Apply again the impact vector
        vx -= fx / m1;
        vy -= fy / m1;
        u.vx += fx / m2;
        u.vy += fy / m2;
}

class Move {
    public:
        Move() {angle = 0; thurst = 0;}
        float angle;
        int thurst;
        Move(const float _angle, const int _thurst) {
            angle = _angle;
            thurst = _thurst;
        }
        void mutate(float amplitude) {
            float ramin = angle - 36.0 * amplitude;
            float ramax = angle + 36.0 * amplitude;

            if (ramin < -18.0) {
                ramin = -18.0;
            }

            if (ramax > 18.0) {
                ramax = 18.0;
            }
            angle = rand() % (int)(ramax - ramin + 1) + ramin;
            int pmin = thurst - 100 * amplitude;
            int pmax = thurst + 100 * amplitude;

            if (pmin < 0) {
                pmin = 0;
            }

            if (pmax > 0) {
                pmax = 100;
            }
            thurst = rand() % (pmax - pmin + 1) + pmin;
            if (thurst >= 20) {
                thurst = 100;
            }
        };
};

class Checkpoint: public Unit {
    public:
        Checkpoint() {r=100;};
};

class Pod: public Unit {
    public:
        Pod() { r = 400;};
        float angle;
        int checkpointsCount;
        int nextCheckpointId = 1;
        int checked = 0;
        int timeout = 100;

        float getAngle(const Point &p) const {
            float d = dist(p);
            float dx = (p.x - x) / d;
            float dy = (p.y - y) / d;

            float a = acos(dx) / 180.0 * 3.14;

            if (dy < 0) {
                a = 360 - a;
            }
            return a;
        };
        Pod &operator=(const Pod &p) {
            x = p.x;
            y = p.y;
            vx = p.vx;
            vy = p.vy;
            angle = p.angle;
            checkpointsCount = p.checkpointsCount;
            nextCheckpointId = p.nextCheckpointId;
            checked = p.checked;
            timeout = p.timeout;
            return *this;
        }
        Pod &operator=(const Move &m) {
            angle += m.angle;
            if (angle >= 360.0) {
                angle = angle - 360.0;
            } else if (angle < 0.0) {
                angle += 360.0;
            }
            boost(m.thurst);
            return *this;
        }

        float diffAngle(const Point &p) const {
            float a = getAngle(p);
            float right = angle <= a ? a - angle : 360.0 - angle + a;
            float left = angle >= a ? angle - a : angle + 360.0 - a;

            if (right < left) {
                return right;
            }
            return -left;
        };

        void rotate(const Point &p) {
            float a = diffAngle(p);

            if (a > 18.0) {
                a = 18.0;
            } else if  (a < -18.0) {
                a = -18.0;
            }
            angle = (angle + a >= 360.0) ? (angle + a - 360.0) : (angle + a < 0.0 ? (angle + a + 360.0) : (angle + a));
        };

        void boost(const int thurst) {

            float ra = angle * 3.14 / 180.0;

            vx += cos(ra) * thurst;
            vy += sin(ra) * thurst;
        };

        void move(const float t) {
            x += vx * t;
            y += vy * t;
        };

        void end() {
            x = round(x);
            y = round(y);
            vx = (int)vx;
            vy = (int)vy;
            timeout--;
        };

        void play(const Point &p, const int thurst) {
            rotate(p);
            boost(thurst);
            move(1.0);
            end();

        };
        void bounceWithCheckpoint(const Checkpoint &checkpoint) {

            timeout = 100;
            checked++;
            nextCheckpointId = nextCheckpointId + 1 >= checkpointsCount ? 0 : nextCheckpointId + 1;
        };
        float score(const Checkpoint &checkpoint) {
            if (timeout <= 0)
                return -9000000;
                
            float cvx = checkpoint.x - x;
            float cvy = checkpoint.y - y;
            float a = (cvx * vx + cvy * vy) / (dist(checkpoint) * sqrt((vx * vx) + (vy * vy)));
            a = acos(a);

            return   (checked * 50000) / (a == 0.0 ? 1 : a) - (dist(checkpoint) * 0.01);
        };

        void output(Move &move) {
            float a = angle + move.angle;

            if (a >= 360.0) {
                a = a - 360.0;
            } else if (a < 0.0) {
                a += 360.0;
            }
            a = a * 3.14 / 180.0;
            float px = x + cos(a) * 10000.0;
            float py = y + sin(a) * 10000.0;
            
            cout << round(px) << " " << round(py) << " " << move.thurst << endl;
        }
};

void play(Pod pods[2], Checkpoint checkpoints[]) {
    float t = 0.0;

    while (t < 1.0) {
        int index = -1;
        Coll *collision = nullptr;
        Coll *firstCol = nullptr;
        for (int i = 0; i < 2 ; i++) {
            if (i + 1 < 2) {
                Coll *col = pods[i].collision(pods[i + 1]);

                if (col != nullptr && col->t + t < 1.0 && (firstCol == nullptr || col->t < firstCol->t)) {
                    firstCol = col;
                }
            }
            collision = pods[i].collision(checkpoints[pods[i].nextCheckpointId]);
            if (collision != nullptr && collision->t + t < 1.0 && (firstCol == nullptr || collision->t < firstCol->t)) {
                firstCol = collision;
                index = i;
            }
        }
        if (firstCol == nullptr) {
            for (int i = 0; i < 2; i++) {
                pods[i].move(1.0 - t);
            }
            t = 1.0;
        } else {
            for (int i = 0; i < 2; i++) {
                pods[i].move(firstCol->t);
            }
            if (index != -1) {
                pods[index].bounceWithCheckpoint(static_cast<Checkpoint &>(firstCol->b));
            } else {
                firstCol->a.bounce(firstCol->b);
            }
            t += firstCol->t;
            if (firstCol->t <= 0.1)
                t = 1.0;
        }
    }
    for (int i = 0; i < 2; i++) {
        pods[i].end();
    }
}

class Solution {
    public:
        Solution() {};
        Move moves1[MAX_TURN];
        Move moves2[MAX_TURN];
        void score(Checkpoint *checkpoints, Pod currentPods[2], float result[2]) {
            Pod pods[2];
            pods[0] = currentPods[0];
            pods[1] = currentPods[1];
            for (int i = 0; i < MAX_TURN; i++) {
                pods[0] = moves1[i];
                pods[1] = moves2[i];
                play(pods, checkpoints);
            }
            result[0] = pods[0].score(checkpoints[pods[0].nextCheckpointId]);
            result[1] = pods[1].score(checkpoints[pods[1].nextCheckpointId]);
        };
};

void makeSolution(Pod pods[2], Checkpoint *checkpoints, Solution previousSol[MAX_POP]) {
    auto start = std::chrono::steady_clock::now();
    Solution solutions[MAX_POP];
    Solution finalMove;
    Solution nextGen[MAX_POP];

    float currentScore[2] = {0, 0};
    for (int i = 0; i < MAX_POP; i++) {
        for (int j = 0; j < MAX_TURN; j++) {
            solutions[i].moves1[j] = previousSol[i].moves1[j];
            solutions[i].moves2[j] = previousSol[i].moves2[j];
        }
    }

    float minScore[2] = {-100000, -100000};
    float amplitude = 1.0;
    int iMove1 = 0;
    int iMove2 = 0;
    int randomIndex = 0;
    auto end = std::chrono::steady_clock::now();

    while (chrono::duration_cast<chrono::milliseconds>(end - start).count() < 75) {
      int tmpRand = randomIndex;
        do {
            randomIndex = rand() % MAX_POP;
        } while (tmpRand == randomIndex);
        Solution solution = solutions[randomIndex];
        solution.score(checkpoints, pods, currentScore);

        if (currentScore[0] > minScore[0] || currentScore[1] > minScore[1]) {
                if (currentScore[0] >= minScore[0]) {
                    finalMove.moves1[0] = solution.moves1[0];
                    minScore[0] = currentScore[0];
                    if (iMove1 >= MAX_POP)
                        iMove1 = MAX_POP - 1;
                    for (int y = 0; y < MAX_TURN; y++) {
                        nextGen[iMove1].moves1[y] = solution.moves1[y];
                    }
                    iMove1++;
                }
                if (currentScore[1] >= minScore[1]) {
                    finalMove.moves2[0] = solution.moves2[0];
                    minScore[1] = currentScore[1];
                    if (iMove2 >= MAX_POP)
                        iMove2 = MAX_POP - 1;
                    for (int y = 0; y < MAX_TURN; y++) {
                        nextGen[iMove2].moves2[iMove2] = solution.moves2[y];
                    }
                    iMove2++;
                }
        } else {
            if (rand() % 3 == 0) {
                for (int y = 0; y < MAX_TURN; y++) {
                    if (iMove1 < MAX_POP)
                        nextGen[iMove1].moves1[y] = solution.moves1[y];
                    if (iMove2 < MAX_POP)
                        nextGen[iMove2].moves2[y] = solution.moves2[y];
                }
                iMove1++;
                iMove2++;
            }
        }
        if (iMove1 >= MAX_POP && iMove2 >= MAX_POP) {
            if (amplitude > 0.1) {
                amplitude -= 0.01;
            } else {
                amplitude = 1.0;
            }
            iMove1 = 0;
            iMove2 = 0;
            for (int sol_index = 0; sol_index < MAX_POP; sol_index++) {
                for (int y = 0; y < MAX_TURN; y++) {
                    if (rand() % 3 == 0) {
                        nextGen[sol_index].moves1[y].mutate(amplitude);
                        nextGen[sol_index].moves2[y].mutate(amplitude);
                    }
                    solutions[sol_index].moves1[y] = nextGen[sol_index].moves1[y];
                    solutions[sol_index].moves2[y] = nextGen[sol_index].moves2[y];
               }
            }
        }
        end = std::chrono::steady_clock::now();
    }

    pods[0].output(finalMove.moves1[0]);
    pods[1].output(finalMove.moves2[0]);
    for (int i = 0; i < MAX_POP; i++) {
        for (int j = 0; j < MAX_TURN; j++) {
            previousSol[i].moves1[j] = solutions[i].moves1[j];
            previousSol[i].moves2[j] = solutions[i].moves2[j];
        }
    }
}

int main()
{
    int laps;
    int checkpointsLength;
    Pod pods[2];
    Pod opponnent[2];
    int currentCheckpointIndex = 0;
    Solution solutions[MAX_POP];


    cin >> laps; cin.ignore();
    cin >> checkpointsLength; cin.ignore();
    pods[0].checkpointsCount = checkpointsLength;
    pods[1].checkpointsCount = checkpointsLength;
    Checkpoint checkpoints[checkpointsLength];

    for (int i = 0; i < checkpointsLength; i++) {
        cin >> checkpoints[i].x >> checkpoints[i].y; cin.ignore();
    }
    for (int i = 0; i < MAX_POP; i++) {
        for (int j = 0; j < MAX_TURN; j++) {
            solutions[i].moves1[j] = Move(rand() % 37 - 18, 100);
            solutions[i].moves2[j] = Move(rand() % 37 - 18, 100);
        }
    }

    // game loop
    while (1) {
        cin >> pods[0].x >> pods[0].y >> pods[0].vx >> pods[0].vy >> pods[0].angle >> pods[0].nextCheckpointId; cin.ignore();
        cin >> pods[1].x >> pods[1].y >> pods[1].vx >> pods[1].vy >> pods[1].angle >> pods[1].nextCheckpointId; cin.ignore();

        cin >> opponnent[0].x >> opponnent[0].y >> opponnent[0].vx >> opponnent[0].vy >> opponnent[0].angle >> opponnent[0].nextCheckpointId; cin.ignore();
        cin >> opponnent[1].x >> opponnent[1].y >> opponnent[1].vx >> opponnent[1].vy >> opponnent[1].angle >> opponnent[1].nextCheckpointId; cin.ignore();
        
        if (pods[0].angle == - 1) {
            pods[0].angle = pods[0].getAngle(checkpoints[1]);
            pods[1].angle = pods[1].getAngle(checkpoints[1]);
            pods[0].vx = pods[0].x - checkpoints[1].x;
            pods[0].vy = pods[0].y - checkpoints[1].y;
            cout << checkpoints[1].x << " " << checkpoints[1].y << " BOOST" << endl;
            cout << checkpoints[1].x << " " << checkpoints[1].y << " " << 100 << endl;
        } else {
            makeSolution(pods, checkpoints, solutions);
            Move tmp = Move(rand() % 37 - 18, 100);
            for (int i = 0; i < MAX_POP; i++) {
                for (int j = MAX_TURN - 1; j >= 0; j--) {
                    Move otherTmp = solutions[i].moves1[j];
                    solutions[i].moves1[j] = tmp;
                    tmp = otherTmp;                
                }
            }     
        }
    }
}