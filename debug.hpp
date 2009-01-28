//==============================================================================

#ifndef ArseDebug
#define ArseDebug

#include <hge.h>
#include <Box2D.h>

class ViewPort;

//------------------------------------------------------------------------------
class DebugDraw: public b2DebugDraw
{
  public:
    DebugDraw( HGE * hge = 0, ViewPort * viewport = 0 );
    ~DebugDraw();
    virtual void DrawPolygon( const b2Vec2 * vertices, int32 vertexCount,
                              const b2Color & color );
    virtual void  DrawSolidPolygon( const b2Vec2 * vertices, int32 vertexCount,
                                    const b2Color & color );
    virtual void  DrawCircle( const b2Vec2 & center, float32 radius,
                              const b2Color & color );
    virtual void  DrawSolidCircle( const b2Vec2 & center, float32 radius,
                                   const b2Vec2 & axis, const b2Color & color );
    virtual void  DrawSegment( const b2Vec2 & p1, const b2Vec2 & p2,
                               const b2Color & color );
    virtual void  DrawXForm( const b2XForm & xf );

  private:
    DWORD _hgeColor( b2Color color );
    void _setVertex( hgeVertex & vertex, const b2Vec2 & vector,
                     const b2Color & color );

    HGE * m_hge;
    ViewPort * m_viewport;
};

#endif

//==============================================================================
