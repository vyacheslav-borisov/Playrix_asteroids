#pragma once

namespace pegas
{
	//экран результатов прохождения уровня
	//выводиться после того как истекло время игры
	//или после того как игрок сбил все мишени
	class FinishScreen : public GUI::Widget
	{
	public:
		FinishScreen(const std::string& name, rapidxml::xml_node<>* elem);

		void Draw();
		void AcceptMessage(const Message& message);
	private:
		void readStats();

		Render::Texture* m_winBackground; //фоновая текстура "вы победили"
		Render::Texture* m_loseBackground;//фоновая текстура "вы проиграли"

		bool m_playerWin; //игрок выиграл? параметр определяет какой фон и шрифты выводить на экран
		int	 m_elapsedTime; //время, затраченное на прохождение уровня
		int  m_targetsKilled; //количество сбитых мишеней
		int  m_numTargtes; //общее количество мишеней
	};
}