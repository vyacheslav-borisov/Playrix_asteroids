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
		//в итоге решил, что астероиды не будут сталкиваться друг с другом
		//смотриться не очень красиво, да и производительность снижаеться
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
			//в мишень попал снаряд

			//рисуем вспышку взрыва на месте бывшего астероида
			Explosion* boom = new Explosion(m_currentPosition);
			m_host->addGameObject(boom);

			//рисуем эффект "анигиляции" материии, от попадания пучка плазмы в астероид
			bool useEffect = (Core::GlobalVars::GetInt("ExplosionUseEffect", 0) == 1);
			if (useEffect)
			{
				ParticleEffectPtr effect = m_host->addParticleEffect("explosion");
				effect->posX = m_currentPosition.x;
				effect->posY = m_currentPosition.y;
				effect->Reset();
			}

			//уведомляем контроллер игрового мира, что поражена очередная цель
			Core::messageManager.putMessage(Message("game", "target_killed"));

			//говорим менеджеру игровых объектов удалить этот астероид в следующем цикле
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
		//обновление кадровой анимации
		m_ellapsedTime += dt;
		if (m_ellapsedTime >= m_frameTime)
		{
			m_ellapsedTime = 0.0f;
			m_currentTextureFrame++;
			m_currentTextureFrame = m_currentTextureFrame % k_numFrames;
		}

		//обновлении позиции астероида на экране
		Vector3 offset = m_currentDirection * m_velocity * dt;
		Vector3 endPosition = m_currentPosition + offset;

		//проверка выхода объекта за границы экрана
		//если астероид вышел за границы экрана - корректируем позицию, и делаем "отскок"
		Vector3 facingPoints[4];
		facingPoints[0] = endPosition + Vector3(0.0f, 1.0f, 0.0f) * m_radius;
		facingPoints[1] = endPosition + Vector3(1.0, 0.0f, 0.0f) * m_radius;
		facingPoints[2] = endPosition + Vector3(0.0, -1.0f, 0.0f) * m_radius;
		facingPoints[3] = endPosition + Vector3(-1.0, 0.0f, 0.0f) * m_radius;

		IPoint leftTop = m_worldBound.LeftTop();
		IPoint rightBottom = m_worldBound.RightBottom();

		//верх
		if (facingPoints[0].y > leftTop.y)
		{
			float correction = facingPoints[0].y - (float)leftTop.y;
			endPosition.y -= correction;
			m_currentDirection.y = -m_currentDirection.y;
		}

		//правая граница
		if (facingPoints[1].x > rightBottom.x)
		{
			float correction = facingPoints[1].x - (float)rightBottom.x;
			endPosition.x -= correction;
			m_currentDirection.x = -m_currentDirection.x;
		}

		//нижняя граница
		if (facingPoints[2].y < rightBottom.y)
		{
			float correction = (float)rightBottom.y - facingPoints[2].y;
			endPosition.y += correction;
			m_currentDirection.y = -m_currentDirection.y;
		}
		
		//левая граница
		if (facingPoints[3].x < leftTop.x)
		{
			float correction = (float)leftTop.x - facingPoints[3].x;
			endPosition.x += correction;
			m_currentDirection.x = -m_currentDirection.x;
		}

		//синхронизируем позицию астероида с его контуром в менеджере столкновений
		offset = endPosition - m_currentPosition;
		CollisionManager* collider = m_host->getCollider();
		collider->moveObject((int)this, offset, false);

		//устанавливаем новую позицию
		m_currentPosition = endPosition;
	}

	void Target::render()
	{
		Render::device.PushMatrix();
		Render::device.MatrixTranslate(m_currentPosition);

		//риуем астероид немного больше размером, чего его ограничивающий контур,
		//участвующий в столкновениях. это нужно чтобы астероиды более реалистично 
		//сталкивались друг с другом
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
		//снаряд представлен материальной точкой в менеджере столкновений
		CollisionManager* collider = host->getCollider();
		collider->registerPoint((int)this, 2, m_currentPosition);

		bool useEffect = (Core::GlobalVars::GetInt("MissleUseEffect", 0) == 1);
		if (useEffect)
		{
			//создаем эффект - шлейф
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
		//куда бы и в кого бы снаряд не попал - они уничтожаеться и должен быть удален
		//из памяти
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
			//снаряд никого не задел и вылетел за пределы видимости
			//удаляем его из памяти
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

		//размещаем пушку по центру внизу экрана
		m_currentPosition.x = (topLeft.x * 1.0f) + (worldBound.Width() * 0.5f);
		m_currentPosition.y = (bottomRight.y * 1.0f) + (m_AABB.Width() * 0.5f);
		m_currentPosition.z = 0.0f;

		//немного смещаем вправо, чтобы она заняла подходящее место
		//на космической платформе
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

		//скорость поворота башни
		int fps = Core::GlobalVars::GetInt("GunRotationSpeed", k_animFPS);
		if (fps == 0)
		{
			fps = k_animFPS;
		}
		m_frameTime = 1.0 / (fps * 1.0f);
		m_ellapsedTime = 0.0f;
		m_fireDelayTime = -1.0f;

		//скорость стрельбы
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
			//поворачиваем башню влево
			rotationLeft = true;
		}
		else if (Core::mainInput.IsKeyDown(VK_RIGHT))
		{
			//..или вправо
			rotationRight = true;
		}

		if (rotationLeft || rotationRight)
		{
			//обновляем анимацию поворота башни
			m_ellapsedTime += dt;
			if (m_ellapsedTime >= m_frameTime)
			{
				m_ellapsedTime = 0.0f;
				if (rotationLeft) m_angle--;
				if (rotationRight) m_angle++;
				//ограничиваем угол поворота слева или справа
				//чтобы игрок не мог стрелять вниз
				if (m_angle < -k_boundAngle) m_angle = -k_boundAngle;
				if (m_angle > k_boundAngle) m_angle = k_boundAngle;
				//устанавливаем новый угол поврота
				setGunAngle(m_angle);
			}
		}

		if (Core::mainInput.IsKeyDown(VK_UP))
		{
			//зажата клавиша выстрела
			if (m_fireDelayTime < 0.0f)
			{
				//делаем первый выстрел, как только нажали клавишу
				m_fireDelayTime = 0.0f;
				fireUp();
			}
			else
			{
				//делаем задержку перед следующим выстрелом
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
			//сбрасываем счетчик задержки
			//чтобы можно было выстрелить сразу при следующем нажатии
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
		//дискретный угол, на котороый поворачивается башня
		float step = (math::PI * 2.0f) / (k_numFrames * 1.0f);
		//вектор направления, в котором летит снаряд
		m_currenDirection.x = math::sin(step * angle);
		m_currenDirection.y = math::cos(step * angle);
		m_currenDirection.z = 0.0f;
		m_currenDirection.Normalize();

		//вычисляем кадр анимации
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
		//вычисляем положение дула пушки, чтобы поместить туда снаряд
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
		//обновляем анимацию выспышки взрыва
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
			//время жизни вспышки истекло -удаляем её из памяти
			m_isDead = true;
		}
		else if (m_lifeTime <= m_halfLifeTime && !m_childsSpawned)
		{
			//создать дочерние выспышки вокруг родительской
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
		//лучше не злоупотреблять количеством выспышек и количеством серий
		//поскольку с каждой выспышкой связан объект на его обработку тратиться время
		//два поколения - разумный компромис между зрелищностью и скоростью
		int numGenerations = Core::GlobalVars::GetInt("ExplosionGenerations", 2);
		if (m_generation >= numGenerations) return;

		//количество дочерних вспышек и их расположение выбираеться случайно
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
			//обновить время до конца игры
			m_timeLeft -= dt;
			
			//обновить HUD (счетчик сбитых мишеней и время)
			updateHUD();
			
			if (m_timeLeft <= 0.0f)
			{
				//время истекло
				m_gameStarted = false;
				if (m_targetsLeft > 0)
				{
					//сбиты не все мишени - проиграли
					onLoose();
				}
				else
				{
					//убиты все - выиграли
					onWin();
				}
			}
		}//if (m_gameStarted)
	}

	void GameWorld::onMessage(const Message& message)
	{
		if (message.getData() == "start")
		{
			//перезапуск игрового уровня
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

		//создаем и помешаем в игру мишени
		for (int i = 0; i < m_numTargets; i++)
		{
			m_host->addGameObject(new Target());
		}

		//обновляем HUD, чтобы не показывал всякие "левые" значения
		//до первого обновления в цикле
		updateHUD();

		//запускаем игру
		m_gameStarted = true;
	}

	void GameWorld::onTargetKilled()
	{
		//сбита очередная цель

		if (!m_gameStarted) return;
		
		//уменьшаем счетчик
		m_targetsLeft--;
		
		//обновляем циферки на экране
		updateHUD();

		//сбиты все цели
		if (m_targetsLeft <= 0)
		{
			//останавливаем игру
			m_gameStarted = false;
			if (m_timeLeft > 0)
			{
				//уложились в отведенное время - выиграли
				onWin();
			}
			else
			{
				//не уложились - проиграли
				onLoose();
			}
		}
	}

	void GameWorld::onWin()
	{
		writeStats(true);
		//посылаем сообщение друг частям движка - показать экран статистики прохождения уровня
		//и большую красивую надпись "вы победили"
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