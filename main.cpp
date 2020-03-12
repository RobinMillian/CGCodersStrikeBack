#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <array>
#include <chrono>


using namespace std;

#define MAX_POP 10000

static int frame = 0;
auto start = std::chrono::steady_clock::now();

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
        bool shield = false;
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
        shield = a.shield;
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
    float _x = x - u.x;
    float _y = y - u.y;
    Point myp = Point(_x, _y);
    float _vx = vx - u.vx;
    float _vy = vy - u.vy;
    Point up(0, 0);

    // On cherche le point le plus proche de u (qui est donc en (0,0)) sur la droite décrite par notre vecteur de vitesse
/*     cerr << x << endl;
    cerr << y << endl;
    cerr << vx << endl;
    cerr << vy << endl;
 */    Point p = up.closest(myp, Point(_x + _vx, _y + _vy));

    // dist au carré entre u et le point le plus proche sur la droite décrite par notre vecteur de vitesse
    float pdist = up.distSquare(p);

    // dist au carré entre nous et ce point
    float mypdist = myp.distSquare(p);

    // Si la dist entre u et cette droite est inférieur à la somme des rayons, alors il y a possibilité de collision
    if (pdist < sr) {
        // Notre vitesse sur la droite
        float length = sqrt(_vx*_vx + _vy*_vy);

        // On déplace le point sur la droite pour trouver le point d'impact
        float backdist = sqrt(sr - pdist);
        p.x = p.x - backdist * (_vx / length);
        p.y = p.y - backdist * (_vy / length);

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

void Unit::bounce(Unit &u) {
     // Si un pod a son bouclier d'activé, sa masse est de 10, sinon elle est de 1
        float m1 = shield ? 10 : 1;
        float m2 = u.shield ? 10 : 1;

        // Si les masses sont égales, le coefficient sera de 2. Sinon il sera de 11/10
        float mcoeff = (m1 + m2) / (m1 * m2);

        float nx = x - u.x;
        float ny = y - u.y;

        // Distance au carré entre les 2 pods. Cette valeur pourrait être écrite en dure car ce sera toujours 800²
        float nxnysquare = nx*nx + ny*ny;

        float dvx = vx - u.vx;
        float dvy = vy - u.vy;

        // fx et fy sont les composantes du vecteur d'impact. product est juste la pour optimiser
        float product = nx*dvx + ny*dvy;
        float fx = (nx * product) / (nxnysquare * mcoeff);
        float fy = (ny * product) / (nxnysquare * mcoeff);

        // On applique une fois le vecteur d'impact à chaque pod proportionnellement à sa masse
        vx -= fx / m1;
        vy -= fy / m1;
        u.vx += fx / m2;
        u.vy += fy / m2;

        // Si la norme du vecteur d'impact est inférieur à 120, on change sa norme pour le mettre à 120
        float impulse = sqrt(fx*fx + fy*fy);
        if (impulse < 120.0) {
            fx = fx * 120.0 / impulse;
            fy = fy * 120.0 / impulse;
        }

        // On applique une deuxième fois le vecteur d'impact à chaque pod proportionnellement à sa masse
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
//                cerr << "toto " << endl;
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
            checked = p.checked;
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
            if (timeout <= 0)
                return -9000000;
                
            float cvx = checkpoint.x - x;
            float cvy = checkpoint.y - y;
            float a = (cvx * vx + cvy * vy) / (dist(checkpoint) * sqrt((vx * vx) + (vy * vy)));
            a = acos(a);
            
/*            float a = getAngle(checkpoint);
            float right = angle <= a ? a - angle : 360.0 - angle + a;
            float left = angle >= a ? angle - a : angle + 360.0 - a;
            if (frame == 99) {
                cerr << "diff angle == " << a << "x = " << x << "y = " << y << " vx = "<< vx << " vy = " << vy <<endl;
                cerr << "ternaire == " << (a == 0.0 ? 1 : a) << endl;
            }*/
            return   (checked * 50000) / (a == 0.0 ? 1 : a) - (dist(checkpoint) * 0.01) /*- ((diffAngle(checkpoint) > 0 ? -diffAngle(checkpoint) : diffAngle(checkpoint)) * 0.01)*/;
        };

        void output(Move &move) {
            float a = angle + move.angle;

            if (a >= 360.0) {
                cerr << "supp at 360 " << endl;
                a = a - 360.0;
            } else if (a < 0.0) {
                cerr << "inf at 0.0 " << endl;
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
        Move moves1[6];
        Move moves2[6];
        void score(Checkpoint *checkpoints, Pod currentPods[2], float result[2]) {
            Pod pods[2];
            pods[0] = currentPods[0];
            pods[1] = currentPods[1];
            for (int i = 0; i < 6; i++) {
                pods[0] = moves1[i];
                pods[1] = moves2[i];
                play(pods, checkpoints);
            }
            result[0] = pods[0].score(checkpoints[pods[0].nextCheckpointId]);
            result[1] = pods[1].score(checkpoints[pods[1].nextCheckpointId]);
        };
};

void makeSolution(Pod pods[2], Checkpoint *checkpoints, Solution previousSol[MAX_POP]) {
    start = std::chrono::steady_clock::now();
    frame++;
    cerr << "frame == " << frame << endl;
    Solution solutions[MAX_POP];
    Solution finalMove;
    float currentScore[2] = {0, 0};
    for (int i = 0; i < MAX_POP; i++) {
        for (int j = 0; j < 6; j++) {
            solutions[i].moves1[j] = previousSol[i].moves1[j];
            solutions[i].moves2[j] = previousSol[i].moves2[j];
        }
    }
    float minScore[2] = {-100000, -100000};
    float amplitude = 1.0;
    Solution nextGen[MAX_POP];
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
                    for (int y = 0; y < 6; y++) {
                        nextGen[iMove1].moves1[y] = solution.moves1[y];
                    }
                    iMove1++;
                }
                if (currentScore[1] >= minScore[1]) {
                    finalMove.moves2[0] = solution.moves2[0];
                    minScore[1] = currentScore[1];
                    if (iMove2 >= MAX_POP)
                        iMove2 = MAX_POP - 1;
                    for (int y = 0; y < 6; y++) {
                        nextGen[iMove2].moves2[iMove2] = solution.moves2[y];
                    }
                    iMove2++;
                }
        } else {
            if (rand() % 3 == 0) {
                for (int y = 0; y < 6; y++) {
                    if (iMove1 < MAX_POP)
                        nextGen[iMove1].moves1[y] = solution.moves1[y];
                    if (iMove2 < MAX_POP)
                        nextGen[iMove2].moves2[y] = solution.moves2[y];
                }
                iMove1++;
                iMove2++;
            }
//            minScore[0] = currentScore[0];
 //           minScore[1] = currentScore[1];
        }
        if (iMove1 >= MAX_POP && iMove2 >= MAX_POP) {
           // cerr << minScore[0] << endl;
        //    cerr << minScore[1] << endl;
            if (amplitude > 0.1) {
                amplitude -= 0.01;
            } else {
                amplitude = 1.0;
            }
            iMove1 = 0;
            iMove2 = 0;
            for (int sol_index = 0; sol_index < MAX_POP; sol_index++) {
                for (int y = 0; y < 6; y++) {
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
        for (int j = 0; j < 6; j++) {
            previousSol[i].moves1[j] = solutions[i].moves1[j];
            previousSol[i].moves2[j] = solutions[i].moves2[j];
        }
    }
}/*
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
        cerr << "Checkpoints == " << checkpoints[i].x << " " << checkpoints[i].y << endl;;
    }
    for (int i = 0; i < MAX_POP; i++) {
        for (int j = 0; j < 6; j++) {
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
            cout << checkpoints[1].x << " " << checkpoints[1].y << " " << 100 << endl;
            cout << checkpoints[1].x << " " << checkpoints[1].y << " " << 100 << endl;
        } else {
            makeSolution(pods, checkpoints, solutions);
            Move tmp = Move(rand() % 37 - 18, 100);
            for (int i = 0; i < MAX_POP; i++) {
                for (int j = 5; j >= 0; j--) {
                    Move otherTmp = solutions[i].moves1[j];
                    solutions[i].moves1[j] = tmp;
                    tmp = otherTmp;                
                }
            }     
        }
        cerr << pods[0].x << " y= " << pods[0].y << " vx = " << pods[0].vx << " vy = " << pods[0].vy << " angle = " <<  pods[0].angle << " nextcheckpointID = " << pods[0].nextCheckpointId << endl;
        cerr << pods[1].x << " y= " << pods[1].y << " vx = " << pods[1].vx << " vy = " << pods[1].vy << " angle = " <<  pods[1].angle << " nextcheckpointID = " << pods[1].nextCheckpointId << endl;

        
        cerr << "checkpointsLength == " << pods[0].checkpointsCount << endl;
        cerr << "checkpointsLength == " << pods[1].checkpointsCount << endl;
    //    cerr << "PODS x == " << pods[0].x << " y = " << pods[0].y << endl;
       // cerr << "PODS2 x == " << pods[1].x << " y = " << pods[1].y << endl;
       
        // Write an action using cout. DON'T FORGET THE "<< endl"
        // To debug: cerr << "Debug messages..." << endl;


    }
}*/
/*int main()
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
        cerr << "Checkpoints == " << checkpoints[i].x << " " << checkpoints[i].y << endl;;
    }
    for (int i = 0; i < MAX_POP; i++) {
        for (int j = 0; j < 6; j++) {
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
            cout << checkpoints[1].x << " " << checkpoints[1].y << " " << 100 << endl;
            cout << checkpoints[1].x << " " << checkpoints[1].y << " " << 100 << endl;
        } else {
            makeSolution(pods, checkpoints, solutions);
            Move tmp = Move(rand() % 37 - 18, 100);
            for (int i = 0; i < MAX_POP; i++) {
                for (int j = MAX_POP; j >= 0; j--) {
                    Move otherTmp = solutions[i].moves1[j];
                    solutions[i].moves1[j] = tmp;
                    tmp = otherTmp;                
                }
            }     
        }
        cerr << pods[0].x << " y= " << pods[0].y << " vx = " << pods[0].vx << " vy = " << pods[0].vy << " angle = " <<  pods[0].angle << " nextcheckpointID = " << pods[0].nextCheckpointId << endl;
        cerr << pods[1].x << " y= " << pods[1].y << " vx = " << pods[1].vx << " vy = " << pods[1].vy << " angle = " <<  pods[1].angle << " nextcheckpointID = " << pods[1].nextCheckpointId << endl;

        
        cerr << "checkpointsLength == " << pods[0].checkpointsCount << endl;
        cerr << "checkpointsLength == " << pods[1].checkpointsCount << endl;
    //    cerr << "PODS x == " << pods[0].x << " y = " << pods[0].y << endl;
       // cerr << "PODS2 x == " << pods[1].x << " y = " << pods[1].y << endl;
       
        // Write an action using cout. DON'T FORGET THE "<< endl"
        // To debug: cerr << "Debug messages..." << endl;


    }
}*/
int main()
{
    int laps;
    int checkpointsLength;
    Pod pods[2];
    Pod opponnent[2];
    int currentCheckpointIndex = 0;
    Solution solutions[MAX_POP];

    for (int i = 0; i < MAX_POP; i++) {
        for (int j = 0; j < 6; j++) {
            solutions[i].moves1[j] = Move(rand() % 36 - 18 + 1, rand() % 100 + 1 >= 20 ? 100 : 0);
            solutions[i].moves2[j] = Move(rand() % 36 - 18 + 1, rand() % 100 + 1>= 20 ? 100 : 0);
        }
    }

/*    cin >> laps; cin.ignore();
    cin >> checkpointsLength; cin.ignore();*/
    laps = 3;
    checkpointsLength = 2;
    Checkpoint checkpoints[checkpointsLength];
    
 /*   for (int i = 0; i < checkpointsLength; i++) {
        int x;
        int y;
        cin >> checkpoints[i].x >> checkpoints[i].y; cin.ignore();
    } */
    checkpoints[0].x = 13066;
    checkpoints[0].y = 1925;
    checkpoints[1].x = 6573;
    checkpoints[1].y = 7821;

    pods[0].x = 12111; pods[0].y = 3643; pods[0].vx = 101; pods[0].vy = 158; pods[0].angle = 75; pods[0].nextCheckpointId = 1;
    pods[1].x = 11441; pods[1].y = 2896; pods[1].vx = 125; pods[1].vy = 141; pods[1].angle = 63; pods[0].nextCheckpointId = 1;
    pods[0].checkpointsCount = checkpointsLength;
    pods[1].checkpointsCount = checkpointsLength;
    // game loop
    while (1) {
        cout << "toto" << endl;
/*        cin >> pods[0].x >> pods[0].y >> pods[0].vx >> pods[0].vy >> pods[0].angle >> pods[0].nextCheckpointId; cin.ignore();
        cin >> pods[1].x >> pods[1].y >> pods[1].vx >> pods[1].vy >> pods[1].angle >> pods[1].nextCheckpointId; cin.ignore();
 
        cin >> opponnent[0].x >> opponnent[0].y >> opponnent[0].vx >> opponnent[0].vy >> opponnent[0].angle >> opponnent[0].nextCheckpointId; cin.ignore();
        cin >> opponnent[1].x >> opponnent[1].y >> opponnent[1].vx >> opponnent[1].vy >> opponnent[1].angle >> opponnent[1].nextCheckpointId; cin.ignore();*/

        makeSolution(pods, checkpoints, solutions);
        // Write an action using cout. DON'T FORGET THE "<< endl"
        // To debug: cerr << "Debug messages..." << endl;


    }
}