#pragma once

#include "cell_grid.h"

using namespace math;

//����� CollisionManager ������������ ��� ��������� ������������ ��� ����������� ������� ��������
//� ������ ������� �������� ����� ������� ���� ��� ��������� �������������� �������� (CollisionHull), 
//���������� �������� � ���������.
//CollisionManager ����������� ��������� ����� �������� � ������ ����� � ���������� ������ �������������� ���.
//������ ������ ����������� � ������������ ������ ������������ (collisionGroup). ���� ������� ����������� ����� ������
//�������� �� �� ������������ �� �����������.
namespace pegas
{
	//��������� ��������������� �������.
	//����� ����������� ��� ���� �������������� ��������:
	//�����, ���������� � �������
	//� ������� ���� �� ���� ��������� ��� ������� ��������, ��� ������
	//CollisionManager
	class ICollisionHull : public RefCounter
	{
	public:
		enum
		{
			k_typePoint = 0,
			k_typeCircle,
			k_typePolygon,
			k_typeTotal
		};
	public:
		ICollisionHull(int id, int collisionGroup):
		  m_id(id), m_collisionGroup(collisionGroup) {}
		virtual ~ICollisionHull() {}

		int getId() const { return m_id; }
		int getCollisionGroup() const { return m_collisionGroup; }
		virtual int getType() = 0;

		virtual void moveObject(const Vector3& offset, bool absolute) = 0;
		virtual void rotateObject(float degreesOffset, bool absolute) = 0;
		virtual void transformObject(const Matrix4& m) = 0;

		virtual Vector3 getPosition() = 0;
		virtual void draw() = 0;

	protected:
		int m_id;
		int m_collisionGroup;
	};

	class CollisionManager
	{
	public:
		typedef std::vector<Vector3> PointList;
		typedef std::pair<int, int> CollisionPair;
		typedef std::list<CollisionPair> CollisionPairList;
		typedef CollisionPairList::iterator CollisionPairListIt;
		
		typedef boost::intrusive_ptr<ICollisionHull> CollisionHullPtr;
		typedef std::map<int, CollisionHullPtr> CollisionHullMap;
		typedef std::set<int> CollisionPairsHashes;

	public:
		CollisionManager();
		~CollisionManager();

		//���������������� ������
		//id - �������� ������������� �������������, �� �������� �� ����� �������������
		//����� ���������� ����� �����, �������� ����� � ������ �������� �������
		//group - ������������ ������. �������� �� ������������ ������������� ������ ����� ���������
		//�� ������ �����.
		bool registerPoint(int id, int group, const Vector3& position);
		bool registerCircle(int id, int group, const Vector3& position, float radius);
		bool registerPoligon(int id, int group, const PointList& points);
		void unregisterCollisionHull(int id);
		
		//�������� ��������� �������
		//absolute - ���� ������ true, ������� ��������� ������������ ���������� ���������
		//��������� ��� ����������� �������, false - ������������ �������� ���������
		void moveObject(int id, const Vector3& offset, bool absolute = true);
		void rotateObject(int id, float degreesOffset, bool absolute = true);
		void transformObject(int id, const Matrix4& m);
		
		//���������� ������ ��������, ����� ����� �������� � ������ ����� ����
		void update();
		//������ ������� � ������ �����
		CollisionPairList& getCollidedPairs();

		bool isIntersects(ICollisionHull* a, ICollisionHull* b);

		//������������ �������� �� ������, ����� � ����� ������� ������������
		void debugDraw();

	private:
		static bool isIntersectsPointCircle(ICollisionHull* point, ICollisionHull* circle);
		static bool isIntersectsCirclePoint(ICollisionHull* circle, ICollisionHull* point);

		static bool isIntersectsPointPolygon(ICollisionHull* point, ICollisionHull* polygon);
		static bool isIntersectsPolygonPoint(ICollisionHull* polygon, ICollisionHull* point);

		static bool isIntersectsCirclePolygon(ICollisionHull* circle, ICollisionHull* polygon);
		static bool isIntersectsPolygonCircle(ICollisionHull* polygon, ICollisionHull* circle);
		
		static bool isIntersectsPointPoint(ICollisionHull* point1, ICollisionHull* point2);
		static bool isIntersectsCircleCircle(ICollisionHull* circle1, ICollisionHull* circle2);
		static bool isIntersectsPolygonPolygon(ICollisionHull* polygon1, ICollisionHull* polygon2);

	private:
		CellGrid<ICollisionHull*> m_cellGrid;
		CollisionHullMap m_collisionHulls;
		CollisionPairsHashes m_previousCollisionPairs;
		CollisionPairList m_pairs;

		typedef bool (*IntersectsChecker)(ICollisionHull* a, ICollisionHull* b);
		IntersectsChecker m_checkers[ICollisionHull::k_typeTotal][ICollisionHull::k_typeTotal];
	};

	//����������� ������ - �����
	//������ �������� ��� "����������" ���� � ��������
	class PointCollisionHull: public ICollisionHull 
	{
	public:
		PointCollisionHull(int id, int group, const Vector3& position);

		virtual int getType() { return k_typePoint; }

		virtual void moveObject(const Vector3& offset, bool absolute);
		virtual void rotateObject(float degreesOffset, bool absolute);
		virtual void transformObject(const Matrix4& m);
		virtual Vector3 getPosition();
		virtual void draw() { }

	protected:
		Vector3 m_initialPosition;
		Vector3 m_currentPosition;
	};

	//���������� ��� ���������� �������� ���������� ���������� ���������
	//�������� ����������. �������� ��������� ����� ��������� � ��������� ��������
	class CircleCollisionHull: public PointCollisionHull
	{
	public:
		CircleCollisionHull(int id, int group, const Vector3& position, float radius);

		virtual int getType() { return k_typeCircle; }
		float getRadius() const { return m_radius; }
		
		virtual void draw();

	protected:
		float m_radius;
	};

	//�������� �������������
	//��� �������� ������� �����, ������� ����� ������ �������� �� ������������
	//�������� ����������� ����������� ��� ����� � ���������
	class PoligonCollisionHull: public ICollisionHull
	{
	public:
		PoligonCollisionHull(int id, int group, const CollisionManager::PointList& points);

		virtual int getType() { return k_typePolygon; }

		virtual void moveObject(const Vector3& offset, bool absolute);
		virtual void rotateObject(float degreesOffset, bool absolute);
		virtual void transformObject(const Matrix4& m);

		virtual Vector3 getPosition();
		CollisionManager::PointList getPoints() const { return m_currentPoints; }

		virtual void draw();

	protected:
		CollisionManager::PointList m_initalPoints;
		CollisionManager::PointList m_currentPoints;
		Vector3 m_initialPosition;
		Vector3 m_currentPosition;
	};
}


