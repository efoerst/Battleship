#include "Board.h"
#include "Game.h"
#include "globals.h"
#include <iostream>

using namespace std;

class BoardImpl
{
  public:
    BoardImpl(const Game& g);
    void clear();
    void block();
    void unblock();
    bool placeShip(Point topOrLeft, int shipId, Direction dir);
    bool unplaceShip(Point topOrLeft, int shipId, Direction dir);
    void display(bool shotsOnly) const;
    bool attack(Point p, bool& shotHit, bool& shipDestroyed, int& shipId);
    bool allShipsDestroyed() const;

  private:
    const Game& m_game;
    char m_grid[MAXROWS][MAXCOLS];
};

BoardImpl::BoardImpl(const Game& g)
 : m_game(g)
{
    for (int r = 0; r < m_game.rows(); r++)
        for (int c = 0; c < m_game.cols(); c++)
            m_grid[r][c] = '.';
}

void BoardImpl::clear()
{
    // Goal: Iterate through the grid and set everythign to '.'
    for (int r = 0; r < m_game.rows(); r++)
        for (int c = 0; c < m_game.cols(); c++)
            m_grid[r][c] = '.';
}

void BoardImpl::block()
{
    int count = 0;
    while (count < (m_game.rows() * m_game.cols())/2){
        // Initialize randoms
        int randR = randInt(m_game.rows());
        int randC = randInt(m_game.cols());
        // If it is not blocked
        if (m_grid[randR][randC] == '.'){
            m_grid[randR][randC] = '#';
            count++;
        }
    }
}

void BoardImpl::unblock()
{
    for (int r = 0; r < m_game.rows(); r++)
        for (int c = 0; c < m_game.cols(); c++)
        {
            // Replace the nono'ed char with the '.' char.
            if (m_grid[r][c] == '#')
                m_grid[r][c] = '.';
        }
}

bool BoardImpl::placeShip(Point topOrLeft, int shipId, Direction dir)
{
    // Based on the properties of logShips as a vactor, if shipID is larger than the number of ships or it is less than 0 then it is not a valid ID in a vector and thus not valid as a ship
    if (shipId > m_game.nShips() || shipId < 0)
        return false;
    // Ensure topOrLeft is in grid
    if (topOrLeft.r < 0 || topOrLeft.c < 0)
        return false;
    if (topOrLeft.r > m_game.rows() || topOrLeft.c > m_game.cols())
        return false;
    // At this point the ship is valid and has a starting point in the grid
    
    // IF direction is invalid
    if (dir != VERTICAL && dir != HORIZONTAL)
        return false;
    
    // IF the length from the top most point to the length of the ship is greater than the number of rows/cols return false.
    if (dir == HORIZONTAL && topOrLeft.c + m_game.shipLength(shipId) > m_game.cols())
        return false;
    if (dir == VERTICAL && topOrLeft.r + m_game.shipLength(shipId) > m_game.rows())
        return false;
    
    // Initialize an endpoint to refer to -- Note: added (-1) to end because of size variations
    Point endPoint;
    if (dir == HORIZONTAL){
        endPoint.r = topOrLeft.r;
        endPoint.c = topOrLeft.c + m_game.shipLength(shipId) - 1;
    }
    if (dir == VERTICAL){
        endPoint.r = topOrLeft.r + m_game.shipLength(shipId) - 1;
        endPoint.c = topOrLeft.c;
    }
    
    // How do i know if a ship has been placed before? Check if the character has been used before
    for (int r = 0; r < m_game.rows(); r++)
        for (int c = 0; c < m_game.cols(); c++)
            if (m_grid[r][c] == m_game.shipSymbol(shipId))
                return false;
    
    // Check for ship overlap (Check if a different character is in the spot already) -- This also checks for ship trying to be placed near blockage. IF every grid location is not a '.' then we can't put a ship there. DUH!
    for (int r = topOrLeft.r; r <= endPoint.r; r++)
        for (int c = topOrLeft.c; c <= endPoint.c; c++)
            if (m_grid[r][c] != '.')
                return false;
    
    
    // At this point the ship passes all requirements -- so make the grid reflect the ship being there
    for (int r = topOrLeft.r; r <= endPoint.r; r++)
        for (int c = topOrLeft.c; c <= endPoint.c; c++)
            m_grid[r][c] = m_game.shipSymbol(shipId);
    
    return true;
}

bool BoardImpl::unplaceShip(Point topOrLeft, int shipId, Direction dir)
{
    // IF the shipId is invalid
    if (shipId > m_game.nShips() || shipId < 0)
        return false;
    
    // Initialize an endpoint to refer to
    Point endPoint;
    if (dir == HORIZONTAL){
        endPoint.r = topOrLeft.r;
        endPoint.c = topOrLeft.c + m_game.shipLength(shipId)-1;
    }
    if (dir == VERTICAL){
        endPoint.r = topOrLeft.r + m_game.shipLength(shipId)-1;
        endPoint.c = topOrLeft.c;
    }
    
    // Check if the board contains the "entire" ship at these positions
    for (int r = topOrLeft.r; r <= endPoint.r; r++)
        for (int c = topOrLeft.c; c <= endPoint.c; c++)
            if (m_grid[r][c] != m_game.shipSymbol(shipId))
                return false;
    
    // At this point the shipID is valid and the entire ship is at the indicated locations -- so 'remove' the ship and return true
    for (int r = topOrLeft.r; r <= endPoint.r; r++)
        for (int c = topOrLeft.c; c <= endPoint.c; c++)
            m_grid[r][c] = '.';
    
    return true;
}

void BoardImpl::display(bool shotsOnly) const
{
    // Print the column numbers
    for (int c = 0; c < m_game.cols(); c++)
        cout << "  " << c;
    cout << '\n';
    
    // Remaining lines
    for (int r = 0; r < m_game.rows(); r++){
        // Print the row number
        cout << r << " ";
        
        for (int c = 0; c < m_game.cols(); c++){
            if (!shotsOnly)
                cout << m_grid[r][c];
            else{
                // If it is a ship character
                if (m_grid[r][c] != '.' && m_grid[r][c] != 'X' && m_grid[r][c] != 'o')
                    cout << '.';
                else
                    cout << m_grid[r][c];
            }
            // Ensure the space
            cout << "  ";
        }
        cout << '\n';
    }
}

bool BoardImpl::attack(Point p, bool& shotHit, bool& shipDestroyed, int& shipId)
{
    // Set values for invalidity
    shotHit = false;
    shipDestroyed = false;
    shipId = -1;
    
    // Ensure the attack point is valid
    if (p.r < 0 || p.c < 0)
        return false;
    if (p.r >= m_game.rows() || p.c >= m_game.cols())
        return false;
    // a.k.a. has been attacked already
    if (m_grid[p.r][p.c] == 'X' || m_grid[p.r][p.c] == 'o')
        return false;
    
    // At this point it can't be 'X' or 'o' -- and if it's not '.' then it HAS TO be a ship
    if (m_grid[p.r][p.c] != '.'){
        int count = 0;
        // Check to see if whole ship was destroyed
        for (int r = 0; r < m_game.rows(); r++){
            for (int c = 0; c < m_game.cols(); c++){
                // If the character exists again then the whole ship is not destroyed
                if (m_grid[r][c] == m_grid[p.r][p.c])
                    count++;
            }
        }
        
        if (count == 1)
            shipDestroyed = true;
        
        for (int shipNum = 0; shipNum < m_game.nShips(); shipNum++){
            if (m_grid[p.r][p.c] == m_game.shipSymbol(shipNum)){
                shipId = shipNum;
                break;
            }
        }
        
        shotHit = true;
        m_grid[p.r][p.c] = 'X';
    }
    // Otherwise it's hit nothing
    else {
        shotHit = false;
        m_grid[p.r][p.c] = 'o';
    }
    
    return true;
}

bool BoardImpl::allShipsDestroyed() const
{
    // If there remains no ship characters then they are all destroyed
    for (int r = 0; r < m_game.rows(); r++)
        for (int c = 0; c < m_game.cols(); c++)
            // IF it doesn't equal these three then it must be a ship char
            if (m_grid[r][c] != '.' && m_grid[r][c] != 'X' && m_grid[r][c] != 'o')
                // Not all ships are destroyed if there reamins a ship character
                return false;
    // If it makes it to this point then they are all destroyed
    return true;
}



//******************** Board functions ********************************

// These functions simply delegate to BoardImpl's functions.
// You probably don't want to change any of this code.

Board::Board(const Game& g)
{
    m_impl = new BoardImpl(g);
}

Board::~Board()
{
    delete m_impl;
}

void Board::clear()
{
    m_impl->clear();
}

void Board::block()
{
    return m_impl->block();
}

void Board::unblock()
{
    return m_impl->unblock();
}

bool Board::placeShip(Point topOrLeft, int shipId, Direction dir)
{
    return m_impl->placeShip(topOrLeft, shipId, dir);
}

bool Board::unplaceShip(Point topOrLeft, int shipId, Direction dir)
{
    return m_impl->unplaceShip(topOrLeft, shipId, dir);
}

void Board::display(bool shotsOnly) const
{
    m_impl->display(shotsOnly);
}

bool Board::attack(Point p, bool& shotHit, bool& shipDestroyed, int& shipId)
{
    return m_impl->attack(p, shotHit, shipDestroyed, shipId);
}

bool Board::allShipsDestroyed() const
{
    return m_impl->allShipsDestroyed();
}

/*
int main(){
    Game g(10,10);
    g.addShip(5, 'q', "charmander");
    g.addShip(3, 'l', "james");
    Board b(g);
    //b.display(false);
    Point t(0,0);
    Point p(1,9);
    b.placeShip(t, 0, HORIZONTAL);
    b.placeShip(p, 1, VERTICAL);
    bool shot = false, shipGone = false;
    int id = -1;
    for (int i = 0; i < g.shipLength(0); i++){
        Point wow(0, i);
        b.attack(wow, shot, shipGone, id);
        assert(shot);
    }
    b.display(false);
    assert(shipGone);
    assert(id == 0);
    cout << g.shipName(id) << " was destroyed in " << g.shipLength(id) << " hits! :D" << endl;
}
*/

/*
int main(){
    Game g(10,10);
    Board b(g);
    b.block();
    b.display(false);
}
*/
