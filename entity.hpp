//==============================================================================

#ifndef ArseThing
#define ArseThing

#include <vector>

struct b2ContactPoint;
class hgeParticleSystem;
class hgeSprite;
class Engine;
class ViewPort;
class Bullet;
class Casing;
class Captain;
class Ship;

enum EntityType
{
    TYPE_BASE = 0,
    TYPE_SHIP = 1,
    TYPE_CAPTAIN = 2,
    TYPE_BULLET = 3,
    TYPE_CASING = 4,
    TYPE_URCHIN = 5,
    TYPE_COIN = 6,
    TYPE_SCENERY
};

//------------------------------------------------------------------------------
class ScoreData
{
    friend class Ship;
    friend class Captain;

  public:
    ScoreData();
    ~ScoreData();
    int getTime();
    int getUrchins();
    int getCoins();
    int getLives();

  protected:
    ScoreData( const ScoreData & );
    ScoreData & operator=( const ScoreData & );

  protected:
    float m_time;
    int m_urchins;
    int m_coins;
    int m_lives;
};

//------------------------------------------------------------------------------
class Entity
{
  public:
    Entity( Engine * engine = 0, float scale = 1.0f );
    virtual ~Entity();

    virtual void init() = 0;
    virtual void update( float dt ) = 0;
    virtual void render() = 0;
    virtual void collide( Entity * entity, b2ContactPoint * point ) = 0;

    void setType( EntityType type );
    EntityType getType();
    void explosion( const b2Vec2 & position, float scale );

  protected:
    Entity( const Entity & );
    Entity & operator=( const Entity & );

  protected:
    Engine * m_engine;
    float m_scale;
    EntityType m_type;
};

//------------------------------------------------------------------------------
class Damageable
{
  public:
    Damageable( float max_strength );
    virtual ~Damageable();

    void updateDamageable( float dt );
    void renderDamageable( const b2Vec2 & position, float scale );
    void addStrength( float amount );
    void takeDamage( float amount );
    bool isDestroyed();

  protected:
    Damageable( const Damageable & );
    Damageable & operator=( const Damageable & );

  private:
    float m_max_strength;
    float m_strength;
    float m_damage;
    float m_timer;
    hgeSprite * m_sprite;
};

//------------------------------------------------------------------------------
class Ship : public Entity, public Damageable
{
  public:
    Ship( Engine * engine = 0, float scale = 1.0f );
    virtual ~Ship();
    virtual void init();
    virtual void update( float dt );
    virtual void render();
    virtual void collide( Entity * entity, b2ContactPoint * point );
    void camera( ViewPort * viewport );
    void cameraAngle( ViewPort * viewport );
    void board();
    b2Body * getBody();
    ScoreData & getScoreData();
    void killed();

  protected:
    Ship( const Ship & );
    Ship & operator=( const Ship & );

  private:
    void _thrust( float magnitude );
    void _disembark();
    void _scale( ViewPort * viewport, float width, float height,
                 float x, float y );
    float _scale( float source, float target );
    void _createShape();
    void _teleport();
    void _spawn();

  private:
    b2Body * m_ship;
    b2Shape * m_shape;
    std::vector< Bullet * > m_bullets;
    std::vector< Casing * > m_casings;
    Captain * m_captain;
    HCHANNEL m_channel;
    ScoreData m_score_data;
};

//------------------------------------------------------------------------------
class Captain : public Entity, public Damageable
{
  public:
    Captain( Engine * engine = 0, float scale = 1.0f );
    virtual ~Captain();
    virtual void init();
    virtual void update( float dt );
    virtual void render();
    virtual void collide( Entity * entity, b2ContactPoint * point );
    void deferredRender();
    b2Body * getBody();
    void setShip( Ship * ship );
    void collect( const b2Vec2 & position, float scale );

  protected:
    Captain( const Captain & );
    Captain & operator=( const Captain & );

  private:
    void _embark();
    bool _isStanding();

  private:
    b2Body * m_body;
    std::vector< Bullet * > m_bullets;
    Ship * m_ship;
    bool m_boarding;
    int m_jump_count;
    int m_frame;
    float m_counter;
};

//------------------------------------------------------------------------------
class Bullet : public Entity
{
  public:
    Bullet( Engine * engine = 0, float scale = 1.0f );
    virtual ~Bullet();
    virtual void init();
    virtual void update( float dt );
    virtual void render();
    virtual void collide( Entity * entity, b2ContactPoint * point );
    b2Body * getBody();
    void setDamage( float damage );
    float getDamage();

  protected:
    Bullet( const Bullet & );
    Bullet & operator=( const Bullet & );

  private:
    b2Body * m_body;
    b2Shape * m_shape;
    float m_lifetime;
    hgeParticleSystem * m_trail;
    hgeParticleSystem * m_bang;
    float m_damage;
};

//------------------------------------------------------------------------------
class Casing : public Entity
{
  public:
    Casing( Engine * engine = 0, float scale = 1.0f );
    virtual ~Casing();
    virtual void init();
    virtual void update( float dt );
    virtual void render();
    virtual void collide( Entity * entity, b2ContactPoint * point );
    b2Body * getBody();

  protected:
    Casing( const Casing & );
    Casing & operator=( const Casing & );

  private:
    b2Body * m_body;
};

//------------------------------------------------------------------------------
class Coin : public Entity
{
  public:
    Coin( Engine * engine = 0, float scale = 1.0f );
    virtual ~Coin();
    virtual void init();
    virtual void update( float dt );
    virtual void render();
    virtual void collide( Entity * entity, b2ContactPoint * point );
    b2Body * getBody();
    bool isDestroyed();

  protected:
    Coin( const Coin & );
    Coin & operator=( const Coin & );

  private:
    b2Body * m_body;
    float m_lifetime;
};

//------------------------------------------------------------------------------
class Urchin : public Entity, public Damageable
{
  public:
    Urchin( Engine * engine = 0, float scale = 1.0f );
    virtual ~Urchin();
    virtual void init();
    virtual void update( float dt );
    virtual void render();
    virtual void collide( Entity * entity, b2ContactPoint * point );
    b2Body * getBody();
    float getScale();

  protected:
    Urchin( const Urchin & );
    Urchin & operator=( const Urchin & );

  private:
    b2Body * m_body;
    std::vector< Coin * > m_coins;
    int m_number;
};

//------------------------------------------------------------------------------
class Scenery : public Entity
{
  public:
    Scenery( Engine * engine = 0, float scale = 1.0f );
    virtual ~Scenery();
    virtual void init();
    virtual void update( float dt );
    virtual void render();
    virtual void collide( Entity * entity, b2ContactPoint * point );
    void init( HTEXTURE tex, b2World * b2d, float x = 0.0f, float y = 0.0f,
               float angle = 0.0f, int index = 0 );

  private:
    hgeSprite * m_sprite;
    b2Body * m_body;
    float m_scale;
    b2World * m_b2d;
};

//------------------------------------------------------------------------------
class Nothing
{
  public:
    Nothing( float scale = 1.0f );
    ~Nothing();
    void init( HTEXTURE tex, float x = 0.0f, float y = 0.0f, int index = 0 );
    void render( const ViewPort & viewport );

  private:
    hgeSprite * m_sprite;
    float m_scale;
    b2Vec2 m_position;
};

#endif

//==============================================================================
