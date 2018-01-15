#pragma once

#include "game_manager.h"

//определения игровых объектов для игры Asteroids
namespace pegas
{
	//мишень - астероид, который надо сбить
	//астероиды, снаряды, пушка и взрывы рисуються как
	//покадровые спрайтовые анимации. Каждый класс содержит свой разделяемый между всеми
	//экземплярами игрового объекта список кадров s_textureRects, представляющий текстурные координаты
	//в текстуре m_texture. 
	class Target: public GameObject
	{
	public:
		Target();

		virtual void onCreate(GameManager* host);
		virtual void onDestroy(GameManager* host);
		virtual void onCollide(GameObject* other);
		virtual void update(float dt);
		virtual void render();

	private:
		enum
		{
			k_numFrames = 62, //количество кадров в анимации
			k_animFPS = 8 //скорость анимации, кадров в секунду
		};

		GameManager*	 m_host;
		Render::Texture* m_texture;
		float			 m_radius;

		//границы игрового мира, за которые астероид не может вылететь
		//совпадают с размерами экрана игры
		IRect			 m_worldBound;
		//текущие позиция, направление и скорость
		math::Vector3    m_currentPosition;
		math::Vector3    m_currentDirection;
		float			 m_velocity;

		//кадровая анимация
		static FRect     s_textureRects[k_numFrames];
		static bool		 s_frameInitialized;
		int				 m_currentTextureFrame;
		//время показа каждого кадра анимации, задаеться вначале при создании объекта
		float			 m_frameTime;
		//время, прошедшее с момента смены кадра анимации, используеться в паре с m_frameTime
		float			 m_ellapsedTime;
	};

	//снаряд, летящий строго по прямой в заданном направлении
	class Missle : public GameObject
	{
	public:
		//spawnPoint - старотвая точка, где появляеться снаряд (дуло пушки)
		//direction - направление, куда он летит
		Missle(const Vector3& spawnPoint, const Vector3& direction);

		virtual void onCreate(GameManager* host);
		virtual void onDestroy(GameManager* host);
		virtual void onCollide(GameObject* other);
		virtual void update(float dt);
		virtual void render();

	private:
		GameManager*	 m_host;
		Render::Texture* m_texture;
		
		//границы игрового мира.
		//если снаряд вылетел за границу (эммигрировал, хаха),
		//не сбив по пути ни одной цели, он удаляеться из памяти
		IRect			 m_worldBound;
		math::Vector3    m_currentPosition;
		math::Vector3    m_currentDirection;
		float			 m_velocity;

		enum
		{
			k_numFrames = 5,
			k_animFPS = 8
		};

		static FRect     s_textureRects[k_numFrames];
		static bool		 s_frameInitialized;
		int				 m_currentTextureFrame;
		float			 m_frameTime;
		float			 m_ellapsedTime;
		
		//шлейф, который тянется за снарядом.
		ParticleEffectPtr  m_effect;
	};

	//турель
	//турель представляет собой вращающюся башню с пушкой (как у танка)
	//игрок может поврокачивать пушку до определенного угла справа и слева
	class Gun : public GameObject
	{
	public:
		Gun();

		virtual void onCreate(GameManager* host);
		virtual void update(float dt);
		virtual void render();		
	private:
		void setGunAngle(int angle); //повернуть башню на заданный угол
		void fireUp(); //выстрелить

		GameManager*	 m_host;
		Render::Texture* m_texture;

		enum
		{
			k_numFrames = 64,
			k_boundAngle = 12,
			k_animFPS = 16
		};
		
		static FRect     s_textureRects[k_numFrames];
		static bool		 s_frameInitialized;
		int				 m_currentTextureFrame;

		IRect			 m_AABB;
		math::Vector3    m_currentPosition;
		math::Vector3    m_currenDirection;
		float			 m_frameTime;
		float			 m_ellapsedTime;
		//угол поворота башни, задаеться как номер кадра анимации
		//башня поворачиваеться на дискретные углы. если m_angle = 0, пушка
		//смотрит строго вверх. отрицательные значения - поворота башни влево,
		//положительные - вправо
		int				 m_angle;
		//время задержки между выстрелами
		//нужно что бы игрок не палил как ненормальный, зажав клавишу выстрела
		//пушка делает два выстрела в секунду.
		float			 m_fireDelayTime; 
		float			 m_maxFireDelay;
	};

	//взрыв, от пападания в мишень
	//может порождать "дочерние" взрывы в несколько "поколений"
	//для большей зрелищности
	class Explosion : public GameObject
	{
	public:
		Explosion(const math::Vector3& spawnPoint, int generation = 0);
		virtual void onCreate(GameManager* host);
		virtual void update(float dt);
		virtual void render();

	private:
		void spawnChilds();
		enum
		{
			k_numFrames = 81,
			k_animFPS = 24
		};

		GameManager*	 m_host;
		Render::Texture* m_texture;
		int				 m_generation; //поколение вспышки взрыва									   	
		math::Vector3	 m_position;
		float			 m_radius; //радиус выспышки взрыва, обычно чуть больше радиуса мишени
								   //у вспышки каждого следующего поколения, размер выспышки становиться
								   //все меньше 	

		static FRect     s_textureRects[k_numFrames];
		static bool		 s_frameInitialized;
		int				 m_currentTextureFrame;
		//время вспышки взрыва от появления до угасания
		float			 m_lifeTime;
		//время после которого инстанцииируються дочерние вспышки следующего поколения
		//обычно половина или треть от времени жизни предка
		float			 m_halfLifeTime; //easter egg, huh!
		float			 m_frameTime;
		float			 m_ellapsedTime;
		//дочерние выспышки порождены
		bool			 m_childsSpawned;
	};

	//игровой мир
	//отвечает за общую логику игры:
	//создание объектов мишеней, подсчет фрагов, проверка условия выигрыша/проигрыша
	class GameWorld : public GameObject, public IMessageHandler
	{
	public:
		GameWorld(float gameTime, int numTargets);

		virtual void onCreate(GameManager* host);
		virtual void onDestroy(GameManager* host);
		virtual void update(float dt);
		virtual void onMessage(const Message& message);

		static IRect getWorldBound();

	private:
		void startNewGame();
		void onTargetKilled();
		void onWin();
		void onLoose();
		void writeStats(bool bWin);
		void updateHUD();

		GameManager* m_host;
		float m_gameTime;
		float m_timeLeft;
		int   m_numTargets;
		int	  m_targetsLeft;
		bool  m_gameStarted;
	};
}