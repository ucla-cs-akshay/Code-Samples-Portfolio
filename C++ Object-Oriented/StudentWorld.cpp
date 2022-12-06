#include "StudentWorld.h"
#include <math.h>
using namespace std;

// creates and returns pointer to new StudentWorld
GameWorld* createStudentWorld(string assetDir)
{
    return new StudentWorld(assetDir); // reutn
}

// constructor, leave blank
StudentWorld::StudentWorld(std::string assetDir)
    : GameWorld(assetDir)
{
}

// destructor, will not be called
// prof says must be empty on piazza
StudentWorld::~StudentWorld()
{
}

// called at start of program, but not allowed to call myself
int StudentWorld::init()
{
    srand(time(NULL)); // for use in randomly generating positions

    goodSpawn = getLevel() * 25 + 300; // 1 in goodSpawn chance of a good spawning

    protesterCount = 0; // record that there are 0 protesters on the field
    protesterCountdown = 0; // generate a new protester on the next (first) tick of the game

    // number of oil barrels to be collected and to be generated
    int L = (2 + getLevel() < 21) ? 2 + getLevel() : 21;

    player = new TunnelMan(this, L); // pointer to new TunnelMan object

    actors.push_back(player); // places player in container of actors

    // fills the screen with Earth and places the Earth in the container of actors
    // also sets the hash table to contain Earth at each location
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 60; j++) {
            pixelArr[i][j] = new pixel(i, j, this, TID_EARTH);

            // leave a center channel empty by clearing the hash table in this location
            if (30 <= i && i <= 33 && j > 3) {
                setEarthInvis(i, j);
            }
        }
    }

    // distribute L barrels randomly across the field
    for (int i = 0; i < L; i++) {
        // randomly generate coordinates
        int x = RNG(0, 60);
        int y = RNG(0, 56);

        // if the coordinates are too close to another object or if they are in the main shaft, repeatedly generate again
        while (distributionCollision(x, y) || (26 <= x && x <= 34 && y > 3)) {
            x = RNG(0, 60);
            y = RNG(0, 56);
        }

        // create a new Barrel object with the generated coordinates and add it to the list of objects
        Barrel* temp = new Barrel(x, y, this);
        actors.push_back(temp);
    }

    // number of gold objects to spawn at beginning
    int G = (5 - getLevel() / 2 > 2) ? 5 - getLevel() / 2 : 2;

    // distribute G gold nuggets randomly across the field
    for (int i = 0; i < G; i++) {
        // randomly generate coordinates
        int x = RNG(0, 60);
        int y = RNG(0, 56);

        // if the coordinates are too close to another object or if they are in the main shaft, repeatedly generate again
        while (distributionCollision(x, y) || (26 <= x && x <= 34 && y > 3)) {
            x = RNG(0, 60);
            y = RNG(0, 56);
        }

        // create a new GoldNugget object with the generated coordinates and add it to the list of objects
        GoldNugget* temp = new GoldNugget(x, y, 1, false, this);
        actors.push_back(temp);
    }

    // number of Boulder objects to spawn at beginning
    int B = (getLevel() / 2 + 2 < 9) ? getLevel() / 2 + 2 : 9;

    // distribute B Boulders randomly across the field
    for (int i = 0; i < B; i++) {
        // randomly generate coordinates
        int x = RNG(1, 59);
        int y = RNG(20, 55);

        // if the coordinates are too close to another object or if they are in the main shaft, repeatedly generate again
        while (distributionCollision(x, y) || (26 <= x && x <= 34 && y > 3)) {
            x = RNG(1, 59);
            y = RNG(20, 55);
        }

        // create a new Boulder object with the generated coordinates and add it to the list of objects
        Boulder* temp = new Boulder(x, y, this);
        actors.push_back(temp);

        // clear the hash table at the coordinates that this Boulder occupies
        // then mark it as containing a Boulder
        for (int i = x; i < x + SPRITE_WIDTH; i++) {
            for (int j = y; j < y + SPRITE_HEIGHT; j++) {
                setEarthInvis(i, j);
                changePixelArrID(i, j, TID_BOULDER);
            }
        }
    }

    return GWSTATUS_CONTINUE_GAME; // continues game
}

// controls actions of actors
int StudentWorld::move()
{
    updateText(); // updates the text at the beginning of the game

    // iterate through all objs in list
    std::list<obj*>::iterator it = actors.begin();
    while (it != actors.end()) {
        (**it).doSomething(); // tell the obj to do something

        // if the player is dead, immediately end the game
        if (!player->getStatus()) // immediately end game if hp <= 0
        {
            decLives(); // decrement player lives

            playSound(SOUND_PLAYER_GIVE_UP); // play the death sound

            return GWSTATUS_PLAYER_DIED; // end game
        }

        // if the player has collected all barrels, move onto the next stage
        if (player->getBarrels() <= 0) {
            playSound(SOUND_FINISHED_LEVEL); // play finish level sound

            return GWSTATUS_FINISHED_LEVEL; // continue to next level
        }

        // if obj is dead, delete it from the list
        if (!((*it)->getStatus())) {
            // if a protester is about to be deleted, decrement the count of protesters
            if ((*it)->getID() == TID_PROTESTER || (*it)->getID() == TID_HARD_CORE_PROTESTER)
                protesterCount--;

            delete *it;
            it = actors.erase(it);
            continue;
        }

        it++; // move onto the next item in the list
    }

    // if there are't the max number of protesters on the field and if enough ticks have passed to add a new protester
    if (protesterCount < int(15 < 2 + getLevel() * 1.5 ? 15 : 2 + getLevel() * 1.5) && protesterCountdown <= 0) {
        // there is a probOfHard% chance that a hard protester will spawn. else, a regular protester will spawn
        int probOfHardProt = (90 < getLevel() * 10 + 30 ? 90 : getLevel() * 10 + 30);

        // simulates a probOfHardProt% draw to determine what type of protester is drawn
        // if the number generated is <= probOfHardProt, a hard protester will be added
        if (RNG(1, 100) <= probOfHardProt) {
            obj* temp = new HardProtester(this);
            actors.push_back(temp);
        }
        // else, a regular protester will be added
        else {
            obj* temp = new RegularProtester(this);
            actors.push_back(temp);
        }

        protesterCount++; // since a protester was just added, increment the count of protesters by 1

        protesterCountdown = (25 > 200 - getLevel() ? 25 : 200 - getLevel()); /// reset protester countdown
    }
    else
        protesterCountdown--; // a new protester was not added, so decrement the protester countdown

    // siulates a 1 / goodSpawn change of generating a sonar/waterpool
    if (RNG(1, goodSpawn) == 1) {
        // simulates a 1/5 chance of the good being a sonarcharge
        if (RNG(1, 5) == 1) {
            // if there is not a sonar on the map and the sonar does not collide with anything
            if (!distributionCollision(0, 60)) {
                // create a new sonar object and add it to the list of obj
                Sonar* temp = new Sonar(0, 60, this);
                actors.push_back(temp);
            }
        }

        // else the good must be a waterpool
        else {
            // randomly generate coordinates
            int x = RNG(0, 60);
            int y = RNG(0, 60);

            // if the coordinates overlap with dirt or if they are too close to another object, repeatedledly regenerate
            while (distributionCollision(x, y) || dirtHere(x, y)) {
                x = RNG(0, 60);
                y = RNG(0, 60);
            }

            // create a new waterpool at (x, y) and add it to the list of objects
            WaterPool* temp = new WaterPool(x, y, this);
            actors.push_back(temp);
        }
    }

    return GWSTATUS_CONTINUE_GAME; // continue the game
}

// destructs objects when game ends
void StudentWorld::cleanUp()
{
    // deletes all objects in the cnotainer of actors
    // unnecessary to individually delete player since it is in actors
    std::list<obj*>::iterator it = actors.begin();
    while (it != actors.end()) {
        delete *it;
        it = actors.erase(it);
    }

    // deletes everything in the hash table
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 60; j++) {
            delete pixelArr[i][j];
        }
    }
}

// returns a pointer to player
TunnelMan* StudentWorld::getPlayer()
{
    return player;
}

// returns the ID that is stored at (x, y) in the hash table
// returns -1 if out of bounds
int StudentWorld::getPixelArrID(int x, int y)
{
    // if empty Earth at top of game, return -1 for no Earth/Boulder
    if (x < 64 && x >= 0 && y >= 60 && y < 64)
        return -1;

    // if out of bounds of array, return OUT_OF_BOUNDS
    if (x >= 64 || y >= 60 || x < 0 || y < 0)
        return OUT_OF_BOUNDS;

    return pixelArr[x][y]->ID; // else return the ID at this location
}

// update the ID of the hashtable at (x, y)
void StudentWorld::changePixelArrID(int x, int y, int ID)
{
    pixelArr[x][y]->changeID(ID); // change the ID
}

// set the Earth to invisible at the location(x, y) on the hash table
void StudentWorld::setEarthInvis(int x, int y)
{
    pixelArr[x][y]->dirt->setVisible(false); // make the Earth invisible

    pixelArr[x][y]->changeID(-1); // clear the value on the hash table
}

// return the list of obj
std::list<obj*>& StudentWorld::getActors()
{
    return actors;
}

// checks if there is dirt overlapping with the sprite
bool StudentWorld::dirtHere(int x, int y)
{
    // for each pixel in the sprite
    for (int i = x; i < x + SPRITE_WIDTH; i++)
        for (int j = y; j < y + SPRITE_HEIGHT; j++)
            // if the hash table corresponding to the pixel has Earth
            if (getPixelArrID(i, j) == TID_EARTH || getPixelArrID(i, j) == TID_BOULDER)
                return true; // return true

    return false; // else there is no overlap, so return false
}

// calculate the Euclidean distance between (x1, y1) and (x2, y2)
double StudentWorld::calcDist(int x1, int y1, int x2, int y2)
{
    return sqrt(pow(1.0 * x1 - x2, 2) + pow(1.0 * y1 - y2, 2)); // the formula
}

// generates a random number from min to max, inclusive
int StudentWorld::RNG(int min, int max)
{
    return (rand() % (max - min + 1)) + min;
}

// check if (x, y) is within six units of something else
bool StudentWorld::distributionCollision(int x, int y)
{
    // iterate through the objs in the list
    std::list<obj*>::iterator it = actors.begin();
    while (it != actors.end()) {
        int ID = (*it)->getID(); // get the ID of the obj

        // if the ID is one of the ones lists
        if (ID == TID_BOULDER || ID == TID_GOLD || ID == TID_BARREL || ID == TID_SONAR || ID == TID_WATER_POOL
            || ID == TID_PLAYER || ID == TID_PROTESTER || ID == TID_HARD_CORE_PROTESTER) {
            // get the obj's coordinates
            int x1 = (*it)->getX();
            int y1 = (*it)->getY();

            // calculate the distance from (x, y) to the obj's coords
            double dist = calcDist(x, y, x1, y1);

            // if the distance <= 6, return true
            if (dist <= 6.0)
                return true;
        }

        it++; // continue onto the next obj in the list
    }

    return false; // there were no objs six units nearby, so return false
}

// updates text at the top of the game screen
void StudentWorld::updateText()
{
    // gets and formats level
    std::string level = std::to_string(getLevel());
    addLeading(level, 2, " ");

    // gets lives (is always one digit long)
    std::string lives = std::to_string(getLives());

    // gets and formats health
    std::string health = std::to_string(player->getHitPoints() * 10);
    addLeading(health, 3, " ");

    // gets and formats water
    std::string water = std::to_string(player->getSquirts());
    addLeading(water, 2, " ");

    // gets and formats gold
    std::string gold = std::to_string(player->getNuggets());
    addLeading(gold, 2, " ");

    // gets and formats oil
    std::string oil = std::to_string(player->getBarrels());
    addLeading(oil, 2, " ");

    // gets and formats sonar
    std::string sonar = std::to_string(player->getSonar());
    addLeading(sonar, 2, " ");

    // gets and formats score
    std::string score = std::to_string(getScore());
    addLeading(score, 6, "0");

    // append the previous strings together
    string status = "Scr: " + score + "  Lvl: " + level + "  Lives: " + lives + "  Hlth: " + health + "%  Wtr: " + water + "  Gld: " + gold + "  Sonar: " + sonar + "  Oil Left: " + oil;

    setGameStatText(status); // set the game text to the generated string
}

// adds add to the beginning of txt until txt is length characters long
void StudentWorld::addLeading(string& txt, int length, string add)
{
    // repeatedly add add to the beginning of txt until txt is >= length
    while (txt.length() < length)
        txt = add + txt;
}
