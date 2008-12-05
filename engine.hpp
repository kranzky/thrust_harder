//==============================================================================

#ifndef ArseEngine
#define ArseEngine

#include <vector>

#include <hge.h>
#include <Box2D.h>

#include <viewport.hpp>

class hgeResourceManager;
class hgeParticleManager;
class hgeSprite;

class DebugDraw;
class Context;

//------------------------------------------------------------------------------
enum EngineState
{
    STATE_NONE = -1, 
    STATE_SPLASH = 0,
    STATE_MENU = 1,
    STATE_GAME = 2,
    STATE_SCORE = 3,
    STATE_EDITOR = 4,
    STATE_INSTRUCTIONS = 5
};

//------------------------------------------------------------------------------
class Engine : public b2BoundaryListener, public b2ContactListener
{
  public:
    static Engine * instance();

  private:
    static bool s_update();
    static bool s_render();
    
  protected:
    Engine();
    Engine( const Engine & );
    Engine & operator=( const Engine & );
    ~Engine();

  public:
    HGE * getHGE();
    b2World * getB2D();
    ViewPort * getViewPort();
    hgeResourceManager * getResourceManager();
    hgeParticleManager * getParticleManager();
    bool isPaused();
    bool isDebug();
    void error( const char * format, ... );
    void start();
    void switchContext( EngineState state );
    Context * getContext();
    void setColour( DWORD colour );
    void showMouse();
    void setMouse( const char * name );
    void hideMouse();
    virtual void Violation( b2Body * body );
    virtual void Add( b2ContactPoint * point );
    virtual void Persist( b2ContactPoint * point );
    virtual void Remove( b2ContactPoint * point );

  private:
    bool _update();
    void _pauseOverlay();
    bool _render();
    void _debugControls( float dt );
    void _initGraphics();
    void _initPhysics();
    void _loadData();

  private:
    static Engine * s_instance;
    hgeResourceManager * m_resource_manager;
    hgeParticleManager * m_particle_manager;
    HGE * m_hge;
    b2World * m_b2d;
    ViewPort m_viewport;
    DWORD m_colour;
    DebugDraw * m_debugDraw;
    hgeSprite * m_overlay;
    std::vector< Context * > m_contexts;
    EngineState m_state;
    bool m_paused;
    bool m_running;
    bool m_mouse;
    hgeSprite * m_mouse_sprite;
};

#endif

//==============================================================================
