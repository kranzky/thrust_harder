//==============================================================================

#include <hge.h>
#include <hgesprite.h>
#include <hgeanim.h>
#include <hgeparticle.h>
#include <hgeresource.h>
#include <Box2D.h>

#include <engine.hpp>
#include <entity.hpp>
#include <viewport.hpp>

//------------------------------------------------------------------------------

namespace
{
    const char * URCHIN[] =
    {
        "urchin_white",
        "urchin_blue",
        "urchin_brown",
        "urchin_pink",
        "urchin_red",
        "urchin_green",
        "urchin_yellow"
    };
    const char * FRAME[] =
    {
        "captain",
        "walk1",
        "captain",
        "walk2"
    };
};

//==============================================================================
ScoreData::ScoreData()
    :
    m_time( 999.0f ),
    m_urchins( 0 ),
    m_coins( 0 ),
    m_lives( 3 )
{
}

//------------------------------------------------------------------------------
ScoreData::~ScoreData()
{
}

//------------------------------------------------------------------------------
int
ScoreData::getTime()
{
    return static_cast<int>( m_time );
}

//------------------------------------------------------------------------------
int
ScoreData::getUrchins()
{
    return m_urchins;
}

//------------------------------------------------------------------------------
int
ScoreData::getCoins()
{
    return m_coins;
}

//------------------------------------------------------------------------------
int
ScoreData::getLives()
{
    return m_lives;
}

//==============================================================================
Entity::Entity( Engine * engine, float scale )
    :
    m_engine( engine ),
    m_scale( scale ),
    m_type( TYPE_BASE )
{
}

//------------------------------------------------------------------------------
Entity::~Entity()
{
}

//------------------------------------------------------------------------------
void
Entity::setType( EntityType type )
{
    m_type = type;
}

//------------------------------------------------------------------------------
EntityType
Entity::getType()
{
    return m_type;
}

//------------------------------------------------------------------------------
void
Entity::explosion( const b2Vec2 & position, float scale )
{
    HGE * hge( m_engine->getHGE() );
    hgeResourceManager * rm( m_engine->getResourceManager() );
    hgeParticleManager * pm( m_engine->getParticleManager() );
    hgeParticleSystem * bang =
        pm->SpawnPS( & rm->GetParticleSystem( "explode" )->info,
                     position.x / scale, position.y / scale );
    bang->SetScale( scale );
    hge->Effect_Play( rm->GetEffect( "explode" ) );
}

//==============================================================================
Damageable::Damageable( float strength )
    :
    m_max_strength( strength ),
    m_strength( strength ),
    m_damage( 0.0f ),
    m_timer( 0.0f )
{
    m_sprite = new hgeSprite( 0, 0, 0, 1, 1 );
}

//------------------------------------------------------------------------------
Damageable::~Damageable()
{
    delete m_sprite;
}

//------------------------------------------------------------------------------
void
Damageable::updateDamageable( float dt )
{
    m_timer -= dt;
    if ( m_timer < 0.0f )
    {
        m_timer = 0.0f;
    }

    if ( dt > 0.0f && m_damage > 0.0f )
    {
        float dd( m_damage * dt );
        m_strength -= dd;
        m_damage -= dd;
        if ( m_damage < 0.1f )
        {
            m_damage = 0.0f;
        }
        m_timer = 0.1f;
    }

    addStrength( 0.1f * dt );
}

//------------------------------------------------------------------------------
void
Damageable::renderDamageable( const b2Vec2 & position, float scale )
{
    if ( m_timer <= 0.0f )
    {
        return;
    }
    m_sprite->SetColor( 0xBB000000 );
    float width( 40.0f );
    float height( 4.0f );
    float x1( position.x - 0.5f * width * scale );
    float y1( position.y - 0.5f * height * scale - 20.0f * scale );
    float x2( position.x + 0.5f * width * scale );
    float y2( position.y + 0.5f * height * scale - 20.0f * scale );
    m_sprite->RenderStretch( x1, y1, x2, y2 );
    float ratio( m_strength / m_max_strength );
    m_sprite->SetColor( 0xBB000000 +
                        ( static_cast<DWORD>( ratio * 255.0f ) << 8 ) +
                        ( static_cast<DWORD>( (1.0f - ratio)*255.0f ) << 16 ) );
    x1 = position.x - 0.5f * width * scale;
    y1 = position.y - 0.5f * height * scale - 20.0f * scale;
    x2 = position.x - 0.5f * width * scale + 40.0f * ratio * scale;
    y2 = position.y + 0.5f * height * scale - 20.0f * scale;
    m_sprite->RenderStretch( x1, y1, x2, y2 );
}

//------------------------------------------------------------------------------
void
Damageable::addStrength( float amount )
{
    m_strength += amount;
    if ( m_strength > m_max_strength )
    {
        m_strength = m_max_strength;
        m_damage = 0.0f;
    }
}

//------------------------------------------------------------------------------
void
Damageable::takeDamage( float amount )
{
    if ( amount >= 0.1f )
    {
        m_damage += amount;
    }
}

//------------------------------------------------------------------------------
bool
Damageable::isDestroyed()
{
    return m_strength <= 0.0f;
}

//==============================================================================
Ship::Ship( Engine * engine, float scale )
    :
    Entity( engine, scale ),
    Damageable( 25.0f ),
    m_ship( 0 ),
    m_shape( 0 ),
    m_bullets(),
    m_casings(),
    m_captain( 0 ),
    m_channel( 0 ),
    m_score_data()
{
    setType( TYPE_SHIP );
}

//------------------------------------------------------------------------------
Ship::~Ship()
{
    HGE * hge( m_engine->getHGE() );

    std::vector< Bullet * >::iterator i;
    for ( i = m_bullets.begin(); i != m_bullets.end(); ++i )
    {
        delete * i;
    }
    std::vector< Casing * >::iterator j;
    for ( j = m_casings.begin(); j != m_casings.end(); ++j )
    {
        delete * j;
    }
    m_engine->getB2D()->DestroyBody( m_ship );
    delete m_captain;
    if ( m_channel != 0 )
    {
        hge->Channel_Stop( m_channel );
        m_channel = 0;
    }
}

//------------------------------------------------------------------------------
void
Ship::init()
{
    hgeResourceManager * rm( m_engine->getResourceManager() );
    b2BodyDef bodyDef;
    bodyDef.userData = static_cast< void * >( this );
    m_ship = m_engine->getB2D()->CreateDynamicBody( & bodyDef );
    _createShape();
    rm->GetParticleSystem( "engine" )->SetScale( m_scale );
    m_captain = 0;
    _spawn();
}

//------------------------------------------------------------------------------
void
Ship::update( float dt )
{
    HGE * hge( m_engine->getHGE() );
    hgeResourceManager * rm( m_engine->getResourceManager() );
    hgeParticleSystem * exhaust( rm->GetParticleSystem( "engine" ) );

    updateDamageable( dt );

    if ( isDestroyed() )
    {
        _teleport();
    }

    m_score_data.m_time -= 3.0f * dt;
    if ( m_score_data.m_time <= 0.0f )
    {
        m_score_data.m_lives = 0;
    }

    std::vector< Bullet * >::iterator i;
    for ( i = m_bullets.begin(); i != m_bullets.end(); )
    {
        ( * i )->update( dt );
        if ( ( * i )->getBody() == 0 )
        {
            i = m_bullets.erase( i );
        }
        else
        {
            ++i;
        }
    }
    std::vector< Casing * >::iterator j;
    for ( j = m_casings.begin(); j != m_casings.end(); ++j )
    {
        ( * j )->update( dt );
    }
    if ( m_captain != 0 )
    {
        m_captain->update( dt );
    }
    if ( exhaust->GetParticlesAlive() > 0 )
    {
        b2Vec2 position( m_ship->GetPosition() );
        b2Vec2 direction( 0.0f, -1.0f );
        direction = b2Mul( m_ship->GetXForm().R, direction );
        position = position - 16.0f * m_scale * direction;
        exhaust->MoveTo( position.x / m_scale, position.y / m_scale, true );
    }
    exhaust->Update( dt );
    hgeParticleSystem * teleport( rm->GetParticleSystem( "teleport" ) );
    teleport->Update( dt );

    if ( m_engine->isPaused() )
    {
        return;
    }

    if ( hge->Input_GetKeyState( HGEK_ESCAPE ) )
    {
        m_score_data.m_lives = 0;
    }

    if ( m_captain != 0 )
    {
        return;
    }

    if ( hge->Input_GetKeyState( HGEK_W ) )
    {
        m_ship->AllowSleeping( true );
        _thrust( 2.0f );
        exhaust->Fire();
    }
    if ( hge->Input_KeyDown( HGEK_W ) )
    {
        m_channel = hge->Effect_PlayEx( rm->GetEffect( "thrust" ),
                                        100, 0, 1, true );
    }
    else if ( hge->Input_KeyUp( HGEK_W ) )
    {
        exhaust->Stop();
        if ( m_channel != 0 )
        {
            hge->Channel_Stop( m_channel );
            m_channel = 0;
        }
    }
    if ( hge->Input_KeyDown( HGEK_A ) )
    {
        m_ship->SetAngularVelocity( -5.0f );
    }
    else if ( hge->Input_KeyDown( HGEK_D ) )
    {
        m_ship->SetAngularVelocity( 5.0f );
    }
    else if ( hge->Input_KeyUp( HGEK_A ) | hge->Input_KeyUp( HGEK_D ) )
    {
        m_ship->SetAngularVelocity( 0.0f );
    }
    if ( hge->Input_KeyDown( HGEK_LBUTTON ) )
    {
        b2Vec2 position( m_ship->GetPosition() );
        b2Vec2 nose( 0.0f, -16.0f * m_scale );
        b2Vec2 offset( 0.0f, -16.0f * m_scale * 0.2f );
        nose = b2Mul( m_ship->GetXForm().R, nose );
        offset = b2Mul( m_ship->GetXForm().R, offset );

//      m_casings.push_back( new Casing( m_engine, m_scale * 0.2f ) );
//      m_casings.back()->init();
        position = position + nose;
        position = position + offset;
/*
        b2Body * casing( m_casings.back()->getBody() );
        casing->SetXForm( position, m_ship->GetAngle() );
        casing->SetLinearVelocity( m_ship->GetLinearVelocity() );
        float spin( hge->Random_Float( 20.0f, 80.0f ) );
        if ( hge->Random_Int( 0, 1 ) == 0 )
        {
            spin *= -1.0f;
        }
        casing->SetAngularVelocity( spin );
        b2Vec2 velocity( hge->Random_Float( -1.0f, 1.0f ) * m_scale,
                         hge->Random_Float( -1.0f, 1.0f ) * m_scale );
        casing->SetLinearVelocity( velocity );
*/
        m_bullets.push_back( new Bullet( m_engine, m_scale * 0.2f ) );
        m_bullets.back()->init();
        m_bullets.back()->setDamage( 1.0f );
        position = position + 2.0f * offset;
        b2Body * bullet( m_bullets.back()->getBody() );
        bullet->SetXForm( position, m_ship->GetAngle() );
        bullet->SetLinearVelocity( m_ship->GetLinearVelocity() );
        b2Vec2 impulse( 0.0f, -0.03f );
        impulse = b2Mul( m_ship->GetXForm().R, impulse );
        bullet->ApplyImpulse( impulse, bullet->GetPosition() );
    }
    if ( m_ship->IsSleeping() && m_captain == 0 )
    {
        _disembark();
    }
}

//------------------------------------------------------------------------------
void
Ship::render()
{
    hgeResourceManager * rm( m_engine->getResourceManager() );
    hgeParticleSystem * exhaust( rm->GetParticleSystem( "engine" ) );
    hgeSprite * sprite( rm->GetSprite( "ship" ) );
    b2Vec2 position( m_ship->GetPosition() );
    float angle( m_ship->GetAngle() );
    sprite->RenderEx( position.x, position.y, angle, m_scale );
    if ( m_captain != 0 )
    {
        m_captain->deferredRender();
    }
    exhaust->info.fDirection = angle - M_PI;
    exhaust->Render();
    hgeParticleSystem * teleport( rm->GetParticleSystem( "teleport" ) );
    teleport->Render();
    renderDamageable( position, m_scale );
}

//------------------------------------------------------------------------------
void
Ship::collide( Entity * entity, b2ContactPoint * point )
{
    if ( entity->getType() == TYPE_BULLET )
    {
        takeDamage( static_cast< Bullet * >( entity )->getDamage() );
    }
    else
    {
        takeDamage( point->normalForce * 0.01f );
    }
}

//------------------------------------------------------------------------------
void
Ship::camera( ViewPort * viewport )
{
    if ( m_captain == 0 )
    {
        b2Vec2 position( m_ship->GetPosition() );
        _scale( viewport, 8.0f, 6.0f, position.x, position.y );
    }
    else
    {
        b2Vec2 position( m_captain->getBody()->GetPosition() );
        _scale( viewport, 0.8f, 0.6f, position.x, position.y );
    }
}

//------------------------------------------------------------------------------
void
Ship::cameraAngle( ViewPort * viewport )
{
    viewport->setAngle( m_ship->GetAngle() );
}

//------------------------------------------------------------------------------
void
Ship::board()
{
    if ( m_captain == 0 )
    {
        return;
    }
    delete m_captain;
    m_captain = 0;
    m_ship->WakeUp();
    m_ship->AllowSleeping( false );
}

//------------------------------------------------------------------------------
b2Body *
Ship::getBody()
{
    return m_ship;
}

//------------------------------------------------------------------------------
ScoreData &
Ship::getScoreData()
{
    return m_score_data;
}

//------------------------------------------------------------------------------
void
Ship::killed()
{
    m_score_data.m_urchins += 1;
}

//------------------------------------------------------------------------------
// private:
//------------------------------------------------------------------------------
void
Ship::_thrust( float magnitude )
{
    b2Vec2 direction( 0.0f, -1.0f );
    direction = b2Mul( m_ship->GetXForm().R, direction );
    m_ship->ApplyForce( magnitude * direction, m_ship->GetPosition() );
}

//------------------------------------------------------------------------------
void
Ship::_disembark()
{
    hgeResourceManager * rm( m_engine->getResourceManager() );
    m_captain = new Captain( m_engine, m_scale * 0.15f );
    m_captain->init();
    m_captain->getBody()->SetXForm( m_ship->GetPosition(), 0.0f );
    m_captain->setShip( this );
}

//------------------------------------------------------------------------------
void
Ship::_scale( ViewPort * viewport, float width, float height, float x, float y )
{
    viewport->bounds().x = _scale( viewport->bounds().x, width );
    viewport->bounds().y = _scale( viewport->bounds().y, height );
    viewport->offset().x = _scale( viewport->offset().x, x );
    viewport->offset().y = _scale( viewport->offset().y, y );
}

//------------------------------------------------------------------------------
float
Ship::_scale( float source, float target )
{
    float delta( target - source );
    return source + 0.01f * delta;
}

//------------------------------------------------------------------------------
void
Ship::_createShape()
{
    b2PolygonDef shapeDef;
    shapeDef.vertexCount = 5;
    shapeDef.vertices[0].Set( 0.0f, -16.0f * m_scale );
    shapeDef.vertices[1].Set( 15.0f * m_scale, 4.0f * m_scale );
    shapeDef.vertices[2].Set( 13.0f * m_scale, 16.0f * m_scale );
    shapeDef.vertices[3].Set( -13.0f * m_scale, 16.0f * m_scale );
    shapeDef.vertices[4].Set( -15.0f * m_scale, 4.0f * m_scale );
    shapeDef.density = 5.0f;
    shapeDef.friction = 0.3f;
    shapeDef.restitution = 0.4f;
    shapeDef.groupIndex = -1;
    m_shape = m_ship->CreateShape( & shapeDef );
    m_ship->SetMassFromShapes();
}

//------------------------------------------------------------------------------
void
Ship::_teleport()
{
    HGE * hge( m_engine->getHGE() );
    hgeResourceManager * rm( m_engine->getResourceManager() );
    hgeParticleManager * pm( m_engine->getParticleManager() );
    m_score_data.m_lives -= 1;
    b2Vec2 position( m_ship->GetPosition() );
    hgeParticleSystem * bang =
        pm->SpawnPS( & rm->GetParticleSystem( "teleport" )->info,
                     position.x / m_scale, position.y / m_scale );
    bang->SetScale( m_scale );
    hge->Effect_Play( rm->GetEffect( "teleport" ) );
    if ( m_score_data.m_lives <= 0 )
    {
        return;
    }
    addStrength( 30.0f );
    position.x = 0.0f;
    position.y = 0.0f;
    m_ship->SetLinearVelocity( position );
    m_ship->SetAngularVelocity( 0.0f );
    _spawn();
}

//------------------------------------------------------------------------------
void
Ship::_spawn()
{
    HGE * hge( m_engine->getHGE() );
    hgeResourceManager * rm( m_engine->getResourceManager() );
    hgeParticleManager * pm( m_engine->getParticleManager() );
    b2Vec2 position( 0.0f, 28.0f * m_scale );
    m_ship->SetXForm( position, 0.0f );
    hgeParticleSystem * bang =
        pm->SpawnPS( & rm->GetParticleSystem( "spawn" )->info,
                     position.x / m_scale, position.y / m_scale );
    bang->SetScale( m_scale );
    hge->Effect_Play( rm->GetEffect( "spawn" ) );
    m_ship->AllowSleeping( false );
}

//==============================================================================
Captain::Captain( Engine * engine, float scale )
    :
    Entity( engine, scale ),
    Damageable( 10.0f ),
    m_body( 0 ),
    m_bullets(),
    m_ship( 0 ),
    m_boarding( false ),
    m_jump_count( 0 ),
    m_frame( 0 ),
    m_counter( 0.0f )
{
    setType( TYPE_CAPTAIN );
}

//------------------------------------------------------------------------------
Captain::~Captain()
{
    m_engine->hideMouse();
    std::vector< Bullet * >::iterator i;
    for ( i = m_bullets.begin(); i != m_bullets.end(); ++i )
    {
        delete * i;
    }
    m_engine->getB2D()->DestroyBody( m_body );
}

//------------------------------------------------------------------------------
void
Captain::init()
{
    m_jump_count = 0;
    HGE * hge( m_engine->getHGE() );
    hgeResourceManager * rm( m_engine->getResourceManager() );
    b2BodyDef bodyDef;
    bodyDef.userData = static_cast< void * >( this );
    m_body = m_engine->getB2D()->CreateDynamicBody( & bodyDef );
    b2PolygonDef shapeDef;
    shapeDef.vertexCount = 6;
    shapeDef.vertices[0].Set( -6.0f * m_scale, -16.0f * m_scale );
    shapeDef.vertices[1].Set( 6.0f * m_scale, -16.0f * m_scale );
    shapeDef.vertices[2].Set( 14.0f * m_scale, -7.0f * m_scale );
    shapeDef.vertices[3].Set( 9.0f * m_scale, 2.0f * m_scale );
    shapeDef.vertices[4].Set( -9.0f * m_scale, 2.0f * m_scale );
    shapeDef.vertices[5].Set( -14.0f * m_scale, -7.0f * m_scale );
    shapeDef.density = 5.0f;
    shapeDef.friction = 0.3f;
    shapeDef.restitution = 0.4f;
    shapeDef.groupIndex = -1;
    m_body->CreateShape( & shapeDef );
    shapeDef.vertexCount = 4;
    shapeDef.vertices[0].Set( 9.0f * m_scale, 2.0f * m_scale );
    shapeDef.vertices[1].Set( 5.0f * m_scale, 16.0f * m_scale );
    shapeDef.vertices[2].Set( -5.0f * m_scale, 16.0f * m_scale );
    shapeDef.vertices[3].Set( -9.0f * m_scale, 2.0f * m_scale );
    m_body->CreateShape( & shapeDef );
    m_body->SetMassFromShapes();
    hgeParticleSystem * spawn( rm->GetParticleSystem( "spawn" ) );
    spawn->SetScale( m_scale );
    spawn->Fire();
    hge->Effect_Play( rm->GetEffect( "spawn" ) );
    m_engine->showMouse();
    m_engine->setMouse( "target" );
}

//------------------------------------------------------------------------------
void
Captain::update( float dt )
{
    HGE * hge( m_engine->getHGE() );
    hgeResourceManager * rm( m_engine->getResourceManager() );

    updateDamageable( dt );

    if ( isDestroyed() )
    {
        _embark();
        return;
    }

    std::vector< Bullet * >::iterator i;
    for ( i = m_bullets.begin(); i != m_bullets.end(); ++i )
    {
        ( * i )->update( dt );
    }

    hgeParticleSystem * spawn( rm->GetParticleSystem( "spawn" ) );
    b2Vec2 position( m_body->GetPosition() );
    spawn->MoveTo( position.x / m_scale, position.y / m_scale, false );
    spawn->Update( dt );

    float speed( abs( m_body->GetLinearVelocity().x ) );
    if ( speed < 0.1f )
    {
        m_frame = 0;
        m_counter = 0.0f;
    }
    else
    {
        m_counter += dt;
        if ( m_counter * speed > 0.06f )
        {
            m_frame += 1;
            m_frame %= 4;
            m_counter = 0.0f;
        }
    }

    if ( m_engine->isPaused() )
    {
        return;
    }

    bool isStanding( _isStanding() );
    if ( isStanding )
    {
        m_jump_count = 0;
    }

    if ( hge->Input_KeyDown( HGEK_S ) )
    {
        if ( isStanding )
        {
            _embark();
            return;
        }
    }
    if ( hge->Input_KeyDown( HGEK_W ) )
    {
        if ( m_jump_count < 2 )
        {
            ++m_jump_count;
            b2Vec2 velocity( m_body->GetLinearVelocity() );
            velocity.y = 0.0f;
            m_body->SetLinearVelocity( velocity );
            b2Vec2 force( 0.0f, -0.002f );
            m_body->ApplyImpulse( force, m_body->GetPosition() );
        }
    }
    if ( hge->Input_GetKeyState( HGEK_A ) )
    {
        if ( m_body->GetLinearVelocity().x > -0.5f )
        {
            b2Vec2 force( -0.01f, 0.0f );
            m_body->ApplyForce( force, m_body->GetPosition() );
        }
    }
    if ( hge->Input_GetKeyState( HGEK_D ) )
    {
        if ( m_body->GetLinearVelocity().x < 0.5f )
        {
            b2Vec2 force( 0.01f, 0.0f );
            m_body->ApplyForce( force, m_body->GetPosition() );
        }
    }
    bool canShoot( b2DistanceSquared( m_ship->getBody()->GetPosition(),
                                      m_body->GetPosition() ) > 0.05f );
    if ( canShoot )
    {
        m_engine->setMouse( "target" );
    }
    else
    {
        m_engine->setMouse( "busy" );
    }
    if ( hge->Input_KeyDown( HGEK_LBUTTON ) && canShoot )
    {
        b2Vec2 position( m_body->GetPosition() );
        b2Vec2 target( 0.0f, 0.0f );
        hge->Input_GetMousePos( & target.x, & target.y );
        m_engine->getViewPort()->screenToWorld( target ); 
        m_bullets.push_back( new Bullet( m_engine, m_scale * 0.2f ) );
        m_bullets.back()->init();
        m_bullets.back()->setDamage( 0.3f );
        b2Body * bullet( m_bullets.back()->getBody() );
        b2Vec2 offset( target - position );
        b2Vec2 vertical( 0.0f, -1.0f );
        offset.Normalize();
        float cross( b2Cross( offset, vertical ) );
        float angle( 0.0f );
        if ( cross > 0.0f )
        {
            angle = acosf( b2Dot( offset, -1.0f * vertical ) ) - M_PI;
        }
        else
        {
            angle = acosf( b2Dot( offset, vertical ) );
        }
        hge->System_Log( "Shoot %f", cross );
        bullet->SetXForm( position + ( 20.0f * m_scale ) * offset, angle );
        bullet->SetLinearVelocity( m_body->GetLinearVelocity() );
        bullet->ApplyImpulse( 0.0001f * offset, bullet->GetPosition() );
    }
    if ( abs( m_body->GetLinearVelocity().x ) < 0.4f )
    {
        m_body->SetAngularVelocity( -4.0f * m_body->GetAngle() );
    }
}

//------------------------------------------------------------------------------
void
Captain::render()
{
}

//------------------------------------------------------------------------------
void
Captain::collide( Entity * entity, b2ContactPoint * point )
{
    if ( entity->getType() == TYPE_BULLET )
    {
        takeDamage( static_cast< Bullet * >( entity )->getDamage() );
    }
    else
    {
        takeDamage( point->normalForce * 0.1f );
    }
}

//------------------------------------------------------------------------------
void
Captain::deferredRender()
{
    hgeResourceManager * rm( m_engine->getResourceManager() );
    hgeSprite * sprite( rm->GetSprite( FRAME[m_frame] ) );
    if ( abs( m_body->GetAngle() ) > 0.1f )
    {
        sprite = rm->GetSprite( "duck" );
        sprite->SetFlip( m_body->GetAngle() < 0.0f, false );
    }
    else if ( abs( m_body->GetLinearVelocity().x ) > 0.02f )
    {
        if ( ! _isStanding( ) )
        {
            sprite = rm->GetSprite( "duck" );
        }
        sprite->SetFlip( m_body->GetLinearVelocity().x > 0.0f, false );
    }
    b2Vec2 position( m_body->GetPosition() );
    float angle( m_body->GetAngle() );
    sprite->RenderEx( position.x, position.y, angle, m_scale );
    hgeParticleSystem * spawn( rm->GetParticleSystem( "spawn" ) );
    spawn->Render();
    renderDamageable( position, m_scale );
}

//------------------------------------------------------------------------------
b2Body *
Captain::getBody()
{
    return m_body;
}

//------------------------------------------------------------------------------
void
Captain::setShip( Ship * ship )
{
    m_ship = ship;
}

//------------------------------------------------------------------------------
void
Captain::collect( const b2Vec2 & position, float scale )
{
    HGE * hge( m_engine->getHGE() );
    hgeResourceManager * rm( m_engine->getResourceManager() );
    hgeParticleManager * pm( m_engine->getParticleManager() );
    hgeParticleSystem * bang =
        pm->SpawnPS( & rm->GetParticleSystem( "collect" )->info,
                     position.x / scale, position.y / scale );
    bang->SetScale( scale );
    hge->Effect_Play( rm->GetEffect( "collect" ) );
    m_ship->getScoreData().m_coins += 1;
}

//------------------------------------------------------------------------------
// private:
//------------------------------------------------------------------------------
void
Captain::_embark()
{
    hgeResourceManager * rm( m_engine->getResourceManager() );
    hgeParticleSystem * teleport( rm->GetParticleSystem( "teleport" ) );
    teleport->SetScale( m_scale );
    b2Vec2 position( m_body->GetPosition() );
    teleport->MoveTo( position.x / m_scale, position.y / m_scale, false );
    teleport->Fire();
    HGE * hge( m_engine->getHGE() );
    hge->Effect_Play( rm->GetEffect( "teleport" ) );
    m_ship->board();
}

//------------------------------------------------------------------------------
bool
Captain::_isStanding()
{
    return m_body->GetContactList() != 0;
}

//==============================================================================
Bullet::Bullet( Engine * engine, float scale )
    :
    Entity( engine, scale ),
    m_body( 0 ),
    m_shape( 0 ),
    m_lifetime( 0.0f ),
    m_trail( 0 ),
    m_bang( 0 ),
    m_damage( 0.0f )
{
    setType( TYPE_BULLET );
}

//------------------------------------------------------------------------------
Bullet::~Bullet()
{
    if ( m_body != 0 )
    {
        m_engine->getB2D()->DestroyBody( m_body );
    }
    delete m_trail;
    delete m_bang;
}

//------------------------------------------------------------------------------
void
Bullet::init()
{
    hgeResourceManager * rm( m_engine->getResourceManager() );
    b2BodyDef bodyDef;
    bodyDef.userData = static_cast< void * >( this );
    m_body = m_engine->getB2D()->CreateDynamicBody( & bodyDef );
    b2PolygonDef shapeDef;
    shapeDef.vertexCount = 5;
    shapeDef.vertices[0].Set( 0.0f, -14.0f * m_scale );
    shapeDef.vertices[1].Set( 11.0f * m_scale, -5.0f * m_scale );
    shapeDef.vertices[2].Set( 11.0f * m_scale, 13.0f * m_scale );
    shapeDef.vertices[3].Set( -11.0f * m_scale, 13.0f * m_scale );
    shapeDef.vertices[4].Set( -11.0f * m_scale, -5.0f * m_scale );
    shapeDef.density = 2.0f;
    shapeDef.friction = 0.3f;
    shapeDef.restitution = 0.4f;
    m_shape = m_body->CreateShape( & shapeDef );
    m_body->SetMassFromShapes();
    m_trail = new hgeParticleSystem( * rm->GetParticleSystem( "bullet" ) );
    m_trail->SetScale( m_scale );
    m_trail->Fire();
    m_bang = new hgeParticleSystem( * rm->GetParticleSystem( "explode" ) );
    m_bang->SetScale( m_scale );
    HGE * hge( m_engine->getHGE() );
    hge->Effect_Play( rm->GetEffect( "shoot" ) );
}

//------------------------------------------------------------------------------
void
Bullet::update( float dt )
{
    HGE * hge( m_engine->getHGE() );
    hgeResourceManager * rm( m_engine->getResourceManager() );

    m_lifetime += dt;
    if ( m_shape != 0 && m_lifetime > 1.0f )
    {
        HGE * hge( m_engine->getHGE() );
        m_bang->Fire();
        if ( m_trail != 0 )
        {
            m_trail->Stop();
        }
        m_body->DestroyShape( m_shape );
        m_shape = 0;
    }
    if ( m_body != 0 )
    {
        b2Vec2 position( m_body->GetPosition() );
        if ( m_lifetime < 100.0f )
        {
            m_bang->MoveTo( position.x / m_scale, position.y / m_scale, true );
        }
        if ( m_trail != 0 )
        {
            b2Vec2 direction( 0.0f, -1.0f );
            direction = b2Mul( m_body->GetXForm().R, direction );
            position = position - 16.0f * m_scale * direction;
            m_trail->MoveTo( position.x / m_scale, position.y / m_scale, true );
        }
    }
    if ( m_trail != 0 )
    {
        m_trail->Update( dt );
        m_bang->Update( dt );
        if ( m_shape == 0 && m_bang->GetAge() == -2.0f &&
             m_bang->GetParticlesAlive() == 0 )
        {
            delete m_trail;
            m_trail = 0;
	        m_engine->getB2D()->DestroyBody( m_body );
	        m_body = 0;
        }
    }
}

//------------------------------------------------------------------------------
void
Bullet::render()
{
    hgeResourceManager * rm( m_engine->getResourceManager() );

    if ( m_body != 0 && m_trail != 0 )
    {
        b2Vec2 position( m_body->GetPosition() );
        float angle( m_body->GetAngle() );
        if ( m_shape != 0 )
        {
            hgeSprite * sprite( rm->GetSprite( "bullet" ) );
            sprite->RenderEx( position.x, position.y, angle, m_scale );
        }
        m_trail->info.fDirection = angle - M_PI;
    }
    if ( m_trail != 0 && m_lifetime < 100.0f )
    {
        m_trail->Render();
    }
    m_bang->Render();
}

//------------------------------------------------------------------------------
void
Bullet::collide( Entity * entity, b2ContactPoint * point )
{
    HGE * hge( m_engine->getHGE() );
    hgeResourceManager * rm( m_engine->getResourceManager() );
    hge->Effect_Play( rm->GetEffect( "pop" ) );
    m_lifetime += 200.0f;
}

//------------------------------------------------------------------------------
b2Body *
Bullet::getBody()
{
    return m_body;
}

//------------------------------------------------------------------------------
void
Bullet::setDamage( float damage )
{
    m_damage = damage;
}

//------------------------------------------------------------------------------
float
Bullet::getDamage()
{
    return m_damage;
}

//==============================================================================
Casing::Casing( Engine * engine, float scale )
    :
    Entity( engine, scale ),
    m_body( 0 )
{
    setType( TYPE_CASING );
}

//------------------------------------------------------------------------------
Casing::~Casing()
{
    m_engine->getB2D()->DestroyBody( m_body );
}

//------------------------------------------------------------------------------
void
Casing::init()
{
    hgeResourceManager * rm( m_engine->getResourceManager() );
    b2BodyDef bodyDef;
    bodyDef.userData = static_cast< void * >( this );
    m_body = m_engine->getB2D()->CreateDynamicBody( & bodyDef );
    b2PolygonDef shapeDef;
    shapeDef.vertexCount = 4;
    shapeDef.vertices[0].Set( 12.0f * m_scale, -11.0f * m_scale );
    shapeDef.vertices[1].Set( 12.0f * m_scale, 11.0f * m_scale );
    shapeDef.vertices[2].Set( -12.0f * m_scale, 11.0f * m_scale );
    shapeDef.vertices[3].Set( -12.0f * m_scale, -11.0f * m_scale );
    shapeDef.density = 1.0f;
    shapeDef.friction = 0.3f;
    shapeDef.restitution = 0.4f;
    m_body->CreateShape( & shapeDef );
    m_body->SetMassFromShapes();
}

//------------------------------------------------------------------------------
void
Casing::update( float dt )
{
}

//------------------------------------------------------------------------------
void
Casing::render()
{
    hgeResourceManager * rm( m_engine->getResourceManager() );
    b2Vec2 position( m_body->GetPosition() );
    float angle( m_body->GetAngle() );
    hgeSprite * sprite( rm->GetSprite( "casing" ) );
    sprite->RenderEx( position.x, position.y, angle, m_scale );
}

//------------------------------------------------------------------------------
void
Casing::collide( Entity * entity, b2ContactPoint * point )
{
    if ( point->normalForce < 0.1f )
    {
        return;
    }
    HGE * hge( m_engine->getHGE() );
    hgeResourceManager * rm( m_engine->getResourceManager() );
    hge->Effect_Play( rm->GetEffect( "bounce" ) );
}

//------------------------------------------------------------------------------
b2Body *
Casing::getBody()
{
    return m_body;
}

//==============================================================================
Coin::Coin( Engine * engine, float scale )
    :
    Entity( engine, scale ),
    m_body( 0 ),
    m_lifetime( 5.0f )
{
    setType( TYPE_COIN );
}

//------------------------------------------------------------------------------
Coin::~Coin()
{
    m_engine->getB2D()->DestroyBody( m_body );
}

//------------------------------------------------------------------------------
void
Coin::init()
{
    hgeResourceManager * rm( m_engine->getResourceManager() );
    b2BodyDef bodyDef;
    bodyDef.userData = static_cast< void * >( this );
    m_body = m_engine->getB2D()->CreateDynamicBody( & bodyDef );
    b2CircleDef shapeDef;
    shapeDef.radius = 16.0f * m_scale;
    shapeDef.density = 1.0f;
    shapeDef.friction = 0.3f;
    shapeDef.restitution = 0.4f;
    m_body->CreateShape( & shapeDef );
    m_body->SetMassFromShapes();
}

//------------------------------------------------------------------------------
void
Coin::update( float dt )
{
    m_lifetime -= dt;
}

//------------------------------------------------------------------------------
void
Coin::render()
{
    if ( m_lifetime <= 0.0f )
    {
        return;
    }
    hgeResourceManager * rm( m_engine->getResourceManager() );
    b2Vec2 position( m_body->GetPosition() );
    float angle( m_body->GetAngle() );
    hgeSprite * sprite( rm->GetSprite( "coin" ) );
    if ( m_lifetime < 1.0f )
    {
        sprite->SetColor( ( static_cast<DWORD>( m_lifetime * 255.0f ) << 24 ) +
                          0x00FFFFFF );
    }
    else
    {
        sprite->SetColor( 0xFFFFFFFF );
    }
    sprite->RenderEx( position.x, position.y, angle, m_scale );
}

//------------------------------------------------------------------------------
void
Coin::collide( Entity * entity, b2ContactPoint * point )
{
    if ( entity->getType() == TYPE_CAPTAIN )
    {
        m_lifetime = 0.0f;
        static_cast< Captain * >( entity )->collect( m_body->GetPosition(),
                                                     m_scale );
        return;
    }
    if ( point->normalForce < 0.01f )
    {
        return;
    }
    HGE * hge( m_engine->getHGE() );
    hgeResourceManager * rm( m_engine->getResourceManager() );
    hge->Effect_Play( rm->GetEffect( "bounce" ) );
}

//------------------------------------------------------------------------------
b2Body *
Coin::getBody()
{
    return m_body;
}

//------------------------------------------------------------------------------
bool
Coin::isDestroyed()
{
    return m_lifetime <= 0.0f;
}

//==============================================================================
Urchin::Urchin( Engine * engine, float scale )
    :
    Entity( engine, scale ),
    Damageable( scale * 500.0f ),
    m_body( 0 ),
    m_number( engine->getHGE()->Random_Int( 0, 6 ) )
{
    setType( TYPE_URCHIN );
}

//------------------------------------------------------------------------------
Urchin::~Urchin()
{
    m_engine->getB2D()->DestroyBody( m_body );
}

//------------------------------------------------------------------------------
void
Urchin::init()
{
    hgeResourceManager * rm( m_engine->getResourceManager() );
    b2BodyDef bodyDef;
    bodyDef.userData = static_cast< void * >( this );
    m_body = m_engine->getB2D()->CreateDynamicBody( & bodyDef );
    b2PolygonDef shapeDef;
    shapeDef.vertexCount = 5;
    shapeDef.vertices[0].Set( -3.0f * m_scale, -16.0f * m_scale );
    shapeDef.vertices[1].Set( 14.0f * m_scale, -9.0f * m_scale );
    shapeDef.vertices[2].Set( 14.0f * m_scale, 9.0f * m_scale );
    shapeDef.vertices[3].Set( -5.0f * m_scale, 15.0f * m_scale );
    shapeDef.vertices[4].Set( -16.0f * m_scale, -1.0f * m_scale );
    shapeDef.density = 1.0f;
    shapeDef.friction = 0.3f;
    shapeDef.restitution = 0.4f;
    m_body->CreateShape( & shapeDef );
    m_body->SetMassFromShapes();
}

//------------------------------------------------------------------------------
void
Urchin::update( float dt )
{
    updateDamageable( dt );
    if ( isDestroyed() )
    {
        explosion( m_body->GetPosition(), m_scale );
        b2ContactEdge * edge( m_body->GetContactList() );
        while ( edge != 0 )
        {
            b2Body * body( edge->other );
            b2Vec2 offset( body->GetPosition() - m_body->GetPosition() );
            offset.Normalize();
            offset = m_scale * 0.3f * offset;
            body->ApplyImpulse( offset, body->GetPosition() );
            float torque( m_scale * 2.0f );
            body->ApplyTorque( m_engine->getHGE()->Random_Float( -torque,
                                                                  torque ) );
            Entity * entity( static_cast<Entity *>( body->GetUserData() ) );
            float amount( m_scale * 200.0f );
            switch ( entity->getType() )
            {
                case TYPE_SHIP:
                {
                    static_cast<Ship *>( entity )->takeDamage( amount );
                    break;
                }
                case TYPE_CAPTAIN:
                {
                    static_cast<Captain *>( entity )->takeDamage( amount );
                    break;
                }
                case TYPE_URCHIN:
                {
                    static_cast<Urchin *>( entity )->takeDamage( amount );
                    break;
                }
              default:
                {
                    break;
                }
            }
            edge = edge->next;
        }
    }
}

//------------------------------------------------------------------------------
void
Urchin::render()
{
    hgeResourceManager * rm( m_engine->getResourceManager() );
    b2Vec2 position( m_body->GetPosition() );
    float angle( m_body->GetAngle() );
    hgeSprite * sprite( rm->GetSprite( URCHIN[m_number] ) );
    sprite->RenderEx( position.x, position.y, angle, m_scale );
    renderDamageable( position, m_scale );
}

//------------------------------------------------------------------------------
void
Urchin::collide( Entity * entity, b2ContactPoint * point )
{
    if ( entity->getType() == TYPE_BULLET )
    {
        takeDamage( static_cast< Bullet * >( entity )->getDamage() );
    }
}

//------------------------------------------------------------------------------
b2Body *
Urchin::getBody()
{
    return m_body;
}

//------------------------------------------------------------------------------
float
Urchin::getScale()
{
    return m_scale;
}

//==============================================================================
Scenery::Scenery( Engine * engine, float scale )
    :
    Entity( engine, scale ),
    m_sprite( 0 ),
    m_body( 0 ),
    m_scale( scale )
{
    setType( TYPE_SCENERY );
}

//------------------------------------------------------------------------------
Scenery::~Scenery()
{
    delete m_sprite;
    m_engine->getB2D()->DestroyBody( m_body );
}

//------------------------------------------------------------------------------
void Scenery::init()
{
}

//------------------------------------------------------------------------------
void
Scenery::update( float dt )
{
}

//------------------------------------------------------------------------------
void
Scenery::render()
{
    b2Vec2 position( m_body->GetPosition() );
    float angle( m_body->GetAngle() );
    m_sprite->RenderEx( position.x, position.y, angle, m_scale );
}

//------------------------------------------------------------------------------
void
Scenery::collide( Entity * entity, b2ContactPoint * point )
{
}

//------------------------------------------------------------------------------
void
Scenery::init( HTEXTURE tex, b2World * b2d, float x, float y,
               float angle, int index )
{
    float size( 16.0f );
    int width( 10 );
    int height( 12 );
    float i( 0.0f );
    float j( 0.0f );
    if ( index > 0 )
    {
        i = size * ( index % width );
        j = size * ( index / width );
    }
    m_sprite = new hgeSprite( tex, i, j, size, size );
    m_sprite->SetHotSpot( 0.5f * size, 0.5f * size );
    b2BodyDef bodyDef;
    bodyDef.position.Set( x, y );
    bodyDef.angle = angle;
    bodyDef.userData = static_cast< void * >( this );
    m_body = b2d->CreateStaticBody( & bodyDef );
    b2PolygonDef shapeDef;
    shapeDef.SetAsBox( 0.5f * size * m_scale, 0.5f * size * m_scale );
    m_body->CreateShape( & shapeDef );
}

//==============================================================================
Nothing::Nothing( float scale )
    :
    m_sprite( 0 ),
    m_scale( scale ),
    m_position()
{
}

//------------------------------------------------------------------------------
Nothing::~Nothing()
{
    delete m_sprite;
}

//------------------------------------------------------------------------------
void
Nothing::init( HTEXTURE tex, float x, float y, int index )
{
    float size( 16.0f );
    int width( 10 );
    int height( 12 );
    float i( 0.0f );
    float j( 0.0f );
    if ( index > 0 )
    {
        i = size * ( index % width );
        j = size * ( index / width );
    }
    m_sprite = new hgeSprite( tex, i, j, size, size );
    m_sprite->SetHotSpot( 0.5f * size, 0.5f * size );
    m_position.x = x;
    m_position.y = y;
}

//------------------------------------------------------------------------------
void
Nothing::render( const ViewPort & viewport )
{
    m_sprite->RenderEx( m_position.x, m_position.y, 0.0f, m_scale );
}

//==============================================================================
