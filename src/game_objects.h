#pragma once

#include "game_manager.h"

//����������� ������� �������� ��� ���� Asteroids
namespace pegas
{
	//������ - ��������, ������� ���� �����
	//���������, �������, ����� � ������ ��������� ���
	//���������� ���������� ��������. ������ ����� �������� ���� ����������� ����� �����
	//������������ �������� ������� ������ ������ s_textureRects, �������������� ���������� ����������
	//� �������� m_texture. 
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
			k_numFrames = 62, //���������� ������ � ��������
			k_animFPS = 8 //�������� ��������, ������ � �������
		};

		GameManager*	 m_host;
		Render::Texture* m_texture;
		float			 m_radius;

		//������� �������� ����, �� ������� �������� �� ����� ��������
		//��������� � ��������� ������ ����
		IRect			 m_worldBound;
		//������� �������, ����������� � ��������
		math::Vector3    m_currentPosition;
		math::Vector3    m_currentDirection;
		float			 m_velocity;

		//�������� ��������
		static FRect     s_textureRects[k_numFrames];
		static bool		 s_frameInitialized;
		int				 m_currentTextureFrame;
		//����� ������ ������� ����� ��������, ��������� ������� ��� �������� �������
		float			 m_frameTime;
		//�����, ��������� � ������� ����� ����� ��������, ������������� � ���� � m_frameTime
		float			 m_ellapsedTime;
	};

	//������, ������� ������ �� ������ � �������� �����������
	class Missle : public GameObject
	{
	public:
		//spawnPoint - ��������� �����, ��� ����������� ������ (���� �����)
		//direction - �����������, ���� �� �����
		Missle(const Vector3& spawnPoint, const Vector3& direction);

		virtual void onCreate(GameManager* host);
		virtual void onDestroy(GameManager* host);
		virtual void onCollide(GameObject* other);
		virtual void update(float dt);
		virtual void render();

	private:
		GameManager*	 m_host;
		Render::Texture* m_texture;
		
		//������� �������� ����.
		//���� ������ ������� �� ������� (������������, ����),
		//�� ���� �� ���� �� ����� ����, �� ���������� �� ������
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
		
		//�����, ������� ������� �� ��������.
		ParticleEffectPtr  m_effect;
	};

	//������
	//������ ������������ ����� ���������� ����� � ������ (��� � �����)
	//����� ����� ������������� ����� �� ������������� ���� ������ � �����
	class Gun : public GameObject
	{
	public:
		Gun();

		virtual void onCreate(GameManager* host);
		virtual void update(float dt);
		virtual void render();		
	private:
		void setGunAngle(int angle); //��������� ����� �� �������� ����
		void fireUp(); //����������

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
		//���� �������� �����, ��������� ��� ����� ����� ��������
		//����� ��������������� �� ���������� ����. ���� m_angle = 0, �����
		//������� ������ �����. ������������� �������� - �������� ����� �����,
		//������������� - ������
		int				 m_angle;
		//����� �������� ����� ����������
		//����� ��� �� ����� �� ����� ��� ������������, ����� ������� ��������
		//����� ������ ��� �������� � �������.
		float			 m_fireDelayTime; 
		float			 m_maxFireDelay;
	};

	//�����, �� ��������� � ������
	//����� ��������� "��������" ������ � ��������� "���������"
	//��� ������� �����������
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
		int				 m_generation; //��������� ������� ������									   	
		math::Vector3	 m_position;
		float			 m_radius; //������ �������� ������, ������ ���� ������ ������� ������
								   //� ������� ������� ���������� ���������, ������ �������� �����������
								   //��� ������ 	

		static FRect     s_textureRects[k_numFrames];
		static bool		 s_frameInitialized;
		int				 m_currentTextureFrame;
		//����� ������� ������ �� ��������� �� ��������
		float			 m_lifeTime;
		//����� ����� �������� ����������������� �������� ������� ���������� ���������
		//������ �������� ��� ����� �� ������� ����� ������
		float			 m_halfLifeTime; //easter egg, huh!
		float			 m_frameTime;
		float			 m_ellapsedTime;
		//�������� �������� ���������
		bool			 m_childsSpawned;
	};

	//������� ���
	//�������� �� ����� ������ ����:
	//�������� �������� �������, ������� ������, �������� ������� ��������/���������
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