//==============================================================================

#ifndef ArseEditor
#define ArseEditor

#include <hge.h>

#include <context.hpp>

class Engine;

//------------------------------------------------------------------------------
class Editor : public Context
{
  public:
    Editor( Engine * engine = 0 );
    virtual ~Editor();

  private:
    Editor( const Editor & );
    Editor & operator=( const Editor & );

  public:
    virtual void init();
    virtual void fini();
    virtual bool update( float dt );
    virtual void render();
};

#endif

//==============================================================================
