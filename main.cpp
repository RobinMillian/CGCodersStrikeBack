#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>

using namespace std;


class Point {
    public:
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

class Unit : public Point {
    public:
        int id;
        float r;
        float vx;
        float vy;
        Unit();
        Unit &operator=(const Unit &a) {
            id = a.id;
            r = a.r;
            vx = a.vx;
            vy = a.vy;
            return *this;
        }
        Coll *collision(const Unit &u) {
            // dist carré
            float dist = distSquare(u);

            // Somme des rayons au carré
            float sr = (r + u.r)*(r + u.r);

            // On prend tout au carré pour éviter d'avoir à appeler un sqrt inutilement. C'est mieux pour les performances

            if (dist < sr) {
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
        };
        void bounce(const Checkpoint &) {

        };
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

class Move {
    public:
        float angle;
        int thurst;
        void mutate(float amplitude) {

        };
};

class Solution {
    public:
        Move moves1[];
        Move moves2[];
        void randomize() {

        };
};

class Checkpoint: public Unit {
    public: 
        void bounce(const Pod &pod) {

        };
};

class Pod: public Unit {
    public:
        float angle;
        int checkpointsCount;
        int nextCheckpointId;
        int checked;
        int timeout;
        int score = 0;
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

/**
 * This code automatically collects game data in an infinite loop.
 * It uses the standard input to place data into the game variables such as x and y.
 * YOU DO NOT NEED TO MODIFY THE INITIALIZATION OF THE GAME VARIABLES.
 **/

void play(Pod pods[], Checkpoint checkpoints[]) {
    float t = 0.0;

    while (t < 1.0) {
        Coll *collision = nullptr;
        for (int i = 0; i < sizeof(pods) / sizeof(*pods); i++) {
            collision = pods[i].collision(checkpoints[pods[i].nextCheckpointId]);
        }
        if (collision == nullptr) {
            for (int i = 0; i < sizeof(pods) / sizeof(*pods); i++) {
                pods[i].move(1.0 - t);
            }
            t = 1.0;
        } else {
            for (int i = 0; sizeof(pods) / sizeof(*pods); i++) {
                pods[i].move(collision->t);
            }
            static_cast<Pod &>(collision->a).bounceWithCheckpoint(static_cast<Checkpoint &>(collision->b));
            t += collision->t;
        }
    }
    for (int i = 0; i < sizeof(pods) / sizeof(*pods); i++) {
        pods[i].end();
    }
}

int main() 
{
 
}