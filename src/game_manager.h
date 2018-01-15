#pragma once

#include "collision_checker.h"

namespace pegas
{
	class GameManager;
	//������� ����� ��� ������� ��������
	//������ ������� ������ ����� ��������� � ������������ ����
	//� ����� ������������ ������� ������������ (�������� - ��������� ������� � ������)
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
		bool m_isDead; //������� ������ �� ����� � ������ ���� ������� �� ������ � ��������� �����
	};

	//��������� ����������� ��������� (�������� ����� � ����������, ��� ���������� �������
	//����� �������� �������� ������). ������� ������ ����� ����������� ���� ��������� � ����������� �����
	//GameManager �� ��������� ���������
	class IMessageHandler
	{
	public:
		virtual ~IMessageHandler() {}
		virtual void onMessage(const Message& message) = 0;
	};

	//�������� ������� ��������
	//��������� ��������� ������ ������� �������� - ���������, �����������, ����������
	//� ��������� �������� �� ������, ����� ��� ���������� �� �����
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
		
		//"���������", �����-��������� ������������ ���������� �������� � ������� ����
		CollisionManager m_collider;
		//��������� ����������� ��������
		EffectsContainer m_effects;
		MessageHandlerMap m_messageHandlers; //����������� �������
		//������� ������� ��������
		//��� �������� ����� �������� ���:
		//������� ������� �������� ������� ������������ � ��������������� � ������� �����,
		//� �� ����� ��� ������ ������������� ��� ���������� ����� ��������
		//���� ������� ������ ������ ���� ������ - �� �� ������������ �� ������ ������� ����� �����
		//����� ����� �������� ������� ����������, � ��� ������� ��������� "������".
		GameObjectList m_gameObjectList[2];
		int m_currentList;
	};
}