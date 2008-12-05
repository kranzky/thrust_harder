//==============================================================================

#ifndef ArseContext
#define ArseContext

class Engine;

//------------------------------------------------------------------------------
class Context
{
  public:
    Context( Engine * engine = 0 );
    virtual ~Context();

  private:
    Context( const Context & );
    Context & operator=( const Context & );

  public:
    virtual void init() = 0;
    virtual void fini() = 0;
    virtual bool update( float dt ) = 0;
    virtual void render() = 0;

  protected:
    Engine * m_engine;
};

#endif

//==============================================================================
