#pragma once

namespace pegas
{
	//����� ����������� ����������� ������
	//���������� ����� ���� ��� ������� ����� ����
	//��� ����� ���� ��� ����� ���� ��� ������
	class FinishScreen : public GUI::Widget
	{
	public:
		FinishScreen(const std::string& name, rapidxml::xml_node<>* elem);

		void Draw();
		void AcceptMessage(const Message& message);
	private:
		void readStats();

		Render::Texture* m_winBackground; //������� �������� "�� ��������"
		Render::Texture* m_loseBackground;//������� �������� "�� ���������"

		bool m_playerWin; //����� �������? �������� ���������� ����� ��� � ������ �������� �� �����
		int	 m_elapsedTime; //�����, ����������� �� ����������� ������
		int  m_targetsKilled; //���������� ������ �������
		int  m_numTargtes; //����� ���������� �������
	};
}