#include "stdafx.h"
#include "game_objects.h"

#include "collision_checker.h"

namespace pegas
{
	//--------------------------------------------------------------------------
	// Target class implementation
	//--------------------------------------------------------------------------
	FRect     Target::s_textureRects[k_numFrames];
	bool	  Target::s_frameInitialized = false;
	Target::Target()		
	{
		m_worldBound = GameWorld::getWorldBound();
		m_worldBound.y = 110;
		m_worldBound.height -= 110;
		
		FPoint spawnPoint = math::random(m_worldBound.LeftTop(), m_worldBound.RightBottom());
		m_currentPosition.x = spawnPoint.x;
		m_currentPosition.y = spawnPoint.y;

		float angle = math::random(math::PI * 2.0f);
		m_currentDirection.x = math::cos(angle);
		m_currentDirection.y = math::sin(angle);
		m_currentDirection.Normalize();

		int minSpeed = Core::GlobalVars::GetInt("TargetMinSpeed", 20);
		int maxSpeed = Core::GlobalVars::GetInt("TargetMaxSpeed", 50);
		m_velocity = (float)math::random(minSpeed, maxSpeed);
		m_radius = Core::GlobalVars::GetInt("TargetSize", 30);

		m_texture = Core::resourceManager.Get<Render::Texture>("Asteroid");
		if (!s_frameInitialized)
		{
			float step = 1.0f / 8.0f;

			for (int i = 0; i < 8; i++)
			{
				for (int j = 0; j < 8; j++)
				{
					int frameIndex = j * 8 + i;
					s_textureRects[frameIndex].xStart = step * i;
					s_textureRects[frameIndex].xEnd = step * (i + 1);
					s_textureRects[frameIndex].yStart = 1 - step * (j + 1);
					s_textureRects[frameIndex].yEnd = 1 - step * j;
				}
			}
			s_frameInitialized = true;
		}
		m_currentTextureFrame = math::random(0, k_numFrames);

		m_frameTime = 1.0 / (k_animFPS * 1.0f);
		m_ellapsedTime = 0.0f;
	}

	void Target::onCreate(GameManager* host)
	{
		m_host = host;
		CollisionManager* collider = host->getCollider();
		//� ����� �����, ��� ��������� �� ����� ������������ ���� � ������
		//���������� �� ����� �������, �� � ������������������ ����������
		collider->registerCircle((int)this, 1, m_currentPosition, m_radius);
		//collider->registerCircle((int)this, (int)this, m_currentPosition, m_radius);
	}

	void Target::onDestroy(GameManager* host)
	{
		CollisionManager* collider = host->getCollider();
		collider->unregisterCollisionHull((int)this);
	}

	void Target::onCollide(GameObject* other)
	{
		if (Missle* missle = dynamic_cast<Missle*>(other))
		{
			//� ������ ����� ������

			//������ ������� ������ �� ����� ������� ���������
			Explosion* boom = new Explosion(m_currentPosition);
			m_host->addGameObject(boom);

			//������ ������ "����������" ��������, �� ��������� ����� ������ � ��������
			bool useEffect = (Core::GlobalVars::GetInt("ExplosionUseEffect", 0) == 1);
			if (useEffect)
			{
				ParticleEffectPtr effect = m_host->addParticleEffect("explosion");
				effect->posX = m_currentPosition.x;
				effect->posY = m_currentPosition.y;
				effect->Reset();
			}

			//���������� ���������� �������� ����, ��� �������� ��������� ����
			Core::messageManager.putMessage(Message("game", "target_killed"));

			//������� ��������� ������� �������� ������� ���� �������� � ��������� �����
			m_isDead = true;
		}

		//if (Target* target = dynamic_cast<Target*>(other))
		//{
			//m_currentDirection.x = -m_currentDirection.x;
			//m_currentDirection.y = -m_currentDirection.y;
		//}
	}

	void Target::update(float dt)
	{
		//���������� �������� ��������
		m_ellapsedTime += dt;
		if (m_ellapsedTime >= m_frameTime)
		{
			m_ellapsedTime = 0.0f;
			m_currentTextureFrame++;
			m_currentTextureFrame = m_currentTextureFrame % k_numFrames;
		}

		//���������� ������� ��������� �� ������
		Vector3 offset = m_currentDirection * m_velocity * dt;
		Vector3 endPosition = m_currentPosition + offset;

		//�������� ������ ������� �� ������� ������
		//���� �������� ����� �� ������� ������ - ������������ �������, � ������ "������"
		Vector3 facingPoints[4];
		facingPoints[0] = endPosition + Vector3(0.0f, 1.0f, 0.0f) * m_radius;
		facingPoints[1] = endPosition + Vector3(1.0, 0.0f, 0.0f) * m_radius;
		facingPoints[2] = endPosition + Vector3(0.0, -1.0f, 0.0f) * m_radius;
		facingPoints[3] = endPosition + Vector3(-1.0, 0.0f, 0.0f) * m_radius;

		IPoint leftTop = m_worldBound.LeftTop();
		IPoint rightBottom = m_worldBound.RightBottom();

		//����
		if (facingPoints[0].y > leftTop.y)
		{
			float correction = facingPoints[0].y - (float)leftTop.y;
			endPosition.y -= correction;
			m_currentDirection.y = -m_currentDirection.y;
		}

		//������ �������
		if (facingPoints[1].x > rightBottom.x)
		{
			float correction = facingPoints[1].x - (float)rightBottom.x;
			endPosition.x -= correction;
			m_currentDirection.x = -m_currentDirection.x;
		}

		//������ �������
		if (facingPoints[2].y < rightBottom.y)
		{
			float correction = (float)rightBottom.y - facingPoints[2].y;
			endPosition.y += correction;
			m_currentDirection.y = -m_currentDirection.y;
		}
		
		//����� �������
		if (facingPoints[3].x < leftTop.x)
		{
			float correction = (float)leftTop.x - facingPoints[3].x;
			endPosition.x += correction;
			m_currentDirection.x = -m_currentDirection.x;
		}

		//�������������� ������� ��������� � ��� �������� � ��������� ������������
		offset = endPosition - m_currentPosition;
		CollisionManager* collider = m_host->getCollider();
		collider->moveObject((int)this, offset, false);

		//������������� ����� �������
		m_currentPosition = endPosition;
	}

	void Target::render()
	{
		Render::device.PushMatrix();
		Render::device.MatrixTranslate(m_currentPosition);

		//����� �������� ������� ������ ��������, ���� ��� �������������� ������,
		//����������� � �������������. ��� ����� ����� ��������� ����� ����������� 
		//������������ ���� � ������
		float renderRadius = m_radius * 1.2;
		FRect rect = FRect(-renderRadius, renderRadius, -renderRadius, renderRadius);
		FRect uv = s_textureRects[m_currentTextureFrame];
		
		m_texture->Bind();
		Render::DrawRect(rect, uv);

		Render::device.PopMatrix();
	}	

	//-------------------------------------------------------------------------------
	// Missle class implementation
	//-------------------------------------------------------------------------------
	FRect Missle::s_textureRects[k_numFrames];
	bool  Missle::s_frameInitialized = false;

	Missle::Missle(const Vector3& spawnPoint, const Vector3& direction)
		:m_host(NULL), m_currentPosition(spawnPoint), m_currentDirection(direction)
	{
		m_velocity = (float)Core::GlobalVars::GetInt("MissleSpeed", 200);
		m_worldBound = GameWorld::getWorldBound();
		m_texture = Core::resourceManager.Get<Render::Texture>("ShootFlares");
		if (!s_frameInitialized)
		{
			float step = 1.0f / 5.0f;

			for (int i = 0; i < k_numFrames; i++)
			{
				s_textureRects[i].xStart = step * i;
				s_textureRects[i].xEnd = step * (i + 1);
				s_textureRects[i].yStart = 0.0f;
				s_textureRects[i].yEnd = 1.0f;				
			}
			s_frameInitialized = true;
		}
		m_currentTextureFrame = math::random(0, k_numFrames);

		m_frameTime = 1.0 / (k_animFPS * 1.0f);
		m_ellapsedTime = 0.0f;
	}

	void Missle::onCreate(GameManager* host)
	{
		m_host = host;
		//������ ����������� ������������ ������ � ��������� ������������
		CollisionManager* collider = host->getCollider();
		collider->registerPoint((int)this, 2, m_currentPosition);

		bool useEffect = (Core::GlobalVars::GetInt("MissleUseEffect", 0) == 1);
		if (useEffect)
		{
			//������� ������ - �����
			m_effect = host->addParticleEffect("missle");
			m_effect->posX = m_currentPosition.x;
			m_effect->posY = m_currentPosition.y;
			m_effect->Reset();
		}
	}

	void Missle::onDestroy(GameManager* host)
	{
		if (m_effect != NULL)
		{
			m_effect->Finish();
		}

		CollisionManager* collider = host->getCollider();
		collider->unregisterCollisionHull((int)this);
	}

	void Missle::onCollide(GameObject* other)
	{
		//���� �� � � ���� �� ������ �� ����� - ��� ������������� � ������ ���� ������
		//�� ������
		m_isDead = true;
	}

	void Missle::update(float dt)
	{
		m_ellapsedTime += dt;
		if (m_ellapsedTime >= m_frameTime)
		{
			m_ellapsedTime = 0.0f;
			m_currentTextureFrame++;
			m_currentTextureFrame = m_currentTextureFrame % k_numFrames;
		}

		Vector3 offset = m_currentDirection * m_velocity * dt;
		m_currentPosition += offset;

		if (m_effect != NULL)
		{
			m_effect->posX = m_currentPosition.x;
			m_effect->posY = m_currentPosition.y;
		}

		CollisionManager* collider = m_host->getCollider();
		collider->moveObject((int)this, offset, false);

		IPoint currentPoint;
		currentPoint.x = m_currentPosition.x;
		currentPoint.y = m_currentPosition.y;

		if (!m_worldBound.Contains(currentPoint))
		{
			//������ ������ �� ����� � ������� �� ������� ���������
			//������� ��� �� ������
			m_isDead = true;
		}
	}

	void Missle::render()
	{
		Render::device.PushMatrix();
		Render::device.MatrixTranslate(m_currentPosition);

		const float radius = 10;
		FRect rect = FRect(-radius, radius, -radius, radius);
		FRect uv = s_textureRects[m_currentTextureFrame];

		m_texture->Bind();
		Render::DrawRect(rect, uv);

		Render::device.PopMatrix();
	}

	//---------------------------------------------------------------------------------------
	// Gun class implementation
	//---------------------------------------------------------------------------------------
	FRect Gun::s_textureRects[k_numFrames];
	bool  Gun::s_frameInitialized = false;

	Gun::Gun()
		:m_host(NULL), m_texture(NULL)
	{
		IRect worldBound = GameWorld::getWorldBound();
		m_AABB = IRect(0, 0, 100, 100);

		IPoint topLeft = worldBound.LeftTop();
		IPoint bottomRight = worldBound.RightBottom();

		//��������� ����� �� ������ ����� ������
		m_currentPosition.x = (topLeft.x * 1.0f) + (worldBound.Width() * 0.5f);
		m_currentPosition.y = (bottomRight.y * 1.0f) + (m_AABB.Width() * 0.5f);
		m_currentPosition.z = 0.0f;

		//������� ������� ������, ����� ��� ������ ���������� �����
		//�� ����������� ���������
		m_currentPosition.x += 25.0f;

		m_texture = Core::resourceManager.Get<Render::Texture>("Gun");
		if (!s_frameInitialized)
		{
			float step = 1.0f / 8.0f;

			for (int i = 0; i < 8; i++)
			{
				for (int j = 0; j < 8; j++)
				{
					int frameIndex = j * 8 + i;
					s_textureRects[frameIndex].xStart = step * i;
					s_textureRects[frameIndex].xEnd = step * (i + 1);
					s_textureRects[frameIndex].yStart = 1 - step * (j + 1);
					s_textureRects[frameIndex].yEnd = 1 - step * j;
				}
			}
			s_frameInitialized = true;
		}

		//�������� �������� �����
		int fps = Core::GlobalVars::GetInt("GunRotationSpeed", k_animFPS);
		if (fps == 0)
		{
			fps = k_animFPS;
		}
		m_frameTime = 1.0 / (fps * 1.0f);
		m_ellapsedTime = 0.0f;
		m_fireDelayTime = -1.0f;

		//�������� ��������
		int gunSpeed = Core::GlobalVars::GetInt("GunFireSpeed", 2);
		if (gunSpeed == 0)
		{
			gunSpeed = 2;
		}
		m_maxFireDelay = 1.0f / (1.0f * gunSpeed);

		m_angle = 0;
		setGunAngle(m_angle);		
	}

	void Gun::onCreate(GameManager* host)
	{
		m_host = host;		
	}

	void Gun::update(float dt)
	{
		bool rotationLeft = false;
		bool rotationRight = false;
		if (Core::mainInput.IsKeyDown(VK_LEFT))
		{
			//������������ ����� �����
			rotationLeft = true;
		}
		else if (Core::mainInput.IsKeyDown(VK_RIGHT))
		{
			//..��� ������
			rotationRight = true;
		}

		if (rotationLeft || rotationRight)
		{
			//��������� �������� �������� �����
			m_ellapsedTime += dt;
			if (m_ellapsedTime >= m_frameTime)
			{
				m_ellapsedTime = 0.0f;
				if (rotationLeft) m_angle--;
				if (rotationRight) m_angle++;
				//������������ ���� �������� ����� ��� ������
				//����� ����� �� ��� �������� ����
				if (m_angle < -k_boundAngle) m_angle = -k_boundAngle;
				if (m_angle > k_boundAngle) m_angle = k_boundAngle;
				//������������� ����� ���� �������
				setGunAngle(m_angle);
			}
		}

		if (Core::mainInput.IsKeyDown(VK_UP))
		{
			//������ ������� ��������
			if (m_fireDelayTime < 0.0f)
			{
				//������ ������ �������, ��� ������ ������ �������
				m_fireDelayTime = 0.0f;
				fireUp();
			}
			else
			{
				//������ �������� ����� ��������� ���������
				m_fireDelayTime += dt;
				if (m_fireDelayTime >= m_maxFireDelay)
				{
					m_fireDelayTime = 0.0f;
					fireUp();					
				}
			}
		}
		else
		{
			//���������� ������� ��������
			//����� ����� ���� ���������� ����� ��� ��������� �������
			m_fireDelayTime = -1.0f;
		}
	}

	void Gun::render()
	{
		Render::device.PushMatrix();
		Render::device.MatrixTranslate(m_currentPosition);

		FRect rect = FRect(m_AABB);
		FRect uv = s_textureRects[m_currentTextureFrame];

		Render::device.MatrixTranslate(-m_AABB.width * 0.5f, -m_AABB.height * 0.5f, 0.0f);

		m_texture->Bind();
		Render::DrawRect(rect, uv);

		Render::device.PopMatrix();
	}

	void Gun::setGunAngle(int angle)
	{
		//���������� ����, �� �������� �������������� �����
		float step = (math::PI * 2.0f) / (k_numFrames * 1.0f);
		//������ �����������, � ������� ����� ������
		m_currenDirection.x = math::sin(step * angle);
		m_currenDirection.y = math::cos(step * angle);
		m_currenDirection.z = 0.0f;
		m_currenDirection.Normalize();

		//��������� ���� ��������
		if (angle >= 0)
		{
			m_currentTextureFrame = angle;
		}
		
		if (angle < 0)
		{
			m_currentTextureFrame = k_numFrames + angle;
		}
	}

	void Gun::fireUp()
	{
		//��������� ��������� ���� �����, ����� ��������� ���� ������
		float radius = m_AABB.height * 0.5f;
		math::Vector3 spawnPoint = m_currentPosition + m_currenDirection * radius;

		Missle* missle = new Missle(spawnPoint, m_currenDirection);
		m_host->addGameObject(missle);
	}

	//---------------------------------------------------------------------------
	// Explosion class implementation
	//---------------------------------------------------------------------------
	FRect Explosion::s_textureRects[k_numFrames];
	bool  Explosion::s_frameInitialized = false;

	Explosion::Explosion(const math::Vector3& spawnPoint, int generation)
		:m_host(NULL), m_texture(NULL), m_generation(generation), 
		m_position(spawnPoint), m_childsSpawned(false)
	{
		m_radius = (float)Core::GlobalVars::GetInt("ExplosionSize", 35);
		for (int i = 0; i < generation; i++)
		{
			m_radius = m_radius * 0.8;
		}

		m_texture = Core::resourceManager.Get<Render::Texture>("ExplosionBig");
		if (!s_frameInitialized)
		{
			float step = 1.0f / 9.0f;

			for (int i = 0; i < 9; i++)
			{
				for (int j = 0; j < 9; j++)
				{
					int frameIndex = j * 9 + i;
					s_textureRects[frameIndex].xStart = step * i;
					s_textureRects[frameIndex].xEnd = step * (i + 1);
					s_textureRects[frameIndex].yStart = 1 - step * (j + 1);
					s_textureRects[frameIndex].yEnd = 1 - step * j;
				}
			}
			s_frameInitialized = true;
		}
		m_currentTextureFrame = 0;
		int fps = Core::GlobalVars::GetInt("ExplosionFPS", k_animFPS);
		if (fps == 0)
		{
			fps = k_animFPS;
		}
		m_frameTime = 1.0 / (fps * 1.0f);
		m_ellapsedTime = 0.0f;
		m_lifeTime = m_frameTime * k_numFrames;
		m_halfLifeTime = m_lifeTime - (m_lifeTime * 0.2f);
	}

	void Explosion::onCreate(GameManager* host)
	{
		m_host = host;
	}

	void Explosion::update(float dt)
	{
		//��������� �������� �������� ������
		m_ellapsedTime += dt;
		if (m_ellapsedTime >= m_frameTime)
		{
			m_ellapsedTime = 0.0f;
			m_currentTextureFrame++;
			m_currentTextureFrame = m_currentTextureFrame % k_numFrames;
		}

		m_lifeTime -= dt;
		if (m_lifeTime <= 0.0f)
		{
			//����� ����� ������� ������� -������� � �� ������
			m_isDead = true;
		}
		else if (m_lifeTime <= m_halfLifeTime && !m_childsSpawned)
		{
			//������� �������� �������� ������ ������������
			m_childsSpawned = true;
			spawnChilds();
		}
	}

	void Explosion::render()
	{
		Render::device.PushMatrix();
		Render::device.MatrixTranslate(m_position);

		FRect rect = FRect(-m_radius, m_radius, -m_radius, m_radius);
		FRect uv = s_textureRects[m_currentTextureFrame];

		m_texture->Bind();
		Render::DrawRect(rect, uv);

		Render::device.PopMatrix();
	}

	void Explosion::spawnChilds()
	{
		//����� �� �������������� ����������� �������� � ����������� �����
		//��������� � ������ ��������� ������ ������ �� ��� ��������� ��������� �����
		//��� ��������� - �������� ��������� ����� ������������ � ���������
		int numGenerations = Core::GlobalVars::GetInt("ExplosionGenerations", 2);
		if (m_generation >= numGenerations) return;

		//���������� �������� ������� � �� ������������ ����������� ��������
		int maxChilds = Core::GlobalVars::GetInt("ExplosionMaxChilds", 7);
		int minChilds = Core::GlobalVars::GetInt("ExplosionMinChilds", 3);
		int numChilds = math::random(minChilds, maxChilds);
		if (numChilds == 0)
		{
			numChilds = 3;
		}
		float step = (math::PI * 2.0f) / numChilds;
		for (int i = 0; i < numChilds; i++)
		{
			float angle = (step * i) + math::random(0.0f, step * 0.5f);
			math::Vector3 spawnPoint(math::cos(angle), math::sin(angle), 0.0f);
			spawnPoint = m_position + (spawnPoint * m_radius);

			Explosion* child = new Explosion(spawnPoint, (m_generation + 1));
			m_host->addGameObject(child);
		}

		/*FPoint topLeft, bottomRight;
		topLeft.x = m_position.x - m_radius;
		topLeft.y = m_position.y + m_radius;
		bottomRight.x = m_position.x + m_radius;
		bottomRight.y = m_position.y - m_radius;
		
		for (int i = 0; i < numChilds; i++)
		{
			FPoint point = math::random(topLeft, bottomRight);
			math::Vector3 spawnPoint(point.x, point.y, 0.0f);

			Explosion* child = new Explosion(spawnPoint, (m_generation + 1));
			m_host->addGameObject(child);
		}*/
	}	

	//--------------------------------------------------------------------------------
	// GameWorld class implemenation
	//--------------------------------------------------------------------------------
	GameWorld::GameWorld(float gameTime, int numTargets)
		:m_host(NULL), m_gameTime(gameTime), m_numTargets(numTargets), m_gameStarted(false)
	{

	}

	void GameWorld::onCreate(GameManager* host)
	{
		m_host = host;
		host->addMessageHandler("game", this);
	}

	void GameWorld::onDestroy(GameManager* host)
	{
		host->removeMessageHandler("game", this);
	}

	void GameWorld::update(float dt)
	{
		if (m_gameStarted)
		{
			//�������� ����� �� ����� ����
			m_timeLeft -= dt;
			
			//�������� HUD (������� ������ ������� � �����)
			updateHUD();
			
			if (m_timeLeft <= 0.0f)
			{
				//����� �������
				m_gameStarted = false;
				if (m_targetsLeft > 0)
				{
					//����� �� ��� ������ - ���������
					onLoose();
				}
				else
				{
					//����� ��� - ��������
					onWin();
				}
			}
		}//if (m_gameStarted)
	}

	void GameWorld::onMessage(const Message& message)
	{
		if (message.getData() == "start")
		{
			//���������� �������� ������
			startNewGame();
			return;
		}

		if (message.getData() == "target_killed")
		{
			onTargetKilled();
			return;
		}
	}

	void GameWorld::startNewGame()
	{
		if (m_gameStarted) return;

		m_targetsLeft = m_numTargets;
		m_timeLeft = m_gameTime;

		//������� � �������� � ���� ������
		for (int i = 0; i < m_numTargets; i++)
		{
			m_host->addGameObject(new Target());
		}

		//��������� HUD, ����� �� ��������� ������ "�����" ��������
		//�� ������� ���������� � �����
		updateHUD();

		//��������� ����
		m_gameStarted = true;
	}

	void GameWorld::onTargetKilled()
	{
		//����� ��������� ����

		if (!m_gameStarted) return;
		
		//��������� �������
		m_targetsLeft--;
		
		//��������� ������� �� ������
		updateHUD();

		//����� ��� ����
		if (m_targetsLeft <= 0)
		{
			//������������� ����
			m_gameStarted = false;
			if (m_timeLeft > 0)
			{
				//��������� � ���������� ����� - ��������
				onWin();
			}
			else
			{
				//�� ��������� - ���������
				onLoose();
			}
		}
	}

	void GameWorld::onWin()
	{
		writeStats(true);
		//�������� ��������� ���� ������ ������ - �������� ����� ���������� ����������� ������
		//� ������� �������� ������� "�� ��������"
		Core::messageManager.putMessage(Message("game", "win"));
	}

	void GameWorld::onLoose()
	{
		writeStats(false);
		//you loose
		Core::messageManager.putMessage(Message("game", "loose"));
	}

	void GameWorld::writeStats(bool bWin)
	{
		Core::GlobalVars::SetBool("player_win", bWin);
		Core::GlobalVars::SetInt("game_time", (m_gameTime - m_timeLeft));
		Core::GlobalVars::SetInt("targets_killed", (m_numTargets - m_targetsLeft));
		Core::GlobalVars::SetInt("num_targets", m_numTargets);
	}

	void GameWorld::updateHUD()
	{
		Core::messageManager.putMessage(Message("hud", "targets", m_targetsLeft));
		Core::messageManager.putMessage(Message("hud", "time", (int)m_timeLeft));
	}

	IRect GameWorld::getWorldBound()
	{
		int width = Render::device.Width();
		int height = Render::device.Height();

		return IRect(0, 0, width, height);
	}
}