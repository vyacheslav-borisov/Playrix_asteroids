#pragma once

#include "game_manager.h"

namespace pegas
{
	//основной экран игры
	//отображает игровой процесс и HUD
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

		//менеджер игровых объектов
		GameManager m_gameManager;

		Render::Texture* m_background; //фоновая текстура космоса 
		Render::Texture* m_background2;//дополнительный фон с космической платформой
									   //на которой стоит пушка	
		//значения выводимые в HUD:
		int				 m_targetsLeft; //оставшиеся мишение, которые еще не сбил игрок
		int				 m_timeLeft; //оставшееся до конца игры время в секундах
		bool			 m_paused; //игра на паузе? 
								   //на паузе останавливаеться обновление игрового мира
								   //и отрисовка HUD
								   //пауза используеться для вывода результатов прохождения уровня	
	};
}