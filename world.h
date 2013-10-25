#ifndef WORLD_H
#define WORLD_H

#include <string>
#include <vector>

#include "tile.h"
#include "enemy.h"

using namespace std;

class World {
    vector<Tile*> tiles;
    vector<Enemy*> enemies;
    int score;

public:
    World() { }

    //is the method that adds score when an enemy is killed
    void enemyDeath();

    // allows the user to buy a tower and returns a bool to determine if succeessfull
    bool towerBuy(int);

    int getScore() { return score;}

    // Creates and returns a tile based on the <type>
    Tile *createObject(const string& type);

    // adds <obj> to tiles in world
    void add(Tile*);

    // returns tile with <id>, or NULL if none
    Tile *getById(int id);

    // returns the vector of tiles
    vector<Tile*>& getTiles() { return tiles; }
    vector<Enemy*>& getEnemies() { return enemies; }

    // Resets world
    void reset();

    // Destroy objects
    ~World();

    /***** singleton implementation ****/
private:
    // the main instance
    static World instance;

public:
    // returns the instance
    static World& getInstance() {
        return instance;
    }

};

#endif // WORLD_H
