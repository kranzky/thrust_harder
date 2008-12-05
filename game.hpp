//==============================================================================

#ifndef ArseGame
#define ArseGame

#include <vector>

#include <hge.h>

#include <context.hpp>

class hgeSprite;

class Engine;
class Ship;
class Scenery;
class Nothing;
class Urchin;
class Coin;

//------------------------------------------------------------------------------
class Game : public Context
{
  public:
    Game( Engine * engine = 0 );
    virtual ~Game();

  private:
    Game( const Game & );
    Game & operator=( const Game & );

  public:
    virtual void init();
    virtual void fini();
    virtual bool update( float dt );
    virtual void render();

  private:
    void _renderBacks();
    void _renderFores();
    void _renderBodies();
    void _renderGui();

  private:
    Ship * m_ship;
    std::vector< Urchin * > m_urchins;
    std::vector< Coin * > m_coins;
    Scenery ** m_tiles;
    int m_num_tiles;
    Nothing ** m_backs;
    int m_num_backs;
    Nothing ** m_fores;
    int m_num_fores;
    hgeSprite * m_sky;
    hgeSprite * m_gui;
    int m_urchin_count;
};

#endif

//==============================================================================
