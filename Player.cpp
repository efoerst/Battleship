#include "Player.h"
#include "Board.h"
#include "Game.h"
#include "globals.h"
#include <iostream>
#include <string>
#include <stack>
#include <map>

using namespace std;

//*********************************************************************
//  AwfulPlayer
//*********************************************************************

class AwfulPlayer : public Player
{
  public:
    AwfulPlayer(string nm, const Game& g);
    virtual bool placeShips(Board& b);
    virtual Point recommendAttack();
    virtual void recordAttackResult(Point p, bool validShot, bool shotHit,
                                                bool shipDestroyed, int shipId);
    virtual void recordAttackByOpponent(Point p);
  private:
    Point m_lastCellAttacked;
};

AwfulPlayer::AwfulPlayer(string nm, const Game& g)
 : Player(nm, g), m_lastCellAttacked(0, 0)
{}

bool AwfulPlayer::placeShips(Board& b)
{
      // Clustering ships is bad strategy
    for (int k = 0; k < game().nShips(); k++)
        if ( ! b.placeShip(Point(k,0), k, HORIZONTAL))
            return false;
    return true;
}

Point AwfulPlayer::recommendAttack()
{
    if (m_lastCellAttacked.c > 0)
        m_lastCellAttacked.c--;
    else
    {
        m_lastCellAttacked.c = game().cols() - 1;
        if (m_lastCellAttacked.r > 0)
            m_lastCellAttacked.r--;
        else
            m_lastCellAttacked.r = game().rows() - 1;
    }
    return m_lastCellAttacked;
}

void AwfulPlayer::recordAttackResult(Point /* p */, bool /* validShot */,
                                     bool /* shotHit */, bool /* shipDestroyed */,
                                     int /* shipId */)
{
      // AwfulPlayer completely ignores the result of any attack
}

void AwfulPlayer::recordAttackByOpponent(Point /* p */)
{
      // AwfulPlayer completely ignores what the opponent does
}

//*********************************************************************
//  HumanPlayer
//*********************************************************************

bool getLineWithTwoIntegers(int& r, int& c)
{
    bool result(cin >> r >> c);
    if (!result)
        cin.clear();  // clear error state so can do more input operations
    cin.ignore(10000, '\n');
    return result;
}

// Human Player
class HumanPlayer: public Player
{
public:
    HumanPlayer(string nm, const Game& g): Player(nm, g), m_lastCellAttacked(0,0), m_name(nm){};
    virtual bool isHuman() const {return true;}
    virtual bool placeShips(Board& b);
    virtual Point recommendAttack();
    virtual void recordAttackResult(Point p, bool validShot, bool shotHit,
                                                bool shipDestroyed, int shipId);
    virtual void recordAttackByOpponent(Point p);
private:
    Point m_lastCellAttacked;
    string m_name;
};

bool HumanPlayer::placeShips(Board& b) {
    // Introduce!
    cout << m_name << " must place " << game().nShips() << " ships." << endl;
    b.display(false);
    
    int k = 0;
    for (; k < game().nShips(); k++){
        // If both -- you can't place all the ships
        if (game().shipLength(k) > game().rows() && game().shipLength(k) > game().cols()){
            cout << "This ship cannot fit in the grid.";
            k = game().nShips() + 1;
        }
        
        char ud;
        int ur = -1, uc = -1;
        cout << "Enter h or v for direction of " << game().shipName(k) << " (length " << game().shipLength(k) << "): ";
        cin >> ud;
        Point up(ur,uc);
        
        // If the direction is invalid...
        if (ud != 'h' && ud != 'v'){
            cout << "Direction must be h or v." << endl;
            k--;
            continue;
        }
        // If the length of the ship in said direction will surpass eligable space...
        if (ud == 'h' && game().shipLength(k) >= game().cols()){
            cout << "There is not enough space in this row to fit your ship!" << endl;
            k--;
            continue;
        }
        if (ud == 'v' && game().shipLength(k) > game().rows()){
            cout << "There is not enough space in this column to fit your ship!" << endl;
            k--;
            continue;
        }
        
        // Otherwise it is valid
        else {
            bool first = true;
            // Initializes at horizontal unless ud is 'v'
            Direction ud_f = HORIZONTAL;
            if (ud == 'v')
                ud_f = VERTICAL;
            
            // Loop until a correct position is inputted
            while (!b.placeShip(up, k, ud_f)){
                if (!first)
                    cout << "The ship can not be placed there." << endl;
                if (ud_f == HORIZONTAL)
                    cout << "Enter row and column of leftmost cell (e.g., 3 5): ";
                if (ud_f == VERTICAL)
                    cout << "Enter row and column of topmost cell (e.g., 3 5): ";
                getLineWithTwoIntegers(ur, uc);
                up.r = ur;
                up.c = uc;
                // Will make the first time false so that the improper location prints a user error
                first = false;
            }
            // At this point it CAN be placed here -- the board function places it so...
            b.display(false);
        }
    }
    
    // IF all ships were able to be placed then k should equal nShips
    if (k == game().nShips())
        return true;
    cout << "Unable to place all ships." << endl;
    return false;
}

Point HumanPlayer::recommendAttack(){
    cout << "Enter the row and column to attack (e.g., 3 5): ";
    int r, c;
    getLineWithTwoIntegers(r, c);
    Point ua(r,c);
    m_lastCellAttacked = ua;
    return m_lastCellAttacked;
}

void HumanPlayer::recordAttackResult(Point /*p*/, bool /*validShot*/, bool /*shotHit*/, bool /*shipDestroyed*/, int /*shipId*/)
{
    // Human player ignores this
}

void HumanPlayer::recordAttackByOpponent(Point /* p */)
{
      // HumanPlayer completely ignores what the opponent does
}

//*********************************************************************
//  MediocrePlayer
//*********************************************************************

class MediocrePlayer: public Player
{
public:
    MediocrePlayer(string nm, const Game& g): Player(nm, g), m_lastCellAttacked(0,0), m_name(nm), m_state(1){
        // Make sure the stack initializes empty
        while (!m_recall.empty())
            m_recall.pop();
    }
    // Destructor to sensure space for struct and vector are released.
    virtual ~MediocrePlayer()
    {
        while (!m_recall.empty())
            m_recall.pop();
        while (!attackLog.empty())
            attackLog.pop_back();
    }
    virtual bool placeShips(Board& b);
    virtual Point recommendAttack();
    virtual void recordAttackResult(Point p, bool validShot, bool shotHit,
                                                bool shipDestroyed, int shipId);
    virtual void recordAttackByOpponent(Point p);
    // Helper Function
    bool placedShips(Point p, int shipId, Board& b);
    class logAttacks{
    public:
        logAttacks(Point p, bool shotHit, bool shipDestroyed, int shipId): mp(p), m_hit(shotHit), m_destroy(shipDestroyed), m_shipId(shipId){};
        Point attackPoint(){return mp;}
        bool attackHit(){return m_hit;}
        bool attackDestroy(){return m_destroy;}
        int shipIdentify(){return m_shipId;}
    private:
        Point mp;
        bool m_hit, m_destroy;
        int m_shipId;
    };
    vector <logAttacks> attackLog;
    
private:
    Point m_lastCellAttacked;
    string m_name;
    int m_state;
    stack<Point> m_recall;
    Point m_Center;
};

bool MediocrePlayer::placedShips(Point p, int shipId, Board& b){
    // Return true if at the last ship
    if (shipId == game().nShips())
        return true;
    
    // "Layering technique"
    if (p.c == game().cols()){
        // Adjust the column to the first position and move the row down one
        p.c = 0;
        p.r++;
    }
    
    // IF we can place the ship HORIZONTALLY at point p
    if (b.placeShip(Point(p.r, p.c), shipId, HORIZONTAL)){
        // Log the point
        m_recall.push(Point(p.r, p.c));
        // Start from top again
        //p.r = 0;
        //p.c = 0;
        if (placedShips(Point(p.r, p.c), shipId+1, b))
            return true;
        // IF another ship couldn't be placed BACKTRACK
        b.unplaceShip(m_recall.top(), shipId, HORIZONTAL);
        if (!m_recall.empty())
            m_recall.pop();
        return placedShips(Point(p.r, p.c + 1), shipId, b);
    }
    
    // IF we can place the ship VERTICALLY at point p
    if (b.placeShip(Point(p.r, p.c), shipId, VERTICAL)){
        // Log the point
        m_recall.push(Point(p.r, p.c));
        // Start from top again
        //p.r = 0;
        //p.c = 0;
        if (placedShips(Point(p.r, p.c), shipId+1, b))
            return true;
        // IF another ship couldn't be placed BACKTRACK
        b.unplaceShip(m_recall.top(), shipId, VERTICAL);
        if (!m_recall.empty())
            m_recall.pop();
        return placedShips(Point(p.r, p.c + 1), shipId, b);
    }
    
    // IF we cannot place the ship HORIZONTALLY/VETICALLY move the point and try here
    else {
        // If we have reached the end of the grid then we cannot place the ship
        if (p.r == game().rows())
            // At this point the "layering technique" states that all spaces have been attempted. Thus it is impossible to place this ship.
            return false;
        
        // Otherwise it's within the bounds of the grid so test the new point.
        return placedShips(Point(p.r, p.c + 1), shipId, b);
    }
}

bool MediocrePlayer::placeShips(Board &b){
    // Try different variations until count is 50
    for (int count = 0; count < 50; count++){
        // Block some parts
        b.block();
        // IF all the ships can be placed return true;
        if (placedShips(Point(0,0), 0, b)){
            // Make sure to unblock before you return true
            b.unblock();
            
            // Make sure we get rid of allocated stack before we return
            while (!m_recall.empty())
                m_recall.pop();
            return true;
        }
        
        // If the ships couldn't be placed try a different block
        b.unblock();
    }
    // Make sure we get rid of allocated stack before we return
    while (!m_recall.empty())
        m_recall.pop();
    
    // Return false -- unable to put any in
    return false;
}

Point MediocrePlayer::recommendAttack(){
    // State 1
    if (m_state == 1){
        // Find a random point
        Point rand = game().randomPoint();
        // Search for unoriginality
        for (int i = 0; i < attackLog.size(); i++){
            // Try a new random point
            if (rand.r == attackLog[i].attackPoint().r && rand.c == attackLog[i].attackPoint().c){
                rand = game().randomPoint();
                i = -1;
            }
        }
        
        // Otherwise it is original!
        m_lastCellAttacked = rand;
    }
    // State 2
    if (m_state == 2){
        Point curr = m_Center;
        
        // Search for unoriginality
        for (int i = 0; i < attackLog.size(); i++){
            // If a repeat
            if (curr.r == attackLog[i].attackPoint().r && curr.c == attackLog[i].attackPoint().c){
                // Reset curr to original center
                curr = m_Center;
                // Try a new point:
                // Go in a random direction
                int dir = randInt(4);
                switch (dir){
                        // UP
                    case 0:
                        // Move up a random amount up to 4 UPWARDS -- if this exceeds the bounds assume the latter
                        curr.r += - 1 - randInt(4);
                        if (curr.r < 0)
                            curr.r = 0;
                        break;
                        // RIGHT
                    case 1:
                        curr.c += 1 + randInt(4);
                        if (curr.c >= game().cols())
                            curr.c = game().cols()-1;
                        break;
                        // DOWN
                    case 2:
                        curr.r += 1 + randInt(4);
                        if (curr.r >= game().rows())
                            curr.r = game().rows()-1;
                        break;
                        // LEFT
                    case 3:
                        curr.c += -1 -randInt(4);
                        if (curr.c < 0)
                            curr.c = 0;
                        break;
                    default:
                        break;
                }
                i = -1;
            }
        }
        
        // Shoot it here
        m_lastCellAttacked = curr;
        
    }
    // Return the value we set!
    return m_lastCellAttacked;
}


void MediocrePlayer::recordAttackResult(Point p, bool validShot, bool shotHit, bool shipDestroyed, int shipId)
{
    // If an invalid shot
    if (!validShot)
        return;
    
    // Otherwise...
    // Log this attack as successful
    attackLog.push_back(logAttacks(p, shotHit, shipDestroyed, shipId));
    
    // Look at states
    // State 1
    if (m_state == 1){
        // IF the shot hits but DOES NOT destroy the ship
        if (shotHit == true && shipDestroyed == false){
            // This is the center of the cross search
            m_Center = attackLog[attackLog.size()-1].attackPoint();
            m_state = 2;
        }
        // Otherwise do nothing
        return;
    }
    // State 2
    else if (m_state == 2){
        // If the shot hits and destroys the ship
        if (shotHit == true && shipDestroyed == true)
            m_state = 1;
        // Otherwise do nothing
        return;
    }
    
}

void MediocrePlayer::recordAttackByOpponent(Point /* p */)
{
      // MediocrePlayer completely ignores what the opponent does
}



//*********************************************************************
//  GoodPlayer
//*********************************************************************

class GoodPlayer: public Player
{
public:
    GoodPlayer(string nm, const Game& g): Player(nm, g), m_lastCellAttacked(0,0), m_state(1), m_countDiagnol(0){
        while (!m_nextOne.empty())
            m_nextOne.pop();
        while (!m_recall.empty())
            m_recall.pop();
    }
    ~GoodPlayer(){
        while (!m_nextOne.empty())
            m_nextOne.pop();
        while (!m_recall.empty())
            m_recall.pop();
    }
    virtual bool placeShips(Board& b);
    virtual Point recommendAttack();
    virtual void recordAttackResult(Point p, bool validShot, bool shotHit,
                                                bool shipDestroyed, int shipId);
    virtual void recordAttackByOpponent(Point p);
    // Helper class
    class logAttacks{
    public:
        logAttacks(Point p, bool shotHit, bool shipDestroyed, int shipId): mp(p), m_hit(shotHit), m_destroy(shipDestroyed), m_shipId(shipId){};
        Point attackPoint(){return mp;}
        bool attackHit(){return m_hit;}
        bool attackDestroy(){return m_destroy;}
        int shipIdentify(){return m_shipId;}
    private:
        Point mp;
        bool m_hit, m_destroy;
        int m_shipId;
    };
    vector <logAttacks> attackLog;
    // Helper Function
    bool diagLeft();
    bool placedShips(Point p, int shipId, Board& b);
private:
    Point m_lastCellAttacked;
    stack<Point> m_nextOne;
    stack<Point> m_recall;
    int m_state, m_countDiagnol;
};

bool GoodPlayer::placedShips(Point p, int shipId, Board& b){
    // Return true if at the last ship
    if (shipId == game().nShips())
        return true;
    
    // "Layering technique"
    if (p.c == game().cols()){
        // Adjust the column to the first position and move the row down one
        p.c = 0;
        p.r++;
    }
    
    // IF we can place the ship HORIZONTALLY at point p
    if (b.placeShip(Point(p.r, p.c), shipId, HORIZONTAL)){
        // Log the point
        m_recall.push(Point(p.r, p.c));
        // Start from top again
        //p.r = 0;
        //p.c = 0;
        if (placedShips(Point(p.r, p.c), shipId+1, b))
            return true;
        // IF another ship couldn't be placed BACKTRACK
        b.unplaceShip(m_recall.top(), shipId, HORIZONTAL);
        m_recall.pop();
        return placedShips(Point(p.r, p.c + 1), shipId, b);
    }
    
    // IF we can place the ship VERTICALLY at point p
    if (b.placeShip(Point(p.r, p.c), shipId, VERTICAL)){
        // Log the point
        m_recall.push(Point(p.r, p.c));
        // Start from top again
        //p.r = 0;
        //p.c = 0;
        if (placedShips(Point(p.r, p.c), shipId+1, b))
            return true;
        // IF another ship couldn't be placed BACKTRACK
        b.unplaceShip(m_recall.top(), shipId, VERTICAL);
        m_recall.pop();
        return placedShips(Point(p.r, p.c + 1), shipId, b);
    }
    
    // IF we cannot place the ship HORIZONTALLY/VETICALLY move the point and try here
    else {
        // If we have reached the end of the grid then we cannot place the ship
        if (p.r == game().rows())
            // At this point the "layering technique" states that all spaces have been attempted. Thus it is impossible to place this ship.
            return false;
        
        // Otherwise it's within the bounds of the grid so test the new point.
        return placedShips(Point(p.r, p.c + 1), shipId, b);
    }
}

// We will block the board. Attempt to place a ship. Then unblock and place the next ship
bool GoodPlayer::placeShips(Board &b){
    // Try different variations until count is 100
    for (int count = 0; count < 100; count++){
        // Block some parts
        b.block();
        // IF all the ships can be placed return true;
        if (placedShips(Point(0,0), 0, b)){
            // Make sure to unblock before you return true
            b.unblock();
            
            // Make sure we get rid of allocated stack before we return
            while (!m_recall.empty())
                m_recall.pop();
            return true;
        }
        
        // If the ships couldn't be placed try a different block
        b.unblock();
    }
    
    // IF unable to place the mediocre way...
    int i = 0;
    for (; i < game().nShips(); i++){
        Point rand = game().randomPoint();
        // If you can place the ship one of two ways
        if (!b.placeShip(rand, i, HORIZONTAL) && !b.placeShip(rand, i, VERTICAL))
            i--;
    }
    
    if (i == game().nShips())
        return true;
    
    
    // Make sure we get rid of allocated stack before we return
    while (!m_recall.empty())
        m_recall.pop();
    
    // Return false -- unable to put any in
    return false;
    
}

bool GoodPlayer::diagLeft(){
    // Go through the attackLog and see if all evens have been taken
    int diagCount = 0;
    for (int i = 0; i < attackLog.size(); i++){
        if ((attackLog[i].attackPoint().r + attackLog[i].attackPoint().c) % 2 == 0)
            diagCount++;
    }
    
    // IF all diagnols have been attacked
    if (diagCount >= (game().rows() * game().cols())/2)
        return false;
    
    return true;
}

Point GoodPlayer::recommendAttack(){
    // State 1 -- HASN'T hit anything
    if (m_state == 1){
        // If there are no diagnols left
        if (!diagLeft()){
            Point tryPoint = game().randomPoint();
            for (int i = 0; i < attackLog.size(); i++){
                // IF it is not original -- but it was already a diagnol
                if (tryPoint.r == attackLog[i].attackPoint().r && tryPoint.c == attackLog[i].attackPoint().c){
                    tryPoint = game().randomPoint();
                    i = -1;
                }
            }
            m_lastCellAttacked = tryPoint;
        }
        
        // If there are still loggable attacks
        else if (!m_nextOne.empty()){
            m_state = 2;
        }
        
        else {
            // Choose from the diagnols at a random -- i + j has to be divisible by 2
            Point tryPoint = game().randomPoint();
        
            while ((tryPoint.r + tryPoint.c) % 2 != 0)
                // Try another point
                tryPoint = game().randomPoint();
        
            // Search for unoriginality
            for (int i = 0; i < attackLog.size(); i++){
                // While it's not a "proper diagnol point"
                while ((tryPoint.r + tryPoint.c) % 2 != 0){
                    // Try another point
                    tryPoint = game().randomPoint();
                }
            
                // IF it is not original -- but it was already a diagnol
                if (tryPoint.r == attackLog[i].attackPoint().r && tryPoint.c == attackLog[i].attackPoint().c){
                    tryPoint = game().randomPoint();
                    i = -1;
                }
            }
            // This is an original diagnol point!
            m_lastCellAttacked = tryPoint;
        }
    }
    // State 2
    if (m_state == 2){
        bool tu = false, tr = false, td = false, tl = false;
        Point curr;
        if (!m_nextOne.empty())
            curr = m_nextOne.top();
        if (m_nextOne.empty())
            curr = game().randomPoint();
        
        // Search for unoriginality
        for (int i = 0; i < attackLog.size(); i++){
            // If a repeat
            if (curr.r == attackLog[i].attackPoint().r && curr.c == attackLog[i].attackPoint().c){
                // If all directions were tried...
                if (tu == true && tr == true && td == true && tl == true){
                    // Pop the top (can't go in this direction)
                    if (!m_nextOne.empty())
                        m_nextOne.pop();
                    tu = false;
                    tr = false;
                    td = false;
                    tl = false;
                }
                
                if (!m_nextOne.empty())
                    curr = m_nextOne.top();
                if (m_nextOne.empty())
                    curr = game().randomPoint();
                // Try a new point:
                // Go in a random direction
                int dir = randInt(4);
                switch (dir){
                        // UP
                    case 0:
                        // Move up a random amount up to 4 UPWARDS -- if this exceeds the bounds assume the latter
                        curr.r -= 1;
                        if (curr.r < 0)
                            curr.r = 0;
                        tu = true;
                        break;
                        // RIGHT
                    case 1:
                        curr.c += 1;
                        if (curr.c >= game().cols())
                            curr.c = game().cols()-1;
                        tr = true;
                        break;
                        // DOWN
                    case 2:
                        curr.r += 1;
                        if (curr.r >= game().rows())
                            curr.r = game().rows()-1;
                        td = true;
                        break;
                        // LEFT
                    case 3:
                        curr.c -=1;
                        if (curr.c < 0)
                            curr.c = 0;
                        tl = true;
                        break;
                    default:
                        break;
                }
                i = -1;
            }
        }
        
        // Shoot it here
        m_lastCellAttacked = curr;
        
    }

    return m_lastCellAttacked;
    
}

void GoodPlayer::recordAttackResult(Point p, bool validShot, bool shotHit, bool shipDestroyed, int shipId){
    // If an invalid shot
    if (!validShot)
        return;

    // Log this attack as successful
    attackLog.push_back(logAttacks(p, shotHit, shipDestroyed, shipId));
    
    // State 1
    if (m_state == 1){
        // IF the shot hits but DOES NOT destroy the ship
        if (shotHit == true && shipDestroyed == false){
            // This is the center of the cross search
            m_nextOne.push(attackLog[attackLog.size()-1].attackPoint());
            m_state = 2;
        }
        // Otherwise do nothing
        return;
    }
    // State 2
    else if (m_state == 2){
        if (shotHit == true && shipDestroyed == false)
            m_nextOne.push(attackLog[attackLog.size()-1].attackPoint());
        // If the shot hits and destroys the ship
        if (shotHit == true && shipDestroyed == true)
            m_state = 1;
        // Otherwise do nothing
        return;
    }
    
}

void GoodPlayer::recordAttackByOpponent(Point /* p */){
    
    // Does not have to do anything;
    
}

//*********************************************************************
//  createPlayer
//*********************************************************************

Player* createPlayer(string type, string nm, const Game& g)
{
    static string types[] = {
        "human", "awful", "mediocre", "good"
    };
    
    int pos;
    for (pos = 0; pos != sizeof(types)/sizeof(types[0])  &&
                                                     type != types[pos]; pos++)
        ;
    switch (pos)
    {
      case 0:  return new HumanPlayer(nm, g);
      case 1:  return new AwfulPlayer(nm, g);
      case 2:  return new MediocrePlayer(nm, g);
      case 3:  return new GoodPlayer(nm, g);
      default: return nullptr;
    }
}


// Tests for HumanPlayer
/*
int main(){
    Game g(6,3);
    g.addShip(5, 't', "Tributary");
    g.addShip(5, 'c', "Clown");
    g.addShip(5, 'l', "Loser");
    g.addShip(4, 'w', "Won't Work");
    Board b(g);
    HumanPlayer me("Johnny", g);
    me.placeShips(b);
    me.recommendAttack();
    
}
*/

// Tests for MediocrePlayer
/*
int main(){
    Game g(10,10);
    g.addShip(5, 'a', "aircraft carrier");
    g.addShip(4, 'b', "battleship");
    g.addShip(3, 'd', "destroyer");
    g.addShip(3, 's', "submarine");
    g.addShip(2, 'p', "patrol boat");
    Board b(g);
    MediocrePlayer you("Mancy", g);
    you.placeShips(b);
    b.display(false);
}
*/


// Tests for GoodPlayer

/*
int main(){
    Game g(10,10);
    g.addShip(5, 'a', "aircraft carrier");
    g.addShip(4, 'b', "battleship");
    g.addShip(3, 'd', "destroyer");
    g.addShip(3, 's', "submarine");
    g.addShip(2, 'p', "patrol boat");
    GoodPlayer us("Charles", g);
    Board b(g);
    assert(us.placeShips(b));
    Game l(2,2);
    l.addShip(2, 's', "SS Sarah");
    l.addShip(2, 'l', "St. Lee");
    Board f(l);
    GoodPlayer eli("Eli", l);
    eli.placeShips(f);
    b.display(false);
    f.display(false);
}
*/
