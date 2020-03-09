#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <array>
#include <chrono>
#include <ctime>


using namespace std;


class Point {
    public:
        Point() {};
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
        Unit() {};
        int id;
        float r;
        float vx;
        float vy;
        Unit &operator=(const Unit &a);
        Coll *collision(const Unit &u);
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
    // dist carré
    float dist = distSquare(u);

    // Somme des rayons au carré
    float sr = (r + u.r)*(r + u.r);

    // On prend tout au carré pour éviter d'avoir à appeler un sqrt inutilement. C'est mieux pour les performances

    if (dist < sr) {
//        cerr << "Objet l'un sur l'autre " << endl;
        // Les objets sont déjà l'un sur l'autre. On a donc une collision immédiate
        return new Coll(*this, u, 0.0);
    }

    // Optimisation. Les objets ont la même vitesse ils ne pourront jamais se rentrer dedans
    if (vx == u.vx && vy == u.vy) {
        return nullptr;
    }

    // On se met dans le référentiel de u. u est donc immobile et se trouve sur le point (0,0) après ça
    float x = x - u.x;
    float y = y - u.y;
    Point myp(x, y);
    float vx = vx - u.vx;
    float vy = vy - u.vy;
    Point up(0, 0);

    // On cherche le point le plus proche de u (qui est donc en (0,0)) sur la droite décrite par notre vecteur de vitesse
    Point p = up.closest(myp, Point(x + vx, y + vy));

    // dist au carré entre u et le point le plus proche sur la droite décrite par notre vecteur de vitesse
    float pdist = up.distSquare(p);

    // dist au carré entre nous et ce point
    float mypdist = myp.distSquare(p);

    // Si la dist entre u et cette droite est inférieur à la somme des rayons, alors il y a possibilité de collision
    if (pdist < sr) {
        // Notre vitesse sur la droite
        float length = sqrt(vx*vx + vy*vy);

        // On déplace le point sur la droite pour trouver le point d'impact
        float backdist = sqrt(sr - pdist);
        p.x = p.x - backdist * (vx / length);
        p.y = p.y - backdist * (vy / length);

        // Si le point s'est éloigné de nous par rapport à avant, c'est que notre vitesse ne va pas dans le bon sens
        if (myp.distSquare(p) > mypdist) {
            return nullptr;
        }

        pdist = p.dist(myp);

        // Le point d'impact est plus loin que ce qu'on peut parcourir en un seul tour
        if (pdist > length) {
            return nullptr;
        }

        // Temps nécessaire pour atteindre le point d'impact
        float t = pdist / length;

        return new Coll(*this, u, t);
    }
    return nullptr;
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

            angle = rand() % int(ramax) + 1 - ramin;
            int pmin = thurst - 200 * amplitude;
            int pmax = thurst + 200 * amplitude;

            if (pmin < 0) {
                pmin = 0;
            }

            if (pmax > 0) {
                pmax = 200;
            }

            thurst = rand() % pmax + 1 - pmin;
        };
};

class Checkpoint: public Unit {
    public:
        Checkpoint() {r=600;};
};

class Pod: public Unit {
    public:
        Pod() { r = 400;};
        float angle;
        int checkpointsCount = 0;
        int nextCheckpointId = 1;
        int checked = 0;
        int timeout = 100;
        Pod *partner;
        bool shield = false;

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
            checkpointsCount = p.checked;
            timeout = p.timeout;
            partner = p.partner;
            shield = p.shield;
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
            if (shield) {
                return;
            }

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
            return checked * 5000 - dist(checkpoint);
        };
        // void bounce() {};

        void output(const Move &move) {
            float a = angle + move.angle;

            if (a >= 360.0) {
                a = a - 360.0;
            } else if (a < 0.0) {
                a += 360.0;
            }

            // On cherche un point pour correspondre à l'angle qu'on veut
            // On multiplie par 10000.0 pour éviter les arrondis
            a = a * 3.14 / 180.0;
            float px = x + cos(a) * 10000.0;
            float py = y + sin(a) * 10000.0;
            cout << round(px) << " " << round(py) << " " << move.thurst << endl;
        }
};

void play(Pod pods[2], Checkpoint checkpoints[]) {
    float t = 0.0;

    while (t < 1.0) {
        Coll *collision = nullptr;
        for (int i = 0; i < 2 ; i++) {
            collision = pods[i].collision(checkpoints[pods[i].nextCheckpointId]);
        }
        if (collision == nullptr) {
            for (int i = 0; i < 2; i++) {
                pods[i].move(1.0 - t);
            }
            t = 1.0;
        } else {

            for (int i = 0; i < 2; i++) {
                pods[i].move(collision->t);
            }
            static_cast<Pod &>(collision->a).bounceWithCheckpoint(static_cast<Checkpoint &>(collision->b));

            t += collision->t;
            if (t <= 0.2)
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
        Move moves1[6];
        Move moves2[6];
        float score(Checkpoint *checkpoints, Pod currentPods[2]) {
            Pod pods[2];
            pods[0] = currentPods[0];
            pods[1] = currentPods[1];
            //cerr << "PODS x == " << pods[0].x << " y = " << pods[0].y << endl;
            //cerr << "PODS2 x == " << pods[1].x << " y = " << pods[1].y << endl;
            for (int i = 0; i < sizeof(moves1) / sizeof(*moves1); i++) {
                pods[0] = moves1[i];
                pods[1] = moves2[i];
                play(pods, checkpoints);
            }
            float result = pods[0].score(checkpoints[pods[0].nextCheckpointId]) + pods[1].score(checkpoints[pods[1].nextCheckpointId]);

            return result;
        };
};


/**
 * This code automatically collects game data in an infinite loop.
 * It uses the standard input to place data into the game variables such as x and y.
 * YOU DO NOT NEED TO MODIFY THE INITIALIZATION OF THE GAME VARIABLES.
 **/

void makeSolution(Pod pods[2], Checkpoint *checkpoints) {
    Solution solutions[5];
    Solution finalMove;
    auto start = std::chrono::system_clock::now();
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = start - end;
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 6; j++) {
            solutions[i].moves1[j] = Move(rand() % 36 - 18 + 1, rand() % 100 + 1 >= 20 ? 100 : 0);
            solutions[i].moves2[j] = Move(rand() % 36 - 18 + 1, rand() % 100 + 1>= 20 ? 100 : 0);
     //       cerr << "Solution[" << i << "]  " << ".moves  [" << j << "] == " << solutions[i].moves1[j].angle << " " << solutions[i].moves1[j].thurst << endl;
       //     cerr << "Solution[" << i << "]  " << ".moves2 [" << j << "] == " << solutions[i].moves2[j].angle << " " << solutions[i].moves2[j].thurst << endl;
        }
    }
    //cerr << "PODS x == " << pods[0].x << " y = " << pods[0].y << endl;
    //cerr << "PODS2 x == " << pods[1].x << " y = " << pods[1].y << endl;
    float currentScore = 0;
    float minScore = -1000000000;
    float amplitude = 1.0;
        cerr << "time == " << elapsed_seconds.count() << endl;
    while (elapsed_seconds.count() <= 1350) {
        Solution solution = solutions[rand() % 5];

        currentScore = solution.score(checkpoints, pods);
        if (currentScore > minScore) {
            finalMove.moves1[0] = solution.moves1[0];
            finalMove.moves2[0] = solution.moves2[0];
          cerr << "currentScore " << currentScore << endl;
            solution.moves1[0].mutate(amplitude);
            solution.moves2[0].mutate(amplitude);
            minScore = currentScore;
            amplitude -= 0.01;
        }

        end = std::chrono::system_clock::now();
        elapsed_seconds += end - start;
    }
    cerr << "END OF TURN" << endl;
    cerr << "pods[0] " << pods[0].nextCheckpointId << endl;
    cerr << "pods[1] " << pods[1].nextCheckpointId << endl;
    pods[0].output(finalMove.moves1[0]);
    pods[1].output(finalMove.moves2[0]);
}

int main()
{
    int laps;
    int checkpointsLength;
    Pod pods[2];
    Pod opponnent[2];
    int currentCheckpointIndex = 0;

    cin >> laps; cin.ignore();
    cin >> checkpointsLength; cin.ignore();
    Checkpoint checkpoints[checkpointsLength];
    for (int i = 0; i < checkpointsLength; i++) {
        int x;
        int y;
        cin >> checkpoints[i].x >> checkpoints[i].y; cin.ignore();
    }
    // game loop
    while (1) {
        cin >> pods[0].x >> pods[0].y >> pods[0].vx >> pods[0].vy >> pods[0].angle >> pods[0].nextCheckpointId; cin.ignore();
        cin >> pods[1].x >> pods[1].y >> pods[1].vx >> pods[1].vy >> pods[1].angle >> pods[1].nextCheckpointId; cin.ignore();
 
        cin >> opponnent[0].x >> opponnent[0].y >> opponnent[0].vx >> opponnent[0].vy >> opponnent[0].angle >> opponnent[0].nextCheckpointId; cin.ignore();
        cin >> opponnent[1].x >> opponnent[1].y >> opponnent[1].vx >> opponnent[1].vy >> opponnent[1].angle >> opponnent[1].nextCheckpointId; cin.ignore();
    //    cerr << "PODS x == " << pods[0].x << " y = " << pods[0].y << endl;
       // cerr << "PODS2 x == " << pods[1].x << " y = " << pods[1].y << endl;
        makeSolution(pods, checkpoints);
        // Write an action using cout. DON'T FORGET THE "<< endl"
        // To debug: cerr << "Debug messages..." << endl;


    }
}