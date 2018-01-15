#include "stdafx.h"
#include "finish_screen.h"

namespace pegas
{
	FinishScreen::FinishScreen(const std::string& name, rapidxml::xml_node<>* elem)
		:GUI::Widget(name, elem)
	{
		m_winBackground = Core::resourceManager.Get<Render::Texture>("YouWin");
		m_loseBackground = Core::resourceManager.Get<Render::Texture>("YouLose");

		m_playerWin = false;
		m_elapsedTime = 0;
		m_targetsKilled = 0;
		m_numTargtes = 0;
	}

	void FinishScreen::Draw()
	{
		if (m_playerWin)
		{
			Render::BindFont("impact_win");
			m_winBackground->Draw();
		}
		else
		{
			Render::BindFont("impact_loose");
			m_loseBackground->Draw();
		}

		Render::PrintString(290, 290, std::string("TIME: "), 1.0f, LeftAlign, BottomAlign);
		Render::PrintString(290, 260, std::string("KILLED: "), 1.0f, LeftAlign, BottomAlign);
		
		std::string str = utils::lexical_cast(m_elapsedTime);
		str+= std::string(" sec");
		Render::PrintString(500, 290, str, 1.0f, LeftAlign, BottomAlign);

		str = utils::lexical_cast(m_targetsKilled);
		str += std::string("/");
		str += utils::lexical_cast(m_numTargtes);
		Render::PrintString(500, 260, str, 1.0f, LeftAlign, BottomAlign);

		if (m_playerWin)
		{
			Render::BindFont("impact_win_small");			
		}
		else
		{
			Render::BindFont("impact_loose_small");			
		}

		Render::PrintString(350, 220, "press any key to continue", 1.0f, LeftAlign, BottomAlign);
	}

	void FinishScreen::AcceptMessage(const Message& message)
	{
		if (message.getPublisher() == "Layer")
		{
			if (message.getData() == "LayerInit")
			{
				readStats();
			}
		}

		if (message.getPublisher() == "KeyPress")
		{
			Message msg("game", "start");
			Core::messageManager.putMessage(msg);
			Core::mainScreen.popLayer();
		}
	}

	void FinishScreen::readStats()
	{
		m_playerWin = Core::GlobalVars::GetBool("player_win");
		m_elapsedTime = Core::GlobalVars::GetInt("game_time");
		m_targetsKilled = Core::GlobalVars::GetInt("targets_killed");
		m_numTargtes = Core::GlobalVars::GetInt("num_targets");
	}
}