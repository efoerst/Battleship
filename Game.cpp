#include "Game.h"
#include "Board.h"
#include "Player.h"
#include "globals.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <cctype>
#include <vector>

using namespace std;

class GameImpl
{
  public:
    GameImpl(int nRows, int nCols);
    int rows() const;
    int cols() const;
    bool isValid(Point p) const;
    Point randomPoint() const;
    bool addShip(int length, char symbol, string name);
    int nShips() const;
    int shipLength(int shipId) const;
    char shipSymbol(int shipId) const;
    string shipName(int shipId) const;
    Player* play(Player* p1, Player* p2, Board& b1, Board& b2, bool shouldPause);
    
private:
    int m_rows;
    int m_cols;
    // Create a private class logShips to keep track of stuff.
    class logShips{
    public:
        logShips(int slength, char schar, string sname): m_length(slength), m_char(schar), m_name(sname){};
        int length() const {return m_length;}
        char character() const {return m_char;}
        string name() const {return m_name;}
    private:
        int m_length;
        char m_char;
        string m_name;
    };
    // Initialize vector template to log data
    vector<logShips> m_log;
};

void waitForEnter()
{
    cout << "Press enter to continue: ";
    cin.ignore(10000, '\n');
}

inline GameImpl::GameImpl(int nRows, int nCols)
{
    // Valid positions
    if (nRows > 0 && nRows <= MAXROWS && nCols > 0 && nCols <= MAXCOLS){
        m_rows = nRows;
        m_cols = nCols;
    }
    // Invalid positions
    else if (nRows <= 0 || nCols <= 0){
        m_rows = 0;
        m_cols = 0;
    }
    else{
        m_rows = MAXROWS;
        m_cols = MAXCOLS;
    }
}

int GameImpl::rows() const
{
    return m_rows;
}

int GameImpl::cols() const
{
    return m_cols;
}

bool GameImpl::isValid(Point p) const
{
    return p.r >= 0  &&  p.r < rows()  &&  p.c >= 0  &&  p.c < cols();
}

Point GameImpl::randomPoint() const
{
    return Point(randInt(rows()), randInt(cols()));
}

bool GameImpl::addShip(int length, char symbol, string name)
{
    // Ensure the proper length corresponds to the right character etc...
    if (length < 1)
        return false;
    if (length > rows() && length > cols())
        return false;
    if (!isascii(symbol) || !isprint(symbol))
        return false;
    if (symbol == 'X' || symbol == '.' || symbol == 'o' || symbol == '#')
        return false;
    // Trace through ships to ensure we can add it
    int totalLength = 0;
    for (int i = 0; i < m_log.size(); i++){
        totalLength += m_log[i].length();
        if (m_log[i].character() == symbol)
            return false;
    }
    if (totalLength + length > rows() * cols())
        return false;
    
    // By this point the ship is properly named, characterized, and has a proper length that can fit within the grid!
    m_log.push_back(logShips(length, symbol, name));
    
    // After we've logged the values of the ship return true because we added it.
    return true;
}

int GameImpl::nShips() const
{
    // Vector size is an unsigned long so we convert it to an int for us to use!
    // Note: shipID will become the value in which the item is inputted into m_log. In the order it's added
    return (int)m_log.size();
}

int GameImpl::shipLength(int shipId) const
{
    return m_log[shipId].length();
}

char GameImpl::shipSymbol(int shipId) const
{
    return m_log[shipId].character();
}

string GameImpl::shipName(int shipId) const
{
    return m_log[shipId].name();
}


Player* GameImpl::play(Player* p1, Player* p2, Board& b1, Board& b2, bool shouldPause)
{
    // IF either board can't place ships return nullptr
    if (!p1->placeShips(b1) || !p2->placeShips(b2))
        return nullptr;
    
    // At this point the ships for both players have been successfully placed
    // While all ships remain on the board
    for (int i = 0; !b1.allShipsDestroyed() && !b2.allShipsDestroyed(); i++){
        // IF the 3rd parameter says to pause
        if (shouldPause && i != 0){
            cout << "Press enter to continue: ";
            cin.get();
        }
        
        // Start with the second player (p2)
        if (i == 0 || i % 2 == 0){
            // Greet player
            cout << p1->name() << "'s turn. Board for " << p2->name() << ":" << endl;
            // If the player is human only show what we would usually see
            if (p1->isHuman())
                b2.display(true);
            // Else
            if (!p1->isHuman())
                b2.display(false);
            
            // Attack as this player
            bool shotHit, shipDestroyed, validShot = false;
            int shipId;
            Point attack = p1->recommendAttack();
            if (b2.attack(attack, shotHit, shipDestroyed, shipId))
                validShot = true;
            p1->recordAttackResult(attack, validShot, shotHit, shipDestroyed, shipId);
            
            // Shoot prompts
            cout << p1->name();
            cout << " attacked (" << attack.r << "," << attack.c << ") and ";
            // IF it hit AND sunk a ship
            if (shotHit && shipDestroyed)
                cout << "destroyed the " << shipName(shipId);
            // IF it only hit a ship
            else if (shotHit)
                cout << "hit something";
            // IF it hit nothing
            else if (!shotHit && !shipDestroyed)
                cout << "missed";
            cout << ", resulting in:" << endl;
            // Display the board again.
            if (p1->isHuman())
                b2.display(true);
            // Else
            if (!p1->isHuman())
                b2.display(false);
        }
        // Otherwise do the first
        else {
            // Greet player
            cout << p2->name() << "'s turn. Board for " << p1->name() << ":" << endl;
            // If the player is human only show what we would usually see
            if (p2->isHuman())
                b1.display(true);
            // Else
            if (!p2->isHuman())
                b1.display(false);
            
            // Attack as this player
            bool shotHit, shipDestroyed, validShot = false;
            int shipId;
            Point attack = p2->recommendAttack();
            if (b1.attack(attack, shotHit, shipDestroyed, shipId))
                validShot = true;
            p2->recordAttackResult(attack, validShot, shotHit, shipDestroyed, shipId);
            
            // Shoot prompts
            cout << p2->name();
            if (validShot)
                cout << " attacked (" << attack.r << "," << attack.c << ") and ";
            if (!validShot)
                cout << " wasted a shot at (" << attack.r << "," << attack.c << ")";
            // IF it hit AND sunk a ship
            if (shotHit && shipDestroyed && validShot)
                cout << "destroyed the " << shipName(shipId);
            // IF it only hit a ship
            else if (shotHit && validShot)
                cout << "hit something";
            // IF it hit nothing
            else if (!shotHit && validShot)
                cout << "missed";
            cout << ", resulting in:" << endl;
            // Display the board again.
            if (p2->isHuman())
                b1.display(true);
            // Else
            if (!p2->isHuman())
                b1.display(false);
        }
    }
    
    // At this point one of the two has all ships destroyed:
    // IF the board of player 1 has all its ships destroyed -- p2 won
    if (b1.allShipsDestroyed()){
        // If the loser is human
        if (p1->isHuman())
            b2.display(true);
        cout << p2->name() << " wins!" << endl;
        return p2;
    }
    // Otherwise if the board of player 2 has all its ships destroyed -- p1 won
    if (b2.allShipsDestroyed()){
        // If the loser is human
        if (p2->isHuman())
            b1.display(true);
        cout << p1->name() << " wins!" << endl;
        return p1;
    }
    
    // Should never happen but if neither player wins
    cout << "Wow this is peculiar! Seems like a draw occured??? Odd... We're working on this!" << endl;
    return nullptr;
    
}

//******************** Game functions *******************************

// These functions for the most part simply delegate to GameImpl's functions.
// You probably don't want to change any of the code from this point down.

Game::Game(int nRows, int nCols)
{
    if (nRows < 1  ||  nRows > MAXROWS)
    {
        cout << "Number of rows must be >= 1 and <= " << MAXROWS << endl;
        exit(1);
    }
    if (nCols < 1  ||  nCols > MAXCOLS)
    {
        cout << "Number of columns must be >= 1 and <= " << MAXCOLS << endl;
        exit(1);
    }
    m_impl = new GameImpl(nRows, nCols);
}

Game::~Game()
{
    delete m_impl;
}

int Game::rows() const
{
    return m_impl->rows();
}

int Game::cols() const
{
    return m_impl->cols();
}

bool Game::isValid(Point p) const
{
    return m_impl->isValid(p);
}

Point Game::randomPoint() const
{
    return m_impl->randomPoint();
}

bool Game::addShip(int length, char symbol, string name)
{
    if (length < 1)
    {
        cout << "Bad ship length " << length << "; it must be >= 1" << endl;
        return false;
    }
    if (length > rows()  &&  length > cols())
    {
        cout << "Bad ship length " << length << "; it won't fit on the board"
             << endl;
        return false;
    }
    if (!isascii(symbol)  ||  !isprint(symbol))
    {
        cout << "Unprintable character with decimal value " << symbol
             << " must not be used as a ship symbol" << endl;
        return false;
    }
    if (symbol == 'X'  ||  symbol == '.'  ||  symbol == 'o')
    {
        cout << "Character " << symbol << " must not be used as a ship symbol"
             << endl;
        return false;
    }
    int totalOfLengths = 0;
    for (int s = 0; s < nShips(); s++)
    {
        totalOfLengths += shipLength(s);
        if (shipSymbol(s) == symbol)
        {
            cout << "Ship symbol " << symbol
                 << " must not be used for more than one ship" << endl;
            return false;
        }
    }
    if (totalOfLengths + length > rows() * cols())
    {
        cout << "Board is too small to fit all ships" << endl;
        return false;
    }
    return m_impl->addShip(length, symbol, name);
}

int Game::nShips() const
{
    return m_impl->nShips();
}

int Game::shipLength(int shipId) const
{
    assert(shipId >= 0  &&  shipId < nShips());
    return m_impl->shipLength(shipId);
}

char Game::shipSymbol(int shipId) const
{
    assert(shipId >= 0  &&  shipId < nShips());
    return m_impl->shipSymbol(shipId);
}

string Game::shipName(int shipId) const
{
    assert(shipId >= 0  &&  shipId < nShips());
    return m_impl->shipName(shipId);
}

Player* Game::play(Player* p1, Player* p2, bool shouldPause)
{
    if (p1 == nullptr  ||  p2 == nullptr  ||  nShips() == 0)
        return nullptr;
    Board b1(*this);
    Board b2(*this);
    return m_impl->play(p1, p2, b1, b2, shouldPause);
}

