//==============================================================================

#ifndef ArseScore
#define ArseScore

#include <hge.h>

#include <vector>
#include <string>

#include <context.hpp>

class Engine;
class hgeSprite;

//------------------------------------------------------------------------------
class Score : public Context
{
  public:
    Score( Engine * engine = 0 );
    virtual ~Score();

  private:
    Score( const Score & );
    Score & operator=( const Score & );

  public:
    virtual void init();
    virtual void fini();
    virtual bool update( float dt );
    virtual void render();
    void calculateScore( int lives, int urchins, int coins, int time );

  private:
    void _updateScore();

  private:
    hgeSprite * m_dark;
    bool m_calculate;
    int m_lives;
    int m_urchins;
    int m_coins;
    int m_time;
    float m_timer;
    int m_buffer;
    std::vector< std::pair< std::string, int > > m_high_score;
    std::string m_name;
};

#endif

//==============================================================================
