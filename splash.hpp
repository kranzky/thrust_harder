//==============================================================================

#ifndef ArseSplash
#define ArseSplash

#include <hge.h>

#include <context.hpp>

class hgeDistortionMesh;

class Engine;

//------------------------------------------------------------------------------
class Splash : public Context
{
  public:
    Splash( Engine * engine = 0 );
    virtual ~Splash();

  private:
    Splash( const Splash & );
    Splash & operator=( const Splash & );

  public:
    virtual void init();
    virtual void fini();
    virtual bool update( float dt );
    virtual void render();

  private:
    bool _onTime( float time );
    void _distortMeshOne();
    void _distortMeshTwo();
    void _fade( float start_in, float start_out, float end_in, float end_out,
                const char * name );

  private:
    HCHANNEL m_channel;
    float m_timer;
    hgeDistortionMesh * m_mesh;
    float m_delta_time;
};

#endif

//==============================================================================
