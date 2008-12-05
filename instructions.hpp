//==============================================================================

#ifndef ArseInstructions
#define ArseInstructions

#include <hge.h>

#include <context.hpp>

class Engine;
class hgeSprite;

//------------------------------------------------------------------------------
class Instructions : public Context
{
  public:
    Instructions( Engine * engine = 0 );
    virtual ~Instructions();

  private:
    Instructions( const Instructions & );
    Instructions & operator=( const Instructions & );

  public:
    virtual void init();
    virtual void fini();
    virtual bool update( float dt );
    virtual void render();

  private:
    hgeSprite * m_dark;
};

#endif

//==============================================================================
