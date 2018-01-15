#pragma once

#include "game_manager.h"

namespace pegas
{
	//�������� ����� ����
	//���������� ������� ������� � HUD
	class GameScreen : public GUI::Widget
	{
	public:
		GameScreen(const std::string& name, rapidxml::xml_node<>* elem);

		void Draw();
		void Update(float dt);
		void AcceptMessage(const Message& message);

	private:
		void readSettings();
		void restartGame();
		void showFinishScreen(bool bWin);

		//�������� ������� ��������
		GameManager m_gameManager;

		Render::Texture* m_background; //������� �������� ������� 
		Render::Texture* m_background2;//�������������� ��� � ����������� ����������
									   //�� ������� ����� �����	
		//�������� ��������� � HUD:
		int				 m_targetsLeft; //���������� �������, ������� ��� �� ���� �����
		int				 m_timeLeft; //���������� �� ����� ���� ����� � ��������
		bool			 m_paused; //���� �� �����? 
								   //�� ����� ���������������� ���������� �������� ����
								   //� ��������� HUD
								   //����� ������������� ��� ������ ����������� ����������� ������	
	};
}