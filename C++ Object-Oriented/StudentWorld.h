#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "GameConstants.h"
#include "Actor.h"
#include <string>
#include <list>

// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp

class StudentWorld : public GameWorld {
public:
    // constructor
    StudentWorld(std::string assetDir);

    // virtual destructor
    virtual ~StudentWorld();

    // runs at start of game
    virtual int init();

    // controls actions of all the actors per tick
    virtual int move();

    // destructs objects when game ends
    virtual void cleanUp();

    // returns a pointer to player
    TunnelMan* getPlayer();

    // returns value of hash table at [x][y]
    int getPixelArrID(int x, int y);

    // changes value of hash table at [x][y]
    void changePixelArrID(int x, int y, int ID);

    // sets Earth invisible at (x, y)
    void setEarthInvis(int x, int y);

    // returns the list of obj in game
    std::list<obj*>& getActors();

    // returns true if there is dirt in this location
    bool dirtHere(int x, int y);

    // calculate the distance between (x1, y1) and (x2, y2)
    double calcDist(int x1, int y1, int x2, int y2);

    // generates a random number from min to max, for coordinate generation
    int RNG(int min, int max);

private:
    // struct for hash table
    struct pixel {
        // constructor
        pixel(int x, int y, StudentWorld* world, int TID)
        {
            dirt = new Earth(x, y, world);
            ID = TID_EARTH;
        }

        // destructor
        ~pixel()
        {
            delete dirt;
        }

        // change ID
        void changeID(int TID)
        {
            ID = TID;
        }

        obj* dirt; // contains Earth
        int ID; // contains TID_EARTH, TID_BOULDER, or -1
    };

    std::list<obj*> actors; // containers all obj except Earth
    pixel* pixelArr[64][60]; // hash table for Earth and Boulders, and also contains all Earth
    TunnelMan* player; // pointer to the player
    int goodSpawn; // chance of goods spawning every tick
    int protesterCount; // keeps track of number of protesters on field
    int protesterCountdown; // keeps track of ticks before generating a new protester

    // returns true if (x, y) has a distributable good within 6 units
    bool distributionCollision(int x, int y);

    // updates game text at the beginning of every tick
    void updateText();

    // formats txt
    void addLeading(std::string& txt, int numSpaces, std::string add);
};

#endif // STUDENTWORLD_H_
