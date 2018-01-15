#include "stdafx.h"
#include "game_screen.h"

#include "game_objects.h"

namespace pegas
{
	GameScreen::GameScreen(const std::string& name, rapidxml::xml_node<>* elem)
		:GUI::Widget(name, elem), m_targetsLeft(0), m_timeLeft(0), m_paused(true)
	{
			m_background = Core::resourceManager.Get<Render::Texture>("Space");
			m_background2 = Core::resourceManager.Get<Render::Texture>("Station");

			readSettings();

			Message msg("game", "start");
			Core::messageManager.putMessage(msg);
	}

	void GameScreen::Draw()
	{
		m_background->Draw();

		IRect texRect = m_background2->getBitmapRect();
		FRect rect(texRect);
		FRect uv(0, 1, 0, 1);

		Render::device.PushMatrix();
		Render::device.MatrixTranslate(0.0f, -texRect.height * 0.3f, 0.0f);
		
		m_background2->Bind();
		Render::DrawRect(rect, uv);
		Render::device.PopMatrix();

		m_gameManager.render();

		//HUD
		if (!m_paused)
		{
			Render::BindFont("impact");

			int y = Render::device.Height() - 25;

			Render::PrintString(5, y, std::string("TIME: ") + utils::lexical_cast(m_timeLeft), 1.0f, LeftAlign, BottomAlign);
			Render::PrintString(100, y, std::string("LEFT: ") + utils::lexical_cast(m_targetsLeft), 1.0f, LeftAlign, BottomAlign);
			//Render::PrintString(200, y, std::string("ENERGY: "), 1.0f, LeftAlign, BottomAlign);
		}
	}

	void GameScreen::Update(float dt)
	{
		if (!m_paused)
		{
			m_gameManager.update(dt);
		}

		while (Core::messageManager.getMessage())
		{
			Message message = Core::messageManager.popMessage();
			AcceptMessage(message);
		}
	}

	void GameScreen::AcceptMessage(const Message& message)
	{
		if (message.getPublisher() == "hud")
		{
			if (message.getData() == "targets")
			{
				m_targetsLeft = message.getIntegerParam();
			}
			else if (message.getData() == "time")
			{
				m_timeLeft = message.getIntegerParam();
			}
		}

		if (message.getPublisher() == "game")
		{
			if (message.getData() == "win")
			{
				showFinishScreen(true);
			}

			if (message.getData() == "loose")
			{
				showFinishScreen(false);
			}

			if (message.getData() == "start")
			{
				restartGame();
			}
		}

		m_gameManager.processMessage(message);
	}

	void GameScreen::restartGame()
	{
		int numTargtes = Core::GlobalVars::GetInt("CountTarget", 10);
		int gameTime = Core::GlobalVars::GetInt("Time", 60);

		m_paused = false;
		m_gameManager.reset();
		m_gameManager.addGameObject(new GameWorld((float)gameTime, numTargtes));
		m_gameManager.addGameObject(new Gun());
	}

	void GameScreen::showFinishScreen(bool bWin)
	{
		m_paused = true;
		Core::mainScreen.pushLayer("FinishScreen");
	}

	void GameScreen::readSettings()
	{
		FILE* pFile = fopen("input.txt", "r");
		if (pFile != NULL)
		{
			std::string line = utils::String::ReadLine(pFile);
			while (line != "")
			{
				std::vector<std::string> pair = utils::String::Split(line, '=', true);
				if (pair.size() == 2)
				{
					std::string name = pair[0];
					int value = utils::lexical_cast<int>(pair[1]);

					Core::GlobalVars::SetInt(name, value);
				}
				line = utils::String::ReadLine(pFile);
			}			

			fclose(pFile);
		}
	}
}