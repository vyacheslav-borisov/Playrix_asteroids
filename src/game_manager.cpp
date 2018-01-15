#include "stdafx.h"
#include "game_manager.h"

namespace pegas
{
	GameManager::GameManager()
		:m_currentList(0)
	{

	}

	void GameManager::update(float dt)
	{
		GameObjectList& currentList = m_gameObjectList[m_currentList];
		GameObjectList& otherList = m_gameObjectList[1 - m_currentList];

		//���������� �������� ����
		for (GameObjectListIt it = currentList.begin(); it != currentList.end(); ++it)
		{
			GameObjectPtr gameObject = (*it);
			gameObject->update(dt);			
		}

		//�������� ������������
		m_collider.update();
		
		//��������� ������������
		CollisionManager::CollisionPairList& pairs = m_collider.getCollidedPairs();
		for (CollisionManager::CollisionPairListIt it = pairs.begin(); it != pairs.end(); ++it)
		{
			GameObject* a = (GameObject*)it->first;
			GameObject* b = (GameObject*)it->second;
			a->onCollide(b);
			b->onCollide(a);
		}

		//��� ������� ������� ���� ���������� ����, ����������� � ��������� �������
		for (GameObjectListIt it = currentList.begin(); it != currentList.end(); ++it)
		{
			GameObjectPtr gameObject = (*it);
			if (!gameObject->isDead())
			{
				otherList.push_back(gameObject);
			}
			else
			{
				gameObject->onDestroy(this);
			}
		}
		//������� ������� �������, ������ �������� ������ �������
		//(����� �������� ����� ����������)
		currentList.clear();
		m_currentList = 1 - m_currentList;

		//��������� ����������� �������
		m_effects.Update(dt);
	}

	void GameManager::render()
	{
		GameObjectList& currentList = m_gameObjectList[m_currentList];
		for (GameObjectListIt it = currentList.begin(); it != currentList.end(); ++it)
		{
			GameObjectPtr gameObject = (*it);
			gameObject->render();
		}

		m_effects.Draw();
	}

	void GameManager::addGameObject(GameObject* gameObject)
	{
		gameObject->onCreate(this);

		//��� ����� ������� ����������� �� ������ ������
		//��� ����� ����� �� �������� �������� ��������� ��������� ������
		//��������� ����� ������� ����� ����������� �� ����� ���������� ��� ��� ��� ����
		GameObjectList& otherList = m_gameObjectList[1 - m_currentList];
		otherList.push_back(GameObjectPtr(gameObject));
	}

	void GameManager::addMessageHandler(const std::string& publisher, IMessageHandler* handler)
	{
		if (m_messageHandlers.count(publisher) == 0)
		{
			m_messageHandlers[publisher] = MessageHandlerList();
		}

		MessageHandlerList& handlers = m_messageHandlers[publisher];
		MessageHandlerListIt found_it = std::find(handlers.begin(), handlers.end(), handler);
		if (found_it == handlers.end())
		{
			handlers.push_back(handler);
		}
	}

	void GameManager::removeMessageHandler(const std::string& publisher, IMessageHandler* handler)
	{
		if (m_messageHandlers.count(publisher) > 0)
		{
			MessageHandlerList& handlers = m_messageHandlers[publisher];
			MessageHandlerListIt found_it = std::find(handlers.begin(), handlers.end(), handler);
			if (found_it != handlers.end())
			{
				handlers.erase(found_it);
			}
		}
	}

	ParticleEffectPtr GameManager::addParticleEffect(const std::string& name)
	{
		return m_effects.AddEffect(name);
	}

	void GameManager::processMessage(const Message& message)
	{
		std::string publisher = message.getPublisher();

		if (m_messageHandlers.count(publisher) > 0)
		{
			MessageHandlerList& handlers = m_messageHandlers[publisher];
			for (MessageHandlerListIt it = handlers.begin(); it != handlers.end(); ++it)
			{
				(*it)->onMessage(message);
			}
		}
	}

	void GameManager::reset()
	{
		for (int i = 0; i < 2; i++)
		{
			GameObjectList& objectList = m_gameObjectList[i];
			for (GameObjectListIt it = objectList.begin(); it != objectList.end(); ++it)
			{
				GameObjectPtr gameObject = (*it);
				gameObject->onDestroy(this);
			}
			objectList.clear();
		}
		m_messageHandlers.clear();
		m_effects.KillAllEffects();
	}	
}