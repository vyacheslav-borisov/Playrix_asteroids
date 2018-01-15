#pragma once

#include "collision_checker.h"

namespace pegas
{
	class GameManager;
	//базовый класс дл€ игровых объектов
	//каждый игровой объект умеет обновл€ть и отрисовывать себ€
	//а также обрабатывать событи€ столкновени€ (например - попадание снар€да в мишень)
	class GameObject : public RefCounter
	{
	public:
		GameObject() :m_isDead(false) {}
		virtual ~GameObject() {}
		
		virtual void onCreate(GameManager* host) {}
		virtual void onDestroy(GameManager* host) {}
		virtual void onCollide(GameObject* other) {}
		virtual void update(float dt) {}
		virtual void render() {}
		virtual bool isDead() { return m_isDead; }
	protected:
		bool m_isDead; //игровой объект не нужен и должен быть удалени из пам€ти в следующем цикле
	};

	//интерфейс обработчика сообщений (например ввода с клавиатуры, или глобальных событий
	//вроде рестарта игрового уровн€). »гровой объект может реализовать этот интерфейс и подписатьс€ через
	//GameManager на обработку сообщений
	class IMessageHandler
	{
	public:
		virtual ~IMessageHandler() {}
		virtual void onMessage(const Message& message) = 0;
	};

	//менеджер игровых объектов
	//управл€ет жизненным циклом игровых объектов - созданием, обновлением, отрисовкой
	//и удалением объектов из пам€ти, когда они станов€тс€ не нужны
	class GameManager
	{
	public:
		GameManager();

		void reset();
		void update(float dt);
		void render();
		
		void addGameObject(GameObject* gameObject);
		CollisionManager* getCollider() { return &m_collider; }
		void addMessageHandler(const std::string& publisher, IMessageHandler* handler);
		void removeMessageHandler(const std::string& publisher, IMessageHandler* handler);
		ParticleEffectPtr addParticleEffect(const std::string& name);
		void processMessage(const Message& message);
	private:
		typedef boost::intrusive_ptr<GameObject> GameObjectPtr;
		typedef std::list<GameObjectPtr> GameObjectList;
		typedef std::list<GameObjectPtr>::iterator GameObjectListIt;
		typedef std::list<IMessageHandler*> MessageHandlerList;
		typedef MessageHandlerList::iterator MessageHandlerListIt;
		typedef std::map<std::string, MessageHandlerList> MessageHandlerMap;
		typedef MessageHandlerMap::iterator MessageHandlerMapIt;
		
		//"коллайдер", класс-дектектор столкновений физических объектов в игровом мире
		CollisionManager m_collider;
		//контейнер партикловых эффектов
		EffectsContainer m_effects;
		MessageHandlerMap m_messageHandlers; //обработчики событий
		//очереди игровых объектов
		//дл€ удобства таких очередей две:
		//игровые объекты активной очереди обновл€ютьс€ и отрисовываютьс€ в текущем цикле,
		//в то врем€ как втора€ используетьс€ дл€ добавлени€ новых объектов
		//если игровой объект должен быть удален - он не добавл€етьс€ во вторую очередь после цикла
		//после цикла активна€ очередь очищаетьс€, и обе очереди мен€ютьс€ "рол€ми".
		GameObjectList m_gameObjectList[2];
		int m_currentList;
	};
}