//
//  BaseDemo.cpp
//  PhysicsTest
//
//  Created by ChildhoodAndy on 14-3-9.
//
//

#include "BaseDemo.h"
#include "VisibleRect.h"
#include "ContentLayer.h"

USING_NS_CC;

#define GRID_SPACE 30
static const int DRAG_BODYS_TAG = 0x80;
bool BaseDemo::init()
{
    if(!Layer::init())
    {
        return false;
    }
    
    Size size = VisibleRect::getVisibleRect().size;
    _bg = LayerColor::create(Color4B(125, 125, 125, 255), size.width, size.height);
    this->addChild(_bg);
    
    auto bg_grid = DrawNode::create();
    int i = 0;
    for (i = 0; i < size.height; i += GRID_SPACE) {
        bg_grid->drawSegment(Point(0, i), Point(size.width, i), 0.5, Color4F(0, 0, 0, 0.4));
    }
    
    for (i = 0; i < size.width; i += GRID_SPACE) {
        bg_grid->drawSegment(Point(i, 0), Point(i, size.height), 0.5, Color4F(0, 0, 0, 0.4));
    }
    this->addChild(bg_grid);
    
    //加入封闭的盒子，用作墙壁
    auto body = PhysicsBody::createEdgeBox(size, PHYSICSBODY_MATERIAL_DEFAULT, 3);
    _wallNode = Node::create();
    body->setGroup(1);
    _wallNode->setPosition(size.width/2, size.height/2);
    _wallNode->setPhysicsBody(body);
    this->addChild(_wallNode);
    
    auto backMenuItem = MenuItemImage::create("backNormal.png", "backSelected.png", CC_CALLBACK_1(BaseDemo::backCallback, this));
    auto restartMenuItem = MenuItemImage::create("refreshNormal.png", "refreshSelected.png", CC_CALLBACK_1(BaseDemo::restartCallback, this));
    
    MenuItemFont::setFontSize(18);
    auto toggleDebugMenuItem = MenuItemFont::create("Toggle debug", CC_CALLBACK_1(BaseDemo::toggleDebugCallback, this));
    
    restartMenuItem->setPosition(VisibleRect::rightTop() + Point(-200, -80));
    backMenuItem->setPosition(restartMenuItem->getPosition() + Point(90, 0));
    toggleDebugMenuItem->setPosition(Point(VisibleRect::right().x-50, VisibleRect::top().y-10));
    auto menu = Menu::create(backMenuItem, restartMenuItem, toggleDebugMenuItem, NULL);
    menu->setPosition(Point::ZERO);
    this->addChild(menu, 300);
    
    auto touchListener = EventListenerTouchOneByOne::create();
    touchListener->onTouchBegan = CC_CALLBACK_2(BaseDemo::onTouchBegan, this);
    touchListener->onTouchMoved = CC_CALLBACK_2(BaseDemo::onTouchMoved, this);
    touchListener->onTouchEnded = CC_CALLBACK_2(BaseDemo::onTouchEnded, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);
    
    return true;
}

void BaseDemo::onEnter()
{
    Layer::onEnter();
    
    auto demo_info_label = Label::create(demo_info(), "Helvetica", 25, Size(1000, 200), TextHAlignment::LEFT);
    this->addChild(demo_info_label);
    demo_info_label->setAnchorPoint(Point(0.0f, 1.0f));
    demo_info_label->setColor(Color3B(255, 255, 255));
    demo_info_label->setPosition(VisibleRect::leftTop() + Point(60, -60));
}

bool BaseDemo::onTouchBegan(Touch* touch, Event* event)
{
    auto location = touch->getLocation();
    auto shapeArr = _world->getShapes(location);
    
    PhysicsBody* body = nullptr;
    for(auto& obj : shapeArr)
    {
        if((obj->getBody()->getTag() & DRAG_BODYS_TAG) != 0)
        {
            body = obj->getBody();
            break;
        }
    }
    
    if(body != nullptr)
    {
        Node* mouse = Node::create();
        mouse->setPhysicsBody(PhysicsBody::create(PHYSICS_INFINITY, PHYSICS_INFINITY));
        mouse->getPhysicsBody()->setDynamic(false);
        mouse->setPosition(location);
        this->addChild(mouse);
        
        PhysicsJointPin* joint = PhysicsJointPin::construct(mouse->getPhysicsBody(), body, location);
        joint->setMaxForce(5000.0f * body->getMass());
        _world->addJoint(joint);
        _mouses.insert(std::make_pair(touch->getID(), mouse));
        
        return true;
        
    }
    return false;
}

void BaseDemo::onTouchMoved(Touch *touch, Event *event)
{
    auto it = _mouses.find(touch->getID());
    if (it != _mouses.end())
    {
        it->second->setPosition(touch->getLocation());
    }
}

void BaseDemo::onTouchEnded(Touch *touch, Event *event)
{
    auto it = _mouses.find(touch->getID());
    if (it != _mouses.end())
    {
        this->removeChild(it->second);
        _mouses.erase(it);
    }
}

Sprite* BaseDemo::createBall(const Point& point, float radius, PhysicsMaterial material)
{
    Sprite* ball = Sprite::create("ball.png");
    ball->setScale(radius/8);
    
    auto body = PhysicsBody::createCircle(radius, material);
    ball->setPhysicsBody(body);
    ball->setPosition(point);
    body->setTag(DRAG_BODYS_TAG);
    
    return ball;
}

Sprite* BaseDemo::createBox(const Point& point, Size size, PhysicsMaterial material)
{
    Sprite* box = Sprite::create("YellowSquare.png");
    box->setScale(size.width/100.0f, size.height/100.0f);
    
    auto body = PhysicsBody::createBox(size);
    box->setPhysicsBody(body);
    box->setPosition(point);
    box->setRotation(CCRANDOM_0_1()* 360);
    body->setTag(DRAG_BODYS_TAG);
    
    return box;
}

Sprite* BaseDemo::createTriangle(const Point& point, Size size, PhysicsMaterial material)
{
    Sprite* triangle = Sprite::create();
    
    Point verts[] =
    {
        Point(0, size.height/2),
        Point(size.width/2, -size.height/2),
        Point(-size.width/2, -size.height/2)
    };
    
    auto body = PhysicsBody::createPolygon(verts, 3, material);
    triangle->setPhysicsBody(body);
    triangle->setPosition(point);
    triangle->setRotation(CCRANDOM_0_1()* 360);
    triangle->setTag(DRAG_BODYS_TAG);
    
    return triangle;
}

std::string BaseDemo::demo_info() const
{
    return "demo_info";
}

void BaseDemo::restartCallback(Ref *pSender)
{
    CCLOG("override restartCallback");
}

void BaseDemo::backCallback(Ref *pSender)
{
    Director::getInstance()->replaceScene(ContentLayer::createScene());
}

void BaseDemo::toggleDebugCallback(Ref* sender)
{
    if (_world->getDebugDrawMask() == PhysicsWorld::DEBUGDRAW_ALL)
    {
        _world->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_NONE);
    }
    else
    {
        _world->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);
    }
}